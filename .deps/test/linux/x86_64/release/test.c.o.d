{
    depfiles_gcc = ".objs/test/linux/x86_64/release/test.c.o: test.c fs.h\
",
    files = {
        "test.c"
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