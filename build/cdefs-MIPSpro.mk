###############################################################################
#
# $Id: cdefs-MIPSpro.mk,v 1.7 2004/02/27 14:26:16 stuart_hc Exp $ $Name:  $
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

#------------------------------------------------------------------------------
#
# cdefs-MIPSpro.mk
#
#	This file contains makefile definitions for 
#	the Irix MIPSpro C++ compiler.
#
# Uses:
#	MIPS_ABI -	application binary interface. It's set in including 
#			platform definition file (like pdefs-MipsIrix.mk).
#
# Requires:
#       MIPS_ABI
#
# Sets:
#	CC -		C++ compiler command.
#	ADD_CFLAGS -	compiler options.
#	DBG_FLAGS -	compiler options to generate debug info.
#	XL - 		compiler option to pass an argument to a linker.
#
#------------------------------------------------------------------------------


#------------------------------------------------------------------------------
# Compiler command
# MIPS_ABI is defined in the platform definition file
#------------------------------------------------------------------------------
CC = CC -$(MIPS_ABI)
CCVERSION = CC -version

#------------------------------------------------------------------------------
# Debug info generation
#------------------------------------------------------------------------------
DBG_FLAGS = -g 


#------------------------------------------------------------------------------
# Compiler flags
#
# -no_auto_include	disables implicit inclusion of template 
#			implementation files.
# -prelink		template prelinker is on.
# -xansi		enables ANSI/ISO C.
# -G			see the explanation of -G 0 in 
#			the gp_overflow(5) manpage.
#------------------------------------------------------------------------------
ADD_CFLAGS = -no_auto_include -prelink -xansi -G 0

#------------------------------------------------------------------------------
# Compiler flags
# 
# Make all char declarations to be signed char declarations.
#------------------------------------------------------------------------------
ADD_CFLAGS += -signed -LANG:std


#------------------------------------------------------------------------------
# This option passes its argument to a linker
#------------------------------------------------------------------------------
XL = -Wl,


