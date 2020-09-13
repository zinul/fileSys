#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#include "fs.h"
#define BLOCKSIZE 1024
#define BLOCK_SIZE_BITS 10 // 数据块长度所占比特位数。
#define NR_OPEN 20
#define NR_INODE 32
#define NR_FILE 64
//#define BLOCKCOUNT 500
//#define INODE_SIZE 48
int block_format(int fd);
off_t set_empty_block(int fd);
off_t b_bitmap_format(int fd);
off_t i_bitmap_format(int fd);
off_t i_format(int fd);
off_t s_format(int fd, long file_sys_size, struct d_super_block *super_block);

off_t i_start;
off_t i_bitmap_start;
off_t b_bitmap_start;
off_t b_start;
off_t curpos;
int file_sys_size;
int main(int argc, char *argv[])
{
    int fd;
    struct d_super_block super_block;
    //struct d_inode *inode;
    if (argc != 3)
    {
        return -1;
    }
    if ((fd = creat(argv[1], 0700)) == -1)
    {
        printf("file has been create\n");
        return -1;
    }
    else //文件已经创建
    {
        if ((fd = open(argv[1], O_RDWR)) == -1)
        {
            printf("open error\n");
            return -1;
        }
    }
    // for(int i=0;i<3;i++)
    //     printf("%s\n",argv[i]);

    if ((file_sys_size = (int)atoi(argv[2])) == -1)
    {
        printf("filesize wrong\n");
        return -1;
    }
    printf("fileSystem size is %d\n",
           file_sys_size);
    i_start = s_format(fd, file_sys_size, &super_block);
    printf("super block has been formated\n");

    i_bitmap_start = i_format(fd);
    printf("inodes has been formated\n");
    b_bitmap_start = i_bitmap_format(fd);
    b_start = b_bitmap_format(fd);
    printf("bitmap has been format\n");
    block_format(fd);
    printf("inode start:%ld\ni_bitmap_start:%ld\nb_bitmap_start:%ld\nb_start:%ld\n",
           i_start, i_bitmap_start, b_bitmap_start, b_start);
    fflush(stdout);
    printf("inodes include %ldK;inode bitmap includes %ldK;block bitmap includes %ldK;block includes %ldK\n",
            (i_bitmap_start-i_start)/1024,(b_bitmap_start-i_bitmap_start)/1024,(b_start-b_bitmap_start)/1024,
            (curpos-b_start)/1024);
    return 0;
}
int block_format(int fd)
{
    for (int i = 0; i < file_sys_size / BLOCKSIZE; i++)
    {
        set_empty_block(fd);
    }
    return 0;
}
off_t i_bitmap_format(int fd)
{
    b_bitmap_start = set_empty_block(fd);  
    return curpos;
}
off_t b_bitmap_format(int fd)
{
    char buf[BLOCKSIZE] = {0};

    b_start = set_empty_block(fd);
    for (int i = file_sys_size / BLOCKSIZE; i < BLOCKSIZE; ++i)
    {
        buf[i] = 1;
    }
    // memset(&buf[file_sys_size/BLOCKSIZE],1,BLOCKSIZE-file_sys_size/BLOCKSIZE);
    write(fd, buf, b_bitmap_start);
    curpos = lseek(fd, 0, SEEK_CUR);

    return curpos;
}
off_t s_format(int fd, long file_sys_size, struct d_super_block *super_block)
{

    super_block->s_ninodes = 0;
    super_block->s_imap_blocks = 1;
    super_block->s_zmap_blocks = 1;
    super_block->s_firstdatazone = 2;
    super_block->s_nzones = (file_sys_size / BLOCKSIZE) - super_block->s_imap_blocks - super_block->s_zmap_blocks - sizeof(struct d_inode);
    super_block->s_max_size = 7 * BLOCKSIZE + (BLOCKSIZE / sizeof(unsigned short)) * BLOCKSIZE +
                              (BLOCKSIZE / sizeof(unsigned short)) * (BLOCKSIZE / sizeof(unsigned short)) * BLOCKSIZE;
    curpos = lseek(fd, 0, SEEK_SET);
    set_empty_block(fd);
    lseek(fd, 0, SEEK_SET);
    write(fd, super_block, sizeof(super_block));
    curpos = lseek(fd, BLOCKSIZE, SEEK_SET);
    return curpos;
}
off_t i_format(int fd)
{
    for (int i = 0; i < file_sys_size / BLOCKSIZE; ++i)
    {
        set_empty_block(fd);
    }
    return curpos;
}
off_t set_empty_block(int fd)
{
    char empty_block[BLOCKSIZE] = {0};
    write(fd, empty_block, sizeof(empty_block));
    curpos = lseek(fd, 0, SEEK_CUR);
    //printf("curpos is %ld\n", curpos);
    return curpos;
}