# NXCord
![GitHub](https://img.shields.io/github/license/Grarak/NXCord)
![GitHub release (latest by date including pre-releases)](https://img.shields.io/github/v/release/Grarak/NXCord?include_prereleases)
![GitHub All Releases](https://img.shields.io/github/downloads/Grarak/NXCord/total)
[![FOSSA Status](https://app.fossa.io/api/projects/git%2Bgithub.com%2FGrarak%2FNXCord.svg?type=shield)](https://app.fossa.io/projects/git%2Bgithub.com%2FGrarak%2FNXCord?ref=badge_shield)

## Unofficial Nintendo Switch Discord client
Discord's TOS forbids the use of third party clients, thus use at your own risk! Also a lot of code has been taken from https://github.com/yourWaifu/Unofficial-Discord-3DS-Client, so thanks a lot for yourWaifu's 3ds client.

<img src="screenshots/1.jpg" width="250"/><img src="screenshots/2.jpg" width="250"/><img src="screenshots/3.jpg" width="250"/><img src="screenshots/4.jpg" width="250"/><img src="screenshots/5.jpg" width="250"/>

## Installation and usage
Download the latest build from [Releases](https://github.com/Grarak/NXCord/releases). Extract it to the root of your sdcard you use for your switch. You do need to reboot your system, otherwise the sysmodule won't start running. After rebooting you can open NXCord in homebrew menu and you will see a login screen, if you it's you first time using it.

### Using a bot
Since selfbotting is not allowed, I'd recommend you to use a bot. If you don't know how to do it, just follow these [simple steps](https://github.com/reactiflux/discord-irc/wiki/Creating-a-discord-bot-&-getting-a-token). After copying the token and inviting the bot to your desired discord server, create a file named ```/config/nxcord/nxcord.ini``` and put your token there. For example your first line in the file should be ```token=<your token>```. You need to restart your switch after modifying that file.

### Selfbotting
Selbotting means that you are using a third pary client to access discord with an user account. It's against the TOS of discord, thus not recommended. I've been using an user account for testing for months already and haven't been banned yet. Might be safe, but I am not responsible for any bans!

## Limitations and bugs
Because this whole application is actually running in the background as a custom sysmodule, there are memory restrictions.
So if you are stuck at the *establishing connection* screen, you will have to use a different account/bot. I'd recommend you to only join one or two not so busy servers.

Also right now a lot of features are missing. Head over to [projects](https://github.com/Grarak/NXCord/projects) to see what possible features are planned for the future.

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
