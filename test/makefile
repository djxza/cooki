CC = $(shell cat project.json | jq .compiler_path)

CCFLAGS  = -std=gnu++2b -Wall -Wpedantic -Wextra
CCFLAGS += -Wno-gnu-zero-variadic-macro-arguments

SRC = $(shell find src -name "*.cpp")
OBJ = $(SRC:.cpp=.o)
HDR = $(shell find src -name "*.h") $(shell find src -name "*.hpp")
ALL_FILES = $(SRC) $(HDR)

BIN = bin/$(subst ",,$(shell cat project.json | jq .name))
DUMP_FILE = build_dump.txt

.PHONY: all clean run dump dump-detailed dump-quick dump-info dump-vars dump-clean

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

# Main dump function - like your bash script
dump:
	@echo "Project dump:" | tee $(DUMP_FILE)
	@echo "Generating $(DUMP_FILE) ..." | tee -a $(DUMP_FILE)
	@echo "" | tee -a $(DUMP_FILE)

	@echo "===== TREE STRUCTURE =====" | tee -a $(DUMP_FILE)
	@echo "" | tee -a $(DUMP_FILE)
	@{ tree -a 2>/dev/null || find . -type d | sed 's|[^/]*/|- |g'; } | tee -a $(DUMP_FILE)
	@echo "" | tee -a $(DUMP_FILE)

	@echo "===== ALL SOURCE FILES =====" | tee -a $(DUMP_FILE)
	@echo "" | tee -a $(DUMP_FILE)
	@SRC_FILES=$$(find src -type f -name "*.cpp" 2>/dev/null); \
	if [ -n "$$SRC_FILES" ]; then \
		for file in $$SRC_FILES; do \
			echo "--- $$file ---" | tee -a $(DUMP_FILE); \
			cat "$$file" | tee -a $(DUMP_FILE); \
			echo "" | tee -a $(DUMP_FILE); \
		done; \
	else \
		echo "No source files found" | tee -a $(DUMP_FILE); \
	fi

	@echo "===== ALL HEADER FILES =====" | tee -a $(DUMP_FILE)
	@echo "" | tee -a $(DUMP_FILE)
	@HDR_FILES=$$(find src -type f \( -name "*.h" -o -name "*.hpp" \) 2>/dev/null); \
	if [ -n "$$HDR_FILES" ]; then \
		for file in $$HDR_FILES; do \
			echo "--- $$file ---" | tee -a $(DUMP_FILE); \
			cat "$$file" | tee -a $(DUMP_FILE); \
			echo "" | tee -a $(DUMP_FILE); \
		done; \
	else \
		echo "No header files found" | tee -a $(DUMP_FILE); \
	fi

	@echo "===== BUILD DUMP COMPLETE =====" | tee -a $(DUMP_FILE)

# Quick dump - just show the most important info
dump-quick:
	@echo "===== QUICK PROJECT DUMP =====" | tee $(DUMP_FILE)
	@echo "Generated on $$(date)" | tee -a $(DUMP_FILE)
	@echo "" | tee -a $(DUMP_FILE)

	@echo "=== File Structure ===" | tee -a $(DUMP_FILE)
	@echo "" | tee -a $(DUMP_FILE)
	@find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) | sort | while read file; do \
		lines=$$(wc -l < "$$file" 2>/dev/null || echo "0"); \
		echo "$$file ($$lines lines)" | tee -a $(DUMP_FILE); \
	done
	@echo "" | tee -a $(DUMP_FILE)

	@echo "=== Makefile (first 50 lines) ===" | tee -a $(DUMP_FILE)
	@echo "" | tee -a $(DUMP_FILE)
	@if [ -f Makefile ] || [ -f makefile ]; then \
		cat Makefile makefile 2>/dev/null | head -n 50 | tee -a $(DUMP_FILE); \
	else \
		echo "No makefile found" | tee -a $(DUMP_FILE); \
	fi
	@echo "" | tee -a $(DUMP_FILE)

	@echo "=== Source Files (first 30 lines each) ===" | tee -a $(DUMP_FILE)
	@echo "" | tee -a $(DUMP_FILE)
	@for file in $(SRC); do \
		if [ -f "$$file" ]; then \
			echo "--- $$file ---" | tee -a $(DUMP_FILE); \
			head -n 30 "$$file" | tee -a $(DUMP_FILE); \
			echo "... [truncated]" | tee -a $(DUMP_FILE); \
			echo "" | tee -a $(DUMP_FILE); \
		fi; \
	done

	@echo "Dump saved to $(DUMP_FILE)"

# Header-only dump
dump-headers:
	@echo "===== HEADER FILES DUMP =====" | tee $(DUMP_FILE)
	@echo "" | tee -a $(DUMP_FILE)
	@for file in $(HDR); do \
		if [ -f "$$file" ]; then \
			echo "--- $$file ---" | tee -a $(DUMP_FILE); \
			cat "$$file" | tee -a $(DUMP_FILE); \
			echo "" | tee -a $(DUMP_FILE); \
			echo "--- END $$file ---" | tee -a $(DUMP_FILE); \
			echo "" | tee -a $(DUMP_FILE); \
		fi; \
	done
	@echo "Header dump complete: $(DUMP_FILE)"

# Show file stats without creating dump file
dump-info:
	@echo "=== PROJECT INFORMATION ==="
	@echo "Compiler: $(CC)"
	@echo "Flags: $(CCFLAGS)"
	@echo "Binary: $(BIN)"
	@echo ""
	@echo "=== FILE COUNTS ==="
	@echo "Total .cpp files: $$(find . -name "*.cpp" | wc -l)"
	@echo "Total .h/.hpp files: $$(find . -type f \( -name "*.h" -o -name "*.hpp" \) | wc -l)"
	@echo "Files in src/: $$(find src -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) | wc -l)"
	@echo ""
	@echo "=== SOURCE FILES ==="
	@for file in $(SRC); do \
		if [ -f "$$file" ]; then \
			lines=$$(wc -l < "$$file"); \
			echo "$$file ($$lines lines)"; \
		fi; \
	done
	@echo ""
	@echo "=== HEADER FILES ==="
	@for file in $(HDR); do \
		if [ -f "$$file" ]; then \
			lines=$$(wc -l < "$$file"); \
			echo "$$file ($$lines lines)"; \
		fi; \
	done

# Clean dump files only
dump-clean:
	@rm -f $(DUMP_FILE)
	@echo "Cleaned dump file: $(DUMP_FILE)"

