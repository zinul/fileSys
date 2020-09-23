#include "list.h"
#include "fs.h"
#include <stdio.h>
struct super_block super_block;
#ifdef _TEST_
int main(int argc,char *argv[])
{
    int fd;
    char buf[BLOCKSIZE];
    if ((fd = open(argv[1], O_RDWR)) == -1)
    {
        printf("open error\n");
        return -1;
    }
    // for(int i=0;i<100;i++)
    // {
    //     my_ialloc(fd);
    //     //printf("alloc block%d\n",my_alloc(fd));
    // }
    my_read(fd,IBITMAPOS,SEEK_SET,buf,BLOCKSIZE);
    for(int i=0;i<10;i++){
        printf("%d:%d@\n",i,buf[i]);
       // fflush(fd);
    } 

    // for(int i=0;i<100;i++)
    // {
    //     my_ifree(fd,i);
    //     //printf("free block%d\n",my_free(fd,i));
    // }
    my_read(fd,BBITMAPOS,SEEK_SET,buf,BLOCKSIZE);
    for(int i=0;i<10;i++){
        printf("%d:%d@\n",i,buf[i]);
       // fflush(fd);
    } 

}
#endif