//@doc
//@class    AAFGPITrigger | Implementation class for AAFGPITrigger
#ifndef __ImplAAFGPITrigger_h__
#define __ImplAAFGPITrigger_h__

/***********************************************\
*                                               *
* Advanced Authoring Format                     *
*                                               *
* Copyright (c) 1998-1999 Avid Technology, Inc. *
*                                               *
\***********************************************/

#ifndef __ImplAAFEvent_h__
#include "ImplAAFEvent.h"
#endif


class ImplAAFGPITrigger : public ImplAAFEvent
{
public:
  //
  // Constructor/destructor
  //
  //********
  ImplAAFGPITrigger ();

protected:
  virtual ~ImplAAFGPITrigger ();

public:


  //****************
  // GetActiveState()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    GetActiveState
        // @parm [out, retval] pointer to the result
        (aafBool *  pActiveState);


  //****************
  // SetActiveState()
  //
  virtual AAFRESULT STDMETHODCALLTYPE
    SetActiveState
        // @parm [in] the active state of the trigger
        (aafBool  ActiveState);

protected:
  OMFixedSizeProperty<aafBool> _activeState;
};

#endif // ! __ImplAAFGPITrigger_h__


