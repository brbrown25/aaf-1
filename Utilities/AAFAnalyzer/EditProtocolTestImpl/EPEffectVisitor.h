//=---------------------------------------------------------------------=
//
// $Id$
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
//=---------------------------------------------------------------------=

#ifndef __EPEffectVisitor_h_
#define __EPEffectVisitor_h_

//Edit Protocol Analyzer Base files
#include <EPTypedVisitor.h>
#include <EPTypedObjNode.h>

//Ax files

//STL files
#include <stack>


namespace aafanalyzer {

class DetailLevelTestResult;

using namespace boost;
using namespace std;

class EPEffectVisitor : public EPTypedVisitor
{

  public:
  
    EPEffectVisitor( wostream& log, shared_ptr<EdgeMap> spEdgeMap );

    virtual ~EPEffectVisitor();

    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPVideoDissolveEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPSMPTEVideoWipeEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPVideoSpeedControlEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPVideoRepeatEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPVideoFlipEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPVideoFlopEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPVideoFlipFlopEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPVideoPositionEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPVideoCropEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPVideoScaleEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPVideoRotateEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPVideoCornerPinningEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPAlphaWithVideoKeyEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPSeparateAlphaKeyEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPLuminanceKeyEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPChromaKeyEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPMonoAudioGainEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPMonoAudioPanEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPSingleParameterAudioDissolveEffect>& node);
    virtual bool PreOrderVisit( EPTypedObjNode<IAAFOperationGroup, EPTwoParameterAudioDissolveEffect>& node);
    virtual bool PreOrderVisit( AAFTypedObjNode<IAAFTransition>& node );
    
    virtual bool PostOrderVisit( AAFTypedObjNode<IAAFOperationGroup>& node );
    virtual bool PostOrderVisit( AAFTypedObjNode<IAAFTransition>& node );
    
    shared_ptr<DetailLevelTestResult> GetResult();
    
  private:
  
    wostream& _log;
    shared_ptr<EdgeMap> _spEdgeMap;
    stack<bool> _isParentTransition;
    
    shared_ptr<DetailLevelTestResult> _spResult;
    
    bool VeirfyTransitionRequirement( AAFTypedObjNode<IAAFOperationGroup>& node, bool withinTransition, const AxString& reqId, const AxString& type );
    
    // prohibited
    EPEffectVisitor();
    EPEffectVisitor( const EPEffectVisitor& );
    EPEffectVisitor& operator=( const EPEffectVisitor& );
  
};

} // end of namespace diskstream

#endif /*__EPEffectVisitor_h_*/