############################################
#                                          #
# Advanced Authoring Format                #
#                                          #
# Copyright (c) 1998 Avid Technology, Inc. #
#                                          #
############################################

#
# Contains elements common to all makefiles in this project.  Requires
# that BLD_CFG_DIR already be set by the including makefile to point
# to this directory.
#

.SUFFIXES: .o .o86 .oix .ont .obj .c .cpp .idl

#
# Get host type, and include appropriate host-specific makefile
#
UNAME := $(shell uname)

include $(BLD_CFG_DIR)/$(UNAME).mk


#
# Other macros
#
AR = ar

RM = rm

#
# common targets
#
baseclean:
	${RM} -f *.o *.o86 *.oix *.ont *.obj
	${RM} -f *.a *.a86 *.aix *.ant *.lib
	${RM} -f vc50.*
	${RM} -f *~ .*~
	${RM} -f makefile.bak makefile.dep makefile.dep.bak
	${RM} -f .objects.*.txt .testobjs.*.txt
	${RM} -f .cmake.state *.core core
