ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

WSLAY_NAME := "wslay-1.1.0"
WSLAY_LINK := "https://github.com/tatsuhiro-t/wslay/releases/download/release-1.1.0/${WSLAY_NAME}.tar.gz"
SHELL := /bin/bash

.PHONY: wslay simple_ini_parser sleepy_discord nxcord submodules plutonium stratosphere client sysmodule all clean

all: nxcord submodules

clean:
	@rm -rf wslay*
	@$(MAKE) -C external/sleepy-discord/buildtools/ -f Makefile.switch clean
	@$(MAKE) -C external/SimpleIniParser/ clean
	@$(MAKE) -C nxcord/ clean
	@$(MAKE) -C external/Plutonium/ clean
	@$(MAKE) -C client/ clean
	@$(MAKE) -C external/Atmosphere-libs/ clean
	@$(MAKE) -C sysmodule/ clean

wslay:
	@if [ ! -f $(WSLAY_NAME).tar.gz ];then echo Downloading $(WSLAY_NAME); wget $(WSLAY_LINK);fi
	@if [ ! -d $(WSLAY_NAME) ];then tar xvf $(WSLAY_NAME).tar.gz;fi
	@if [ ! -f $(WSLAY_NAME)/Makefile ];then cd $(WSLAY_NAME) && autoreconf -i && automake && autoconf && \
	source $(DEVKITPRO)/switchvars.sh && \
	./configure --prefix=$(CURDIR)/wslay_build --host=aarch64-none-elf --disable-shared --enable-static;fi
	source $(DEVKITPRO)/switchvars.sh && $(MAKE) -C $(WSLAY_NAME)/
	source $(DEVKITPRO)/switchvars.sh && $(MAKE) -C $(WSLAY_NAME)/ install

simple_ini_parser:
	@$(MAKE) -C external/SimpleIniParser/

sleepy_discord:
	@$(MAKE) -C external/sleepy-discord/buildtools/ -f Makefile.switch

nxcord: wslay sleepy_discord simple_ini_parser
	@$(MAKE) -C nxcord/

submodules: client sysmodule

plutonium:
	@$(MAKE) -C external/Plutonium/

client: plutonium
	@$(MAKE) -C client/

stratosphere:
	@$(MAKE) -C external/Atmosphere-libs/

sysmodule: nxcord stratosphere
	@$(MAKE) -C sysmodule/
