// EventLog.cpp — Implementation. No SFML. No STL. Lab style.
#include "EventLog.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static Event makeZeroEvent()
{
    Event e;
    e.type     = EV_DONE;
    e.nodeA    = -1;
    e.nodeB    = -1;
    e.key      = -1;
    e.result   = -1;
    e.side     = -1;
    e.mode     = -1;
    e.oldValue = -1;
    e.newValue = -1;
    e.extra    = -1;
    return e;
}

// ---------------------------------------------------------------------------
// Constructors / destructor
// ---------------------------------------------------------------------------
EventLog::EventLog()
    : enabled(true)
    , m_data(nullptr)
    , m_count(0)
    , m_capacity(0)
{
    m_capacity = INITIAL_CAP;
    m_data = new Event[m_capacity];
}

EventLog::EventLog(const EventLog& other)
    : enabled(other.enabled)
    , m_data(nullptr)
    , m_count(0)
    , m_capacity(0)
{
    copyFrom(other);
}

EventLog& EventLog::operator=(const EventLog& other)
{
    if (this != &other)
    {
        freeData();
        enabled = other.enabled;
        copyFrom(other);
    }
    return *this;
}

EventLog::~EventLog()
{
    freeData();
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------
void EventLog::add(int type,
                   int nodeA,
                   int nodeB,
                   int key,
                   int result,
                   int side,
                   int mode,
                   int oldValue,
                   int newValue,
                   int extra)
{
    if (!enabled) return;

    if (m_count == m_capacity) grow();

    Event& e = m_data[m_count];
    e.type     = type;
    e.nodeA    = nodeA;
    e.nodeB    = nodeB;
    e.key      = key;
    e.result   = result;
    e.side     = side;
    e.mode     = mode;
    e.oldValue = oldValue;
    e.newValue = newValue;
    e.extra    = extra;
    ++m_count;
}

void EventLog::clear()
{
    m_count = 0;
}

int EventLog::getCount() const
{
    return m_count;
}

Event EventLog::getAt(int index) const
{
    if (index < 0 || index >= m_count) return makeZeroEvent();
    return m_data[index];
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------
void EventLog::grow()
{
    int newCap = m_capacity * 2;
    Event* newData = new Event[newCap];
    for (int i = 0; i < m_count; ++i)
        newData[i] = m_data[i];
    delete[] m_data;
    m_data     = newData;
    m_capacity = newCap;
}

void EventLog::copyFrom(const EventLog& other)
{
    m_capacity = other.m_capacity;
    m_count    = other.m_count;
    m_data     = new Event[m_capacity];
    for (int i = 0; i < m_count; ++i)
        m_data[i] = other.m_data[i];
}

void EventLog::freeData()
{
    delete[] m_data;
    m_data     = nullptr;
    m_count    = 0;
    m_capacity = 0;
}
