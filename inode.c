#include "fs.h"

extern struct file fileTable[];  
extern int fileTableCount;
extern struct d_super_block super_block;
unsigned short my_ialloc(int fd)
{
    int count;
    char inodes_map[BLOCKSIZE];
    //struct d_inode *inode = (struct d_inode *)malloc(sizeof(struct d_inode));

    my_read(fd, IBITMAPOS, SEEK_SET, inodes_map, BLOCKSIZE);
    for (count = super_block.s_rember_node; count < BLOCKSIZE; count++)
    {
        if (inodes_map[count] == 0)
        {
            inodes_map[super_block.s_rember_node] = 1;
            super_block.s_rember_node = count;
            my_write(fd, IBITMAPOS, SEEK_SET, inodes_map, BLOCKSIZE);
            printf("inodemap %d is %d\n", count, inodes_map[count]);
            super_block.s_ninodes++;
            my_write(fd, 0, SEEK_SET, &super_block, sizeof(super_block));
            return count;
        }
    }
    if (count == BLOCKSIZE)
    {
        printf("no free inode\n");
        return 0;
    }
    return 0;
}
int my_ifree(int fd,struct d_inode *inode)
{
    char inode_map[BLOCKSIZE];

    super_block.s_ninodes++;

    if (super_block.s_rember_node > inode->i_cnt)
    {
        super_block.s_rember_node = inode->i_cnt;
    }
    my_write(fd, 0, SEEK_SET, &super_block, sizeof(super_block));

    if (my_read(fd, IBITMAPOS, SEEK_SET, inode_map, BLOCKSIZE) == -1)
    {
        printf("ifree_read error");
        return -1;
    }
    inode_map[inode->i_cnt] = 0;
    if (my_write(fd, IBITMAPOS, SEEK_SET, inode_map, BLOCKSIZE) == -1)
    {
        printf("ifree_write error\n");
        return -1;
    }
    return 0;
}

struct d_inode *my_iget(int fd, unsigned short inode_cnt)
{
    if(!getInodeBit(fd,inode_cnt))
    {
        printf("inode%d is null\n",inode_cnt);
        return NULL;
    }
    struct d_inode *inode = (struct d_inode *)malloc(sizeof(struct d_inode));

    if (my_read(fd, inode_cnt * INODESIZE + INODEPOS, SEEK_SET, inode, BLOCKSIZE) == -1)
    {
        printf("iget_read error\n");
        return NULL;
    }
    // if (my_write(fd, inode_cnt * INODESIZE + INODEPOS, SEEK_SET, inode, BLOCKSIZE) == -1)
    // {
    //     printf("iget_write error\n");
    //     return NULL;
    // }

    return inode;
}
void my_iput(int fd, struct d_inode *inode)
{
    // if (--inode->i_nlinks == 0)
    // {
    //     for (int i = 0; i < 9; i++)
    //     {
    //         my_free(fd, inode->i_zone[i]);
    //     }
    //     inode->i_mode = 0;
    //     my_ifree(fd, inode_cnt);
    // }
    my_write(fd, INODEPOS + INODESIZE * inode->i_cnt, SEEK_SET, inode, INODESIZE);
    free(inode);
    return;
}

off_t my_bmap(int fd, struct d_inode *inode, off_t offset)
{
    unsigned short block_cnt;
    off_t pos;

    void *indirect_buf = malloc(sizeof(char) * BLOCKSIZE);
    // char buf[BLOCKSIZE];
    if (offset > inode->i_size)
    {
        printf("offset is bigger than filesize\n");
        return -1;
    }
    if (offset < BLOCKSIZE * 7)
    {
        block_cnt = inode->i_zone[offset / BLOCKSIZE];
    }
    else if (offset < BLOCKSIZE * (BLOCKSIZE / sizeof(unsigned short)) * BLOCKSIZE + 7)
    {
        my_read(fd, BLOCKPOS + (inode->i_zone[7]) * BLOCKSIZE, SEEK_SET, indirect_buf, BLOCKSIZE);
        block_cnt = offset / BLOCKSIZE;
        block_cnt = *(((unsigned short *)indirect_buf) + (block_cnt - 7));
    }
    else
    {
        my_read(fd, BLOCKPOS + (inode->i_zone[8]) * BLOCKSIZE, SEEK_SET, indirect_buf, BLOCKSIZE);
        block_cnt = offset / BLOCKSIZE / BLOCKSIZE;
        block_cnt = *((unsigned short *)indirect_buf + block_cnt);
        // my_read(fd,BLOCKPOS+BLOCKSIZE*block_cnt,SEEK_SET,buf,BLOCKSIZE);
    }
    pos = BLOCKPOS + block_cnt * BLOCKSIZE + offset % BLOCKSIZE;
    free(indirect_buf);
    return pos;
}
char *trim(char *str)
{
    int len = strlen(str);
    char *temp = (char *)malloc(len);
    int j = 0;
    for (int i = 0; i < len; i++)
    {
        if (str[i] == ' ')
        {
            continue;
        }
        else
        {
            temp[j++] = str[i];
        }
    }
    temp[j] = '\0';
    strcpy(str,temp);
    free(temp);
    return str;
}
struct d_inode *my_namei(int fd, const char *path)
{
    struct d_inode *work_inode;
    int len = strlen(path);
    struct dir work_dir;
    char *temp;
    char *delim = "/";
    temp = trim(path);
    if (temp[0] == '/')
    {
        work_inode = my_iget(fd, IROOT);
    }
    else
    {
        printf("path wrong\n");
        return NULL;
    }
    temp = strtok(temp, delim);
    if(temp==NULL)
    {
        return work_inode;
    }
    while (temp = strtok(NULL, delim))
    {
        if((work_inode->i_mode&(O_DIRECTORY|O_RDONLY))==(O_DIRECTORY|O_RDONLY))
        {
            my_read(fd,BLOCKPOS+work_inode->i_zone[0]*BLOCKSIZE,SEEK_SET,&work_dir,sizeof(work_dir));
            if(work_dir.item[0].inode_cnt==0&&strcmp(temp,".."))
            {
                continue;
            }
            for(int i=0;i<BLOCKSIZE/sizeof(struct dir_item);++i)
            {
                unsigned short cnt=work_dir.item[i].inode_cnt;
                if(i==(BLOCKSIZE/sizeof(struct dir_item)-1))
                {
                    printf("no dir or file name is %s",temp);
                    return NULL;
                }
                else if(strcmp(work_dir.item[i].name,temp))
                {
                    my_iput(fd,work_inode);
                    work_inode = my_iget(fd,cnt);
                    break;
                }
                
            }
        }
    }
    return work_inode; 
}
int getBlockBit(int fd,unsigned short cnt)
{
    char buf[BLOCKSIZE];
    my_read(fd,BBITMAPOS,SEEK_SET,buf,BLOCKSIZE);
    return buf[cnt];
}
int getInodeBit(int fd,unsigned short cnt)
{
    char buf[BLOCKSIZE];
    my_read(fd,IBITMAPOS,SEEK_SET,buf,BLOCKSIZE);
    return buf[cnt];
}