###############################################################################
#
# $Id: GenDepend2.sh,v 1.4 2004/02/27 14:26:18 stuart_hc Exp $ $Name:  $
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

for file in ${PRIVATE_AAFOBJECTS} ${HUMAN_TYPED_IMPL} ; do \
	grep -v $file\.impl depend.tmp | grep -v $file\.comt > depend.tmp2 ; \
	rm -f depend.tmp ; \
	mv depend.tmp2 depend.tmp ; \
done
