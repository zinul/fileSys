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
    file_item->f_pos = fileTableCount;
    file_item->f_flags = flags;
    file_item->f_inode = inode;
    file_item->f_mode = modes;

    for (int i = 0; i < 100; ++i)
    {
        if (!fileTable[i].f_inode)
        {
            fileTableCount = i;
        }
    }
    if ((modes & O_APPEND) == O_APPEND)
    {
        file_item->f_pos = inode->i_size;
    }
    else
    {
        free_all_blocks(fd, inode);
        inode->i_size = 0;
    }

    return fileTableCount;
}
size_t sys_read(int fd, int my_fd, void *buf, size_t nbytes)
{
    struct file *file = &fileTable[my_fd];
    if ((file->f_mode & O_RDONLY) != O_RDONLY)
    {
        printf("no read permision\n");
        return 0;
    }
    struct d_inode *inode = file->f_inode;
    size_t read_bytes = file->f_pos;
    size_t remain_bytes = nbytes;
    while (remain_bytes > 0)
    {
        size_t pos;
        size_t offset;
        size_t distanceToBlockEnd;
        char funcBuf[BLOCKSIZE];

        memset(funcBuf, 0, BLOCKSIZE);
        pos = my_bmap(fd, inode, read_bytes);
        offset = pos % BLOCKSIZE;
        distanceToBlockEnd = BLOCKSIZE - offset;

        if (pos == (size_t)(-1) || remain_bytes == 0)
        {
            break;
        }
        else if (remain_bytes <= distanceToBlockEnd)
        {
            my_read(fd, pos, SEEK_SET, funcBuf, remain_bytes);
            memcpy((char *)buf + read_bytes, funcBuf, remain_bytes);
            read_bytes += remain_bytes;
            remain_bytes = 0;
        }
        else
        {
            my_read(fd, pos, SEEK_SET, funcBuf, distanceToBlockEnd);
            memcpy((char *)buf + read_bytes, funcBuf, distanceToBlockEnd);
            read_bytes += distanceToBlockEnd;
            remain_bytes -= distanceToBlockEnd;
        }
    }
    nbytes = read_bytes - file->f_pos;
    file->f_pos = read_bytes;
    return nbytes;
}
size_t sys_write(int fd, int my_fd, void *buf, size_t count)
{
    struct file *file = &fileTable[my_fd];
    if ((file->f_mode & O_RDWR) != O_RDWR)
    {
        printf("no write permision\n");
        return 0;
    }
    if (count + file->f_pos > super_block.s_max_size)
    {
        printf("write over the limition\n");
        return 0;
    }
    struct d_inode *inode = file->f_inode;
    size_t filePos = file->f_pos;
    size_t blockPos;
    size_t writedBytes = 0;
    size_t thisTimeWrite;

    while (count > 0)
    {
        // 直接块
        if (filePos <= BLOCKSIZE * 7)
        {
            if (inode->i_zone[filePos / BLOCKSIZE] == 0)
            {
                inode->i_zone[filePos / BLOCKSIZE] = my_alloc(fd);
            }
        }
        // 一次间接
        else if (filePos <= BLOCKSIZE * (BLOCKSIZE / sizeof(unsigned short)))
        {
            unsigned short indirectBuf[BLOCKSIZE / sizeof(unsigned short)];
            if (inode->i_zone[7] == 0)
            {
                inode->i_zone[7] = my_alloc(fd);
            }
            my_read(fd, inode->i_zone[7] * BLOCKSIZE + BLOCKPOS, SEEK_SET, indirectBuf, BLOCKSIZE);
            if (indirectBuf[filePos / BLOCKSIZE - 7] == 0)
            {
                indirectBuf[filePos / BLOCKSIZE - 7] = my_alloc(fd);
                my_write(fd, inode->i_zone[7] * BLOCKSIZE + BLOCKPOS, SEEK_SET, indirectBuf, BLOCKSIZE);
            }
        }
        // else
        // {
        //     unsigned short indirectBuf[BLOCKSIZE / sizeof(unsigned short)];
        //     unsigned short doubleIndirectBuf[BLOCKSIZE / sizeof(unsigned short)];
        //     if (inode->i_zone[8] == 0)
        //     {
        //         inode->i_zone[8] = my_alloc(fd);
        //     }
        //     my_read(fd, inode->i_zone[8] * BLOCKSIZE + BLOCKPOS, SEEK_SET, indirectBuf, BLOCKSIZE);
        //     if (indirectBuf[(filePos / BLOCKSIZE - 7 - BLOCKSIZE * (BLOCKSIZE / sizeof(unsigned short)))/BLOCKSIZE] == 0)
        //     {
        //         indirectBuf[(filePos / BLOCKSIZE - 7 - BLOCKSIZE * (BLOCKSIZE / sizeof(unsigned short)))/BLOCKSIZE]=my_alloc(fd);
        //         my_write(fd,inode->i_zone[8] * BLOCKSIZE + BLOCKPOS, SEEK_SET, indirectBuf, BLOCKSIZE);
        //     }
        //     my_read(fd,indirectBuf[(filePos / BLOCKSIZE - 7 - BLOCKSIZE * (BLOCKSIZE / sizeof(unsigned short)))/BLOCKSIZE],
        //     SEEK_SET,doubleIndirectBuf,BLOCKSIZE);

        // }
        blockPos = my_bmap(fd, inode, filePos);
        if (count >= BLOCKSIZE - blockPos % BLOCKSIZE)
        {
            thisTimeWrite = BLOCKSIZE - blockPos % BLOCKSIZE;
        }
        else
        {
            thisTimeWrite = count;
        }
        my_write(fd, blockPos, SEEK_SET, (char *)buf + writedBytes, thisTimeWrite);

        count -= thisTimeWrite;
        file->f_pos += thisTimeWrite;
        filePos += thisTimeWrite;
        writedBytes += thisTimeWrite;
    }
    return writedBytes;
}

off_t sys_lseek(int fd, int my_fd, off_t offset, int whence)
{
    struct file *file = &fileTable[my_fd];
    off_t prePos = file->f_pos;
    if ((whence & SEEK_SET) == SEEK_SET)
    {
        if (offset < 0)
        {
            printf("lseek pos can't less than 0\n");
            return prePos;
        }
        file->f_pos = offset;
    }
    else if ((whence & SEEK_CUR) == SEEK_CUR)
    {
        if (offset + file->f_pos < 0)
        {
            printf("lseek pos can't less than 0\n");
            return prePos;
        }
        file->f_pos = offset + file->f_pos;
    }
    else if ((whence & SEEK_END) == SEEK_END)
    {
        if (file->f_inode->i_size + offset < 0)
        {
            printf("lseek pos can't less than 0\n");
            return prePos;
        }
        file->f_pos = offset + file->f_inode->i_size;
    }
    else
    {
        printf("error whence in lseek\n");
    }
    return prePos;
}
void sys_close(int fd, int my_fd)
{
    my_iput(fd, fileTable[my_fd].f_inode);
    memset(&fileTable[my_fd], 0, sizeof(struct file));
}
int sys_creat(int fd, char *path, mode_t modes)
{
    char *dirPath = (char *)malloc(strlen(path) + 1);
    char *fileName;
    struct d_inode *file_inode;
    strcpy(dirPath, path);
    int i;
    for (i = strlen(path); i >= 0; --i)
    {
        if (path[i] == '/')
        {
            fileName = &dirPath[i + 1];
            dirPath[i] = 0;
            break;
        }
    }
    if (strlen(fileName) >= 14)
    {
        printf("file name is too long\n");
        free(dirPath);
        return -1;
    }
    file_inode = my_namei(fd, path);
    if (file_inode != NULL)
    {
        printf("dir is existed\n");
        free(dirPath);
        return -1;
    }
    else
    {
        struct d_inode *dir_inode = my_namei(fd, dirPath);
        struct dir dir;

        my_read(fd, dir_inode->i_zone[0] * BLOCKSIZE + BLOCKPOS, SEEK_SET, &dir, BLOCKSIZE);

        unsigned short inode_cnt = my_ialloc(fd);
        file_inode = my_iget(fd, inode_cnt);

        file_inode->i_uid = 0;
        file_inode->i_gid = 0;
        file_inode->i_nlinks = 1;
        file_inode->i_size = 0;
        file_inode->i_zone[0] = my_alloc(fd);
        file_inode->i_cnt = inode_cnt;
        file_inode->i_mode=modes;
        for (int i = 2; i < BLOCKSIZE / (sizeof(struct dir_item)); ++i)
        {
            if (dir.item[i].inode_cnt == (unsigned short)(-1))
            {
                dir.item[i].inode_cnt = file_inode->i_cnt;
                strcpy(dir.item[i].name, fileName);
                //dir.item[i + 1].inode_cnt = (unsigned short)(-1);
                my_write(fd, dir_inode->i_zone[0] * BLOCKSIZE + BLOCKPOS, SEEK_SET, &dir, BLOCKSIZE);
                break;
            }
        }
    }
    int my_fd = fileTableCount;
    fileTable[my_fd].f_inode = file_inode;
    fileTable[my_fd].f_mode = modes;
    fileTable[my_fd].f_pos = 0;
    fileTable[my_fd].f_count = 1;
    for (int i = 0; i < 100; ++i)
    {
        if (!fileTable[i].f_inode)
        {
            fileTableCount = i;
        }
    }
    free(dirPath);
    return my_fd;
}
void sys_unlink(int fd, char *path)
{
    struct d_inode *dir_inode;
    struct d_inode *file_inode;
    struct dir dir;
    char *temp = (char *)malloc(strlen(path) + 1);
    char *filePath = (char *)malloc(strlen(path) + 1);
    char *fileName;
    char *delim = "/";

    strcpy(temp, path);
    temp = trim(temp);
    for (int i = strlen(temp); i >= 0; i--)
    {
        if (temp[i] == '/')
        {
            fileName = &temp[i + 1];
            temp[i] = 0;
            break;
        }
    }
    dir_inode = my_namei(fd, temp);
    file_inode = my_namei(fd, filePath);

    my_read(fd, dir_inode->i_zone[0] * BLOCKSIZE + BLOCKPOS, SEEK_SET, &dir, BLOCKSIZE);
    for (int i = 0; i < BLOCKSIZE / sizeof(struct dir_item); ++i)
    {
        unsigned short cnt = dir.item[i].inode_cnt;
        if (i == (BLOCKSIZE / sizeof(struct dir_item) - 1))
        {
            printf("no dir or file name is %s", fileName);
            free(temp);
            free(filePath);
            return;
        }
        else if (strcmp(dir.item[i].name, fileName))
        {
            dir.item[i].inode_cnt = (unsigned short)(-1);
            my_iput(fd, dir_inode);
            memset(dir.item[i].name, 0, sizeof(char) * 14);
            free_all_blocks(fd, file_inode);
            my_ifree(fd, file_inode);
        }
    }
    free(temp);
    free(filePath);
    return;
}