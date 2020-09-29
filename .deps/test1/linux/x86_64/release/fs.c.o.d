{
    depfiles_gcc = ".objs/test1/linux/x86_64/release/fs.c.o: fs.c fs.h\
",
    files = {
        "fs.c"
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