//=---------------------------------------------------------------------=
//
// This file was GENERATED for the AAF SDK
//
// $Id: CAAFStrongRefValue.cpp,v 1.6 2004/02/27 14:26:45 stuart_hc Exp $ $Name:  $
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



#include "CAAFStrongRefValue.h"
#include "ImplAAFStrongRefValue.h"
#include "AAFResult.h"
#include "CAAFEnumValidation.h"

#include "OMAssertions.h"
#include "OMExceptions.h"

#include <assert.h>
#include <string.h>


// CLSID for AAFStrongRefValue 
// {4142d581-6892-11d4-a812-8f71e157e3f3}
EXTERN_C const CLSID CLSID_AAFStrongRefValue = { 0x4142d581, 0x6892, 0x11d4, { 0xa8, 0x12, 0x8f, 0x71, 0xe1, 0x57, 0xe3, 0xf3 } };





CAAFStrongRefValue::CAAFStrongRefValue (IUnknown * pControllingUnknown, aafBool doInit)
  : CAAFPropertyValue (pControllingUnknown, kAAFFalse)
{
  if (doInit)
    {
      ImplAAFStrongRefValue * newRep;
      newRep = new ImplAAFStrongRefValue;
      assert (newRep);
      InitRep (newRep);
    }
}


CAAFStrongRefValue::~CAAFStrongRefValue ()
{
}


//
// 
// 
inline int EQUAL_UID(const GUID & a, const GUID & b)
{
  return (0 == memcmp((&a), (&b), sizeof (aafUID_t)));
}
HRESULT CAAFStrongRefValue::InternalQueryInterface
(
    REFIID riid,
    void **ppvObj)
{
    if (NULL == ppvObj)
        return E_INVALIDARG;

    // We only support the IClassFactory interface 
    if (EQUAL_UID(riid,IID_IAAFStrongRefValue)) 
    { 
        *ppvObj = (IAAFStrongRefValue *)this; 
        ((IUnknown *)*ppvObj)->AddRef();
        return S_OK;
    }

    // Always delegate back to base implementation.
    return CAAFPropertyValue::InternalQueryInterface(riid, ppvObj);
}

//
// Define the contrete object support implementation.
// 
AAF_DEFINE_FACTORY(AAFStrongRefValue)

