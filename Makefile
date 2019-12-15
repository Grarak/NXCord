.PHONY: all clean

all:
	@$(MAKE) -C sysmodule/

clean:
	@$(MAKE) -C sysmodule/ clean
