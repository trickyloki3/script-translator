OBJECT+=utility.o
OBJECT+=array.o
OBJECT+=pool.o
OBJECT+=map.o
OBJECT+=list.o
OBJECT+=sector.o
OBJECT+=pool_map.o
OBJECT+=csv_parser.o
OBJECT+=csv_scanner.o
OBJECT+=csv.o

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
	@rm -f csv_parser.c
	@rm -f csv_parser.h
	@rm -f csv_parser.output
	@rm -f csv_scanner.c
	@rm -f csv_scanner.h
	@rm -f lex.backup
	@rm -f pj59
