ZLIB_NG_LIBRARY = libz.a

$(ZLIB_NG_LIBRARY):
	@cd zlib-ng; ./configure --zlib-compat; make $@; cp $@ ../

.PHONY: clean
clean:
	rm -f $(ZLIB_NG_LIBRARY)
