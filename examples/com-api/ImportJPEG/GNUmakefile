###############################################################################
#
# $Id: GNUmakefile,v 1.2 2006/03/11 14:36:10 stuart_hc Exp $
#
# The contents of this file are subject to the AAF SDK Public
# Source License Agreement (the "License"); You may not use this file
# except in compliance with the License.  The License is available in
# AAFSDKPSL.TXT, or you may obtain a copy of the License from the AAF
# Association or its successor.
# 
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
# the License for the specific language governing rights and limitations
# under the License.
# 
# The Original Code of this file is Copyright 1998-2001, Licensor of the
# AAF Association.
# 
# The Initial Developer of the Original Code of this file and the
# Licensor of the AAF Association is Avid Technology.
# All rights reserved.
#
###############################################################################


EXAMPLE = ImportJPEG

include ../comexamplerules.mk

TEST_CMD = env LD_LIBRARY_PATH=$(AAFSDKBINDIR):$(LD_LIBRARY_PATH) $(AAFSDKBINDIR)/ImportJPEG$(EXE)
.PHONY : check
check : $(AAFSDKBINDIR)/ImportJPEG$(EXE)
	@for f in 2:1 3:1 10:1 20:1 15:1 ; do if [ -r ../ExportJPEG/sdlake_uyvy_$$f.aaf ] ; then $(TEST_CMD) -d ../ExportJPEG/sdlake_uyvy_$$f.aaf || exit $$? ; $(TEST_CMD) ../ExportJPEG/sdlake_uyvy_$$f.aaf || exit $$? ; fi ; done
	@for f in 2:1 3:1 10:1 20:1 15:1 ; do if [ -r ../ExportJPEG/sdlake_uyvy_ntsc_$$f.aaf ] ; then $(TEST_CMD) -d ../ExportJPEG/sdlake_uyvy_ntsc_$$f.aaf || exit $$? ; $(TEST_CMD) ../ExportJPEG/sdlake_uyvy_ntsc_$$f.aaf || exit $$? ; fi ; done
	@for f in 2:1 3:1 10:1 20:1 15:1 ; do if [ -r ../ExportJPEG/colorbars_$$f.aaf ] ; then $(TEST_CMD) -d ../ExportJPEG/colorbars_$$f.aaf || exit $$? ; $(TEST_CMD) ../ExportJPEG/colorbars_$$f.aaf || exit $$? ; fi ; done
	@for f in 2:1 3:1 10:1 20:1 15:1 ; do if [ -r ../ExportJPEG/colorbars_ntsc_$$f.aaf ] ; then $(TEST_CMD) -d ../ExportJPEG/colorbars_ntsc_$$f.aaf || exit $$? ; $(TEST_CMD) ../ExportJPEG/colorbars_ntsc_$$f.aaf || exit $$? ; fi ; done