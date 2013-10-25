#
# Board-specific definitions for the NAVSTIK
#

#
# Configure the toolchain
#
CONFIG_ARCH			 = CORTEXM4F
CONFIG_BOARD                     = NAVSTIK_V1

include $(NAVSTIK_MK_DIR)/toolchain_gnu-arm-eabi.mk
