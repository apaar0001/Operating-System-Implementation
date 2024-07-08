all: bin/loader bin/launcher bin/test

bin/loader:
	$(MAKE) -C loader

bin/launcher:
	$(MAKE) -C launcher

bin/test:
	$(MAKE) -C test
clean:
	$(MAKE) -C loader clean
	$(MAKE) -C launcher clean
	$(MAKE) -C test clean
