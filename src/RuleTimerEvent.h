/***************************************************************************
 *  RuleTimerEvent.h - Rule Event Timer
 *
 *  Created: 2018-07-09 18:18:29
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __RuleTimerEvent_H__
#define __RuleTimerEvent_H__

#include "SysTime.h"
#include <memory>
#include <set>

using namespace UTILS;

namespace HB {

#define TIMER_VT_ALL    0
#define TIMER_VT_RANGE  1
#define TIMER_VT_SET    2

#define RET_NOVAL  -1
#define RET_OK      0
#define RET_RESET   1
#define RET_NULL    2
#define RET_IGNORE  3

typedef enum {
    eAny,
    eRange,
    eSet,
    eNull,
} TimeValueType;

typedef enum {
    eYear,
    eMonth,
    eWeek,
    eDay,
    eHour,
    eMinute,
    eSecond,
} TimeNodeType;

class RuleTimerEvent;
class TimeNode {
public:
    TimeNode(TimeNodeType ntype, TimeValueType vtype);
    virtual ~TimeNode();

    TimeNodeType type() { return mNodeType; }
    void setValueType(TimeValueType type) { mValueType = type; }
    TimeValueType valueType() { return mValueType; }
    void setRange(int min, int max) { mMin = min; mMax = max; }
    TimeNode& append(int val) { mValues.insert(val); return *this; }

    virtual int nextDate(SysTime::DateTime &dt, time_t &elapse, bool adjust) = 0;

    TimeNode* setNextNode(TimeNode *node);

    TimeNode* nextNode() { return mNextNode; }

    const int& current() const { return mCurrent; }
    void setResetFlag(bool flag) { mReset = flag; }
    bool getResetFlag() { return mReset; }

public:
    virtual int _UpdateValue(int max);
    virtual int _GetMaxDay(int year, int month);
    virtual int _ResetValue(bool flag = true);

protected:
    friend class RuleTimerEvent;
    TimeNode *mNextNode;
    TimeNode *mPreNode;
    TimeNodeType mNodeType;
    TimeValueType mValueType;
    bool mReset;
    int mCurrent;
    int mMin;
    int mMax;
    std::set<int> mValues;
}; /* class TimeNode */

class TimeYear : public TimeNode {
public:
    TimeYear(TimeValueType vtype):TimeNode(eYear, vtype){ mCurrent = 2018; }
    ~TimeYear(){}

    int nextDate(SysTime::DateTime &dt, time_t &elapse, bool adjust);
private:

}; /* class TimeYear */

class TimeMonth : public TimeNode {
public:
    TimeMonth(TimeValueType vtype):TimeNode(eMonth, vtype){};
    ~TimeMonth(){};

    int nextDate(SysTime::DateTime &dt, time_t &elapse, bool adjust);

}; /* class TimeMonth */

class TimeDay : public TimeNode {
public:
    TimeDay(TimeValueType vtype):TimeNode(eDay, vtype){}
    ~TimeDay(){}

    int nextDate(SysTime::DateTime &dt, time_t &elapse, bool adjust);

private:

}; /* class TimeDay */

class TimeWeek : public TimeNode {
public:
    TimeWeek(TimeValueType vtype):TimeNode(eWeek, vtype){}
    ~TimeWeek(){}

    int nextDate(SysTime::DateTime &dt, time_t &elapse, bool adjust);

private:

}; /* class TimeWeek */

class TimeHour : public TimeNode {
public:
    TimeHour(TimeValueType vtype):TimeNode(eHour, vtype){}
    ~TimeHour(){}

    int nextDate(SysTime::DateTime &dt, time_t &elapse, bool adjust);

private:

}; /* class TimeHour */

class TimeMinute : public TimeNode {
public:
    TimeMinute(TimeValueType vtype):TimeNode(eMinute, vtype){}
    ~TimeMinute(){}

    int nextDate(SysTime::DateTime &dt, time_t &elapse, bool adjust);
private:

}; /* class TimeMinute */

class TimeSecond : public TimeNode {
public:
    TimeSecond(TimeValueType vtype):TimeNode(eSecond, vtype){}
    ~TimeSecond(){}

    int nextDate(SysTime::DateTime &dt, time_t &elapse, bool adjust);
private:

}; /* class TimeSecond : public TimeNode */

class RuleTimerEvent {
public:
    RuleTimerEvent(int eventid, bool flag = false);
    ~RuleTimerEvent();

    TimeNode* getTimeNode(TimeNodeType type);

    int nextDate(SysTime::DateTime &dt, time_t &elapse);

    int startTimer(time_t secs);
    int cancelTimer();

    int eventID() { return mEventID; }
private:
    TimeNode *mNodeHeader;
    int mEventID;
    timer_t mTimerID;
}; /* class RuleTimerEvent */

} /* namespace HB */

#ifdef __cplusplus


#endif /* __cplusplus */

#endif /* __RuleTimerEvent_H__ */
