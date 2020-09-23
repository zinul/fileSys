#include "fs.h"



int printDir(int fd,struct dir dir)
{
    int i=0;
    for(int i=0;i<10;i++)
    {
        if(dir.item[i].inode_cnt==0xFFFF)
        {
            break;
        }
        printf("name:%s inode_cnt:%d\n",dir.item[i].name,dir.item[i].inode_cnt);
    }
    return 0;
}
void psuper_block(int fd);
struct d_super_block super_block;
#ifdef _TEST_
int main(int argc,char *argv[])
{
    //super_block=(struct d_super_block *)malloc(sizeof(struct d_super_block));
    int fd;
    char buf[BLOCKSIZE];
    struct d_inode *inode;
    struct dir dir;
    // =(struct dir*)malloc(sizeof(struct dir));
    
    if ((fd = open(argv[1], O_RDWR)) == -1)
    {
        printf("open error\n");
        return -1;
    }
    my_read(fd,0,SEEK_SET,&super_block,sizeof(struct d_super_block)); 
    // for(int i=0;i<100;i++)
    // {
    //     my_ialloc(fd);
    //     //printf("alloc block%d\n",my_alloc(fd));
    // }
    my_read(fd,IBITMAPOS,SEEK_SET,buf,BLOCKSIZE);
    for(int i=0;i<10;i++){
        printf("%d:%d@\n",i,buf[i]);
       // fflush(fd);
    } 

    // for(int i=0;i<100;i++)
    // {
    //     my_ifree(fd,i);
    //     //printf("free block%d\n",my_free(fd,i));
    // }
    my_read(fd,BBITMAPOS,SEEK_SET,buf,BLOCKSIZE);
    for(int i=0;i<10;i++){
        printf("%d:%d@\n",i,buf[i]);
       // fflush(fd);
    } 
    psuper_block(fd);
    
    //  dir=getDir(fd,inode);

    inode=my_iget(fd,0);
    my_read(fd,inode->i_zone[0]*BLOCKSIZE+BLOCKPOS,SEEK_SET,&dir,sizeof(struct dir));
    printDir(fd,dir);
}
#endif
struct dir *getDir(int fd,struct d_inode *inode)
{
    //struct d_inode *lnode = (struct d_inode *)malloc(sizeof(struct d_inode));
    struct dir *dir;
    my_read(fd,inode->i_zone[0]*BLOCKSIZE+BLOCKPOS,SEEK_SET,dir,sizeof(struct dir));
    return dir;
}
void psuper_block(int fd)
{
    printf("block use %d\n",super_block.s_nzones);
    printf("inode use %d\n",super_block.s_ninodes);
    printf("first block%d\n",super_block.s_firstdatazone);
    printf("remembernode %d\n",super_block.s_rember_node);
    printf("file max size %ld\n",super_block.s_max_size);
}