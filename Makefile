include config.mk
export

.PHONY: all build run clean debug

all: clean build

# src

$(ODIR):
	mkdir $@

build: $(ODIR)
build:
	cd $(SDIR) && $(MAKE) -f Makefile.src

debug: DEBUG = 1
debug: build


# misc

clean:
	rm -rf $(ODIR)/*
