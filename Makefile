.PHONY: nxcord submodules client sysmodule all clean

all: nxcord submodules

clean:
	@$(MAKE) -C external/sleepy-discord/buildtools/ -f Makefile.switch clean
	@$(MAKE) -C external/SimpleIniParser/ clean
	@$(MAKE) -C nxcord/ clean
	@$(MAKE) -C external/Plutonium/ clean
	@$(MAKE) -C client/ clean
	@$(MAKE) -C external/Atmosphere-libs/ clean
	@$(MAKE) -C sysmodule/ clean

simple_ini_parser:
	@$(MAKE) -C external/SimpleIniParser/

sleepy_discord:
	@$(MAKE) -C external/sleepy-discord/buildtools/ -f Makefile.switch

nxcord: sleepy_discord simple_ini_parser
	@$(MAKE) -C nxcord/

submodules: client sysmodule

client:
	@$(MAKE) -C external/Plutonium/
	@$(MAKE) -C client/

sysmodule: nxcord
	@$(MAKE) -C external/Atmosphere-libs/
	@$(MAKE) -C sysmodule/
