# Dev Environment Setup

Assuming Fedora base

Install pacman
> sudo dnf install pacman

Initialize keys
> sudo pacman-key --init

Set environment variables
> export DEVKITPRO=/opt/devkitpro
> export DEVKITARM=/opt/devkitpro/devkitARM
> export DEVKITPPC=/opt/devkitpro/devkitPPC

Import signing keys
> sudo pacman-key --recv BC26F752D25B92CE272E0F44F7FD5492264BB9D0 --keyserver keyserver.ubuntu.com
> sudo pacman-key --lsign BC26F752D25B92CE272E0F44F7FD5492264BB9D0

Install devkitpro keyring
> sudo pacman -U https://pkg.devkitpro.org/devkitpro-keyring.pkg.tar.zst

Add repositories to /etc/pacman.conf:

```
[dkp-libs]
Server = https://pkg.devkitpro.org/packages

[dkp-linux]
Server = https://pkg.devkitpro.org/packages/linux/$arch/
```

Sync the pacman database
> sudo pacman -Syu

Install wii toolchain
> sudo pacman -S wii-dev

Everything will be installed in /opt/devkitpro

To compile, add devkitpro tools to path
> export PATH=${DEVKITPRO}/tools/bin:$PATH
or
> /etc/profile.d/devkit-env.sh
