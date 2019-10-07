CC = gcc
LD = $(CC)

CFLAGS += -std=gnu89 -O3
CPPFLAGS += -Wall -Wextra -pedantic

OBJS = main.o archive.o
EXECUTABLE = biik-tool

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(LD) $(LDFLAGS) $(TARGET_ARCH) -o $@ $^

clean:
	$(RM) $(EXECUTABLE)
	$(RM) $(OBJS)
