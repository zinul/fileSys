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
            inodes_map[count] = 1;
            super_block.s_rember_node = count;
            my_write(fd, IBITMAPOS, SEEK_SET, inodes_map, BLOCKSIZE);
            printf("alloc inode%d:\n", count);
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
int my_ifree(int fd, struct d_inode *inode)
{
    char inode_map[BLOCKSIZE];

    super_block.s_ninodes--;

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
    struct d_inode *inode;
    if (inode_cnt == ((unsigned short)(-1)))
    {
        return NULL;
    }
    if (!getInodeBit(fd, inode_cnt))
    {
        printf("inode%d is null\n", inode_cnt);
        return NULL;
    }
    inode = (struct d_inode *)malloc(sizeof(struct d_inode));
    if (my_read(fd, inode_cnt * INODESIZE + INODEPOS, SEEK_SET, inode, sizeof(struct d_inode)) == -1)
    {
        printf("iget_read error\n");
        return NULL;
    }

    return inode;
}
void my_iput(int fd, struct d_inode *inode)
{
    my_write(fd, INODEPOS + INODESIZE * (inode->i_cnt), SEEK_SET, inode, sizeof(struct d_inode));
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
        printf("filesize:%d ",inode->i_size);
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
char *ltrim(char *s)
{
    int i = 0, j = 0;

    if (s == NULL)
        return (char *)NULL;

    while (' ' == s[i] || '\t' == s[i])
        i++;

    if (0 == i)
        return s;

    while ('\0' != s[j + i])
    {
        s[j] = s[j + i];
        j++;
    }

    s[j] = '\0';

    return s;
}

/*去除字符串右边的空格*/
char *rtrim(char *s)
{
    int i;

    if (s == NULL)
        return (char *)NULL;

    for (i = strlen(s); i > 0; i--)
    {
        if (s[i - 1] == ' ' || s[i - 1] == '\t')
            s[i - 1] = '\0';
        else
            break;
    }

    return s;
}

/*去除字符串两边的空格*/
char *trim(char *s)
{
    ltrim(s);
    rtrim(s);
    return s;
}

unsigned short my_namei(int fd, char *path)
{
    struct d_inode *work_inode;
    int len = strlen(path);
    struct dir work_dir;
    char *delim = "/";
    unsigned short cnt;
    if (path[0] == '/')
    {   
        work_inode = my_iget(fd, IROOT);
    }
    else
    {
        printf("path wrong\n");
        return -1;
    }

    path = strtok(path, delim);
    // //fflush(stdout);

    if (path == NULL)
    {
        return work_inode->i_cnt;
    }
    do
    {
        //printf("%o\n%o\n", IS_DIR, work_inode->i_mode);
        
        if ((work_inode->i_mode & O_RDONLY) == O_RDONLY)
        {
            my_read(fd, BLOCKPOS + work_inode->i_zone[0] * BLOCKSIZE, SEEK_SET, &work_dir, sizeof(work_dir));
            if (work_dir.item[0].inode_cnt == 0 && !strcmp(path, ".."))
            {
                continue;
            }
            for (int i = 0; i < BLOCKSIZE / sizeof(struct dir_item); ++i)
            {

                cnt = work_dir.item[i].inode_cnt;
                // printf("cnt:%o\n", cnt);
                if (i == (BLOCKSIZE / sizeof(struct dir_item) - 1))
                {
                    my_iput(fd, work_inode);
                    return -1;
                }
                else if (!strcmp(work_dir.item[i].name, path))
                {
                    my_iput(fd, work_inode);
                    work_inode = my_iget(fd, cnt);
                    break;
                }
            }           
        }
    } while (path = strtok(NULL, delim));
    my_iput(fd,work_inode);
    return cnt;
}
int getBlockBit(int fd, unsigned short cnt)
{
    char buf[BLOCKSIZE];
    my_read(fd, BBITMAPOS, SEEK_SET, buf, BLOCKSIZE);
    return buf[cnt];
}
int getInodeBit(int fd, unsigned short cnt)
{
    char buf[BLOCKSIZE];
    my_read(fd, IBITMAPOS, SEEK_SET, buf, BLOCKSIZE);
    return buf[cnt];
}