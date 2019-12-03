OBJECT+=utility.o
OBJECT+=pool.o
OBJECT+=list.o
OBJECT+=map.o
OBJECT+=range.o
OBJECT+=array.o
OBJECT+=heap.o
OBJECT+=csv_parser.o
OBJECT+=csv_scanner.o
OBJECT+=csv.o
OBJECT+=json_parser.o
OBJECT+=json_scanner.o
OBJECT+=json.o
# OBJECT+=yaml_parser.o
# OBJECT+=yaml_scanner.o
# OBJECT+=yaml.o
# OBJECT+=db.o
# OBJECT+=data.o
# OBJECT+=logic.o

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
	@rm -f json_parser.c
	@rm -f json_parser.h
	@rm -f json_parser.output
	@rm -f json_scanner.c
	@rm -f json_scanner.h
	@rm -f yaml_parser.c
	@rm -f yaml_parser.h
	@rm -f yaml_parser.output
	@rm -f yaml_scanner.c
	@rm -f yaml_scanner.h
	@rm -f pj59
