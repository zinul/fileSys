TARGET=test

C_FILES=$(shell ls *.c)
BASE=$(basename $(C_FILES))
OBJS=$(addsuffix .o ,$(BASE))

$(TARGET):$(OBJS)
	-rm -f $@
	gcc -o $(TARGET) $(OBJS)

%.o:%.c
	gcc -c -o $@ $<

clean:
	-rm -f test
	-rm -f *.o