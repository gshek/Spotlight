SUBDIRS := $(wildcard */.)

.PHONY: $(SUBDIRS)
$(SUBDIRS):
	@for dir in $(SUBDIRS); do $(MAKE) -C $$dir; done

.PHONY: clean
clean:
	rm -f *.a *.o */*.o
