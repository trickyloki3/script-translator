OBJECT+=utility.o
OBJECT+=array.o
OBJECT+=pool.o
OBJECT+=map.o
OBJECT+=list.o

all: clean pj59

pj59: $(OBJECT)
	$(CC) $(CFLAGS) -o $@ pj59.c $^ $(LDFLAGS) $(LDLIBS)

%.c: %.y
	bison $^

%.c: %.l
	flex $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

.PHONY: all clean

clean:
	@rm -f *.o
	@rm -f pj59
