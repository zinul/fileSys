{
    depfiles_gcc = ".objs/format/linux/x86_64/release/format.c.o: format.c fs.h\
",
    files = {
        "format.c"
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