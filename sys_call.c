#include "fs.h"

extern struct file fileTable[];  
extern int fileTableCount;
extern struct d_super_block super_block;

int sys_open(int fd, char *pathname, unsigned short flags, unsigned short modes)
{
    struct d_inode *inode;
    struct file *file_item;
    inode = my_namei(fd, pathname);
    if (!inode)
    {
        return -1;
    }
    if ((inode->i_mode & modes) != modes)
    {
        printf("don't have permissions\n");
        return -1;
    }
    file_item = &(fileTable[fileTableCount]);
    file_item->f_count = fileTableCount++;
    file_item->f_flags = flags;
    file_item->f_inode = inode;
    file_item->f_mode = modes;

    if ((modes & O_APPEND) == O_APPEND)
    {
        file_item->f_pos = inode->i_size;
    }
    else
    {
        free_all_blocks(fd,inode);
        inode->i_size=0;
    }
    
    return fileTableCount;
}
size_t sys_read(int fd,int my_fd,void *buf,size_t nbytes)
{
    struct file *file=&fileTable[my_fd];
    if((file->f_mode&O_RDONLY)!=O_RDONLY)
    {
        printf("no read permision\n");
        return 0;
    }
    struct d_inode *inode=file->f_inode;
    size_t read_bytes=file->f_count;
    size_t remain_bytes=nbytes;
    while(remain_bytes>0)
    {
        size_t pos;
        size_t offset;
        size_t distanceToBlockEnd;
        char funcBuf[BLOCKSIZE];

        memset(funcBuf,0,BLOCKSIZE);
        pos=my_bmap(fd,inode,read_bytes);
        offset=pos%BLOCKSIZE;
        distanceToBlockEnd= BLOCKSIZE-offset;

        if(pos==(size_t)(-1)||remain_bytes==0)
        {
            break;
        }
        else if(remain_bytes<=distanceToBlockEnd)
        {
            my_read(fd,pos,SEEK_SET,funcBuf,remain_bytes);
            memcpy((char *)buf+read_bytes,funcBuf,remain_bytes);
            read_bytes+=remain_bytes;
            remain_bytes=0;
        }
        else
        {
            my_read(fd,pos,SEEK_SET,funcBuf,distanceToBlockEnd);
            memcpy((char *)buf+read_bytes,funcBuf,distanceToBlockEnd);
            read_bytes+=distanceToBlockEnd;
            remain_bytes-=distanceToBlockEnd;
        }
        
    }
    nbytes=read_bytes-file->f_count;
    file->f_count=read_bytes;
    return nbytes;
}