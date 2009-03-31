OBJECTS_app-template := debug.o ipc.o main.o mainstate.o process_frame.o
OBJECTS_app-template_host := oscar/staging/lib/libosc_host.a
OBJECTS_app-template_target := oscar/staging/lib/libosc_target.a

PRODUCTS := $(patsubst OBJECTS_%, %, $(filter OBJECTS_%, $(.VARIABLES)))
PRODUCTS := $(filter-out $(addsuffix _host, $(PRODUCTS)) $(addsuffix _target, $(PRODUCTS)), $(PRODUCTS))

SUBDIRS := cgi

CLEAN_FILES := *.o *.gdb $(PRODUCTS) $(addsuffix _host, $(PRODUCTS)) $(addsuffix _target, $(PRODUCTS)) Makefile.deps
CLEAN_FILES := $(wildcard $(CLEAN_FILES))

PEDANTIC := -Wall
OPTIMIZE := -O3

HOST_LIBS := -lm
TARGET_LIBS := -lbfdsp -lm

FLT_OPTS := -elf2flt="-s 1048576"

CC_STD := -std=gnu99

CC_HOST := gcc -c -DOSC_HOST $(CC_STD) $(PEDANTIC) $(OPTIMIZE)
LD_HOST := gcc -fPIC $(HOST_LIBS)

CC_TARGET := bfin-uclinux-gcc -c -DOSC_TARGET $(CC_STD) $(PEDANTIC) $(OPTIMIZE)
LD_TARGET := bfin-uclinux-gcc $(FLT_OPTS) $(TARGET_LIBS)

SHELL := $(shell which bash)
MAKE += --no-print-directory

-include .config

.PHONY: all host target
all: host target

.PHONY: install
install:
	@ set -e; for i in $(SUBDIRS); do echo "MAKE     $@ => $$i/"; $(MAKE) -C $$i $@; done

.PHONY: host
host: $(addsuffix _host, $(PRODUCTS))
	@ set -e; for i in $(SUBDIRS); do echo "MAKE     $@ => $$i/"; $(MAKE) -C $$i $@; done

.PHONY: target
target: $(addsuffix _target, $(PRODUCTS))
	@ set -e; for i in $(SUBDIRS); do echo "MAKE     $@ => $$i/"; $(MAKE) -C $$i $@; done

.PHONY: deploy
deploy: target .config_
	@ echo "SCP      runapp.sh app-template_target cgi/www.tar.gz => $(CONFIG_TARGET_IP):/opt"
	@ scp -rp runapp.sh app-template_target cgi/www.tar.gz root@$(CONFIG_TARGET_IP):/opt || true

.PHONY: run
run: .config_ $(filter deploy, $(MAKECMDGOALS))
	@ echo "SSH      $(CONFIG_TARGET_IP)"
	@ ssh root@$(CONFIG_TARGET_IP) /opt/runapp.sh || true

.PHONY: config_
.config_:
	@ [ -e ".config" ] || $(MAKE) config

.PHONY: config
config:
	@ echo "CONFIG   .config"
	@ ./configure

.PHONY: get
reconfigure:
	@ echo "CONFIG   oscar/"
ifeq '$(CONFIG_PRIVATE_FRAMEWORK)' 'n'
	@ ln -fs $(CONFIG_FRAMEWORK_PATH) "oscar"
endif
	@ cd oscar; ./configure

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

ifeq '$(filter clean config, $(MAKECMDGOALS))' ''
  -include Makefile.deps
endif
