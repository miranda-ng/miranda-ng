#pragma once


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