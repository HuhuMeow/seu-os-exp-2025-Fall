#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>
#include <linux/mpage.h>
#include "naive_fs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huhu");
MODULE_DESCRIPTION("NaiveFS: A simple file system for OS experiment");

// --- 核心：物理块映射 ---
static int naive_get_block(struct inode *inode, sector_t iblock,
                           struct buffer_head *bh_result, int create)
{
    struct super_block *sb = inode->i_sb;
    struct buffer_head *bh;
    struct naive_inode *ni;
    int phys_block;

    // 1. 读取 Inode 表 (Block 1)
    bh = sb_bread(sb, 1);
    if (!bh) return -EIO;

    // 获取对应的 inode 结构
    ni = (struct naive_inode *)(bh->b_data + 
         (inode->i_ino * sizeof(struct naive_inode)) % NAIVE_BLOCK_SIZE);

    // 2. 获取数据块号 (只支持直接块)
    if (iblock >= 10) { 
        brelse(bh); 
        return -EFBIG; 
    }
    phys_block = ni->data_block[iblock];
    brelse(bh);

    // 3. 映射物理块
    if (phys_block == 0) return 0; // 稀疏文件/空洞
    map_bh(bh_result, sb, phys_block);
    return 0;
}

// --- 文件读取 ---
static int naive_read_folio(struct file *file, struct folio *folio) {
    return mpage_read_folio(folio, naive_get_block);
}

static const struct address_space_operations naive_aops = {
    .read_folio = naive_read_folio,
};

// --- 目录操作: Iterate (ls) ---
static int naive_iterate(struct file *filp, struct dir_context *ctx) {
    struct inode *inode = file_inode(filp);
    struct buffer_head *bh;
    struct naive_dir_entry *de;
    int i;

    if (ctx->pos >= inode->i_size) return 0;

    // 简化：假设目录数据都在 Block 2 (更完善的做法是调用 get_block)
    bh = sb_bread(inode->i_sb, 2);
    if (!bh) return -EIO;

    de = (struct naive_dir_entry *)bh->b_data;
    for (i = 0; i < NAIVE_BLOCK_SIZE / sizeof(struct naive_dir_entry); i++) {
        if (de[i].inode_no != 0) {
            if (!dir_emit(ctx, de[i].name, strnlen(de[i].name, NAIVE_FILENAME_MAX),
                          de[i].inode_no, DT_UNKNOWN)) {
                break;
            }
        }
        ctx->pos += sizeof(struct naive_dir_entry);
    }
    brelse(bh);
    return 0;
}

const struct file_operations naive_dir_operations = {
    .owner = THIS_MODULE,
    .iterate_shared = naive_iterate,
    .read = generic_read_dir,
    .llseek = generic_file_llseek,
};

// --- 目录操作: Lookup (查找文件) ---
static struct dentry *naive_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags) {
    struct super_block *sb = dir->i_sb;
    struct buffer_head *bh;
    struct naive_dir_entry *de;
    int i;

    bh = sb_bread(sb, 2); // 同样硬编码读 Block 2
    if (!bh) return ERR_PTR(-EIO);
    de = (struct naive_dir_entry *)bh->b_data;

    for (i = 0; i < NAIVE_BLOCK_SIZE / sizeof(struct naive_dir_entry); i++) {
        if (de[i].inode_no != 0 && 
            strncmp(de[i].name, dentry->d_name.name, NAIVE_FILENAME_MAX) == 0) {
            
            struct inode *inode = iget_locked(sb, de[i].inode_no);
            if (!inode) { brelse(bh); return ERR_PTR(-ENOMEM); }
            
            if (inode->i_state & I_NEW) {
                // 初始化 VFS inode
                inode_init_owner(&nop_mnt_idmap, inode, dir, S_IFREG | 0644);
                inode->i_fop = &naive_dir_operations; // 暂时指向目录操作方便测试
                inode->i_mapping->a_ops = &naive_aops;
                unlock_new_inode(inode);
            }
            brelse(bh);
            return d_splice_alias(inode, dentry);
        }
    }
    brelse(bh);
    return NULL;
}

const struct inode_operations naive_dir_inode_ops = {
    .lookup = naive_lookup,
};

// --- 挂载逻辑: 读取 Superblock ---
static int naive_fill_super(struct super_block *sb, void *data, int silent) {
    struct buffer_head *bh;
    struct naive_super_block *nsb;
    struct inode *root;

    // 1. 设置块大小并读取 Block 0
    if (!sb_set_blocksize(sb, NAIVE_BLOCK_SIZE)) return -EINVAL;
    bh = sb_bread(sb, 0);
    if (!bh) return -EIO;

    nsb = (struct naive_super_block *)bh->b_data;
    
    // 2. 验证 Magic Number
    if (nsb->magic != NAIVE_MAGIC) {
        printk(KERN_ERR "NaiveFS: Magic mismatch. Expected 0x%X, Got 0x%X\n", 
               NAIVE_MAGIC, nsb->magic);
        brelse(bh);
        return -EINVAL;
    }
    printk(KERN_INFO "NaiveFS: Mounted successfully. Magic matches.\n");
    brelse(bh);

    sb->s_magic = NAIVE_MAGIC;
    sb->s_op = NULL; // 不需要高级操作

    // 3. 创建根 Inode (伪造内存对象，实际上应该读 Block 1)
    root = new_inode(sb);
    root->i_ino = 1;
    inode_init_owner(&nop_mnt_idmap, root, NULL, S_IFDIR | 0755);
    root->i_sb = sb;
    root->i_op = &naive_dir_inode_ops;
    root->i_fop = &naive_dir_operations;
    root->i_size = sizeof(struct naive_dir_entry) * 2;
    
    sb->s_root = d_make_root(root);
    if (!sb->s_root) return -ENOMEM;
    
    return 0;
}

static struct dentry *naive_mount(struct file_system_type *fs_type,
    int flags, const char *dev_name, void *data) {
    return mount_bdev(fs_type, flags, dev_name, data, naive_fill_super);
}

// 定义文件系统类型
static struct file_system_type naive_fs_type = {
    .owner = THIS_MODULE,
    .name = "naive",       // 这里的名字决定了 mount -t naive
    .mount = naive_mount,
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
