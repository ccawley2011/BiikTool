CC = wcc386
LD = wlink
RM = rm -f

system = nt
CFLAGS += -bt=$(system) -zq -oaxt -d0 -wx -fo=.obj
LDFLAGS += system $(system)

OBJS = main.obj debug.obj archive.obj extract.obj compress.obj
EXECUTABLE = biik-tool.exe

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJS)
	$(LD) $(LDFLAGS) name $@ file { $< }

.c.obj:
	$(CC) $(CFLAGS) $[*

clean: .symbolic
	$(RM) $(EXECUTABLE)
	$(RM) $(OBJS)
	$(RM) *.err
