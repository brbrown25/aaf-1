//=---------------------------------------------------------------------=
//
// $Id: OMDataTypes.cpp,v 1.1 2007/02/23 00:41:35 akharkev Exp $ $Name:  $
//
// The contents of this file are subject to the AAF SDK Public
// Source License Agreement (the "License"); You may not use this file
// except in compliance with the License.  The License is available in
// AAFSDKPSL.TXT, or you may obtain a copy of the License from the AAF
// Association or its successor.
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
// the License for the specific language governing rights and limitations
// under the License.
//
// The Original Code of this file is Copyright 1998-2007, Licensor of the
// AAF Association.
//
// The Initial Developer of the Original Code of this file and the
// Licensor of the AAF Association is Avid Technology.
// All rights reserved.
//
//=---------------------------------------------------------------------=

// @doc OMEXTERNAL

// @module OMDataTypes | Host independent data type definitions used
//         by the Object Manager.

#include "OMDataTypes.h"

// Constants
//
template<>
/*static*/ const OMUniqueObjectIdentification
OMConstant<OMUniqueObjectIdentification>::null =
{0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};

template<>
/*static*/ const OMUniqueMaterialIdentification
OMConstant<OMUniqueMaterialIdentification>::null =
{{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
 0, 0, 0, 0,
 {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}}};