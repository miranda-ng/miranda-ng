#ifndef guard_general_observer_subject_h
#define guard_general_observer_subject_h
//==============================================================================
// General Code, © 2002 Ryan Winter
//==============================================================================

#include "observer.h"

#include <list>

class Subject
{
  public:
    Subject();
    virtual ~Subject();

    //--------------------------------------------------------------------------
    // Description : Notify all observers of a state change
    //--------------------------------------------------------------------------
    void notify();

    //--------------------------------------------------------------------------
    // Description : Attach an observer to the subject
    // Parameters  : observer - the observer
    //--------------------------------------------------------------------------
    void attach(Observer &observer);

    //--------------------------------------------------------------------------
    // Description : Detach an observer from the subject
    // Parameters  : observer - the observer
    //--------------------------------------------------------------------------
    void detach(const Observer &observer);

  private:
    //--------------------------------------------------------------------------
    // Description : Disallow assignment operator and copy constructor
    //--------------------------------------------------------------------------
    Subject(const Subject &rhs);
    const Subject & operator=(const Subject &rhs);

    std::list<Observer *> m_observer_list;
};

//==============================================================================
//
//  Summary     : Subject side of the observer pattern
//
//  Description : Instances of this class are what is observered by observers
//
//==============================================================================

#endif
