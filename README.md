# Vita Ping Pong

Ping Pong game for PS Vita.

![Game Preview](./sce_sys/livearea/contents/startup.png)

## Controls
|Action|Key              |
|-----|------------------|
|move | Left Joystick    |
|shoot| x                |

## Prerequisite

- Enable homebrew on your PS Vita. [https://henkaku.xyz/](https://henkaku.xyz/)
- Download and setup [VitaSDK](https://vitasdk.org/).
- Download FTP client for transfer VPK file to vita.

## Setup

Download this project:

```bash
git clone git@github.com:saracalihan/psvita-ball-hunter.git
```

Move into project:

```bash
cd psvita-ball-hunter
```

compile:

```bash
# Create Makefile
cmake .

# Compile code to vpk file
make
```

This command will create `BallHunter.vpk` file. Transfer this file using FTP or PlayStation Content Manager Assistant via USB then install it with [VitaShell](https://www.cfwaifu.com/vitashell/)

## License
This project is under the [MIT License](./LICENSE).
