OBJECTS_cgi_template := cgi_template.o
OBJECTS_cgi_template_host := oscar/staging/lib/libosc_host.a
OBJECTS_cgi_template_target := oscar/staging/lib/libosc_target.a

PRODUCTS := $(patsubst OBJECTS_%, %, $(filter OBJECTS_%, $(.VARIABLES)))
PRODUCTS := $(filter-out $(addsuffix _host, $(PRODUCTS)) $(addsuffix _target, $(PRODUCTS)), $(PRODUCTS))

SUBDIRS := 

CLEAN_FILES := *.o *.gdb $(PRODUCTS) $(addsuffix _host, $(PRODUCTS)) $(addsuffix _target, $(PRODUCTS)) Makefile.deps www.tar.gz www/cgi-bin/template.cgi
CLEAN_FILES := $(wildcard $(CLEAN_FILES))

PEDANTIC := -Wall
OPTIMIZE := -O3

HOST_LIBS := -lm
TARGET_LIBS := -lbfdsp -lm

FLT_OPTS := -elf2flt="-s 10240"

CC_STD := -std=gnu99

CC_HOST := gcc -c -DOSC_HOST $(CC_STD) $(PEDANTIC) $(OPTIMIZE)
LD_HOST := gcc -fPIC $(HOST_LIBS)

CC_TARGET := bfin-uclinux-gcc -c -DOSC_TARGET $(CC_STD) $(PEDANTIC) $(OPTIMIZE)
LD_TARGET := bfin-uclinux-gcc $(FLT_OPTS) $(TARGET_LIBS)

SHELL := $(shell which bash)
MAKE += --no-print-directory

.PHONY: all host target
all: host target

.PHONY: install
install: cgi_template_host host
	@ echo "INSTALL  www => /var/www"
	@ cp $< www/cgi-bin/template.cgi
	@ cp -r www/* /var/www
	@ set -e; for i in $(SUBDIRS); do echo "MAKE     $@ => $$i/"; $(MAKE) -C $$i $@; done

www.tar.gz: cgi_template_target $(MAKEFILE_LIST) $(shell find www)
	@ echo "TAR_GZ   www => www.tar.gz"
	@ cp $< www/cgi-bin/template.cgi
	@ tar c -C www . | gzip > www.tar.gz

.PHONY: host
host: $(addsuffix _host, $(PRODUCTS))
	@ set -e; for i in $(SUBDIRS); do echo "MAKE     $@ => $$i/"; $(MAKE) -C $$i $@; done

.PHONY: target
target: $(addsuffix _target, $(PRODUCTS))
	@ set -e; for i in $(SUBDIRS); do echo "MAKE     $@ => $$i/"; $(MAKE) -C $$i $@; done

.SUFFIXES:
Makefile.deps: $(SOURCES) $(MAKEFILE_LIST)
	@ echo "DEPS     $?"
	@ for i in $(SOURCES); do cpp -MM $$i; done > $@
	@ echo "$@: $$(cut -d " " -f 2- < $@ | tr "\n" " ")" >> $@

oscar/staging/inc/* oscar/staging/lib/*:
	@ echo "MAKE     oscar"
	@ $(MAKE) -C oscar

%_host.o: %.c $(MAKEFILE_LIST) oscar/staging/inc/*
	@ echo "GCC      $*.c => $@"
	@ $(CC_HOST) -o $@ $*.c

%_target.o: %.c $(MAKEFILE_LIST) oscar/staging/inc/*
	@ echo "GCC_BF   $*.c => $@"
	@ $(CC_TARGET) -o $@ $*.c

.SECONDEXPANSION:
$(addsuffix _host, $(PRODUCTS)): INPUTS = $(patsubst %.o,%_host.o,$(OBJECTS_$(patsubst %_host,%,$@))) $(OBJECTS_$@)
$(addsuffix _host, $(PRODUCTS)): $$(INPUTS) $(MAKEFILE_LIST) oscar/staging/lib/*
	@ echo "LD       $(INPUTS) => $@"
	@ $(LD_HOST) $(INPUTS) -o $@ 

$(addsuffix _target, $(PRODUCTS)): INPUTS = $(patsubst %.o,%_target.o,$(OBJECTS_$(patsubst %_target,%,$@))) $(OBJECTS_$@)
$(addsuffix _target, $(PRODUCTS)): $$(INPUTS) $(MAKEFILE_LIST) oscar/staging/lib/*
	@ echo "LD_BF    $(INPUTS) => $@"
	@ $(LD_TARGET) $(INPUTS) -o $@

.PHONY: clean
clean:
ifneq '$(CLEAN_FILES)' ''
	@ echo "CLEAN   " $(CLEAN_FILES)
	@ rm -f $(CLEAN_FILES)
endif
	@ set -e; for i in $(SUBDIRS); do echo "MAKE     $@ => $$i/"; $(MAKE) -C $$i $@; done

ifeq '$(filter-out clean, $(MAKECMDGOALS))' '$(MAKECMDGOALS)'
  -include Makefile.deps
endif
