{
    depfiles_gcc = ".objs/test/linux/x86_64/release/sys_call.c.o: sys_call.c fs.h\
",
    files = {
        "sys_call.c"
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