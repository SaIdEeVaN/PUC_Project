# Matrix Calculator

CC      ?= cc
CFLAGS  ?= -std=c11 -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -O2
TARGET   = puc
SRC      = PUC.c

.PHONY: all run debug clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

# Build with debug symbols and the address/UB sanitizers.
debug: CFLAGS += -g -O0 -fsanitize=address,undefined
debug: clean $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
