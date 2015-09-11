CFLAGS = $(shell pkg-config fuse --cflags) 
LDFLAGS = $(shell pkg-config fuse --libs)
CPPFLAGS = -I$(top_srcdir)/include -D_FILE_OFFSET_BITS=64 -D_REENTRANT
mfs: mfs.o
	gcc -o $@ mfs.o $(LDFLAGS)

.c.o:
	gcc $(CFALGS) -c $< $(CPPFLAGS)
