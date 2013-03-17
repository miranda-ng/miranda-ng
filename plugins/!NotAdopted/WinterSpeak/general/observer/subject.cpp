//==============================================================================
// General Code, © 2002 Ryan Winter
//==============================================================================

#include "subject.h"

#include <general/debug/debug.h>

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
Subject::Subject()
    :
    m_observer_list()
{
    CLASSCERR("Subject::Subject");
}

//------------------------------------------------------------------------------
Subject::~Subject()
{
    CLASSCERR("Subject::~Subject");
}

//------------------------------------------------------------------------------
void
Subject::notify()
{
    CLASSCERR("Subject::notify");

    // call update for all observers in the list
    for (std::list<Observer *>::iterator iter = m_observer_list.begin();
        iter != m_observer_list.end();
        ++iter)
    {
        (*iter)->update(*this);
    }
}

//------------------------------------------------------------------------------
void
Subject::attach(Observer &observer)
{
    CLASSCERR("Subject::attach(" << &observer << ")");

    for (std::list<Observer *>::iterator iter = m_observer_list.begin();
        iter != m_observer_list.end();
        ++iter)
    {
        if (*iter == &observer)
        {
            return;
        }
    }

    m_observer_list.push_back(&observer);
}

//------------------------------------------------------------------------------
void
Subject::detach(const Observer &observer)
{
    CLASSCERR("Subject::detach(" << &observer << ")");

    for (std::list<Observer *>::iterator iter = m_observer_list.begin();
        iter != m_observer_list.end();
        ++iter)
    {
        if (*iter == &observer)
        {
            m_observer_list.erase(iter);
        }
    }
}

//==============================================================================

