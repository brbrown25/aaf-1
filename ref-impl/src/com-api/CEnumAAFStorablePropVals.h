//@doc
//@class    EnumAAFStorablePropVals | Implementation class for EnumAAFStorablePropVals
#ifndef __CEnumAAFStorablePropVals_h__
#define __CEnumAAFStorablePropVals_h__

//=---------------------------------------------------------------------=
//
// This file was GENERATED for the AAF SDK
//
// $Id: CEnumAAFStorablePropVals.h,v 1.3 2004/02/27 14:26:46 stuart_hc Exp $ $Name:  $
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
// The Original Code of this file is Copyright 1998-2004, Licensor of the
// AAF Association.
//
// The Initial Developer of the Original Code of this file and the
// Licensor of the AAF Association is Avid Technology.
// All rights reserved.
//
//=---------------------------------------------------------------------=



#ifndef __AAF_h__
#include "AAF.h"
#endif





#ifndef __CEnumAAFPropertyValues_h__
#include "CEnumAAFPropertyValues.h"
#endif


class CEnumAAFStorablePropVals
  : public IEnumAAFStorablePropVals,
    public CEnumAAFPropertyValues
{
protected:

  //********
  //
  // Constructor/destructor
  //
  CEnumAAFStorablePropVals (IUnknown * pControllingUnknown, aafBool doInit = kAAFTrue);
  virtual ~CEnumAAFStorablePropVals ();

public:



protected:
  // 
  // Declare the QI that implements for the interfaces
  // for this module. This will be called by CAAFUnknown::QueryInterface().
  // 
  STDMETHOD(InternalQueryInterface)(REFIID riid, void **ppvObjOut);


public:
  //
  // This class as concrete. All AAF objects can be constructed from
  // a CLSID. This will allow subclassing all "base-classes" by
  // aggreggation.
  // 
  AAF_DECLARE_CONCRETE();
  //
  //********
};

#endif // ! __CEnumAAFStorablePropVals_h__


