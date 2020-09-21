#include "list.h"
#include "fs.h"
#include <stdio.h>
int main(int argc,char *argv[])
{
    int fd;
    char buf[BLOCKSIZE];
    if ((fd = open(argv[1], O_RDWR)) == -1)
    {
        printf("open error\n");
        return -1;
    }
    // for(int i=0;i<1025;i+=2)
    // {
    //     my_alloc(fd);
    //     //printf("alloc block%d\n",my_alloc(fd));
    // }
    my_read(fd,BBITMAPOS,SEEK_SET,buf,BLOCKSIZE);
    for(int i=0;i<1024;i++){
        printf("%d:%d@\n",i,buf[i]);
       // fflush(fd);
    }    
    // for(int i=0;i<1024;i++)
    // {
    //     my_free(fd,i);
    //     //printf("free block%d\n",my_free(fd,i));
    // }

}