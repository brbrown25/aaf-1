//@doc
//@class    AAFRefValue | Implementation class for AAFPropertyValue
#ifndef __ImplAAFRefValue_h__
#define __ImplAAFRefValue_h__


/***********************************************************************
 *
 *              Copyright (c) 2000 Avid Technology, Inc.
 *
 * Permission to use, copy and modify this software and accompanying 
 * documentation, and to distribute and sublicense application software
 * incorporating this software for any purpose is hereby granted, 
 * provided that (i) the above copyright notice and this permission
 * notice appear in all copies of the software and related documentation,
 * and (ii) the name Avid Technology, Inc. may not be used in any
 * advertising or publicity relating to the software without the specific,
 *  prior written permission of Avid Technology, Inc.
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


class ImplAAFRoot;

#ifndef __ImplAAFPropertyValue_h__
#include "ImplAAFPropertyValue.h"
#endif

#ifndef __ImplAAFPropValData_h__
#include "ImplAAFPropValData.h"
#endif

class ImplAAFTypeDefObjectRef;

class OMProperty;
//class OMReferenceProperty; // TBD: the base class for singleton references


//class ImplAAFRefValue : public ImplAAFPropertyValue // TBD:
class ImplAAFRefValue : public ImplAAFPropValData
{
public:

protected:
  //
  // Constructor/destructor
  //
  //********
  ImplAAFRefValue ();
  virtual ~ImplAAFRefValue ();

  // non-published method to initialize this object.
  // NOTE: The given property's type must be a reference type.
  AAFRESULT Initialize (const ImplAAFTypeDefObjectRef *referenceType,
                        OMProperty *property);
  
  // Return the propertyType as a referenceType.
  const ImplAAFTypeDefObjectRef *referenceType(void) const;             
  
public:

  virtual AAFRESULT STDMETHODCALLTYPE WriteTo(OMProperty* pOmProp);
};

//
// smart pointer
//

#ifndef __ImplAAFSmartPointer_h__
// caution! includes assert.h
#include "ImplAAFSmartPointer.h"
#endif

typedef ImplAAFSmartPointer<ImplAAFRefValue> ImplAAFRefValueSP;

#endif // ! __ImplAAFRefValue_h__
