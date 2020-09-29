{
    depfiles_gcc = ".objs/test1/linux/x86_64/release/block.c.o: block.c fs.h\
",
    files = {
        "block.c"
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