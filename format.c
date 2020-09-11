#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "fs.h"
#define BLOCKSIZE 1024
#define BLOCK_SIZE_BITS 10	// 数据块长度所占比特位数。
#define NR_OPEN 20		
#define NR_INODE 32
#define NR_FILE 64

struct d_super_block* s_format(int fd,long file_sys_size);

off_t curpos;
int main(int argc, char *argv[])
{
    int fd;
    if (argc != 3)
    {
        return -1;
    }
    if ((fd = creat(argv[1], 0700)) == -1)
    {
        printf("file has been create\n");
        return -1;
    }
    else                //文件已经创建
    {
        if((fd=open(argv[1],O_RDWR))==-1)
        {
            printf("open error\n");
            return -1;
        }

    }
    long file_sys_size;
    if((file_sys_size=atoi(argv[1]))==-1)
    {
        printf("filesize wrong\n");
        return -1;
    }
    printf("fileSystem size is %ld\nsuper_block size is %ld\n",
        file_sys_size,sizeof(struct d_super_block));
    s_format(fd,file_sys_size);
}

struct d_super_block* s_format(int fd,long file_sys_size){
    struct d_super_block *super_block;

    super_block->s_ninodes=0;
    super_block->s_imap_blocks=1;
    super_block->s_zmap_blocks=1;
    super_block->s_firstdatazone=2;
    super_block->s_nzones=(file_sys_size/BLOCKSIZE)-super_block->s_imap_blocks-super_block->s_zmap_blocks;
    super_block->s_max_size=7*BLOCKSIZE+(BLOCKSIZE/sizeof(unsigned short))*BLOCKSIZE+
                            (BLOCKSIZE/sizeof(unsigned short))*(BLOCKSIZE/sizeof(unsigned short))*BLOCKSIZE;
    curpos=lseek(fd,0,SEEK_SET);
    write(fd,super_block,sizeof(super_block));
    return &super_block;
}