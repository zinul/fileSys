{
    files = {
        "./.objs/test1/linux/x86_64/release/test1.c.o",
        "./.objs/test1/linux/x86_64/release/block.c.o",
        "./.objs/test1/linux/x86_64/release/sys_call.c.o",
        "./.objs/test1/linux/x86_64/release/inode.c.o",
        "./.objs/test1/linux/x86_64/release/fs.c.o"
    },
    values = {
        "/usr/bin/g++",
        {
            "-L/usr/local/lib",
            "-L/usr/lib",
            "-m64"
        }
    }
}