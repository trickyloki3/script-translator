OBJECT+=panic.o
OBJECT+=slab.o
OBJECT+=array.o
OBJECT+=stack.o
OBJECT+=map.o
OBJECT+=meta.o
OBJECT+=range.o
OBJECT+=logic.o
OBJECT+=store.o
OBJECT+=heap.o
OBJECT+=csv_scanner.o
OBJECT+=csv.o
OBJECT+=yaml_scanner.o
OBJECT+=yaml.o
OBJECT+=yaml_reader.o
OBJECT+=parser.o
OBJECT+=table.o
OBJECT+=script_parser.o
OBJECT+=script_scanner.o
OBJECT+=script.o
LDLIBS+=-lm

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
	@rm -f csv_scanner.c
	@rm -f csv_scanner.h
	@rm -f lex.backup
	@rm -f yaml_scanner.c
	@rm -f yaml_scanner.h
	@rm -f script_parser.c
	@rm -f script_parser.h
	@rm -f script_parser.output
	@rm -f script_scanner.c
	@rm -f script_scanner.h
	@rm -f pj59
