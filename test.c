#include "fs.h"

int printDir(int fd, struct dir dir);
void psuper_block(int fd);

struct file fileTable[100];
int fileTableCount = 0;
struct d_super_block super_block;

int main(int argc, char *argv[])
{
    //super_block=(struct d_super_block *)malloc(sizeof(struct d_super_block));
    int fd;
    char blockSizeBuf[BLOCKSIZE];
    char *buf;
    struct d_inode *inode;
    struct dir dir;
    int my_fd;
    char *str = "hello world\n";
    for (int i = 0; i < 100; ++i)
    {
        fileTable[i].f_inode=NULL;
    }
    if ((fd = open(argv[1], O_RDWR)) == -1)
    {
        printf("open error\n");
        return -1;
    }
    my_read(fd, SUPERBPOS, SEEK_SET, &super_block, sizeof(struct d_super_block));
    psuper_block(fd); 
    my_fd=sys_creat(fd,"/first",O_RDWR);
    my_fd=sys_creat(fd,"/secend",O_RDWR);
    my_fd=sys_creat(fd,"/third",O_RDWR);
    // if ((my_fd = sys_open(fd, "/first", 0, O_RDWR)) == -1)
    // {
    //     printf("don't have permissions\n");
    //     return 0;
    // }
    if (sys_write(fd, my_fd, str, strlen(str) + 1) == -1)
    {
        printf("write error\n");
    }

    buf = malloc(strlen(str) + 1);

    sys_lseek(fd,my_fd,0,SEEK_SET);
    sys_read(fd, my_fd, buf, strlen(str) + 1);
    
    sys_close(fd,my_fd);
    sys_read(fd, my_fd, buf, strlen(str) + 1);
    sys_unlink(fd,"/third");

    my_read(fd, IBITMAPOS, SEEK_SET, blockSizeBuf, BLOCKSIZE);
    printf("inode bitmap:\ncount\tstatus\t\n");
    for (int i = 0; i < 10; i++)
    {
        printf("%d\t%d\t\n", i, blockSizeBuf[i]);
    }

    my_read(fd, BBITMAPOS, SEEK_SET, blockSizeBuf, BLOCKSIZE);
    printf("block bitmap:\ncount\tstatus\t\n");
    for (int i = 0; i < 10; i++)
    {
        printf("%d\t%d\t\n", i, blockSizeBuf[i]);
    }

    dir=getDir(fd,inode);
    printf("root dir:\n");
    printDir(fd, dir);
   
}
struct dir getDir(int fd, struct d_inode *inode)
{
    struct dir dir;
    my_read(fd, inode->i_zone[0] * BLOCKSIZE + BLOCKPOS, SEEK_SET, &dir, sizeof(struct dir));
    return dir;
}
void psuper_block(int fd)
{
    printf("block use %d\n", super_block.s_nzones);
    printf("inode use %d\n", super_block.s_ninodes);
    printf("first block %d\n", super_block.s_firstdatazone);
    printf("remembernode %d\n", super_block.s_rember_node);
    printf("file max size %ld\n\n", super_block.s_max_size);
}
int printDir(int fd, struct dir dir)
{
    int i = 0;
    for (int i = 0; i < 10; i++)
    {
        if (dir.item[i].inode_cnt == 0xFFFF)
        {
            break;
        }
        printf("name:%10s\t\tinode:%d\n", dir.item[i].name, dir.item[i].inode_cnt);
    }
    return 0;
}