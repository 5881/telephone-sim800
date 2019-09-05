PROJECT = mobile
BUILD_DIR = bin

SHARED_DIR = ../inc-lib
CFILES = mobile.c
CFILES += 25q32.c  4x4key.c sim800.c rprintf.c  st7735_128x160.c
CFILES += st_printf.c  xmodem.c menu.c
#AFILES += api-asm.S

# TODO - you will need to edit these two lines!
DEVICE=stm32f103c8t6
#OOCD_FILE = interface/stlink-v2.cfg
OOCD_INTERFACE = stlink-v2
OOCD_TARGET = stm32f1x

# You shouldn't have to edit anything below here.
VPATH += $(SHARED_DIR)
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
OPENCM3_DIR=../libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk
include ../rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk
