#*****************************************************************************
#           Makefile Build System for Fawkes: Webview Plugin Config
#                            -------------------
#   Created on Mon Oct 13 18:00:51 2008
#   Copyright (C) 2008 by Tim Niemueller, AllemaniACs RoboCup Team
#
#*****************************************************************************
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#*****************************************************************************

ifneq ($(wildcard $(SYSROOT)/usr/include/microhttpd.h),)
  HAVE_LIBMICROHTTPD=1
  CFLAGS_LIBMICROHTTPD  = -DHAVE_LIBMICROHTTPD
  LDFLAGS_LIBMICROHTTPD = -lmicrohttpd
endif
