# Matrix Calculator

CC      ?= cc
CFLAGS  ?= -std=c11 -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -O2
LDLIBS  ?= -lm
TARGET   = puc
SRC      = PUC.c

.PHONY: all run test debug clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

# Build with debug symbols and the address/UB sanitizers.
debug: CFLAGS += -g -O0 -fsanitize=address,undefined
debug: clean $(TARGET)

test: $(TARGET)
	./tests/run_tests.sh ./$(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
