#include "fs.h"

extern struct file fileTable[];
extern int fileTableCount;
extern struct d_super_block super_block;

int sys_open(int fd, char *pathname, unsigned short flags, unsigned short modes)
{
    struct d_inode *inode;
    struct file *file_item;
    unsigned short inode_cnt;
    inode_cnt = my_namei(fd, pathname);
    inode = my_iget(fd, inode_cnt);
    int my_fd;
    if (!inode)
    {
        return -1;
    }
    if ((inode->i_mode & modes) != modes)
    {
        printf("don't have permissions to open\n");
        return -1;
    }
    file_item = &(fileTable[fileTableCount]);
    file_item->f_pos = 0;
    file_item->f_flags = flags;
    file_item->f_inode = inode;
    file_item->f_mode = modes;
    file_item->f_count = fileTableCount;
    for (int i = 0; i < 100; ++i)
    {
        if (!fileTable[i].f_inode)
        {
            fileTableCount = i;
            break;
        }
    }
    if ((flags & O_APPEND) == O_APPEND)
    {
        file_item->f_pos = inode->i_size;
    }
    else
    {
        free_all_blocks(fd, inode);
        inode->i_size = 0;
    }

    return file_item->f_count;
}
size_t sys_read(int fd, int my_fd, void *buf, size_t nbytes)
{
    struct file *file = &fileTable[my_fd];
    if (file->f_count == (unsigned short)(-1))
    {
        printf("fd:%d was closed\n",my_fd);
        return 0;
    }
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
        size_t diskPos;
        size_t offset;
        size_t distanceToBlockEnd;
        char funcBuf[BLOCKSIZE];

        memset(funcBuf, 0, BLOCKSIZE);
        diskPos = my_bmap(fd, inode, read_bytes);
        offset = diskPos % BLOCKSIZE;
        distanceToBlockEnd = BLOCKSIZE - offset;

        if (diskPos == (size_t)(-1) || remain_bytes == 0)
        {
            break;
        }
        else if (remain_bytes <= distanceToBlockEnd)
        {
            my_read(fd, diskPos, SEEK_SET, funcBuf, remain_bytes);
            memcpy((char *)buf + read_bytes, funcBuf, remain_bytes);
            read_bytes += remain_bytes;
            remain_bytes = 0;
        }
        else
        {
            my_read(fd, diskPos, SEEK_SET, funcBuf, distanceToBlockEnd);
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
    if (file->f_count == (unsigned short)(-1))
    {
        printf("fd:%d was closed\n",my_fd);
        return 0;
    }
    if ((file->f_mode & O_RDWR) != O_RDWR)
    {
        printf("fmode:%o;RDWR:%o\n", file->f_mode, O_RDWR);
        printf("no write permision\n");
        return -1;
    }
    if (count + file->f_pos > super_block.s_max_size)
    {
        printf("write over the limition%ld\n", super_block.s_max_size);
        return -1;
    }
    struct d_inode *inode = file->f_inode;
    size_t filePos = file->f_pos;
    size_t diskPos;
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

        diskPos = my_bmap(fd, inode, filePos);
        // count大于本块剩余空间
        if (count >= BLOCKSIZE - diskPos % BLOCKSIZE)
        {
            thisTimeWrite = BLOCKSIZE - diskPos % BLOCKSIZE;
        }
        else
        {
            thisTimeWrite = count;
        }
        my_write(fd, diskPos, SEEK_SET, (char *)buf + writedBytes, thisTimeWrite);

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
    if (file->f_count == (unsigned short)(-1))
    {
        printf("fd:%d was closed\n",my_fd);
        return 0;
    }
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
    fileTable[my_fd].f_count = (unsigned short)(-1);
}
int sys_creat(int fd, char *path, mode_t modes)
{
    char *dirPath = (char *)malloc(strlen(path) + 1);
    char *fileName;
    struct d_inode *fileInode = NULL;
    unsigned short fileInodeCnt;
    unsigned short dirInodeCnt;
    int i;

    for (i = strlen(path); i >= 0; --i)
    {
        if (path[i] == '/')
        {
            fileName = malloc(strlen(path) - i);
            memcpy(fileName, &path[i + 1], strlen(path) - i);
            memcpy(dirPath, path, i + 1);
            // dirPath[i + 1] = '\0';
            break;
        }
    }

    if (strlen(fileName) >= 14)
    {
        printf("file name is too long\n");
        free(dirPath);
        return -1;
    }

    fileInodeCnt = my_namei(fd, path);
    if (fileInodeCnt != (unsigned short)(-1))
    {        
        fileInode = my_iget(fd, fileInodeCnt);
    }
    else
    {
        dirInodeCnt = my_namei(fd, dirPath);

        struct d_inode *dirInode = my_iget(fd, dirInodeCnt);
        struct dir dir;

        my_read(fd, dirInode->i_zone[0] * BLOCKSIZE + BLOCKPOS, SEEK_SET, &dir, sizeof(struct dir));

        unsigned short fileInodeCnt = my_ialloc(fd);

        fileInode = my_iget(fd, fileInodeCnt);

        fileInode->i_uid = 0;
        fileInode->i_gid = 0;
        fileInode->i_nlinks = 1;
        fileInode->i_size = 0;
        fileInode->i_zone[0] = my_alloc(fd);
        fileInode->i_cnt = fileInodeCnt;
        fileInode->i_mode = modes;

        for (int i = 2; i < BLOCKSIZE / (sizeof(struct dir_item)); ++i)
        {
            if (dir.item[i].inode_cnt == (unsigned short)(-1))
            {
                dir.item[i].inode_cnt = fileInode->i_cnt;
                strcpy(dir.item[i].name, fileName);
                //dir.item[i + 1].inode_cnt = (unsigned short)(-1);
                my_write(fd, dirInode->i_zone[0] * BLOCKSIZE + BLOCKPOS, SEEK_SET, &dir, sizeof(struct dir));
                // super_block.s_ninodes++;
                // super_block.s_nzones++;
                my_write(fd, SUPERBPOS, SEEK_SET, &super_block, sizeof(struct d_super_block));
                break;
            }
        }
    }

    int my_fd = fileTableCount;
    printf("fileTableCount%d\n",fileTableCount);
    fileTable[my_fd].f_inode = fileInode;
    fileTable[my_fd].f_mode = modes;
    fileTable[my_fd].f_pos = 0;
    fileTable[my_fd].f_count = 1;
    for (int i = 0; i < 100; ++i)
    {
        if (fileTable[i].f_inode==NULL)
        {
            fileTableCount = i;
            break;
        }
    }
    // my_iput(fd, fileInode);
    free(dirPath);
    return my_fd;
}
void sys_unlink(int fd, char *filePath)
{
    struct d_inode *dirInode;
    struct d_inode *fileInode;
    unsigned short dirInodeCnt;
    unsigned short fileInodeCnt;
    struct dir dir;
    char *dirPath = (char *)malloc(strlen(filePath) + 1);
    char *fileName;
    char *delim = "/";
    int i;
    for (i = strlen(filePath); i >= 0; --i)
    {
        if (filePath[i] == '/')
        {
            fileName = malloc(strlen(filePath) - i);
            memcpy(fileName, &filePath[i + 1], strlen(filePath) - i);
            memcpy(dirPath, filePath, i + 1);
            // dirPath[i + 1] = '\0';
            break;
        }
    }
    // strcpy(temp, path);
    // temp = trim(temp);
    // for (int i = strlen(temp); i >= 0; i--)
    // {
    //     if (temp[i] == '/')
    //     {
    //         fileName = &temp[i + 1];
    //         temp[i] = 0;
    //         break;
    //     }
    // }

    dirInodeCnt = my_namei(fd, dirPath);
    dirInode = my_iget(fd, dirInodeCnt);
    fileInodeCnt = my_namei(fd, filePath);
    fileInode = my_iget(fd, fileInodeCnt);
    // 目录则递归删除
    if (((fileInode->i_mode & O_DIRECTORY) & O_DIRECTORY) == O_DIRECTORY)
    {
        for (int i = 0; i < BLOCKSIZE / sizeof(struct dir_item); ++i)
        {
            if (dir.item[i].inode_cnt = (unsigned short)(-1))
            {
                continue;
            }
            my_read(fd, fileInode->i_zone[0] * BLOCKSIZE + BLOCKPOS, SEEK_SET, &dir, BLOCKSIZE);
            filePath = (char *)realloc(filePath, sizeof(filePath) + 14 * sizeof(char));
            strcat(filePath, dir.item[i].name);
            sys_unlink(fd, filePath);
        }
    }
    else
    {
        my_read(fd, dirInode->i_zone[0] * BLOCKSIZE + BLOCKPOS, SEEK_SET, &dir, BLOCKSIZE);
        for (int i = 0; i < BLOCKSIZE / sizeof(struct dir_item); ++i)
        {
            unsigned short cnt = dir.item[i].inode_cnt;
            if (i == (BLOCKSIZE / sizeof(struct dir_item) - 1))
            {
                printf("no dir or file name is %s", fileName);
                free(dirPath);
                return;
            }
            else if (!strcmp(dir.item[i].name, fileName))
            {
                dir.item[i].inode_cnt = (unsigned short)(-1);
                memset(dir.item[i].name, 0, sizeof(char) * 14);
                my_write(fd,BLOCKPOS+BLOCKSIZE*(dirInode->i_zone[0]),SEEK_SET,&dir,sizeof(struct dir));

                my_iput(fd, dirInode);
                free_all_blocks(fd, fileInode);
                my_ifree(fd, fileInode);

                my_write(fd,SUPERBPOS,SEEK_SET,&super_block,sizeof(struct d_super_block));
                break;
            }
        }
    }

    free(dirPath);
    free(fileName);
    return;
}