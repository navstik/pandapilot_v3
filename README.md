Clone NavStik NuttX Repository 
==========

In pandapilot_v3 directory, Issue below command:

git clone https://github.com/navstik/NuttX.git

Code Compilation And Generating Binary File
==========

Go to the pandapilot_v3 directory and undergo below commands:

1. ~/pandapilot_v3$ make clean
2. ~/pandapilot_v3$ make distclean
3. ~/pandapilot_v3$ make archives
4. ~/pandapilot_v3$ make all

After doing this on Terminal, The last lines of compilation output are as below:

ROMFS: romfs.img

OBJ: romfs.o

CC: romfs.o.c

LINK: /path/to/pandapilot_v3/Build/navstik-v1_default.build/firmware.elf

BIN: /path/to/pandapilot_v3/Build/navstik-v1_default.build/firmware.bin

Generating /path/to/pandapilot_v3/Build/navstik-v1_default.build/firmware.ns

make[1]: Leaving directory ` /path/to/pandapilot_v3/Build/navstik-v1_default.build'

%% Copying /path/to/pandapilot_v3/Images/navstik-v1_default.ns

Downloading Binary File to NavStik
==========

Download bin file by executing below command:

sudo dfu-util --device 0483:df11 -a0 --dfuse-address 0x8000000 -D Build/navstik-v1_default.build/firmware.bin

For more instructions, Visit NavStik wiki page: 
-
wiki.navstik.org
