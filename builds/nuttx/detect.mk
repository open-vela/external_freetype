#
# FreeType 2 configuration file to detect an NuttX host platform.
#


# Copyright (C) 2021 by Xiaomi Corporation.
#
# This file is part of the FreeType project, and may only be used, modified,
# and distributed under the terms of the FreeType project license,
# LICENSE.TXT.  By continuing to use, modify, or distribute this file you
# indicate that you have read the license and understand and accept it
# fully.


.PHONY: setup


ifeq ($(PLATFORM),ansi)

  ifdef APPDIR

    PLATFORM := nuttx

  endif # test MACHTYPE nuttx
endif

ifeq ($(PLATFORM),nuttx)

  PLATFORM_DIR := $(TOP_DIR)/builds/nuttx
  CONFIG_FILE  := nuttx.mk

  setup: std_setup

endif   # test PLATFORM nuttx


# EOF
