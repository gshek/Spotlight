ifeq ($(shell which pkg-config), )
	$(error You need to install pkg-config in order to compile this sources)
endif

VERSION  = $(shell git describe --tags 2> /dev/null || basename `pwd`)

CFLAGS  += $(shell pkg-config fuse --cflags) -DFUSE_USE_VERSION=26 -std=gnu99 -g3 -Wall -Wextra
CFLAGS  += -DEXT4FUSE_VERSION=\"$(VERSION)\"
CFLAGS  += -c -O3
#CFLAGS 	+= -DNDEBUG

ifeq ($(shell uname), Darwin)
	CFLAGS  += -mmacosx-version-min=10.5

	# fuse.pc pulls this flag in for me, but it seems that some old versions don't
	CFLAGS  += -D_FILE_OFFSET_BITS=64
endif

ifeq ($(shell uname), FreeBSD)
	CFLAGS  += -I/usr/local/include -L/usr/local/lib
endif

BINARY = ext4fuse.a
SOURCES += fuse-main.o logging.o extents.o disk.o super.o inode.o dcache.o
SOURCES += op_read.o op_readdir.o op_readlink.o op_init.o op_getattr.o op_open.o

$(BINARY): $(SOURCES)
	ar rcs $@ $^
	mv $@ ../

clean:
	rm -f *.o
