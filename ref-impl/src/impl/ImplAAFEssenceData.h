//@doc
//@class    AAFEssenceData | Implementation class for AAFEssenceData
#ifndef __ImplAAFEssenceData_h__
#define __ImplAAFEssenceData_h__

#include "OMDataStreamProperty.h"

//=---------------------------------------------------------------------=
//
// $Id: ImplAAFEssenceData.h,v 1.24 2007/03/16 18:00:26 akharkev Exp $ $Name:  $
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

class ImplAAFSourceMob;


#ifndef __ImplAAFObject_h__
#include "ImplAAFObject.h"
#endif


class ImplAAFPlainEssenceData;
class OMDataStreamPropertyFilter;


class ImplAAFEssenceData : public ImplAAFObject
{
public:
  //
  // Constructor/destructor
  //
  //********
  ImplAAFEssenceData ();

protected:
  virtual ~ImplAAFEssenceData ();

public:

/****/
  //****************
  // Initialize()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    Initialize
		// @parm [in] reference to a file mob
        (ImplAAFSourceMob * pFileMob);

  //****************
  // Write()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    Write
        (// @parm [in] write this many bytes
		 aafUInt32  bytes,

		 // @parm [out, size_is(buflen), length_is(*bytesRead)] here is the buffer
         aafDataBuffer_t  buffer,

		 // @parm [out,ref] 
		 aafUInt32 *  bytesWritten);

/****/
  //****************
  // Read()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    Read
        (// @parm [in] read this many bytes
		 aafUInt32  bytes,

		 // @parm [out, size_is(buflen), length_is(*bytesRead)] here is the buffer
         aafDataBuffer_t  buffer,

		 // @parm [out,ref] 
		 aafUInt32 *  bytesRead);

/****/
  //****************
  // SetPosition()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    SetPosition
		// @parm [in] offset from beginning of essence
        (aafPosition_t  offset);

/****/
  //****************
  // SetPosition()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    GetPosition
		// @parm [in] offset from beginning of essence
        (aafPosition_t  *pOffset);

/****/
  //****************
  // GetSize()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    GetSize
		// @parm [out] size of essence data
        (aafLength_t *  pSize );

  //****************
  // WriteSampleIndex()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    WriteSampleIndex
        (// @parm [in] write this many bytes
		 aafUInt32  bytes,

		 // @parm [out, size_is(buflen), length_is(*bytesRead)] here is the buffer
         aafDataBuffer_t  buffer,

		 // @parm [out,ref] 
		 aafUInt32 *  bytesWritten);

/****/
  //****************
  // ReadSampleIndex()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    ReadSampleIndex
        (// @parm [in] read this many bytes
		 aafUInt32  bytes,

		 // @parm [out, size_is(buflen), length_is(*bytesRead)] here is the buffer
         aafDataBuffer_t  buffer,

		 // @parm [out,ref] 
		 aafUInt32 *  bytesRead);

/****/
  //****************
  // SetSampleIndexPosition()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    SetSampleIndexPosition
		// @parm [in] offset from beginning of essence
        (aafPosition_t  offset);

/****/
  //****************
  // GetSampleIndexPosition()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    GetSampleIndexPosition
		// @parm [in] offset from beginning of essence
        (aafPosition_t  *pOffset);

/****/
  //****************
  // GetSampleIndexSize()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    GetSampleIndexSize
		// @parm [out] size of essence data
        (aafLength_t *  pSize );
  
/****/
  //****************
  // SetFileMob()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    SetFileMob
		// @parm [in] reference to a file mob
        (ImplAAFSourceMob * pFileMob);

/****/
  //****************
  // GetFileMob()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    GetFileMob
		// @parm [in] reference to a file mob
        (ImplAAFSourceMob ** ppFileMob);

/****/
  //****************
  // GetFileMobID()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    GetFileMobID
		// @parm [out] the file mob id associated with essence
        (aafMobID_t *  pFileMobID);

/****/
  //****************
  // GetPlainEssenceData()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    GetPlainEssenceData
        (// @parm [in] Reserved for future use
         aafUInt32 reserved,

         // @parm [in] file encoding-independent essence data
         ImplAAFPlainEssenceData ** pPlainEssenceData);


  //
  // Methods used internally by the SDK.
  //

  //****************
  // GetEssenceElementKey()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    GetEssenceElementKey
        (// @parm [out,ref] key of the essence elements in this stream
         aafUID_t *  pEssenceElementKey);


/****/
  //****************
  // SetEssenceElementKey()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    SetEssenceElementKey
        (// @parm [in] new stream alignment grid size
         aafUID_constref  key);


  virtual const OMMaterialIdentification& identification(void) const;

  OMDataStreamPropertyFilter* createMediaDataFilter();

private:
  OMFixedSizeProperty<aafMobID_t> _fileMobID;
  OMDataStreamProperty _mediaData;
  OMDataStreamProperty _indexData;
};

#endif // ! __ImplAAFEssenceData_h__

