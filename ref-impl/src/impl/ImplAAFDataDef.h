//@doc
//@class    AAFDataDef | Implementation class for AAFDataDef
#ifndef __ImplAAFDataDef_h__
#define __ImplAAFDataDef_h__


//=---------------------------------------------------------------------=
//
// $Id: ImplAAFDataDef.h,v 1.13 2004/02/27 14:26:47 stuart_hc Exp $ $Name:  $
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


#ifndef __ImplAAFDefObject_h__
#include "ImplAAFDefObject.h"
#endif

class ImplAAFDataDef : public ImplAAFDefObject
{
public:
  //
  // Constructor/destructor
  //
  //********
  ImplAAFDataDef ();

protected:
  virtual ~ImplAAFDataDef ();

public:
  //****************
  // Initialize()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    Initialize
        // @parm [in] Pointer to an AUID reference
        (const aafUID_t & id,
		 const aafCharacter *name,
		 const aafCharacter *description);

  //****************
  // IsPictureKind()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    IsPictureKind
        // @parm [retval,out] pointer to the return value
        (aafBool *  bIsPictureKind);

  //****************
  // IsMatteKind()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    IsMatteKind
        // @parm [retval,out] pointer to the return value
        (aafBool *  bIsMatteKind);

  //****************
  // IsPictureWithMatteKind()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    IsPictureWithMatteKind
        // @parm [retval,out] pointer to the return value
        (aafBool *  bIsPictureWithMatteKind);

  //****************
  // IsSoundKind()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    IsSoundKind
        // @parm [retval,out] pointer to the return value
        (aafBool *  bIsSoundKind);

  //****************
  // DoesDataDefConvertTo()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    DoesDataDefConvertTo
        (// @parm [in,ref] def to compare against
         ImplAAFDataDef * pDataDef,

         // @parm [retval,out] pointer to result
         aafBool *  bDoesConvertTo);
		   
  //****************
  // IsDataDefOf()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    IsDataDefOf
        (// @parm [in,ref] data def to compare against
         ImplAAFDataDef * pDataDef,

         // @parm [retval,out] pointer to result
         aafBool *  bIsDataDefOf);

  //****************
  // DoesDataDefConvertFrom()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    DoesDataDefConvertFrom
        (// @parm [in,ref] data def to compare against
         ImplAAFDataDef * pDataDef,

         // @parm [retval,out] pointer to result
         aafBool *  bDoesConvertFrom);

private:
  // *NOT* reference counted!
  ImplAAFDictionary * _pCachedDict;

  ImplAAFDictionary * GetDict();
};

//
// smart pointer
//

#ifndef __ImplAAFSmartPointer_h__
// caution! includes assert.h
#include "ImplAAFSmartPointer.h"
#endif

typedef ImplAAFSmartPointer<ImplAAFDataDef> ImplAAFDataDefSP;


#endif // ! __ImplAAFDataDef_h__
