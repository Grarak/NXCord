NXCORD_DIR		:=	$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

NXCORD_FLAGS	:=	`aarch64-none-elf-pkg-config --cflags opus` \
					`aarch64-none-elf-pkg-config --cflags libsodium` \
					-DSLEEPY_CUSTOM_CLIENT -DSLEEPY_VOICE_ENABLED \
					`aarch64-none-elf-pkg-config --cflags zlib`

CFLAGS			+=	$(NXCORD_FLAGS)

CXXFLAGS		+=	$(NXCORD_FLAGS) -std=gnu++17

LIBS			+=	-lnxcord -lnx -lwslay -lsleepy_discord \
					`aarch64-none-elf-pkg-config --libs opus` \
					`aarch64-none-elf-pkg-config --libs libsodium` \
					-lmbedtls -lmbedx509 -lmbedcrypto \
					`aarch64-none-elf-pkg-config --libs zlib` \
					-lSimpleIniParser
LIBDIRS			+=	$(NXCORD_DIR) $(NXCORD_DIR)/../wslay_build \
					$(NXCORD_DIR)/../external/sleepy-discord/buildtools \
					$(NXCORD_DIR)/../external/SimpleIniParser
INCLUDES		+=	../include ../wslay_build/include ../external/sleepy-discord/include \
					../external/sleepy-discord/include/sleepy_discord/IncludeNonexistent \
					../external/SimpleIniParser/include
