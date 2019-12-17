# Unofficial Nintendo Switch Discord client
Currently WIP and not really usable for regular people. Also Discord's TOS forbids the use of third party clients, thus use at your own risk!. Also a lot of code has been taken from https://github.com/yourWaifu/Unofficial-Discord-3DS-Client, so thanks a lot for yourWaifu's 3ds client.

## Status
| Feature         | Available | Status                  |
|-----------------|-----------|-------------------------|
| HTTPS           | Yes       | 100%                    |
| Websocket       | Yes       | 100%                    |
| Receive content | Yes       | 100%                    |
| Send content    | No        | 50% - API done. No GUI. |
| UDP connection  | Yes       | 100%                    |
| Receive voice   | No        | 0%                      |
| Send voice      | No        | 50% - Need some input   |

## Build
For build you need to install a few dependencies first.
- libnx (pacman)
- switch-libsodium (pacman)
- switch-libopus (pacman)
- switch-mbedtls (pacman)
- switch-zlib (pacman)
- wslay
- sleepy-discord

### Building wslay
And use this PKGBUILD file:
```pkgname=switch-libwslay
pkgver=1.1.0
pkgrel=1
pkgdesc='The WebSocket library in C '
arch=('any')
url='https://tatsuhiro-t.github.io/wslay/'
license=('MIT')
options=(!strip libtool staticlibs)
makedepends=('devkitpro-pkgbuild-helpers')
source=("https://github.com/tatsuhiro-t/wslay/releases/download/release-${pkgver}/wslay-${pkgver}.tar.gz")
sha256sums=('0de975a31818f1c660fa3c674b17bbcbda6ad9c866402ac8ab46a1847325118e')
groups=('switch-portlibs')

build() {
  cd wslay-${pkgver}

  source /opt/devkitpro/switchvars.sh

  ./configure --prefix="${PORTLIBS_PREFIX}" --host=aarch64-none-elf \
    --disable-shared --enable-static
  make
}

package() {
  cd wslay-${pkgver}

  source /opt/devkitpro/switchvars.sh

  make DESTDIR="$pkgdir" install

  # license
  install -Dm644 LICENSE "$pkgdir"${PORTLIBS_PREFIX}/licenses/$pkgname/LICENSE
}
```

### Building sleepy-discord
```
$ git clone https://github.com/Grarak/sleepy-discord.git -b develop
$ cd sleepy-discord/buildtools
$ make -f Makefile.switch
```
Then copy the generated files to your portlibs folder.
Also create this pc file there.
```
# sleepy_discord installed pkg-config file

prefix=/opt/devkitpro/portlibs/switch
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: sleepy_discord
Description:
Version: 1.4
Requires: opus libsodium
Conflicts:
Libs: -L${libdir} -lsleepy_discord
Cflags: -I${includedir} -I${includedir}/sleepy_discord/IncludeNonexistent
```

### Building NXCord
After installing all the dependencies, you need to define your
discord token first.
```
$ echo <your_token> > sysmodule/.token
```
Without defining this first you might run into problems during runtime.
Executing ```make``` should be enough to generate a nsp file.
If you want to build the sysmodule as nro file:
```
$ cd sysmodule
$ make applet
```
