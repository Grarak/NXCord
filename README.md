# NXCord
![GitHub](https://img.shields.io/github/license/Grarak/NXCord)
![GitHub release (latest by date including pre-releases)](https://img.shields.io/github/v/release/Grarak/NXCord?include_prereleases)
![GitHub All Releases](https://img.shields.io/github/downloads/Grarak/NXCord/total)
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2FGrarak%2FNXCord.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2FGrarak%2FNXCord?ref=badge_shield)

## Unofficial Nintendo Switch Discord client
Discord's TOS forbids the use of third party clients, thus use at your own risk! Also a lot of code has been taken from https://github.com/yourWaifu/Unofficial-Discord-3DS-Client, so thanks a lot for yourWaifu's 3ds client.

<img src="screenshots/1.jpg" width="250"/><img src="screenshots/2.jpg" width="250"/><img src="screenshots/3.jpg" width="250"/><img src="screenshots/4.jpg" width="250"/><img src="screenshots/5.jpg" width="250"/>

## Dependencies
For building you need to install dependencies first.

Assuming you have devkitpro and libnx installed, just run:
```
$ (dkp-)pacman -S devkitpro-pkgbuild-helpers switch-pkg-config switch-libsodium switch-mbedtls switch-zlib switch-sdl2 switch-sdl2_ttf switch-sdl2_image switch-sdl2_gfx switch-sdl2_mixer switch-mesa switch-glad switch-glm switch-libdrm_nouveau switch-libwebp switch-libpng switch-freetype switch-bzip2 switch-libjpeg-turbo switch-opusfile switch-libopus
```

## Building NXCord
```
$ git clone --recurse-submodules git@github.com:Grarak/NXCord.git
$ cd NXCord
$ make
```

### Build options
There are several different build variants:
- Client without IPC:
```
$ cd client
$ make standalone
```
- Sysmodule as client:
```
$ cd sysmodule
$ make application
```

## License
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2FGrarak%2FNXCord.svg?type=large)](https://app.fossa.io/projects/git%2Bgithub.com%2FGrarak%2FNXCord?ref=badge_large)
