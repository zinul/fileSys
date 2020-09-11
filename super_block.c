#include "super_block.h"
#include <fcntl.h>

int s_format(int fd,int size_b){
    struct super_block sup;
    int save_blocks=13;
    sup.s_inodes_count=0;
    //一个索引节点占128字节，一个块包含8个索引节点,预留80个索引节点的位置
    sup.s_r_blocks_count=save_blocks;
    sup.s_blocks_count=size_b+save_blocks;
    sup.s_free_blocks_count=size_b;
    sup.s_inodes_count=0;
    sup.s_first_block_size=1;
    sup.s_log_block_size=1024;
}