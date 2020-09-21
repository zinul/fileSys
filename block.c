#include "fs.h"

int my_alloc(int fd)
{
    int block_pos;
    char buf[BLOCKSIZE];
    if (my_read(fd, BBITMAPOS, SEEK_SET, buf, BLOCKSIZE) == -1)
    {
        return -1;
    }

    for (block_pos = 0; block_pos < 1024; block_pos++)
    {
        if (buf[block_pos] == 1)
        {
            continue;
        }
        buf[block_pos] = 1;
        if (my_write(fd, BBITMAPOS, SEEK_SET, buf, BLOCKSIZE) == -1)
        {
            return -1;
        }
        return block_pos;
    }
    // 此时表示无空闲块
    printf("no free block\n");
    return -1;
}

int my_free(int fd, int blk_cnt)
{
    char buf[BLOCKSIZE];

    if (my_read(fd, BBITMAPOS, SEEK_SET, buf, BLOCKSIZE) == -1)
    {
        return -1;
    }
    if(buf[blk_cnt]!=0)
    {
        buf[blk_cnt]= 0;
    } 
    else
    {
        printf("block is free\n");
        return blk_cnt;
    }
    
    if (my_write(fd, BBITMAPOS, SEEK_SET, buf, BLOCKSIZE) == -1)
    {
        return -1;
    }

    set_empty_block(fd, BLOCKPOS + BLOCKSIZE * blk_cnt);
    return blk_cnt;
}
