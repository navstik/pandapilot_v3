pandapilot_v3
=============

Autopilot Codebase

Pandapilot_v3

Clone NavStik NuttX repository in pandapilot_v3 directory as Follows:
~/pandapilot_v3$ git clone https://github.com/navstik/NuttX.git


1. Go to the pandapilot_v3 directory and undergo below commands
2.~/pandapilot_v3$ make clean
3.~/pandapilot_v3$ make distclean
4.~/pandapilot_v3$ make archives
5.~/pandapilot_v3$ make all

After doing this on Terminal, The last lines of compilation output are as below:

ROMFS: romfs.img
OBJ: romfs.o
CC: romfs.o.c
LINK: /path/to/pandapilot_v3/Build/navstik-v1_default.build/firmware.elf
BIN: /path/to/pandapilot_v3/Build/navstik-v1_default.build/firmware.bin
Generating /path/to/pandapilot_v3/Build/navstik-v1_default.build/firmware.ns
make[1]: Leaving directory ` /path/to/pandapilot_v3/Build/navstik-v1_default.build'
%% Copying /path/to/pandapilot_v3/Images/navstik-v1_default.ns

6.Download bin file by execuing below command:
sudo dfu-util --device 0483:df11 -a0 --dfuse-address 0x8000000 -D Build/navstik-v1_default.build/firmware.bin

For more instructions, Visit NavStik wiki page: wiki.navstik.org
