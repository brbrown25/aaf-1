/***********************************************************************
 *
 *              Copyright (c) 1998-1999 Avid Technology, Inc.
 *
 * Permission to use, copy and modify this software and accompanying 
 * documentation, and to distribute and sublicense application software
 * incorporating this software for any purpose is hereby granted, 
 * provided that (i) the above copyright notice and this permission
 * notice appear in all copies of the software and related documentation,
 * and (ii) the name Avid Technology, Inc. may not be used in any
 * advertising or publicity relating to the software without the specific,
 * prior written permission of Avid Technology, Inc.
 *
 * THE SOFTWARE IS PROVIDED AS-IS AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL AVID TECHNOLOGY, INC. BE LIABLE FOR ANY DIRECT,
 * SPECIAL, INCIDENTAL, PUNITIVE, INDIRECT, ECONOMIC, CONSEQUENTIAL OR
 * OTHER DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE AND
 * ACCOMPANYING DOCUMENTATION, INCLUDING, WITHOUT LIMITATION, DAMAGES
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, AND WHETHER OR NOT
 * ADVISED OF THE POSSIBILITY OF DAMAGE, REGARDLESS OF THE THEORY OF
 * LIABILITY.
 *
 ************************************************************************/


#ifndef __ImplAAFDataDef_h__
#include "ImplAAFDataDef.h"
#endif

#ifndef __ImplAAFMob_h__
#include "ImplAAFMob.h"
#endif

#ifndef __ImplAAFSourceClip_h__
#include "ImplAAFSourceClip.h"
#endif

#include "AAFStoredObjectIDs.h"
#include "AAFPropertyIDs.h"

#include <assert.h>
#include "aafCvt.h" 
#include "AAFResult.h"
#include "AAFUtils.h"
#include "AAFDefUIDs.h"
#include "AAFDataDefs.h"
#include "ImplAAFHeader.h"

#include "ImplAAFSmartPointer.h"
typedef ImplAAFSmartPointer<ImplAAFDataDef>    ImplAAFDataDefSP;
typedef ImplAAFSmartPointer<ImplAAFDictionary> ImplAAFDictionarySP;

ImplAAFSourceClip::ImplAAFSourceClip ():
	_fadeInLength(		PID_SourceClip_FadeInLength,		L"FadeInLength"),
	_fadeInType(	PID_SourceClip_FadeInType,		L"FadeInType"),
	_fadeOutLength(	PID_SourceClip_FadeOutLength,		L"FadeOutLength"),
	_fadeOutType(	PID_SourceClip_FadeOutType,		L"FadeOutType"),
	_startTime(		PID_SourceClip_StartTime,		L"StartTime")
{
	_persistentProperties.put(		_fadeInLength.address());
	_persistentProperties.put(		_fadeInType.address());
	_persistentProperties.put(		_fadeOutLength.address());
	_persistentProperties.put(		_fadeOutType.address());
	_persistentProperties.put(		_startTime.address());

}


ImplAAFSourceClip::~ImplAAFSourceClip ()
{}


AAFRESULT STDMETHODCALLTYPE
    ImplAAFSourceClip::Initialize(ImplAAFDataDef *       pDataDef,
								  const aafLength_t &    length,
								  const aafSourceRef_t & sourceRef)
{
  if (! pDataDef)
	return AAFRESULT_NULL_PARAM;

  SetDataDef( pDataDef );
  SetLength( length );
  SetSourceID( sourceRef.sourceID );
  SetSourceMobSlotID( sourceRef.sourceSlotID );
  _startTime = sourceRef.startTime;

  _fadeInLength		= 0;
  _fadeInType		= kAAFFadeNone;
	
  _fadeOutLength		= 0;
  _fadeOutType	= kAAFFadeNone;

  return AAFRESULT_SUCCESS;
}


AAFRESULT STDMETHODCALLTYPE
   ImplAAFSourceClip::GetFade (aafLength_t	*fadeInLen,
                           aafFadeType_t	*fadeInType,
                           aafBool			*fadeInPresent,
                           aafLength_t		*fadeOutLen,
                           aafFadeType_t	*fadeOutType,
                           aafBool			*fadeOutPresent)
{
    AAFRESULT aafError = AAFRESULT_SUCCESS;

	if (fadeInLen      == NULL ||
		fadeInType     == NULL ||
		fadeInPresent  == NULL ||
		fadeOutLen     == NULL ||
		fadeOutType    == NULL ||
		fadeOutPresent == NULL )
	{
		aafError = 	AAFRESULT_NULL_PARAM;
	}
	else


	if (!_fadeInLength.isPresent()   ||
		!_fadeInType.isPresent()  ||
		!_fadeOutLength.isPresent()  ||
		!_fadeOutType.isPresent() )
	{
		aafError =  AAFRESULT_PROP_NOT_PRESENT;
	}

	else
	{
		
		*fadeInLen      = _fadeInLength;
		*fadeInType		= _fadeInType;
		if (_fadeInLength > 0)
		{
			*fadeInPresent	= kAAFTrue;
		}
		else
		{
			*fadeInPresent = kAAFFalse;
		}
		
		*fadeOutLen		= _fadeOutLength;
		*fadeOutType	= _fadeOutType;
		if (_fadeOutLength > 0)
		{
			*fadeOutPresent	= kAAFTrue;
		}
		else
		{
			*fadeOutPresent = kAAFFalse;
		}
	}

	return aafError;
}


/*************************************************************************
 * Function: ResolveRef()
 *
 * 		Given a source clip object, this function returns a pointer to
 *      the mob that it references.  If this mob does not exist, the
 *      error AAFRESULT_MOB_NOT_FOUND is returned.
 *
 * Argument Notes:
 *
 * ReturnValue:
 *		Error code .
 *
 * Possible Errors:
 *		Standard errors .
 *************************************************************************/
AAFRESULT STDMETHODCALLTYPE
    ImplAAFSourceClip::ResolveRef (ImplAAFMob ** mob)
{
    aafSourceRef_t sourceRef;
    ImplAAFHeader *head = NULL;
    AAFRESULT aafError = AAFRESULT_SUCCESS;

	*mob = NULL;

	XPROTECT()
	  {
		if (!mob)
		  {
			RAISE(AAFRESULT_NULL_PARAM);
		  }

		/* NOTE: This function should really be doing more checking,
		 * i.e., does track exist in mob, does position (adjusted by
		 * editrate) exist in track, etc.
		 */

		CHECK(GetSourceReference(&sourceRef));
		CHECK(MyHeadObject(&head));
		CHECK(head->LookupMob(sourceRef.sourceID, mob));

		head->ReleaseReference();
		head = NULL;
	  } /* XPROTECT */

	XEXCEPT
	  {
		if(head)
		  head->ReleaseReference();
		head = 0;
	  }
	XEND;

	return(aafError);
}


/*************************************************************************
 * Function: GetSourceReference()
 *
 * 		This function returns the 3 properties on a source Clip that
 *      make of the "source reference" (sourceID, sourceTrackID, and
 *      startTime).
 *
 * Argument Notes:
 *
 * ReturnValue:
 *		Error code.
 *
 * Possible Errors:
 *		Standard errors.
 *************************************************************************/
AAFRESULT STDMETHODCALLTYPE
    ImplAAFSourceClip::GetSourceReference (aafSourceRef_t*	pSourceRef)
{
    AAFRESULT aafError = AAFRESULT_SUCCESS;
	aafMobID_t	sourceID;
	aafSlotID_t slotID;

	XPROTECT()
	{	
		if (pSourceRef)
		{
			if (! _startTime.isPresent())
				RAISE(AAFRESULT_PROP_NOT_PRESENT);

			CHECK(GetSourceID( &sourceID ));
			CHECK(GetSourceMobSlotID( &slotID ));
			pSourceRef->sourceID = sourceID;
			pSourceRef->sourceSlotID = slotID;
			pSourceRef->startTime = _startTime;
		} /* if sourceRef */
		else
		{
			RAISE(AAFRESULT_NULL_PARAM);
		}
	 } /* XPROTECT */

	XEXCEPT
	  {
		//return(XCODE());
	  }
	XEND;

	return(aafError);
}


 AAFRESULT STDMETHODCALLTYPE
   ImplAAFSourceClip::SetFade (aafInt32		fadeInLen,
                           aafFadeType_t	fadeInType,
                           aafInt32			fadeOutLen,
                           aafFadeType_t	fadeOutType)
 {
	 AAFRESULT aafError = AAFRESULT_SUCCESS;
	 
	 ImplAAFDataDef	*dataDef;
	 aafBool		isSound;
	 
	 GetDataDef (&dataDef);
	 dataDef->IsSoundKind(&isSound);
	 if(isSound)
	 {
		 if (fadeInLen > 0)
		 {
			 _fadeInLength	= fadeInLen;
			 _fadeInType	= fadeInType;
		 }
		 
		 if (fadeOutLen > 0)
		 {
			 _fadeOutLength		= fadeOutLen;
			 _fadeOutType	= fadeOutType;
		 }
	 }
	 else
		 aafError = AAFRESULT_INVALID_DATADEF;		// Must be sound Datadef for this call
	 
	 return aafError;
 }


AAFRESULT STDMETHODCALLTYPE
    ImplAAFSourceClip::SetSourceReference (aafSourceRef_t  sourceRef)
{
	AAFRESULT   aafError = AAFRESULT_SUCCESS;
	static const aafMobID_t nullMobID = {0};
	
	/* If MobID is NUL - make the rest of the fields 0 too. */
	if(memcmp(&sourceRef.sourceID, &nullMobID, sizeof(sourceRef.sourceID)) == 0)
	{
		sourceRef.sourceSlotID = 0;
		CvtInt32toPosition(0, sourceRef.startTime);	
	}
	SetSourceID( sourceRef.sourceID );
	SetSourceMobSlotID( sourceRef.sourceSlotID) ;
	_startTime = sourceRef.startTime;

	return(aafError);
}



AAFRESULT ImplAAFSourceClip::TraverseToClip(aafLength_t length,
											ImplAAFSegment **sclp,
											ImplAAFPulldown ** /*pulldownObj*/,
											aafInt32 * /*pulldownPhase*/,
											aafLength_t *sclpLen,
											aafBool * /*isMask*/)
{
	XPROTECT()
	{
		*sclp = this;
		// We are returning a reference to this object so bump the ref count
		AcquireReference();
		CHECK((*sclp)->GetLength(sclpLen));
		if (Int64Less(length, *sclpLen))
		{
			*sclpLen = length;
		}
	} /* XPROTECT */
	
	XEXCEPT
	{
	}
	XEND;
	
	return(AAFRESULT_SUCCESS);
}
