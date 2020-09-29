{
    files = {
        "./.objs/test/linux/x86_64/release/test.c.o",
        "./.objs/test/linux/x86_64/release/block.c.o",
        "./.objs/test/linux/x86_64/release/sys_call.c.o",
        "./.objs/test/linux/x86_64/release/inode.c.o",
        "./.objs/test/linux/x86_64/release/fs.c.o"
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