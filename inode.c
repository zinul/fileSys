#include "fs.h"

extern struct super_block super_block;
struct d_inode *my_ialloc(int fd)
{
    int count;
    char inodes_map[BLOCKSIZE];
    struct d_inode *inode = (struct d_inode *)malloc(sizeof(struct d_inode));

    my_read(fd, IBITMAPOS, SEEK_SET, inodes_map, BLOCKSIZE);
    for (count = super_block.s_rember_node + 1; count < BLOCKSIZE; count++)
    {
        if (inodes_map[count] == 0)
        {
            inodes_map[super_block.s_rember_node] = 1;
            super_block.s_rember_node = count;
            my_write(fd, IBITMAPOS, SEEK_SET, inodes_map, BLOCKSIZE);
            super_block.s_ninodes++;
            my_write(fd, 0, SEEK_SET, &super_block, sizeof(super_block));
            return inode;
        }
    }
    if (count == BLOCKSIZE)
    {
        printf("no free inode\n");
        return NULL;
    }
    return NULL;
}
int my_ifree(int fd, unsigned short inode_cnt)
{
    char inode_map[BLOCKSIZE];

    super_block.s_ninodes++;

    if (super_block.s_rember_node > inode_cnt)
    {
        super_block.s_rember_node = inode_cnt;
    }
    my_write(fd, 0, SEEK_SET, &super_block, sizeof(super_block));

    if(my_read(fd,IBITMAPOS,SEEK_SET,inode_map,BLOCKSIZE)==-1)
    {
        printf("ifree_read error");
        return -1;
    }
    inode_map[inode_cnt]=0;
    if(my_write(fd,IBITMAPOS,SEEK_SET,inode_map,BLOCKSIZE)==-1)
    {
        printf("ifree_write error\n");
        return -1;
    }
    return 0;
}

struct d_inode *my_iget(int fd, unsigned short inode_cnt)
{
    // char inodes_map[BLOCKSIZE];
    // int count;
    struct d_inode *inode = (struct d_inode *)malloc(sizeof(struct d_inode));

    if (my_read(fd, inode_cnt * INODESIZE + INODEPOS, SEEK_SET, inode, BLOCKSIZE) == -1)
    {
        printf("iget_read error\n");
        return NULL;
    }
    //inode->i_time = time(NULL);
    if (my_write(fd, inode_cnt * INODESIZE + INODEPOS, SEEK_SET, inode, BLOCKSIZE) == -1)
    {
        printf("iget_write error\n");
        return NULL;
    }

    return inode;
}
void my_iput(int fd, struct d_inode *inode, unsigned short inode_cnt)
{
    if (--inode->i_nlinks == 0)
    {
        for (int i = 0; i < 9; i++)
        {
            my_free(fd, inode->i_zone[i]);
        }
        inode->i_mode = 0;
        my_ifree(fd, inode_cnt);
    }
    my_write(fd, INODEPOS + INODESIZE * inode_cnt, SEEK_SET, inode, INODESIZE);
    free(inode);
    return;
}

