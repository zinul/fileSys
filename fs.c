#include "fs.h"

int my_read(int fd, off_t pos, int whence, const void *buf, size_t n)
{
    if (lseek(fd, pos, whence) == -1)
    {
        printf("read_lseek error\n");
        return -1;
    }
    read(fd, buf, n);
    return 0;
}
int my_write(int fd, off_t pos, int whence, const void *buf, size_t n)
{
    if (lseek(fd, pos, whence) == -1)
    {
        printf("write_lseek error\n");
        return -1;
    }
    write(fd, buf, n);
    return 0;
}
void set_empty_block(int fd,off_t pos)
{
    char empty_block[BLOCKSIZE] = {0};
    my_write(fd,pos,SEEK_SET,empty_block,BLOCKSIZE);

    return ;
}