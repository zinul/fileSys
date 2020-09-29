{
    depfiles_gcc = ".objs/fileSys/linux/x86_64/release/inode.c.o: inode.c fs.h\
",
    files = {
        "inode.c"
    },
    values = {
        "/usr/bin/gcc",
        {
            "-I/usr/local/include",
            "-I/usr/include",
            "-m64"
        }
    }
}