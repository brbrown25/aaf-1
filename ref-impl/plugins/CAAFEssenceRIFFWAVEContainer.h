//@doc
//@class    AAFEssenceRIFFWAVEContainer | Implementation class for AAFEssenceRIFFWAVEContainer
#ifndef __CAAFEssenceRIFFWAVEContainer_h__
#define __CAAFEssenceRIFFWAVEContainer_h__

//=---------------------------------------------------------------------=
//
// $Id: CAAFEssenceRIFFWAVEContainer.h,v 1.1 2004/11/04 15:49:26 stuart_hc Exp $ $Name:  $
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
// Portions created by British Broadcasting Corporation are
// Copyright 2004, British Broadcasting Corporation.  All rights reserved.
//
//=---------------------------------------------------------------------=



#ifndef __CAAFUnknown_h__
#include "CAAFUnknown.h"
#endif

#ifndef __CAAFEssenceRIFFWAVEStream_h__
#include "CAAFEssenceRIFFWAVEStream.h"
#endif

//
// Include the standard plugin interfaces.
//
#include "AAFPlugin.h"




class CAAFEssenceRIFFWAVEContainer
  : public IAAFEssenceContainer,
    public IAAFPlugin,
    public CAAFUnknown
{
protected:

  //********
  //
  // Constructor/destructor
  //
  CAAFEssenceRIFFWAVEContainer (IUnknown * pControllingUnknown);
  virtual ~CAAFEssenceRIFFWAVEContainer ();


public:
  //
  // This class as concrete. All AAF objects can be constructed from
  // a CLSID. This will allow subclassing all "base-classes" by
  // aggreggation.
  //
  // Macro also declares the IUnknown methods
  // 
  AAF_DECLARE_CONCRETE();


  //
  // IAAFPlugin interface methods.
  //

  STDMETHOD (CountDefinitions)(aafUInt32 *pDefCount);
  STDMETHOD (GetIndexedDefinitionID)(aafUInt32 index, aafUID_t *result);
  STDMETHOD (GetPluginDescriptorID)(aafUID_t *result);

  STDMETHOD (GetIndexedDefinitionObject)(aafUInt32 index, IAAFDictionary *dict, IAAFDefObject **def);

  STDMETHOD (CreateDescriptor)(IAAFDictionary *dict, IAAFPluginDef **desc);

  //
  // IAAFEssenceRIFFWAVEContainer interface methods.
  //

  // Attempt to create an essence stream in the container 
  // for reading and writing.
  STDMETHOD (CreateEssenceStream)
    (/*[in,string]*/ const aafCharacter * pName, // String key to identify the new essence within container
     /*[in]*/ aafMobID_constptr pMobID, // Optional mobID identifying the essence
     /*[out,retval]*/ IAAFEssenceStream ** ppEssenceStream); // address of local variable to contain the essence stream pointer. 

  // Attempt to create an essence stream in the container 
  // for writing.
  STDMETHOD (CreateEssenceStreamWriteOnly)
    (/*[in,string]*/ const aafCharacter * pPath, // String key to identify the new essence within container
     /*[in]*/ aafMobID_constptr pMobID, // Optional mobID identifying the essence
     /*[out,retval]*/ IAAFEssenceStream ** ppEssenceStream); // address of local variable to contain the essence stream pointer. 

  // Attempt to open an essence stream for reading.
  // Returns one of the following:
  STDMETHOD (OpenEssenceStreamReadOnly)
    (/*[in,string]*/ const aafCharacter * pFilePath, // String key to identify the existing essence within container
     /*[in]*/ aafMobID_constptr pMobID, // Optional mobID identifying the essence
     /*[out,retval]*/ IAAFEssenceStream ** ppEssenceStream); // address of local variable to contain the essence stream pointer. 

  // Attempt to open an essence stream for appending.
  // Returns one of the following:
  STDMETHOD (OpenEssenceStreamAppend)
    (/*[in,string]*/ const aafCharacter * pFilePath, // String key to identify the existing essence within container
     /*[in]*/ aafMobID_constptr pMobID, // Optional mobID identifying the essence
     /*[out,retval]*/ IAAFEssenceStream ** ppEssenceStream); // address of local variable to contain the essence stream pointer. 



protected:
  // 
  // Declare the QI that implements for the interfaces
  // for this module. This will be called by CAAFUnknown::QueryInterface().
  // 
  STDMETHOD(InternalQueryInterface)(REFIID riid, void **ppvObjOut);

public:
  // Accessors
  CAAFEssenceRIFFWAVEStream* LastFileStream() const;
  void SetLastFileStream(CAAFEssenceRIFFWAVEStream *pLastFileStream);
  
  // Check to see if it is ok to create of open the stream with given
  // path and mode.
  HRESULT CheckExistingStreams(const wchar_t *pwcPath, FileStreamMode streamMode);


private:
  // List of all open file streams...
  CAAFEssenceRIFFWAVEStream* _pLastFileStream;
};


EXTERN_C const CLSID CLSID_AAFEssenceRIFFWAVEContainer;


#endif // ! __CAAFEssenceRIFFWAVEContainer_h__



