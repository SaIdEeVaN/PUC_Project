# Matrix Calculator

CC       ?= cc
CFLAGS   ?= -std=c11 -Wall -Wextra -Wstrict-prototypes -Wmissing-prototypes -O2
CPPFLAGS ?= -Isrc
LDLIBS   ?= -lm

TARGET    = puc
SRCS      = $(wildcard src/*.c)
OBJS      = $(SRCS:.c=.o)
DEPS      = $(OBJS:.o=.d)

DEBUG_CFLAGS = -std=c11 -Wall -Wextra -Wstrict-prototypes \
               -Wmissing-prototypes -g -O0 -fsanitize=address,undefined

.PHONY: all run test debug clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDLIBS)

# -MMD -MP writes a .d file per object, so a header change rebuilds
# everything that includes it.
%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./tests/run_tests.sh ./$(TARGET)

# Recursive so the rebuild cannot race the clean under `make -j`.
debug:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(DEBUG_CFLAGS)" $(TARGET)

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS)
