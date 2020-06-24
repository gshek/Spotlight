###############################################################################
# Requirements
###############################################################################
ifeq ($(shell which pkg-config), )
	$(error Install pkg-config to compile)
endif

###############################################################################
# Compile and linking flags
###############################################################################
CFLAGS  += -std=gnu99 -Wall -Wextra
LDFLAGS += $(shell pkg-config fuse liblzma --libs)

ifeq ($(shell uname), FreeBSD)
	LDFLAGS += -lexecinfo
endif

###############################################################################
# Binary name
###############################################################################
BINARY = spotlight
COMPRESSION_READER_BINARY = compression-reader

###############################################################################
# Directories and sources
###############################################################################
COMPDIR = compression
FSDIR = fs
LIBDIR = libs

SOURCES += $(wildcard *.c)
COMPSOURCES += $(shell find $(COMPDIR) -name "*.c")
FSSOURCES += $(shell find $(FSDIR) -maxdepth 1 -name "*.c") $(FSDIR)/*.a
LIBSOURCES += $(LIBDIR)/*.a

###############################################################################
# Targets
###############################################################################
.PHONY: all
all: $(BINARY) $(COMPRESSION_READER_BINARY)

.PHONY: debug
debug: CFLAGS += -g3 -g -Og
debug: $(SOURCES) $(COMPSOURCES) $(FSSOURCES) $(LIBSOURCES)
	$(CC) $(CFLAGS) -o $(BINARY) $^ $(LDFLAGS)

$(BINARY): CFLAGS += -DNDEBUG -O3
$(BINARY): $(SOURCES) $(COMPSOURCES) $(FSSOURCES) $(LIBSOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(FSDIR)/%:
	$(MAKE) -C $(FSDIR)

$(LIBDIR)/%:
	$(MAKE) -C $(LIBDIR)

$(COMPRESSION_READER_BINARY): CFLAGS += -g3 -g -Og
$(COMPRESSION_READER_BINARY): examples/$(COMPRESSION_READER_BINARY).c $(SOURCES) $(COMPSOURCES) $(LIBSOURCES)
	$(CC) $(CFLAGS) -o $@ $(filter-out main.c, $^) $(LDFLAGS)

.PHONY: test
test:
	$(MAKE) -C ../tests

.PHONY: clean
clean:
	rm -f $(BINARY)

.PHONY: clean-all
clean-all: clean
	$(MAKE) -C $(LIBDIR) clean
	$(MAKE) -C $(FSDIR) clean
