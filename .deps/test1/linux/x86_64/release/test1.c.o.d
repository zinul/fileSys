{
    depfiles_gcc = ".objs/test1/linux/x86_64/release/test1.c.o: test1.c fs.h\
",
    files = {
        "test1.c"
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