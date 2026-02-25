# Edulution LINBO GUI

Enhanced boot interface for [LINBO](https://github.com/linuxmuster/linuxmuster-linbo7) (Linux-based Network Boot). Drop-in replacement for `linuxmuster-linbo-gui7`.

[![Build and Release](https://github.com/edulution-io/edulution-linbo-gui/actions/workflows/build-release.yml/badge.svg)](https://github.com/edulution-io/edulution-linbo-gui/actions)
[![GitHub Release](https://img.shields.io/github/v/release/edulution-io/edulution-linbo-gui?logo=github&logoColor=white)](https://github.com/edulution-io/edulution-linbo-gui/releases/latest)
[![License: AGPL v3](https://img.shields.io/badge/License-AGPL%20v3-blue.svg)](https://www.gnu.org/licenses/agpl-3.0)

## Features

- Modernized glassmorphism design with dark gradient background
- Refined pill layout for OS action buttons
- Dark-themed terminal dialog
- Additional OS icons (including Windows 11)
- Edulution branding
- Full compatibility with linuxmuster.net 7.3

## Installation

### On a linuxmuster.net server

Download the latest `.deb` from [Releases](https://github.com/edulution-io/edulution-linbo-gui/releases/latest) and install:

```bash
wget https://github.com/edulution-io/edulution-linbo-gui/releases/latest/download/edulution-linbo-gui7_1.1.0_all.deb
dpkg -i edulution-linbo-gui7_1.1.0_all.deb
```

The package automatically:
- Replaces `linuxmuster-linbo-gui7` if installed
- Sets an APT pin to prevent the official package from overwriting it
- Triggers `update-linbofs` to rebuild the boot filesystem

### Verify installation

```bash
dpkg -l | grep gui7
cat /etc/apt/preferences.d/edulution-linbo-gui
ls -la /srv/linbo/linbo_gui64_7.tar.lz
```

## Reverting to official package

```bash
apt remove edulution-linbo-gui7
apt install linuxmuster-linbo-gui7
```

The APT pin is automatically removed on uninstall.

## Building

Prerequisites: Docker (for the Qt6 cross-compilation container).

```bash
git clone https://github.com/edulution-io/edulution-linbo-gui.git
cd edulution-linbo-gui
./build.sh
```

The `.deb` package is created in the parent directory.

## Creating a release

1. Update `debian/changelog` with the new version
2. Update `GUI_VERSION` in `CMakeLists.txt`
3. Commit and push
4. Tag: `git tag v1.1.0 && git push origin --tags`
5. GitHub Actions builds the `.deb` and creates a release

## Based on

Forked from [linuxmuster-linbo-gui](https://github.com/linuxmuster/linuxmuster-linbo-gui) v7.3.3 by Dorian Zedler and the linuxmuster.net community.

## License

AGPL-3.0 - see [LICENSE](LICENSE).
