#
# FreeType 2 configuration rules for a NuttX system
#


# Copyright (C) 2021 by Xiaomi Corporation.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


include $(TOP_DIR)/builds/nuttx/nuttx-def.mk

OLD_CC := $(CC)
include $(TOP_DIR)/builds/compiler/gcc.mk
CC = $(OLD_CC) -c

LINK_LIBRARY = $(call ARLOCK, $(BIN), $(OBJECTS_LIST))
CLEAN_LIBRARY =

FTSYS_SRC = $(PLATFORM_DIR)/ftsystem.c
include $(TOP_DIR)/builds/link_std.mk


# EOF
