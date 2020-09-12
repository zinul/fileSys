#include "list.h"
#include "fs.h"
#include <stdio.h>
int main(){
    struct d_inode inode;
    printf("%ld，%ld\n",
        sizeof(inode),sizeof(inode.i_gid));
}