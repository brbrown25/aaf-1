//=---------------------------------------------------------------------=
//
// $Id: ImplAAFClassDef.cpp,v 1.58 2004/02/27 14:26:46 stuart_hc Exp $ $Name:  $
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

// Conditional symbol for testing the creation of a new optional 
// strong reference set property on the dictionary. This is experimental
// because the AAFDictionary is still considered "axiomatic" by
// ImplAAFBuiltinClasses and itself. The problem is that the
// class definitions for such axiomatic objects are unpersisted
// but NOT used by the DM, the "builtin" class definition is used instead!
// 2000-SEPT-14 transdel.
#ifndef SUPPORT_EXPERIMENTAL_OPTIONAL_SETS
#define SUPPORT_EXPERIMENTAL_OPTIONAL_SETS 0
#endif


#ifndef __ImplEnumAAFPropertyDefs_h__
#include "ImplEnumAAFPropertyDefs.h"
#endif

#include "ImplAAFSmartPointer.h"
typedef ImplAAFSmartPointer<ImplEnumAAFPropertyDefs> ImplEnumAAFPropertyDefsSP;

#ifndef __ImplAAFPropertyDef_h__
#include "ImplAAFPropertyDef.h"
#endif

#ifndef __ImplAAFTypeDef_h__
#include "ImplAAFTypeDef.h"
#endif

#ifndef __ImplAAFClassDef_h__
#include "ImplAAFClassDef.h"
#endif

#ifndef __ImplAAFDictionary_h__
#include "ImplAAFDictionary.h"
#endif


#include "AAFStoredObjectIDs.h"
#include "AAFPropertyIDs.h"
#include "ImplAAFObjectCreation.h"
#include "ImplAAFBuiltinDefs.h"
#include "ImplAAFCloneResolver.h"
#include "AAFUtils.h"

#include <assert.h>
#include <string.h>

extern "C" const aafClassID_t CLSID_EnumAAFPropertyDefs;
extern "C" const aafClassID_t CLSID_AAFObject;

//
// locals obj mgmt utils
//
#define check_result(result)     \
  if (AAFRESULT_FAILED (result)) \
	return result;

/* Note!  Will modify argument... */
#define release_if_set(pIfc)     \
{                                \
  if (pIfc)                      \
	{                            \
	  pIfc->ReleaseReference (); \
	  pIfc = 0;                  \
	}                            \
}

/* Note!  Will modify argument... */
#define release_and_zero(pIfc)   \
{                                \
  assert (pIfc);                 \
  pIfc->ReleaseReference ();     \
  pIfc = 0;                      \
}



ImplAAFClassDef::ImplAAFClassDef ()
  : _ParentClass  ( PID_ClassDefinition_ParentClass,  
                    L"ParentClass",
                    L"/MetaDictionary/ClassDefinitions", 
                    PID_MetaDefinition_Identification),
    _Properties   ( PID_ClassDefinition_Properties,
                    L"Properties",
                    PID_MetaDefinition_Identification),
    _IsConcrete   ( PID_ClassDefinition_IsConcrete,   
                    L"IsConcrete"),
	_propTypesLoaded (false),
	_BootstrapParent(0)
{
  _persistentProperties.put(_ParentClass.address());
  _persistentProperties.put(_Properties.address());
  _persistentProperties.put(_IsConcrete.address());
}


ImplAAFClassDef::~ImplAAFClassDef ()
{
	OMStrongReferenceSetIterator<OMUniqueObjectIdentification, ImplAAFPropertyDef>propertyDefinitions(_Properties);
	while(++propertyDefinitions)
	{
		ImplAAFPropertyDef *pProperty = propertyDefinitions.clearValue();
		if (pProperty)
		{
		  pProperty->ReleaseReference();
		  pProperty = 0;
		}
	}
}


AAFRESULT STDMETHODCALLTYPE
    ImplAAFClassDef::Initialize (
      const aafUID_t & classID,
      ImplAAFClassDef * pParentClass,
      const aafCharacter * pClassName,
	  aafBool isConcrete)
{
  if (!pClassName) return AAFRESULT_NULL_PARAM;

  return pvtInitialize (classID, pParentClass, pClassName, isConcrete);
}


AAFRESULT STDMETHODCALLTYPE
    ImplAAFClassDef::pvtInitialize (
      const aafUID_t & classID,
      ImplAAFClassDef * pParentClass,
      const aafCharacter * pClassName,
	  aafBool isConcrete)
{
//	ImplAAFClassDef	*oldParent;
  if (!pClassName) return AAFRESULT_NULL_PARAM;	
  if (pParentClass && pParentClass != this && !pParentClass->attached())  // a root class has itself as a parent
    return AAFRESULT_OBJECT_NOT_ATTACHED;	
	
  HRESULT hr;
  hr = ImplAAFMetaDefinition::Initialize(classID, pClassName, NULL);
  if (AAFRESULT_FAILED (hr))
    return hr;
	
  SetParent (pParentClass);
  pvtSetIsConcrete (isConcrete);

  return AAFRESULT_SUCCESS;
}


aafBool ImplAAFClassDef::pvtIsConcrete () const
{
  return _IsConcrete;
}

void ImplAAFClassDef::pvtSetIsConcrete (aafBoolean_t isConcrete)
{
  _IsConcrete = isConcrete;
}

AAFRESULT STDMETHODCALLTYPE
    ImplAAFClassDef::GetPropertyDefs (
      ImplEnumAAFPropertyDefs ** ppEnum)
{
  ImplEnumAAFPropertyDefs * theEnum = 0;

  if (NULL == ppEnum)
	return AAFRESULT_NULL_PARAM;

  theEnum = (ImplEnumAAFPropertyDefs *)CreateImpl (CLSID_EnumAAFPropertyDefs);
  if (NULL == theEnum)
  	return AAFRESULT_NOMEMORY;

  AAFRESULT hr;
  OMStrongReferenceSetIterator<OMUniqueObjectIdentification, ImplAAFPropertyDef>* iter = 
	new OMStrongReferenceSetIterator<OMUniqueObjectIdentification, ImplAAFPropertyDef>(_Properties);
  if(iter == 0)
	hr = AAFRESULT_NOMEMORY;
  else
	hr = theEnum->Initialize(&CLSID_EnumAAFPropertyDefs,this,iter);
  if (AAFRESULT_FAILED (hr))
	{
		theEnum->ReleaseReference();
		theEnum = NULL;
		return hr;
	}
	
  *ppEnum = theEnum;

  return AAFRESULT_SUCCESS;
}


AAFRESULT STDMETHODCALLTYPE
    ImplAAFClassDef::CountPropertyDefs (
      aafUInt32 *  pCount)
{
  if (! pCount) return AAFRESULT_NULL_PARAM;

  assert (pCount);
  if (!_Properties.isPresent())
	*pCount = 0;

  else  *pCount = _Properties.count ();
	
  return AAFRESULT_SUCCESS;
}


AAFRESULT STDMETHODCALLTYPE
    ImplAAFClassDef::RegisterNewPropertyDef (
      const aafUID_t &      id,
      const aafCharacter *  pName,
      ImplAAFTypeDef *      pTypeDef,
      aafBool               isOptional,
      aafBool               isIsUniqueIdentifier,
      ImplAAFPropertyDef ** ppPropDef)
{
  // This will only work if this class has not yet been registered.

  if (! pTypeDef)
	return AAFRESULT_NULL_PARAM;

  AAFRESULT hr;
  ImplAAFDictionarySP pDict;
  hr = GetDictionary (&pDict);
  if (AAFRESULT_SUCCEEDED (hr))
	{
	  // A dictionary has been identified; see if we're registered in
	  // it.
	  aafUID_t thisClassID;

	  // get our class ID
	  hr = GetAUID (&thisClassID);
	  assert (AAFRESULT_SUCCEEDED (hr));

	  if (pDict->PvtIsClassPresent(thisClassID))
		{
		  // pClassDef is unused; we only want to know the result of
		  // the lookup.
		  return AAFRESULT_OBJECT_ALREADY_ATTACHED;
		}
	}

  // If we're here, we're not already registered.  OK to continue.

  aafUID_t typeId;
  hr = pTypeDef->GetAUID (&typeId);
  if (AAFRESULT_FAILED (hr))
	return hr;

  // Check that this class or any parent class is already uniquely
  // identified.
  if (isIsUniqueIdentifier)
  {
    aafBoolean_t alreadyUniquelyIdentified = kAAFFalse;
    hr = IsUniquelyIdentified(&alreadyUniquelyIdentified);
    if (AAFRESULT_FAILED(hr))
      return hr;
    
    // "There can be only one" unique identifier per class.
    if (alreadyUniquelyIdentified)
      return AAFRESULT_ALREADY_UNIQUELY_IDENTIFIED;
  }

	//GMSB! 05-Dec-2001 Documentation says AAFRESULT_BAD_PARAM for this case (but AAFRESULT_BAD_PARAM doesn't really exist!)
	//  and the code had AAFRESULT_SUCCESS before (which doesn't seem right, because ppPropDef won't have been filled in)
	if(PvtIsPropertyDefRegistered(id))
		return AAFRESULT_INVALID_PARAM;
	else
	{
		bool	isDuplicate;
		
		hr = pDict->PvtIsPropertyDefDuplicate(id, this, &isDuplicate);
		if (AAFRESULT_FAILED(hr))
			return hr;
		if(isDuplicate)
			return AAFRESULT_PROPERTY_DUPLICATE;
		return pvtRegisterPropertyDef (id,
									pName,
									typeId,
									isOptional,
									isIsUniqueIdentifier,
									ppPropDef);
	}
}	


AAFRESULT STDMETHODCALLTYPE
    ImplAAFClassDef::RegisterOptionalPropertyDef (
      const aafUID_t &      id,
      const aafCharacter *  pName,
      ImplAAFTypeDef *      pTypeDef,
      ImplAAFPropertyDef ** ppPropDef)
{
  AAFRESULT hr;
  if (ppPropDef)
    *ppPropDef = NULL;
  if (! pTypeDef)
	return AAFRESULT_NULL_PARAM;

  ImplAAFDictionarySP pDict;
  hr = GetDictionary (&pDict);
  assert (AAFRESULT_SUCCEEDED (hr));

  aafUID_t myAuid;
  hr = GetAUID (&myAuid);
  assert (AAFRESULT_SUCCEEDED (hr));

  // Test to see if this class is axiomatic; if so, then we can't
  // augment it.
  if (pDict->IsAxiomaticClass (myAuid))
  {
		return AAFRESULT_NOT_EXTENDABLE;
	}

  aafUID_t typeId;
  hr = pTypeDef->GetAUID (&typeId);
  if (AAFRESULT_FAILED (hr))
	return hr;

	//GMSB! 05-Dec-2001 Documentation says AAFRESULT_BAD_PARAM for this case (but AAFRESULT_BAD_PARAM doesn't really exist!)
	//  and the code had AAFRESULT_SUCCESS before (which doesn't seem right, because ppPropDef won't have been filled in)
	if(PvtIsPropertyDefRegistered(id))
		return AAFRESULT_INVALID_PARAM;
	else
	{
		bool	isDuplicate;
		
		hr = pDict->PvtIsPropertyDefDuplicate(id, this, &isDuplicate);
		if (AAFRESULT_FAILED(hr))
			return hr;
		if(isDuplicate)
			return AAFRESULT_PROPERTY_DUPLICATE;
		ImplAAFPropertyDefSP pNewPropertyDef;
	  hr = pvtRegisterPropertyDef (id,
								 pName,
								 typeId,
								 kAAFTrue,
								 kAAFFalse, /* cannot be a unique identifier */
								 &pNewPropertyDef);
								 
		if (AAFRESULT_SUCCEEDED(hr))
		{
		  // Check to make sure that the new optional property
		  // is complete for this class defintion.
		  hr = pNewPropertyDef->CompleteClassRegistration();
		  if (AAFRESULT_FAILED(hr))
		  {
		    // remove the invalid property definition from the set of properties.
		    _Properties.removeValue(pNewPropertyDef);
		    pNewPropertyDef->ReleaseReference(); // remove the reference from the set.
		  }
		  else
		  {
		    if (ppPropDef)
		    {
		      *ppPropDef = pNewPropertyDef;
		      (*ppPropDef)->AcquireReference();
		    }
		  }
		}
	  		 
    return hr;	  
	}
}


  // Low-level recursive methods the OM find method instead of a linear search
  // that will necessarily load all of the property definitions for the class
  // definition instance.
AAFRESULT
    ImplAAFClassDef::generalLookupPropertyDef (
      aafUID_constref propId,
      ImplAAFPropertyDef ** ppPropDef)
{
	AAFRESULT result = AAFRESULT_SUCCESS;
  // NOTE: The following type cast is temporary. It should be removed as soon
	// as the OM has a declarative sytax to include the type
	// of the key used in the set. (trr:2000-MAR-11)
	if (_Properties.find((*reinterpret_cast<const OMObjectIdentification *>(&propId)),
                             *ppPropDef))
	{
		assert(NULL != *ppPropDef);
    (*ppPropDef)->AcquireReference();
	}
	else
	{
    // no recognized class guid in dictionary
    result = AAFRESULT_NO_MORE_OBJECTS; // s/b AAFRESULT_PROPERTY_NOT_FOUND
	}

	return (result);
}

/******/
bool
    ImplAAFClassDef::PvtIsPropertyDefRegistered (
      aafUID_constref propId)
{
  // NOTE: The following type cast is temporary. It should be removed as soon
	// as the OM has a declarative sytax to include the type
	// of the key used in the set. (trr:2000-MAR-11)
	return (_Properties.contains((*reinterpret_cast<const OMObjectIdentification *>(&propId))));
}

AAFRESULT /*STDMETHODCALLTYPE*/
    ImplAAFClassDef::generalLookupPropertyDef (
      const pvtPropertyIdentifier & propId,
      ImplAAFPropertyDef ** ppPropDef)
{
		///!!!JeffB: Leave this in original loop form until the OM  genericizes the lookup key
  if (! ppPropDef)
	return AAFRESULT_NULL_PARAM;

  ImplEnumAAFPropertyDefsSP pPropEnum;
  ImplAAFPropertyDefSP pPropDef;

  check_result (GetPropertyDefs (&pPropEnum));
  assert (pPropEnum);

  aafUInt32 count;
  check_result (CountPropertyDefs (&count));
  aafUInt32 i;
  for (i = 0; i < count; i++)
	{
	  assert (pPropEnum);
	  check_result (pPropEnum->NextOne (&pPropDef));
	  assert (pPropDef);
	  if (propId.DoesMatch (pPropDef))
		{
		  // Yup, this is the one.
		  assert (ppPropDef);
		  *ppPropDef = pPropDef;
		  assert (*ppPropDef);
		  (*ppPropDef)->AcquireReference ();

		  // Get us outta the loop
		  return AAFRESULT_SUCCESS;
		}
	  else
		{
		  // nope, this prop doesn't match.  Try the next one.
		  pPropDef = 0;
		}
	}
  // If we're here, there aren't any more props so that prop
  // wasn't found in this class; try any base classes.
  ImplAAFClassDefSP parentSP;
  aafBool			isRootObject;
  AAFRESULT hr = IsRoot (&isRootObject);
  if (AAFRESULT_FAILED (hr))
	  return hr;
  if(isRootObject)
	  return AAFRESULT_NO_MORE_OBJECTS;
  else
  {
	  hr = GetParent (&parentSP);
	  if (AAFRESULT_FAILED (hr))
	     return hr;
      return parentSP->generalLookupPropertyDef (propId, ppPropDef);
  }
}


AAFRESULT STDMETHODCALLTYPE
    ImplAAFClassDef::LookupPropertyDef (
      const aafUID_t & propID,
      ImplAAFPropertyDef ** ppPropDef) const
{
  const pvtPropertyIdentifierAUID generalPropId = propID;

  // cast away bitwise const-ness; maintaining conceptual const-ness
  return ((ImplAAFClassDef*)this)->generalLookupPropertyDef
	(generalPropId, ppPropDef);
}



AAFRESULT STDMETHODCALLTYPE
    ImplAAFClassDef::LookupPropertyDefbyOMPid (
      OMPropertyId omPid,
      ImplAAFPropertyDef ** ppPropDef) const
{
  const pvtPropertyIdentifierOMPid generalPropId = omPid;

  return ((ImplAAFClassDef*)this)->generalLookupPropertyDef
	(generalPropId, ppPropDef);
}


AAFRESULT STDMETHODCALLTYPE
    ImplAAFClassDef::GetParent (
      ImplAAFClassDef ** ppClassDef)
{
	aafBool		isRoot;
	if (! ppClassDef) return AAFRESULT_NULL_PARAM;
	
	IsRoot (&isRoot);
	if(isRoot)
		return AAFRESULT_IS_ROOT_CLASS;

	*ppClassDef = bootstrapClassWeakReference(_ParentClass);
	assert(*ppClassDef != NULL);
	if(*ppClassDef != NULL)
		(*ppClassDef)->AcquireReference ();
	
	return AAFRESULT_SUCCESS;
}

AAFRESULT STDMETHODCALLTYPE
    ImplAAFClassDef::IsRoot (
      aafBool * isRoot)
{
	aafUID_t	uid;
	
	if (! isRoot) return AAFRESULT_NULL_PARAM;
	
	GetAUID (&uid);
	*isRoot = memcmp(&uid, &AUID_AAFObject, sizeof(aafUID_t)) == 0;
  	if (!(*isRoot))
	  *isRoot = memcmp(&uid, &AUID_AAFMetaDefinition, sizeof(aafUID_t)) == 0;
  	if (!(*isRoot))
	  *isRoot = memcmp(&uid, &AUID_AAFMetaDictionary, sizeof(aafUID_t)) == 0;

	return AAFRESULT_SUCCESS;
}


AAFRESULT STDMETHODCALLTYPE
    ImplAAFClassDef::IsConcrete (
      aafBool * pResult)
{
	if (! pResult) return AAFRESULT_NULL_PARAM;
	*pResult = pvtIsConcrete ();
	return AAFRESULT_SUCCESS;
}


// Check that this class or any parent class is already uniquely
// identified.
AAFRESULT STDMETHODCALLTYPE
    ImplAAFClassDef::IsUniquelyIdentified (
      aafBoolean_t * isUniquelyIdentified)
{
	if (! isUniquelyIdentified)
    return AAFRESULT_NULL_PARAM;
	
  if (NULL != pvtGetUniqueIdentifier())
    *isUniquelyIdentified = kAAFTrue;
  else
    *isUniquelyIdentified = kAAFFalse;


  return AAFRESULT_SUCCESS;
}


// Check that this class or any parent class is already uniquely
// identified.
AAFRESULT STDMETHODCALLTYPE
    ImplAAFClassDef::GetUniqueIdentifier (
      ImplAAFPropertyDef ** ppUniqueIdentifier)
{
	if (! ppUniqueIdentifier)
    return AAFRESULT_NULL_PARAM;

  *ppUniqueIdentifier = pvtGetUniqueIdentifier();
  if (*ppUniqueIdentifier)
  {
    (*ppUniqueIdentifier)->AcquireReference();
    return AAFRESULT_SUCCESS;
  }
  else
  {
    // The unique identifier property in property definition
    // is optional. So if our low-level search did not find
    // a property definition with this property we should 
    // return corresponding result code.
    return AAFRESULT_PROP_NOT_PRESENT;
  }
}

const OMPropertyDefinition* ImplAAFClassDef::propertyDefinition(
                            const OMUniqueObjectIdentification& id ) const
{
  ImplAAFPropertyDef* pDef = 0;

  // The result bool return value is intentionally ignored. A null
  // pDef value returned if the find() failed.
  _Properties.find(id, pDef);

  return pDef;
}

const OMPropertyDefinition* ImplAAFClassDef::propertyDefinition(
 		            const OMStorable* pDstStorable,
			    const OMPropertyDefinition *pSrcOMPropertyDef )
{
  ImplAAFPropertyDef *pSrcAAFPropertyDef =
	  dynamic_cast<ImplAAFPropertyDef*>( const_cast<OMPropertyDefinition*>(pSrcOMPropertyDef) );

  if ( !pSrcAAFPropertyDef ) {
    return 0;
  }

  HRESULT hr;

  // Get the source AUID.  The destintion auid must be identical.
  aafUID_t auid;
  hr = pSrcAAFPropertyDef->GetAUID(&auid);
  if ( AAFRESULT_SUCCESS != hr ) {
    return 0;
  }

    // Now lookup the type id using the dst storable's dictionary.
  ImplAAFObject* pDstObj = dynamic_cast<ImplAAFObject*>( const_cast<OMStorable*>(pDstStorable) );
  if ( !pDstObj ) {
    return 0;
  }

  ImplAAFPropertyDef* pDstPropertyDef;
  hr = LookupPropertyDef( auid, &pDstPropertyDef ); 
  if ( hr == AAFRESULT_NO_MORE_OBJECTS ) {

    // Get the source name, the desitnation name must be identical.
    aafUInt32 srcNameBufLen;
    hr = pSrcAAFPropertyDef->GetNameBufLen(&srcNameBufLen);
    if ( AAFRESULT_SUCCESS != hr ) {
      return 0;
    }
    
    // Allocate space for src name, round srcNameBufLen up to the nearest wchar_t size.
    int srcNameLen = (srcNameBufLen+1)/2;
    wchar_t *srcName = new wchar_t [srcNameLen];
    if ( !srcName ) {
      return 0;
    }
    hr = pSrcAAFPropertyDef->GetName( srcName, srcNameLen*sizeof(wchar_t) );
    if ( AAFRESULT_SUCCESS != hr ) {
      return 0;
    }
    
    // Get the source type def, and lookup the destination type def.
    ImplAAFSmartPointer<ImplAAFTypeDef> spTypeDef;
    hr = pSrcAAFPropertyDef->GetTypeDef( &spTypeDef );
    if ( AAFRESULT_SUCCESS != hr ) {
      return 0;
    }
    
    // Sanity check.  There is not reason for this to be called
    // for other than optional properties.
    aafBool isOptional;
    hr = pSrcAAFPropertyDef->GetIsOptional(&isOptional);
    if ( AAFRESULT_SUCCESS != hr ) {
      return 0;
    }
    if ( !isOptional ) {
      return 0;
    }
    
    aafUID_t srcTypeId;
    hr = spTypeDef->GetAUID( &srcTypeId );
    if ( AAFRESULT_SUCCESS != hr ) {
      return 0;
    }
    
    ImplAAFSmartPointer<ImplAAFDictionary> spDstDict;
    hr = pDstObj->GetDictionary(&spDstDict);
    if ( AAFRESULT_SUCCESS != hr ) {
      return 0;
    }
    
    // Get the destination type def.
    ImplAAFSmartPointer<ImplAAFTypeDef> spDstTypeDef;
    hr = spDstDict->LookupTypeDef(srcTypeId, &spDstTypeDef);
    assert( AAFRESULT_SUCCESS == hr );
    
    hr = RegisterOptionalPropertyDef( auid, srcName, spDstTypeDef, &pDstPropertyDef );
    if ( AAFRESULT_SUCCESS != hr ) {
      return 0;
    }
  }
  else if ( AAFRESULT_SUCCESS != hr ) {
    return 0;
  }
  
  hr = pDstObj->CreatePropertyInstanceAndAdd( pDstPropertyDef );
  if ( AAFRESULT_SUCCESS != hr ) {
    return 0;
  }
  
  return pDstPropertyDef;
}

// Find the unique identifier property defintion for this class or any parent class
// (RECURSIVE)
ImplAAFPropertyDef * ImplAAFClassDef::pvtGetUniqueIdentifier(void) // result is NOT reference counted.
{
  ImplAAFPropertyDef *result = NULL;

  aafBoolean_t isRoot, isUniquelyIdentified;
  AAFRESULT hr = IsRoot(&isRoot);
  if (AAFRESULT_FAILED(hr))
    return NULL;

  // Look for the unique identifier in the parent class(es) before looking in the
  // current class definition.
  if (!isRoot)
  {
    ImplAAFClassDefSP pParent;
    hr = GetParent(&pParent);
    if (AAFRESULT_FAILED(hr))
      return NULL;

    result = pParent->pvtGetUniqueIdentifier();
  }

  // If we have not already found the unique identifier in a parent
  // class then continue to look in this class' propert definitions.
  if (!result)
  {
	  OMStrongReferenceSetIterator<OMUniqueObjectIdentification, ImplAAFPropertyDef>propertyDefinitions(_Properties);
	  while(++propertyDefinitions)
	  {
		  ImplAAFPropertyDef *pProperty = propertyDefinitions.value();
		  if (pProperty)
		  {
		    hr = pProperty->GetIsUniqueIdentifier(&isUniquelyIdentified);
        if (AAFRESULT_FAILED(hr))
          return NULL;
        if (isUniquelyIdentified)
        {
          result = pProperty;
          break;
        }
		  }
	  }
  }

  return result;
}


// SetParent is SDK INTERNAL
AAFRESULT STDMETHODCALLTYPE
    ImplAAFClassDef::SetParent (
      ImplAAFClassDef *pClassDef)
{
	if (! pClassDef)
		return AAFRESULT_NULL_PARAM;

	_ParentClass = pClassDef;
  return AAFRESULT_SUCCESS;
}

AAFRESULT STDMETHODCALLTYPE
    ImplAAFClassDef::SetBootstrapParent (
	ImplAAFClassDef *pClassDef)
{
	if (! pClassDef)
		return AAFRESULT_NULL_PARAM;
	
	_BootstrapParent = pClassDef;
	return AAFRESULT_SUCCESS;
}

AAFRESULT STDMETHODCALLTYPE
    ImplAAFClassDef::CreateInstance (
      ImplAAFObject ** ppObject)
{
  if (! ppObject)
	return AAFRESULT_NULL_PARAM;

  if (! pvtIsConcrete ())
	return AAFRESULT_ABSTRACT_CLASS;

  AAFRESULT hr;
  ImplAAFDictionarySP pDict;
  hr = GetDictionary (&pDict);
  if (AAFRESULT_FAILED (hr))
	return hr;
  assert (pDict);
  ImplAAFObject * pObj =
	pDict->CreateAndInit (this);
  assert (pObj);
  *ppObject = pObj;
  // don't bother with *ppObject->addref() and pObj->ReleaseRef();
  // consider the ref count to be transferred from pObj to *ppObject.
  pObj = 0;  // to keep boundschecker happy
  return AAFRESULT_SUCCESS;
}


AAFRESULT STDMETHODCALLTYPE
    ImplAAFClassDef::pvtRegisterPropertyDef (
      const aafUID_t &      id,
      const aafCharacter *  pName,
      const aafUID_t &      typeId,
      aafBool               isOptional,
      aafBool               isUniqueIdentifier,
      ImplAAFPropertyDef ** ppPropDef)
{
  if (! pName) return AAFRESULT_NULL_PARAM;

  ImplAAFDictionarySP pDict;
  ImplAAFPropertyDefSP pd;

  check_result (GetDictionary (&pDict));
  assert (pDict);
  OMPropertyId omPid;
  check_result (pDict->GenerateOmPid (id, omPid));

  AAFRESULT hr;
  hr = pDict->CreateMetaInstance(AUID_AAFPropertyDef, (ImplAAFMetaDefinition **)&pd);
  if (AAFRESULT_FAILED (hr))
	return hr;

  check_result (pd->pvtInitialize (id,
								omPid,
								pName,
								typeId,
								isOptional,
                isUniqueIdentifier));

//  ImplAAFPropertyDef * pdTemp = pd;
//  _Properties.appendValue(pdTemp);
//  pdTemp->AcquireReference ();
  pvtRegisterExistingPropertyDef(pd);

  if (ppPropDef)
  {
	*ppPropDef = pd;
	(*ppPropDef)->AcquireReference ();
  }

  return AAFRESULT_SUCCESS;
}


AAFRESULT STDMETHODCALLTYPE
ImplAAFClassDef::pvtRegisterExistingPropertyDef
(ImplAAFPropertyDef * pPropDef)
{
  if (!pPropDef)
	return AAFRESULT_NULL_PARAM;
  if (pPropDef->attached())
	return AAFRESULT_OBJECT_ALREADY_ATTACHED;

  _Properties.appendValue(pPropDef);
  pPropDef->AcquireReference ();

  return AAFRESULT_SUCCESS;
}


//
// Implementations of private class
//
aafBool ImplAAFClassDef::pvtPropertyIdentifierOMPid:: DoesMatch
  (const ImplAAFPropertyDef * pTestPropDef) const
{
  OMPropertyId testPid;
  assert (pTestPropDef);
  testPid = pTestPropDef->OmPid ();
  return (_id == testPid) ? kAAFTrue : kAAFFalse;
}

aafBool ImplAAFClassDef::pvtPropertyIdentifierAUID::DoesMatch
  (const ImplAAFPropertyDef * pTestPropDef) const
{
  aafUID_t testUID;
  assert (pTestPropDef);
  AAFRESULT hr = pTestPropDef->GetAUID (&testUID);
  assert (AAFRESULT_SUCCEEDED (hr));
  return (EqualAUID (&_id, &testUID) ? kAAFTrue : kAAFFalse);
}


void ImplAAFClassDef::AssurePropertyTypesLoaded ()
{
  if (! _propTypesLoaded)
	{
	  _propTypesLoaded = true;

	  AAFRESULT hr;
	  ImplAAFClassDefSP spDef;
	  spDef	= this;
	  assert (spDef);
	  while (1)
		{
		  // Loop through this class and all its parents
		  ImplEnumAAFPropertyDefsSP pdEnumSP;
		  hr = spDef->GetPropertyDefs (&pdEnumSP);
		  assert (AAFRESULT_SUCCEEDED (hr));

		  ImplAAFPropertyDefSP propDefSP;

		  // Loop through all properties of this class
		  while (AAFRESULT_SUCCEEDED (pdEnumSP->NextOne (&propDefSP)))
			{
			  // return value unused; just force the lookup.
			  propDefSP->type ();
			}

		  // Look at the parent of this class
		  ImplAAFClassDefSP parentSP;
		  aafBool	isRoot;

		  hr = spDef->IsRoot(&isRoot);
		  assert (AAFRESULT_SUCCEEDED (hr));
		  if (isRoot)
			break;
		  hr = spDef->GetParent (&parentSP);
		  assert (AAFRESULT_SUCCEEDED (hr));
//!!!		  if (parentSP == NULL)
//		  {
//			 ImplAAFDictionary *pDict;
//			 ImplAAFClassDef	*curClass;
//			aafUID_t	uid;
//
//			hr = spDef->GetAUID (&uid);
//		    assert (AAFRESULT_SUCCEEDED (hr));
//			hr = spDef->GetDictionary(&pDict);
//		    assert (AAFRESULT_SUCCEEDED (hr));
//			hr = pDict->LookupClassDef (uid, &curClass);
//		    assert (AAFRESULT_SUCCEEDED (hr));
//			hr = curClass->GetParent(&parentSP);
//		    assert (AAFRESULT_SUCCEEDED (hr));
//		  }

		  assert (parentSP != NULL);
		  spDef = parentSP;
		}
	}
}






// override from OMStorable.
const OMClassId& ImplAAFClassDef::classId(void) const
{
  return (*reinterpret_cast<const OMClassId *>(&AUID_AAFClassDef));
}

// Override callbacks from OMStorable
void ImplAAFClassDef::onSave(void* clientContext) const
{
  ImplAAFMetaDefinition::onSave(clientContext);
}

void ImplAAFClassDef::onRestore(void* clientContext) const
{
  ImplAAFMetaDefinition::onRestore(clientContext);
}

// Method is called after associated class has been added to MetaDictionary.
// If this method fails the class is removed from the MetaDictionary and the
// registration method will fail.
HRESULT ImplAAFClassDef::CompleteClassRegistration(void)
{
  AAFRESULT result = AAFRESULT_SUCCESS;

  // Make sure that all of the registration of properties can be
  // completed.
  ImplAAFPropertyDef *pProperty;
	OMStrongReferenceSetIterator<OMUniqueObjectIdentification, ImplAAFPropertyDef>propertyDefinitions(_Properties);
	while(++propertyDefinitions && AAFRESULT_SUCCEEDED(result))
	{
		pProperty = propertyDefinitions.value();
		assert(pProperty); // there should never be a NULL object in a set!
		if (pProperty)
		{
		  result = pProperty->CompleteClassRegistration();
		}
	}

  return result;
}

void ImplAAFClassDef::onCopy(void* clientContext) const
{
  ImplAAFMetaDefinition::onCopy(clientContext);
  
  if ( clientContext ) {
    ImplAAFCloneResolver* pResolver = reinterpret_cast<ImplAAFCloneResolver*>(clientContext);
    pResolver->ResolveWeakReference(_ParentClass);
  }
}
