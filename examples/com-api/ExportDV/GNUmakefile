###############################################################################
#
# $Id: GNUmakefile,v 1.8 2011/03/17 19:11:31 philipn Exp $ $Name:  $
#
# The contents of this file are subject to the AAF SDK Public Source
# License Agreement Version 2.0 (the "License"); You may not use this
# file except in compliance with the License.  The License is available
# in AAFSDKPSL.TXT, or you may obtain a copy of the License from the
# Advanced Media Workflow Association, Inc., or its successor.
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
# the License for the specific language governing rights and limitations
# under the License.  Refer to Section 3.3 of the License for proper use
# of this Exhibit.
#
# WARNING:  Please contact the Advanced Media Workflow Association,
# Inc., for more information about any additional licenses to
# intellectual property covering the AAF Standard that may be required
# to create and distribute AAF compliant products.
# (http://www.amwa.tv/policies).
#
# Copyright Notices:
# The Original Code of this file is Copyright 1998-2009, licensor of the
# Advanced Media Workflow Association.  All rights reserved.
#
# The Initial Developer of the Original Code of this file and the
# licensor of the Advanced Media Workflow Association is
# Avid Technology.
# All rights reserved.
#
###############################################################################


EXAMPLE = ExportDV

include ../comexamplerules.mk

TEST_CMD = $(APP_RUN_ENV) $(AAFSDKBINDIR)/ExportDV$(EXE)
.PHONY : check
check : $(AAFSDKBINDIR)/ExportDV$(EXE)
	@# Only run compression tests if ffmpeg available
ifdef FFMPEG_PATH
	@# Test default args
	$(TEST_CMD)
	@# Test mxf output
	$(TEST_CMD) -o ExportDV.mxf -mxf
	@# Test compression using all codec flavours
	@for f in \
	LegacyDV_625_50 \
	LegacyDV_525_60 \
	IEC_DV_625_50 \
	IEC_DV_525_60 \
	DV_Based_25Mbps_625_50 \
	DV_Based_25Mbps_525_60 \
	DV_Based_50Mbps_625_50 \
	DV_Based_50Mbps_525_60 \
	DV_Based_100Mbps_720x50P \
	DV_Based_100Mbps_720x5994P \
	DV_Based_100Mbps_1080x50I \
	DV_Based_100Mbps_1080x5994I \
	SMPTE_D10_50Mbps_625x50I \
	SMPTE_D10_50Mbps_525x5994I \
	SMPTE_D10_40Mbps_625x50I \
	SMPTE_D10_40Mbps_525x5994I \
	SMPTE_D10_30Mbps_625x50I \
	SMPTE_D10_30Mbps_525x5994I; do \
	  rm -f $$f.aaf; echo $(TEST_CMD) -o ExportDV_$$f.aaf -f $$f; $(TEST_CMD) -o ExportDV_$$f.aaf -f $$f || exit 1; \
	done
endif
	@# Run test using pre-compressed DV frame (default IEC_DV_625_50 flavour)
	rm -f ExportDV_NoCompression.aaf
	$(TEST_CMD) -o ExportDV_NoCompression.aaf -raw ../../../test/com/ScaleTest/1frame.dv && test -f ExportDV_NoCompression.aaf
