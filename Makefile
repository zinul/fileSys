OUTPUT1=test
OUTPUT2=format
OUTPUTLIST=$(OUTPUT1) $(OUTPUT2)

C_FILES=$(shell ls *.c)
BASE=$(basename $(C_FILES))
OBJS=$(addsuffix .o ,$(BASE))
TEMPBASE=$(BASE)
BASE:=$(filter-out $(OUTPUTLIST),$(TEMPBASE))

$(OUTPUT1):$(OBJS)
	-rm -f  $(OUTPUTLIST)
	gcc -c $(OUTPUT1).c  -D _$(shell echo $(@)|tr a-z A-Z)_
	#OBJS=$(filter-out format.o,$(TEMPOBJS))
	gcc $^ $(OUTPUT1).o -o $@ 
	./$(OUTPUT1) bigFile
$(OUTPUT2):$(OBJS)
	-rm -f  $(OUTPUTLIST)
	gcc -c $(OUTPUT2).c  -D _$(shell echo $(@)|tr a-z A-Z)_
	#OBJS=$(filter-out format.o,$(TEMPOBJS))
	gcc $^ $(OUTPUT2).o -o $@
	./$(OUTPUT2) bigFile 102400
%.o:%.c
	gcc  -c $<

clean:
	-rm -f $(OUTPUTLIST)
	-rm -f *.o