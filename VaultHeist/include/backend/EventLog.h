#pragma once
// EventLog.h — Pure data class. No SFML. No STL containers. Lab style.
// Records a sequence of tree/heap animation events for the GUI animator to replay.

// ---------------------------------------------------------------------------
// Event type constants
// ---------------------------------------------------------------------------
const int EV_VISIT              = 0;
const int EV_COMPARE            = 1;
const int EV_INSERT             = 2;
const int EV_UNLINK             = 3;
const int EV_SWAP_VALUE         = 4;
const int EV_ROTATE             = 5;
const int EV_UPDATE_HEIGHT      = 6;
const int EV_UPDATE_SIZE        = 7;
const int EV_BALANCE_ALERT      = 8;
const int EV_HIGHLIGHT_SUBTREE  = 9;
const int EV_HEAP_BUILD         = 10;
const int EV_HEAP_SWAP          = 11;
const int EV_HEAPIFY_START      = 12;
const int EV_HEAPIFY_DONE       = 13;
const int EV_EXTRACT_MAX        = 14;
const int EV_MARK_SORTED        = 15;
const int EV_HEAP_SIZE_CHANGE   = 16;
const int EV_HEAP_SORT_COMPLETE = 17;
const int EV_DONE               = 18;

// compare result codes
const int CMP_LESS    = -1;
const int CMP_EQUAL   =  0;
const int CMP_GREATER =  1;

// rotation type codes  (used in Event::extra)
const int ROT_LL = 0;
const int ROT_RR = 1;
const int ROT_LR = 2;
const int ROT_RL = 3;

// side codes
const int SIDE_LEFT  = 0;
const int SIDE_RIGHT = 1;

// ---------------------------------------------------------------------------
// Event record
// ---------------------------------------------------------------------------
struct Event
{
    int type;       // one of EV_* constants above
    int nodeA;      // primary node id  (-1 = none)
    int nodeB;      // secondary node id (-1 = none)
    int key;        // key / value involved (-1 = none)
    int result;     // CMP_* for EV_COMPARE; -1 otherwise
    int side;       // SIDE_LEFT / SIDE_RIGHT; -1 otherwise
    int mode;       // sub-mode for EV_HIGHLIGHT_SUBTREE; -1 otherwise
    int oldValue;   // before-value for EV_SWAP_VALUE / EV_UPDATE_*
    int newValue;   // after-value  for EV_SWAP_VALUE / EV_UPDATE_*
    int extra;      // ROT_* for EV_ROTATE; -1 otherwise
};

// ---------------------------------------------------------------------------
// EventLog
// ---------------------------------------------------------------------------
class EventLog
{
public:
    EventLog();
    EventLog(const EventLog& other);
    EventLog& operator=(const EventLog& other);
    ~EventLog();

    // Add a fully specified event
    void add(int type,
             int nodeA    = -1,
             int nodeB    = -1,
             int key      = -1,
             int result   = -1,
             int side     = -1,
             int mode     = -1,
             int oldValue = -1,
             int newValue = -1,
             int extra    = -1);

    void clear();
    int  getCount() const;
    Event getAt(int index) const;   // returns zeroed Event if index out of range

    bool enabled;   // when false, add() is a no-op; lets backend skip logging cheaply

private:
    static const int INITIAL_CAP = 64;

    Event* m_data;
    int    m_count;
    int    m_capacity;

    void grow();
    void copyFrom(const EventLog& other);
    void freeData();
};
