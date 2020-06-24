SRCDIR = ../src
SPOTLIGHT_BINARY = $(SRCDIR)/spotlight
COMPRESSION_READER_BINARY = $(SRCDIR)/compression-reader

BINARIES = $(SPOTLIGHT_BINARY) $(COMPRESSION_READER_BINARY)

define NOTICE

 ###############################################################################
 #################################### TESTS ####################################
 ###############################################################################

endef

.PHONY: test
test: $(BINARIES)
	$(info $(NOTICE))
	@for D in test-*/*/; do\
		echo "";\
		echo "***** $$D *****";\
		for T in $$D[0-9]*-*.sh; do\
			SKIP_SLOW_TESTS=1 ./$$T;\
		done; done

.PHONY: test-all
test-all: $(BINARIES)
	$(info $(NOTICE))
	@for D in test-*/*/; do\
		echo "";\
		echo "***** $$D *****";\
		for T in $$D[0-9]*-*.sh; do\
			./$$T;\
		done; done

test-%: $(BINARIES)
	$(info $(NOTICE))
	@for D in $@/*/; do\
		echo "";\
		echo "***** $$D *****";\
		for T in $$D[0-9]*-*.sh; do\
			./$$T;\
		done; done

.PHONY: build
build:
	$(MAKE) -C $(SRCDIR)
	cp -f $(BINARIES) .

$(SPOTLIGHT_BINARY): build
$(COMPRESSION_READER_BINARY): build

.PHONY: clean
clean:
	rm -rf $(BINARIES) logs/
