#include "Common.h"
#include "Subject.h"
#include "Observer.h"


//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
Subject::Subject() : m_observer_list()
{
}

//------------------------------------------------------------------------------
Subject::~Subject()
{
}

//------------------------------------------------------------------------------
void Subject::notify()
{
    // call update for all observers in the list
    for (std::list<Observer *>::iterator iter = m_observer_list.begin(); iter != m_observer_list.end(); ++iter)
    {
        (*iter)->update(*this);
    }
}

//------------------------------------------------------------------------------
void Subject::attach(Observer &observer)
{
    for (std::list<Observer *>::iterator iter = m_observer_list.begin(); iter != m_observer_list.end(); ++iter)
    {
        if (*iter == &observer)
        {
            return;
        }
    }
    m_observer_list.push_back(&observer);
}

//------------------------------------------------------------------------------
void Subject::detach(const Observer &observer)
{
    for (std::list<Observer *>::iterator iter = m_observer_list.begin(); iter != m_observer_list.end(); ++iter)
    {
        if (*iter == &observer)
        {
            m_observer_list.erase(iter);
        }
    }
}

//==============================================================================


