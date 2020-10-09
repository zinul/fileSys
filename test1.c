#include "fs.h"
struct file fileTable[100];  
int fileTableCount=0;
struct d_super_block super_block;
int main(char argc,char *argv[])
{
    char *a=malloc(1);
    printf("%p\n%d\n",a,a);
//     char *a;
//     struct d_inode *inode0;
//     struct d_inode *inode1;
//     int fd;
//     char buf[BLOCKSIZE];
//     struct dir dir;
//     int my_fd;
//     char *str = "hello world\n";
//     //chdir("/home/zmz/fileSys");
//     chdir("/home/zmz/fileSys/build/linux/x86_64/release");
//     if ((fd = open(argv[1], O_RDWR)) == -1)
//     {
//         printf("open error\n");
//         return -1;
//     }
//     // my_namei(fd, "/");
//     my_iget(fd,0);
//         printf("path:\n");
//     fflush(stdout);  
//     a = malloc(1);
//     a = malloc(1);
//     //a = malloc(sizeof(struct d_inode));
//     // inode0 = (struct d_inode *)malloc(sizeof(struct d_inode));
//     // inode1 = (struct d_inode *)malloc(sizeof(struct d_inode));
//     // printf("xinode:%d\n", inode0->i_cnt);
//     // printf("xinode:%d\n", inode1->i_cnt);
//     fflush(stdout);
 }
