#
# FreeType 2 configuration rules for a NuttX system
#
# this is similar to the "ansi-def.mk" file, except for BUILD and PLATFORM
#


# Copyright (C) 2024 by Xiaomi Corporation.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


DELETE       := rm -f
CAT          := cat
COPY         := cp
SEP          := /
PLATFORM_DIR := $(TOP_DIR)/builds/nuttx
PLATFORM     := nuttx


# The directory where all library files are placed.
#
# By default, this is the same as $(OBJ_DIR); however, this can be changed
# to suit particular needs.
#
LIB_DIR := $(APPDIR)


# The name of the final library file.  Note that the DOS-specific Makefile
# uses a shorter (8.3) name.
#
LIBRARY := libapps


# EOF
