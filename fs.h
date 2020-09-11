#include <sys/types.h>
#include <list.h>

struct d_inode
{
  unsigned short i_mode;	// 文件类型和属性(rwx 位)。
  unsigned short i_uid;		// 用户id（文件拥有者标识符）。
  unsigned long i_size;		// 文件大小（字节数）。
  unsigned long i_time;		// 修改时间（自1970.1.1:0 算起，秒）。
  unsigned char i_gid;		// 组id(文件拥有者所在的组)。
  unsigned char i_nlinks;	// 链接数（多少个文件目录项指向该i 节点）。
  unsigned short i_zone[9];	// 直接(0-6)、间接(7)或双重间接(8)逻辑块号。
// zone 是区的意思，可译成区段，或逻辑块。
};

// 这是在内存中的i 节点结构。前7 项与d_inode 完全一样。
struct m_inode
{
  unsigned short i_mode;	// 文件类型和属性(rwx 位)。
  unsigned short i_uid;		// 用户id（文件拥有者标识符）。
  unsigned long i_size;		// 文件大小（字节数）。
  unsigned long i_mtime;	// 修改时间（自1970.1.1:0 算起，秒）。
  unsigned char i_gid;		// 组id(文件拥有者所在的组)。
  unsigned char i_nlinks;	// 文件目录项链接数。
  unsigned short i_zone[9];	// 直接(0-6)、间接(7)或双重间接(8)逻辑块号。
/* these are in memory also */
//  struct task_struct *i_wait;	// 等待该i 节点的进程。
  unsigned long i_atime;	// 最后访问时间。
  unsigned long i_ctime;	// i 节点自身修改时间。
//   unsigned short i_dev;		// i 节点所在的设备号。
  unsigned short i_num;		// i 节点号。
//  unsigned short i_count;	// i 节点被使用的次数，0 表示该i 节点空闲。
//  unsigned char i_lock;		// 锁定标志。
  unsigned char i_dirt;		// 已修改(脏)标志。
  unsigned char i_pipe;		// 管道标志。
  unsigned char i_mount;	// 安装标志。
//  unsigned char i_seek;		// 搜寻标志(lseek 时)。
//  unsigned char i_update;	// 更新标志。
};

// 文件结构（用于在文件句柄与i 节点之间建立关系）
struct file
{
  unsigned short f_mode;	// 文件操作模式（RW 位）
  unsigned short f_flags;	// 文件打开和控制的标志。
  unsigned short f_count;	// 对应文件句柄（文件描述符）数。
  struct m_inode *f_inode;	// 指向对应i 节点。
  off_t f_pos;			// 文件位置（读写偏移值）。
};

// 内存中磁盘超级块结构。
struct super_block
{
  unsigned short s_ninodes;	// 节点数。
  unsigned short s_nzones;	// 逻辑块数。
  unsigned short s_imap_blocks;	// i 节点位图所占用的数据块数。
  unsigned short s_zmap_blocks;	// 逻辑块位图所占用的数据块数。
  unsigned short s_firstdatazone;	// 第一个数据逻辑块号。
  //unsigned short s_log_zone_size;	// log(数据块数/逻辑块)。（以2 为底）。
  unsigned long s_max_size;	// 文件最大长度。
  //unsigned short s_magic;	// 文件系统魔数。

/* These are only in memory */
//   struct buffer_head *s_imap[8];	// i 节点位图缓冲块指针数组(占用8 块，可表示64M)。
//   struct buffer_head *s_zmap[8];	// 逻辑块位图缓冲块指针数组（占用8 块）。
//   unsigned short s_dev;		// 超级块所在的设备号。
  struct m_inode *s_isup;	// 被安装的文件系统根目录的i 节点。(isup-super i)
  struct m_inode *s_imount;	// 被安装到的i 节点。
  unsigned long s_time;		// 修改时间。
//  struct task_struct *s_wait;	// 等待该超级块的进程。
  unsigned char s_lock;		// 被锁定标志。
  unsigned char s_rd_only;	// 只读标志。
  unsigned char s_dirt;		// 已修改(脏)标志。
};

// 磁盘上超级块结构。上面125-132 行完全一样。
struct d_super_block
{
  unsigned short s_ninodes;	// 节点数。
  unsigned short s_nzones;	// 逻辑块数。
  unsigned short s_imap_blocks;	// i 节点位图所占用的数据块数。
  unsigned short s_zmap_blocks;	// 逻辑块位图所占用的数据块数。
  unsigned short s_firstdatazone;	// 第一个数据逻辑块。
//  unsigned short s_log_zone_size;	// log(数据块数/逻辑块)。（以2 为底）。
  unsigned long s_max_size;	// 文件最大长度。
//  unsigned short s_magic;	// 文件系统魔数。
};


//// 以下是文件系统操作管理用的函数原型。
// 将i 节点指定的文件截为0。
extern void truncate (struct m_inode *inode);
// 逻辑块(区段，磁盘块)位图操作。取数据块block 在设备上对应的逻辑块号。
extern int bmap (struct m_inode *inode, int block);
// 创建数据块block 在设备上对应的逻辑块，并返回在设备上的逻辑块号。
extern int create_block (struct m_inode *inode, int block);
// 获取指定路径名的i 节点号。
extern struct m_inode *namei (const char *pathname);
// 根据路径名为打开文件操作作准备。
extern int open_namei (const char *pathname, int flag, int mode,
		       struct m_inode **res_inode);
// 释放一个i 节点(回写入设备)。
extern void iput (struct m_inode *inode);
// 从设备读取指定节点号的一个i 节点。
extern struct m_inode *iget (int dev, int nr);
// 从i 节点表(inode_table)中获取一个空闲i 节点项。
extern struct m_inode *get_empty_inode (void);
// 获取（申请一）管道节点。返回为i 节点指针（如果是NULL 则失败）。
extern struct m_inode *get_pipe_inode (void);
// 在哈希表中查找指定的数据块。返回找到块的缓冲头指针。
extern struct buffer_head *get_hash_table (int dev, int block);
// 从设备读取指定块（首先会在hash 表中查找）。
extern struct buffer_head *getblk (int dev, int block);
// 读/写数据块。
extern void ll_rw_block (int rw, struct buffer_head *bh);
// 释放指定缓冲块。
extern void brelse (struct buffer_head *buf);
// 读取指定的数据块。
extern struct buffer_head *bread (int dev, int block);
// 读4 块缓冲区到指定地址的内存中。
extern void bread_page (unsigned long addr, int dev, int b[4]);
// 读取头一个指定的数据块，并标记后续将要读的块。
extern struct buffer_head *breada (int dev, int block, ...);
// 向设备dev 申请一个磁盘块（区段，逻辑块）。返回逻辑块号
extern int new_block (int dev);
// 释放设备数据区中的逻辑块(区段，磁盘块)block。复位指定逻辑块block 的逻辑块位图比特位。
extern void free_block (int dev, int block);
// 为设备dev 建立一个新i 节点，返回i 节点号。
extern struct m_inode *new_inode (int dev);
// 释放一个i 节点（删除文件时）。
extern void free_inode (struct m_inode *inode);
// 刷新指定设备缓冲区。
extern int sync_dev (int dev);
// 读取指定设备的超级块。
extern struct super_block *get_super (int dev);
extern int ROOT_DEV;

// 安装根文件系统。
extern void mount_root (void);