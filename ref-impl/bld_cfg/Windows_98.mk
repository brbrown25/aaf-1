###############################################################################
#
# $Id: Windows_98.mk,v 1.4 2004/02/27 14:26:39 stuart_hc Exp $ $Name:  $
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
# The Original Code of this file is Copyright 1998-2004, Licensor of the
# AAF Association.
#
# The Initial Developer of the Original Code of this file and the
# Licensor of the AAF Association is Avid Technology.
# All rights reserved.
#
###############################################################################

#
# Contains elements for Windows 98 builds.
#

#
# Hack! Windows doesn't return the proper uname, so when common.mk
# includes $(UNAME).mk, on WinNT it will only include ".mk", not
# "Windows_NT.mk".  That's why a file called ".mk" exists in this
# directory, which includes this file.
#

PTFM_SUFX = w98
OBJ = obj
AAF_LIB = lib
EXE = exe

CMD_DIR = c:/packages/mksnt
VC_DIR = c:/Progra~1/Micros~8/VC98/bin

RM = ${CMD_DIR}/rm.exe
ECHO = ${CMD_DIR}/echo.exe
CP = ${CMD_DIR}/cp.exe
CAT = ${CMD_DIR}/cat.exe
CHMOD = ${CMD_DIR}/chmod.exe
MV = ${CMD_DIR}/mv.exe
SH = ${CMD_DIR}/sh.exe
TOUCH = ${CMD_DIR}/touch.exe
PERL = ${CMD_DIR}/perl.exe

SHELL = ${CMD_DIR}/sh.exe
MAKE_SUFFIX =

CC = ${VC_DIR}/cl
LD = ${VC_DIR}/cl
AAF_CMD_LINK = ${VC_DIR}/link

AAF_LINK_FLAGS = -lib -nologo
AAF_LINK_OUTPUT_FLAG = -out:

LFLAGS = 
CFLAGS = /nologo /Gi- /MDd /W3 /GX /Z7 /Od /Yd /D "WIN32" /D "_WIN32" /D "_DEBUG" /D "_WINDOWS"\
	/D "_UNICODE" /D "UNICODE"
#CFLAGS = /nologo /Gi-
OBJFILE = /Fo$*.${OBJ}
EXEFLAG = /Fo
DLL_LINK_FLAGS = kernel32.lib user32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib\
	/nologo /DLL /subsystem:windows /debug /pdb:none 
ARFLAGS = -r

MIDL = midl
MIDL_SYS_CFLAGS = /server none /client none
