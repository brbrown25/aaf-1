//@doc
//@class    AAFDataEssenceDescriptor | Implementation class for AAFDataEssenceDescriptor
#ifndef __CAAFDataEssenceDescriptor_h__
#define __CAAFDataEssenceDescriptor_h__

//=---------------------------------------------------------------------=
//
// This file was GENERATED for the AAF SDK
//
// $Id: CAAFDataEssenceDescriptor.h,v 1.1 2006/03/24 18:18:38 jlow Exp $ $Name:  $
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
// The Original Code of this file is Copyright 1998-2005, Licensor of the
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







#ifndef __CAAFFileDescriptor_h__
#include "CAAFFileDescriptor.h"
#endif


class CAAFDataEssenceDescriptor
  : public IAAFDataEssenceDescriptor,
    public CAAFFileDescriptor
{
protected:

  //********
  //
  // Constructor/destructor
  //
  CAAFDataEssenceDescriptor (IUnknown * pControllingUnknown, aafBool doInit = kAAFTrue);
  virtual ~CAAFDataEssenceDescriptor ();

public:



  //***********************************************************
  //
  // SetDataEssenceCoding()
  //
  // Sets the DataEssenceCoding property
  /// This property is optional.  
  /// 
  /// If this method fails the Data Definition property will not be
  /// changed.
  /// 
  /// This method will return the following codes:
  /// 
  /// AAFRESULT_SUCCESS
  ///   - succeeded.  (This is the only code indicating success.)
  //
  STDMETHOD (SetDataEssenceCoding) (
    // Optional. 
    /*[in]*/ aafUID_constref  dataEssenceCoding);


  //***********************************************************
  //
  // GetDataEssenceCoding()
  //
  // Gets the DataEssenceCoding property
  /// information of the video essence data.  This property is
  /// optional.
  ///
  /// Succeeds if all of the following are true:
  /// - the pDataEssenceCoding pointer is valid.
  /// 
  /// If this method fails nothing will be written to *pDataEssenceCoding.
  /// 
  /// This method will return the following codes:
  /// 
  /// AAFRESULT_SUCCESS
  ///   - succeeded.  (This is the only code indicating success.)
  ///
  /// AAFRESULT_NULL_PARAM
  ///   - pDataEssenceCoding arg is NULL.
  //
  STDMETHOD (GetDataEssenceCoding) (
    // Optional. 
    /*[out]*/ aafUID_t *  pDataEssenceCoding);


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

#endif // ! __CAAFDataEssenceDescriptor_h__

