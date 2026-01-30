CC = $(shell cat project.json | jq .compiler_path)

CCFLAGS  = -std=gnu++2b -Wall -Wpedantic -Wextra
CCFLAGS += -Wno-gnu-zero-variadic-macro-arguments
CCFLAGS += $(INCFLAGS)

INCFLAGS  = -I$(shell cat project.json | jq .inc_dir)

EXT = $(shell cat project.json | jq .ext)
SRC = $(shell find $(shell cat project.json | jq .src_dir) -name "*$(EXT)")
OBJ = $(SRC:.cpp=.o)

BIN = bin/$(subst ",,$(shell cat project.json | jq .name))
DUMP_FILE = build_dump.txt

.PHONY: all clean run

all: $(BIN)

$(BIN): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(OBJ) $(BIN) $(DUMP_FILE)

