#include "fs.h"

extern struct file fileTable[];  
extern int fileTableCount;
extern struct d_super_block super_block;
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
void set_empty_block(int fd, off_t pos)
{
    char empty_block[BLOCKSIZE] = {0};
    my_write(fd, pos, SEEK_SET, empty_block, BLOCKSIZE);

    return;
}
int free_all_blocks(int fd, struct d_inode *inode)
{
    // 这里只做了直接块的释放
    for (int i = 0; i < 7; i++)
    {
        if (inode->i_zone[i] > 0)
        {
            my_free(fd, inode->i_zone[i]);
            inode->i_zone[i] = 0;
        }
    }
    // 一次间接块
    if (inode->i_zone[7] > 0)
    {
        unsigned short indirect_block[BLOCKSIZE / sizeof(unsigned short)];
        my_read(fd, inode->i_zone[7] * BLOCKSIZE + BLOCKPOS, SEEK_SET, indirect_block, BLOCKSIZE);
        for (int i = 0; i < BLOCKSIZE / sizeof(unsigned short); ++i)
        {
            if (indirect_block[i] > 0)
            {
                my_free(fd, indirect_block[i]);
            }
        }
        my_free(fd, inode->i_zone[7]);
        inode->i_zone[7] = 0;
    }
    if (inode->i_zone[8] > 0)
    {
        unsigned short indirect_block[BLOCKSIZE / sizeof(unsigned short)];
        unsigned short double_indirect_block[BLOCKSIZE / sizeof(unsigned short)];
        my_read(fd, inode->i_zone[8] * BLOCKSIZE + BLOCKPOS, SEEK_SET, indirect_block, BLOCKSIZE);
        for (int i = 0; i < BLOCKSIZE / sizeof(unsigned short); ++i)
        {
            if (indirect_block[i] > 0)
            {
                my_read(fd, indirect_block[i] * BLOCKSIZE + BLOCKPOS, SEEK_SET, double_indirect_block, BLOCKSIZE);
                for (int j = 0; j < BLOCKSIZE / sizeof(unsigned short); j++)
                {
                    if (double_indirect_block[j] > 0)
                    {
                        my_free(fd, double_indirect_block[j]);
                    }
                }
                my_free(fd, indirect_block[i]);
            }
        }
        my_free(fd, inode->i_zone[8]);
    }
    return 0;
}