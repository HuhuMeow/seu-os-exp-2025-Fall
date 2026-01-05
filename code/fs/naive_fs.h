#ifndef _NAIVE_FS_H
#define _NAIVE_FS_H

#include <linux/types.h>

// 魔数：0x114514
#define NAIVE_MAGIC 0x114514 
#define NAIVE_BLOCK_SIZE 512
#define NAIVE_FILENAME_MAX 28

// Block 0: 超级块
struct naive_super_block {
    __le32 magic;
    __le32 block_count;
    __le32 inode_count;
    __le32 root_inode;
};

// Inode 结构
struct naive_inode {
    __le32 mode;
    __le32 uid;
    __le32 gid;
    __le32 size;
    __le32 ctime;
    __le32 blocks;
    __le32 data_block[10]; // 极简设计：只支持10个直接块
};

// 目录项结构
struct naive_dir_entry {
    char name[NAIVE_FILENAME_MAX];
    __le32 inode_no;
};

#endif
