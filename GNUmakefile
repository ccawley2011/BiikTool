CC = gcc
LD = $(CC)

CFLAGS += -std=gnu89 -O3
CPPFLAGS += -Wall -Wextra -pedantic

OBJS = main.o debug.o archive.o extract.o compress.o musx.o adpcm.o g711.o
EXECUTABLE = biik-tool

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(LD) $(LDFLAGS) $(TARGET_ARCH) -o $@ $^

clean:
	$(RM) $(EXECUTABLE)
	$(RM) $(OBJS)
