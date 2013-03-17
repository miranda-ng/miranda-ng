#ifndef guard_general_observer_observer_observer_h
#define guard_general_observer_observer_observer_h
//==============================================================================
// General Code, © 2002 Ryan Winter
//==============================================================================

class Subject;

class Observer
{
  public:
    virtual ~Observer();

    //--------------------------------------------------------------------------
    // Description : Called by a subject that this observer is observing
    //               to signify a change in state
    // Parameters  : subject - the subject that changed
    //--------------------------------------------------------------------------
    virtual void update(Subject &subject) = 0;
};

//==============================================================================
//
//  Summary     : Observer side of the observer pattern
//
//  Description : 
//
//==============================================================================

#endif
