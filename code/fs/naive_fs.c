#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>
#include <linux/mpage.h>
#include <linux/fs_context.h>
#include <linux/time.h>
#include <linux/uio.h>
#include "naive_fs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huhu");
MODULE_DESCRIPTION("NaiveFS: A simple file system for OS experiment");

// --- 前向声明 ---
static const struct super_operations naive_sb_ops;
static const struct inode_operations naive_dir_inode_ops;
static const struct inode_operations naive_file_inode_ops;
static const struct file_operations naive_dir_operations;
static const struct file_operations naive_file_operations;
static const struct address_space_operations naive_aops;

// --- 辅助函数：分配空闲块 ---
static int naive_alloc_block(struct super_block *sb) {
    static int next_block = 10;
    if (next_block >= 2000) return -ENOSPC;
    return next_block++;
}

// --- 辅助函数：分配空闲 inode ---
static int naive_alloc_inode_no(struct super_block *sb) {
    static int next_inode = 2;
    if (next_inode >= 100) return -ENOSPC;
    return next_inode++;
}

// --- 辅助函数：将 inode 写回磁盘 ---
static int naive_write_inode_to_disk(struct super_block *sb, unsigned long ino, 
                                      struct naive_inode *ni) {
    struct buffer_head *bh;
    struct naive_inode *disk_ni;
    
    bh = sb_bread(sb, 1);
    if (!bh) return -EIO;
    
    disk_ni = (struct naive_inode *)(bh->b_data + 
              (ino * sizeof(struct naive_inode)) % NAIVE_BLOCK_SIZE);
    memcpy(disk_ni, ni, sizeof(struct naive_inode));
    
    mark_buffer_dirty(bh);
    sync_dirty_buffer(bh);
    brelse(bh);
    return 0;
}

// --- 辅助函数：从磁盘读取 inode ---
static int naive_read_inode_from_disk(struct super_block *sb, unsigned long ino,
                                       struct naive_inode *ni) {
    struct buffer_head *bh;
    struct naive_inode *disk_ni;
    
    bh = sb_bread(sb, 1);
    if (!bh) return -EIO;
    
    disk_ni = (struct naive_inode *)(bh->b_data + 
              (ino * sizeof(struct naive_inode)) % NAIVE_BLOCK_SIZE);
    memcpy(ni, disk_ni, sizeof(struct naive_inode));
    
    brelse(bh);
    return 0;
}

// --- 核心：物理块映射 ---
static int naive_get_block(struct inode *inode, sector_t iblock,
                           struct buffer_head *bh_result, int create)
{
    struct super_block *sb = inode->i_sb;
    struct naive_inode ni;
    int phys_block;
    int err;

    err = naive_read_inode_from_disk(sb, inode->i_ino, &ni);
    if (err) return err;

    if (iblock >= 10) return -EFBIG;
    
    phys_block = ni.data_block[iblock];
    
    if (phys_block == 0 && create) {
        phys_block = naive_alloc_block(sb);
        if (phys_block < 0) return phys_block;
        
        ni.data_block[iblock] = phys_block;
        ni.blocks++;
        naive_write_inode_to_disk(sb, inode->i_ino, &ni);
        
        set_buffer_new(bh_result);
    }
    
    if (phys_block == 0) return 0;
    
    map_bh(bh_result, sb, phys_block);
    return 0;
}

// --- 文件读取 ---
static int naive_read_folio(struct file *file, struct folio *folio) {
    return mpage_read_folio(folio, naive_get_block);
}

// --- 文件写入 (Linux 6.18+ API: 使用 kiocb) ---
static int naive_write_begin(const struct kiocb *iocb, struct address_space *mapping,
                             loff_t pos, unsigned len,
                             struct folio **foliop, void **fsdata) {
    return block_write_begin(mapping, pos, len, foliop, naive_get_block);
}

static int naive_write_end(const struct kiocb *iocb, struct address_space *mapping,
                           loff_t pos, unsigned len, unsigned copied,
                           struct folio *folio, void *fsdata) {
    struct inode *inode = mapping->host;
    int ret;
    
    ret = generic_write_end(iocb, mapping, pos, len, copied, folio, fsdata);
    
    if (ret > 0) {
        struct naive_inode ni;
        naive_read_inode_from_disk(inode->i_sb, inode->i_ino, &ni);
        ni.size = i_size_read(inode);
        naive_write_inode_to_disk(inode->i_sb, inode->i_ino, &ni);
    }
    
    return ret;
}

static const struct address_space_operations naive_aops = {
    .read_folio = naive_read_folio,
    .write_begin = naive_write_begin,
    .write_end = naive_write_end,
    .dirty_folio = block_dirty_folio,
    .invalidate_folio = block_invalidate_folio,
};

// --- 目录操作: Iterate (ls) ---
static int naive_iterate(struct file *filp, struct dir_context *ctx) {
    struct inode *inode = file_inode(filp);
    struct super_block *sb = inode->i_sb;
    struct buffer_head *bh;
    struct naive_dir_entry *de;
    struct naive_inode ni;
    int i, data_block;

    if (ctx->pos >= inode->i_size) return 0;

    naive_read_inode_from_disk(sb, inode->i_ino, &ni);
    data_block = ni.data_block[0];
    
    if (data_block == 0) return 0;
    
    bh = sb_bread(sb, data_block);
    if (!bh) return -EIO;

    de = (struct naive_dir_entry *)bh->b_data;
    for (i = 0; i < NAIVE_BLOCK_SIZE / sizeof(struct naive_dir_entry); i++) {
        if (ctx->pos >= inode->i_size) break;
        
        if (de[i].inode_no != 0) {
            unsigned char dtype = DT_UNKNOWN;
            struct naive_inode target_ni;
            
            if (naive_read_inode_from_disk(sb, de[i].inode_no, &target_ni) == 0) {
                if (S_ISDIR(target_ni.mode)) dtype = DT_DIR;
                else if (S_ISREG(target_ni.mode)) dtype = DT_REG;
            }
            
            if (!dir_emit(ctx, de[i].name, strnlen(de[i].name, NAIVE_FILENAME_MAX),
                          de[i].inode_no, dtype)) {
                break;
            }
        }
        ctx->pos += sizeof(struct naive_dir_entry);
    }
    brelse(bh);
    return 0;
}

static const struct file_operations naive_dir_operations = {
    .owner = THIS_MODULE,
    .iterate_shared = naive_iterate,
    .read = generic_read_dir,
    .llseek = generic_file_llseek,
};

// --- 文件操作 ---
static const struct file_operations naive_file_operations = {
    .owner = THIS_MODULE,
    .read_iter = generic_file_read_iter,
    .write_iter = generic_file_write_iter,
    .mmap = generic_file_mmap,
    .fsync = generic_file_fsync,
    .llseek = generic_file_llseek,
};

// --- 辅助函数：初始化 VFS inode ---
static void naive_fill_inode(struct inode *inode, struct naive_inode *ni) {
    inode->i_mode = ni->mode;
    inode->i_size = ni->size;
    i_uid_write(inode, ni->uid);
    i_gid_write(inode, ni->gid);
    
    if (S_ISDIR(inode->i_mode)) {
        inode->i_op = &naive_dir_inode_ops;
        inode->i_fop = &naive_dir_operations;
    } else if (S_ISREG(inode->i_mode)) {
        inode->i_op = &naive_file_inode_ops;
        inode->i_fop = &naive_file_operations;
        inode->i_mapping->a_ops = &naive_aops;
    }
}

// --- 目录操作: Lookup ---
static struct dentry *naive_lookup(struct inode *dir, struct dentry *dentry, 
                                   unsigned int flags) {
    struct super_block *sb = dir->i_sb;
    struct buffer_head *bh;
    struct naive_dir_entry *de;
    struct naive_inode dir_ni, target_ni;
    int i, data_block;

    naive_read_inode_from_disk(sb, dir->i_ino, &dir_ni);
    data_block = dir_ni.data_block[0];
    
    if (data_block == 0) return NULL;
    
    bh = sb_bread(sb, data_block);
    if (!bh) return ERR_PTR(-EIO);
    
    de = (struct naive_dir_entry *)bh->b_data;

    for (i = 0; i < NAIVE_BLOCK_SIZE / sizeof(struct naive_dir_entry); i++) {
        if (de[i].inode_no != 0 && 
            strncmp(de[i].name, dentry->d_name.name, NAIVE_FILENAME_MAX) == 0) {
            
            struct inode *inode = iget_locked(sb, de[i].inode_no);
            if (!inode) { brelse(bh); return ERR_PTR(-ENOMEM); }
            
            if (inode->i_state & I_NEW) {
                naive_read_inode_from_disk(sb, de[i].inode_no, &target_ni);
                naive_fill_inode(inode, &target_ni);
                unlock_new_inode(inode);
            }
            brelse(bh);
            return d_splice_alias(inode, dentry);
        }
    }
    brelse(bh);
    return NULL;
}

// --- 辅助函数：添加目录项 ---
static int naive_add_dir_entry(struct inode *dir, const char *name, int ino) {
    struct super_block *sb = dir->i_sb;
    struct buffer_head *bh;
    struct naive_dir_entry *de;
    struct naive_inode dir_ni;
    int i, data_block;
    
    naive_read_inode_from_disk(sb, dir->i_ino, &dir_ni);
    data_block = dir_ni.data_block[0];
    
    if (data_block == 0) return -EIO;
    
    bh = sb_bread(sb, data_block);
    if (!bh) return -EIO;
    
    de = (struct naive_dir_entry *)bh->b_data;
    
    for (i = 0; i < NAIVE_BLOCK_SIZE / sizeof(struct naive_dir_entry); i++) {
        if (de[i].inode_no == 0) {
            memset(&de[i], 0, sizeof(struct naive_dir_entry));
            strncpy(de[i].name, name, NAIVE_FILENAME_MAX - 1);
            de[i].inode_no = ino;
            
            mark_buffer_dirty(bh);
            sync_dirty_buffer(bh);
            brelse(bh);
            
            dir->i_size += sizeof(struct naive_dir_entry);
            dir_ni.size = dir->i_size;
            naive_write_inode_to_disk(sb, dir->i_ino, &dir_ni);
            
            return 0;
        }
    }
    
    brelse(bh);
    return -ENOSPC;
}

// --- 辅助函数：删除目录项 ---
static int naive_remove_dir_entry(struct inode *dir, const char *name) {
    struct super_block *sb = dir->i_sb;
    struct buffer_head *bh;
    struct naive_dir_entry *de;
    struct naive_inode dir_ni;
    int i, data_block;
    
    naive_read_inode_from_disk(sb, dir->i_ino, &dir_ni);
    data_block = dir_ni.data_block[0];
    
    if (data_block == 0) return -EIO;
    
    bh = sb_bread(sb, data_block);
    if (!bh) return -EIO;
    
    de = (struct naive_dir_entry *)bh->b_data;
    
    for (i = 0; i < NAIVE_BLOCK_SIZE / sizeof(struct naive_dir_entry); i++) {
        if (de[i].inode_no != 0 && 
            strncmp(de[i].name, name, NAIVE_FILENAME_MAX) == 0) {
            de[i].inode_no = 0;
            memset(de[i].name, 0, NAIVE_FILENAME_MAX);
            
            mark_buffer_dirty(bh);
            sync_dirty_buffer(bh);
            brelse(bh);
            return 0;
        }
    }
    
    brelse(bh);
    return -ENOENT;
}

// --- 目录操作: Create (创建文件) ---
static int naive_create(struct mnt_idmap *idmap, struct inode *dir,
                        struct dentry *dentry, umode_t mode, bool excl) {
    struct super_block *sb = dir->i_sb;
    struct inode *inode;
    struct naive_inode ni;
    int ino, err;
    
    ino = naive_alloc_inode_no(sb);
    if (ino < 0) return ino;
    
    inode = new_inode(sb);
    if (!inode) return -ENOMEM;
    
    inode->i_ino = ino;
    inode_init_owner(idmap, inode, dir, mode);
    inode->i_op = &naive_file_inode_ops;
    inode->i_fop = &naive_file_operations;
    inode->i_mapping->a_ops = &naive_aops;
    inode->i_size = 0;
    
    memset(&ni, 0, sizeof(ni));
    ni.mode = inode->i_mode;
    ni.uid = i_uid_read(inode);
    ni.gid = i_gid_read(inode);
    ni.size = 0;
    ni.ctime = ktime_get_real_seconds();
    
    err = naive_write_inode_to_disk(sb, ino, &ni);
    if (err) {
        iput(inode);
        return err;
    }
    
    err = naive_add_dir_entry(dir, dentry->d_name.name, ino);
    if (err) {
        iput(inode);
        return err;
    }
    
    d_instantiate(dentry, inode);
    printk(KERN_INFO "NaiveFS: Created file '%s' (inode %d)\n", 
           dentry->d_name.name, ino);
    return 0;
}

// --- 目录操作: Mkdir (创建目录) - Linux 6.18+ 返回 struct dentry* ---
static struct dentry *naive_mkdir(struct mnt_idmap *idmap, struct inode *dir,
                                  struct dentry *dentry, umode_t mode) {
    struct super_block *sb = dir->i_sb;
    struct inode *inode;
    struct naive_inode ni;
    struct buffer_head *bh;
    struct naive_dir_entry *de;
    int ino, data_block, err;
    
    ino = naive_alloc_inode_no(sb);
    if (ino < 0) return ERR_PTR(ino);
    
    data_block = naive_alloc_block(sb);
    if (data_block < 0) return ERR_PTR(data_block);
    
    inode = new_inode(sb);
    if (!inode) return ERR_PTR(-ENOMEM);
    
    inode->i_ino = ino;
    inode_init_owner(idmap, inode, dir, S_IFDIR | mode);
    inode->i_op = &naive_dir_inode_ops;
    inode->i_fop = &naive_dir_operations;
    inode->i_size = sizeof(struct naive_dir_entry) * 2;
    
    bh = sb_bread(sb, data_block);
    if (!bh) {
        iput(inode);
        return ERR_PTR(-EIO);
    }
    
    memset(bh->b_data, 0, NAIVE_BLOCK_SIZE);
    de = (struct naive_dir_entry *)bh->b_data;
    
    strncpy(de[0].name, ".", NAIVE_FILENAME_MAX);
    de[0].inode_no = ino;
    
    strncpy(de[1].name, "..", NAIVE_FILENAME_MAX);
    de[1].inode_no = dir->i_ino;
    
    mark_buffer_dirty(bh);
    sync_dirty_buffer(bh);
    brelse(bh);
    
    memset(&ni, 0, sizeof(ni));
    ni.mode = inode->i_mode;
    ni.uid = i_uid_read(inode);
    ni.gid = i_gid_read(inode);
    ni.size = inode->i_size;
    ni.blocks = 1;
    ni.data_block[0] = data_block;
    ni.ctime = ktime_get_real_seconds();
    
    err = naive_write_inode_to_disk(sb, ino, &ni);
    if (err) {
        iput(inode);
        return ERR_PTR(err);
    }
    
    err = naive_add_dir_entry(dir, dentry->d_name.name, ino);
    if (err) {
        iput(inode);
        return ERR_PTR(err);
    }
    
    d_instantiate(dentry, inode);
    printk(KERN_INFO "NaiveFS: Created directory '%s' (inode %d, data block %d)\n", 
           dentry->d_name.name, ino, data_block);
    return NULL;  // 成功返回 NULL
}

// --- 目录操作: Unlink (删除文件) ---
static int naive_unlink(struct inode *dir, struct dentry *dentry) {
    struct inode *inode = d_inode(dentry);
    int err;
    
    err = naive_remove_dir_entry(dir, dentry->d_name.name);
    if (err) return err;
    
    drop_nlink(inode);
    printk(KERN_INFO "NaiveFS: Removed file '%s'\n", dentry->d_name.name);
    return 0;
}

// --- 目录操作: Rmdir (删除目录) ---
static int naive_rmdir(struct inode *dir, struct dentry *dentry) {
    struct inode *inode = d_inode(dentry);
    
    int err = naive_remove_dir_entry(dir, dentry->d_name.name);
    if (err) return err;
    
    drop_nlink(inode);
    drop_nlink(dir);
    printk(KERN_INFO "NaiveFS: Removed directory '%s'\n", dentry->d_name.name);
    return 0;
}

static const struct inode_operations naive_dir_inode_ops = {
    .lookup = naive_lookup,
    .create = naive_create,
    .mkdir = naive_mkdir,
    .unlink = naive_unlink,
    .rmdir = naive_rmdir,
};

static const struct inode_operations naive_file_inode_ops = {
    .setattr = simple_setattr,
    .getattr = simple_getattr,
};

// --- Super Operations ---
static const struct super_operations naive_sb_ops = {
    .statfs = simple_statfs,
};

// --- 新版挂载 API: Fill Super ---
static int naive_fill_super(struct super_block *sb, struct fs_context *fc) {
    struct buffer_head *bh;
    struct naive_super_block *nsb;
    struct naive_inode root_ni;
    struct inode *root;

    if (!sb_set_blocksize(sb, NAIVE_BLOCK_SIZE)) return -EINVAL;
    
    sb->s_magic = NAIVE_MAGIC;
    sb->s_op = &naive_sb_ops;
    
    bh = sb_bread(sb, 0);
    if (!bh) return -EIO;

    nsb = (struct naive_super_block *)bh->b_data;
    
    if (nsb->magic != NAIVE_MAGIC) {
        printk(KERN_ERR "NaiveFS: Magic mismatch. Expected 0x%X, Got 0x%X\n", 
               NAIVE_MAGIC, nsb->magic);
        brelse(bh);
        return -EINVAL;
    }
    printk(KERN_INFO "NaiveFS: Mounted successfully. Magic matches.\n");
    brelse(bh);

    if (naive_read_inode_from_disk(sb, 1, &root_ni)) return -EIO;

    root = new_inode(sb);
    if (!root) return -ENOMEM;
    
    root->i_ino = 1;
    root->i_sb = sb;
    naive_fill_inode(root, &root_ni);
    
    sb->s_root = d_make_root(root);
    if (!sb->s_root) return -ENOMEM;
    
    return 0;
}

// --- 新版挂载 API ---
static int naive_get_tree(struct fs_context *fc) {
    return get_tree_bdev(fc, naive_fill_super);
}

static const struct fs_context_operations naive_context_ops = {
    .get_tree = naive_get_tree,
};

static int naive_init_fs_context(struct fs_context *fc) {
    fc->ops = &naive_context_ops;
    return 0;
}

static struct file_system_type naive_fs_type = {
    .owner = THIS_MODULE,
    .name = "naive",
    .init_fs_context = naive_init_fs_context,
    .kill_sb = kill_block_super,
    .fs_flags = FS_REQUIRES_DEV,
};

static int __init naive_init(void) {
    printk(KERN_INFO "NaiveFS: Module loaded\n");
    return register_filesystem(&naive_fs_type);
}

static void __exit naive_exit(void) {
    unregister_filesystem(&naive_fs_type);
    printk(KERN_INFO "NaiveFS: Module unloaded\n");
}

module_init(naive_init);
module_exit(naive_exit);
