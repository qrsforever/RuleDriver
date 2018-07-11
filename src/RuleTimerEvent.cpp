/***************************************************************************
 *  RuleTimerEvent.cpp - Rule Event Timer
 *
 *  Created: 2018-07-09 18:18:38
 *
 *  Copyright QRS
 ****************************************************************************/

#include "RuleTimerEvent.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include "Log.h"

#include <map>

#define DAY_SECONDS 86400
#define MAX_TIMER_EVENTS 36

namespace HB {

TimeNode::TimeNode(TimeNodeType ntype, TimeValueType vtype)
    : mNextNode(0), mPreNode(0)
    , mNodeType(ntype), mValueType(vtype)
    , mReset(false), mCurrent(RET_NOVAL)
{
    if (mValueType == eNull) {
        switch (mNodeType) {
            case eMonth:
            case eDay:
                mCurrent = 1;
            default:
                mCurrent = 0;
        }
    }
}

TimeNode::~TimeNode()
{
    mValues.clear();
    if (mNextNode)
        delete mNextNode;
    mNextNode = 0;
}

TimeNode* TimeNode::setNextNode(TimeNode *node)
{
    mNextNode = node;
    node->mPreNode = this;
    return node;
}

int TimeNode::_UpdateValue(int max)
{
    if (getResetFlag()) {
        setResetFlag(false);
        return mCurrent;
    }
    int val = mCurrent;
    switch (mValueType) {
        case eAny:
            if (mCurrent < max)
                val++;
            break;
        case eRange:
            if (mMax > max)
                mMax = max;
            if (mCurrent < mMax)
                val++;
            break;
        case eSet:
            for (auto it = mValues.begin(); it != mValues.end(); ++it) {
                if (mCurrent < *it) {
                    val = *it;
                    break;
                }
            }
            break;
        case eNull:
            return RET_NULL;
    }
    if (mCurrent != val) {
        mCurrent = val;
        if (nextNode())
            nextNode()->_ResetValue();
        return mCurrent;
    }
    return RET_NOVAL;
}

int TimeNode::_ResetValue(bool flag)
{
    if (flag) {
        if (mNextNode)
            mNextNode->_ResetValue(flag);
    }

    if (getResetFlag())
        return mCurrent;

    switch (mValueType) {
        case eAny:
            switch (mNodeType) {
                case eDay:
                case eMonth:
                    mCurrent = 1;
                    break;
                default:
                    mCurrent = 0;
            }
            break;
        case eRange:
            mCurrent = mMin;
            break;
        case eSet:
            mCurrent = *(mValues.begin());
            break;
        case eNull:
        default:
            break;
    }
    setResetFlag(true);
    return mCurrent;
}

int TimeNode::_GetMaxDay(int year, int month)
{
    int day = 30;
    switch (month) {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            day = 31;
            break;
        case 2:
            day = (year % 4) ? 28 : 29; /* I can live over 2100 year */
            break;
    }
    return day;
}

int TimeYear::nextDate(SysTime::DateTime &dt, time_t &elapse, bool adjust)
{
    LOGI("adjust = [%d]\n", adjust);
    int val = current();
    if (adjust) {
        while (val < dt.mYear) {
            val = _UpdateValue(2099);
            if (val == RET_NOVAL)
                return RET_RESET;
        }
        LOGD("TimeYear = [%d]\n", val);
    }
    int ret = nextNode()->nextDate(dt, elapse, adjust ? val <= dt.mYear : false);
    if (ret == RET_IGNORE)
        return ret;
    switch (ret) {
        case RET_NULL:
        case RET_RESET:
            val = _UpdateValue(2099);
            if (val == RET_NOVAL)
                return RET_RESET;
            if (ret != RET_NULL)
                nextNode()->nextDate(dt, elapse, false);
            break;
        case RET_OK:
        default:
            ;
    }
    dt.mYear = current();
    setResetFlag(false);
    if (0 == elapse)
        elapse = 366 * DAY_SECONDS;
    return RET_OK;
}

int TimeMonth::nextDate(SysTime::DateTime &dt, time_t &elapse, bool adjust)
{
    LOGI("adjust = [%d]\n", adjust);
    int val = current();
    if (adjust) {
        while (val < dt.mMonth) {
            val = _UpdateValue(12);
            if (val == RET_NOVAL)
                return RET_RESET;
        }
        LOGD("TimeMonth = [%d]\n", val);
    }
    int ret = nextNode()->nextDate(dt, elapse, adjust ? val <= dt.mMonth : false);
    if (ret == RET_IGNORE)
        return ret;
    switch (ret) {
        case RET_NULL:
        case RET_RESET:
            val = _UpdateValue(12);
            if (val == RET_NOVAL)
                return RET_RESET;
            if (ret != RET_NULL)
                nextNode()->nextDate(dt, elapse, false);
            break;
        case RET_OK:
        default:
            ;
    }
    dt.mMonth = current();
    setResetFlag(false);
    if (0 == elapse)
        elapse = _GetMaxDay(dt.mYear, dt.mMonth) * DAY_SECONDS;
    LOGD("TimeMonth = [%d]\n", current());
    return RET_OK;
}

int TimeWeek::nextDate(SysTime::DateTime &dt, time_t &elapse, bool adjust)
{
    LOGI("adjust = [%d]\n", adjust);
    int val = current();
    if (adjust) {
        while (val < dt.mDayOfWeek) {
            val = _UpdateValue(6);
            if (val == RET_NOVAL)
                return RET_RESET;
        }
        LOGD("TimeWeek = [%d]\n", val);
    }
    int ret = nextNode()->nextDate(dt, elapse, adjust ? val <= dt.mDayOfWeek : false);
    switch (ret) {
        case RET_NULL:
        case RET_RESET:
            val = _UpdateValue(6);
            if (val == RET_NOVAL)
                val = _ResetValue() + 7;
            if (ret != RET_NULL)
                nextNode()->nextDate(dt, elapse, false);
            dt = secondsToDateTime(
                dateTimeToSeconds(dt) + (val-dt.mDayOfWeek)*DAY_SECONDS);
            return RET_IGNORE;
        case RET_OK:
        default:
            ;
    }
    dt.mDayOfWeek = current();
    setResetFlag(false);
    if (0 == elapse)
        elapse = 7 * DAY_SECONDS;
    return RET_OK;
}

int TimeDay::nextDate(SysTime::DateTime &dt, time_t &elapse, bool adjust)
{
    LOGI("adjust = [%d]\n", adjust);
    int days = _GetMaxDay(dt.mYear, dt.mMonth);
    int val = current();
    if (adjust) {
        while (val < dt.mDay) {
            val = _UpdateValue(days);
            LOGD("TimeDay [%d] vs [%d]\n", val, dt.mDay);
            if (val == RET_NOVAL)
                return RET_RESET;
        }
        LOGD("TimeDay = [%d]\n", val);
    }
    int ret = nextNode()->nextDate(dt, elapse, adjust ? val <= dt.mDay : false);
    switch (ret) {
        case RET_NULL:
        case RET_RESET:
            val = _UpdateValue(days);
            if (val == RET_NOVAL)
                return RET_RESET;
            if (ret != RET_NULL)
                nextNode()->nextDate(dt, elapse, false);
            break;
        case RET_OK:
        default:
            ;
    }
    dt.mDay = current();
    setResetFlag(false);
    if (0 == elapse)
        elapse = DAY_SECONDS;
    return RET_OK;
}

int TimeHour::nextDate(SysTime::DateTime &dt, time_t &elapse, bool adjust)
{
    LOGI("adjust = [%d]\n", adjust);
    if (valueType() == eNull) {
        dt.mHour = current();
        return nextNode()->nextDate(dt, elapse, adjust);
    }
    int val = current();
    if (adjust) {
        while (val < dt.mHour) {
            val = _UpdateValue(23);
            if (val == RET_NOVAL)
                return RET_RESET;
        }
        LOGD("TimeHour = [%d]\n", val);
    }
    int ret = nextNode()->nextDate(dt, elapse, adjust ? val <= dt.mHour : false);
    switch (ret) {
        case RET_NULL:
        case RET_RESET:
            val = _UpdateValue(23);
            if (val == RET_NOVAL)
                return RET_RESET;
            if (ret != RET_NULL)
                nextNode()->nextDate(dt, elapse, false);
            break;
        case RET_OK:
        default:
            ;
    }
    dt.mHour = current();
    setResetFlag(false);
    if (0 == elapse)
        elapse = 3600;
    return RET_OK;
}

int TimeMinute::nextDate(SysTime::DateTime &dt, time_t &elapse, bool adjust)
{
    LOGI("adjust = [%d]\n", adjust);
    if (valueType() == eNull) {
        dt.mMinute = current();
        return nextNode()->nextDate(dt, elapse, adjust);
    }
    int val = current();
    if (adjust) {
        while (val < dt.mMinute) {
            val = _UpdateValue(59);
            if (val < 0)
                return RET_RESET;
        }
        LOGD("TimeMinute = [%d]\n", val);
    }
    int ret = nextNode()->nextDate(dt, elapse, adjust ? val <= dt.mMinute : false);
    switch (ret) {
        case RET_NULL:
        case RET_RESET:
            val = _UpdateValue(59);
            if (val == RET_NOVAL)
                return RET_RESET;
            if (ret != RET_NULL)
                nextNode()->nextDate(dt, elapse, false);
            break;
        case RET_OK:
        default:
            ;
    }
    dt.mMinute = current();
    setResetFlag(false);
    if (0 == elapse)
        elapse = 60;
    return RET_OK;
}

int TimeSecond::nextDate(SysTime::DateTime &dt, time_t &elapse, bool adjust)
{
    LOGI("adjust = [%d]\n", adjust);
    if (valueType() == eNull) {
        dt.mSecond = current();
        return RET_NULL;
    }
    int val = current();
    if (adjust) {
        while (val < dt.mSecond) {
            val = _UpdateValue(59);
            LOGD("TimeSecond [%d] vs [%d]\n", val, dt.mSecond);
            if (val == RET_NOVAL)
                return RET_RESET;
        }
        LOGD("TimeSecond = [%d]\n", val);
    }
    val = _UpdateValue(59);
    if (val == RET_NOVAL)
        return RET_RESET;
    dt.mSecond = current();
    setResetFlag(false);
    elapse = 1;
    return RET_OK;
}


typedef std::map<int, RuleTimerEvent*> TimerEvents;

static TimerEvents gTimers;

static void _TimerThread(union sigval v)
{
    LOGTT();
    TimerEvents::iterator it = gTimers.find(v.sival_int);
    if (it == gTimers.end())
        return;
    time_t elapse = 0;
    SysTime::DateTime dt;
    SysTime::GetDateTime(&dt);
    int secs1 = dateTimeToSeconds(dt);
    int ret = it->second->nextDate(dt, elapse);
    if (ret < 0) {
        delete it->second;
        gTimers.erase(it);
        return;
    }
    LOGD("%04d%02d%02d %02d:%02d:%02d %lu\n", dt.mYear, dt.mMonth, dt.mDay, dt.mHour, dt.mMinute, dt.mSecond, elapse);
    int secs2 = dateTimeToSeconds(dt);
    if (secs2 > secs1) {
        it->second->startTimer(secs2 - secs1);
    } else
        LOGE("check me!\n");
}

RuleTimerEvent::RuleTimerEvent(int eventid, bool flag)
    : mNodeHeader(0)
    , mEventID(eventid), mTimerID(0)
{
    if (gTimers.size() > MAX_TIMER_EVENTS) {
        LOGE("beyond the max value\n");
        return;
    }

    gTimers.insert(std::pair<int, RuleTimerEvent*>(eventid, this));

    /* Year / Month / Week|Day / Hour / Minute / Second */
    TimeNode *node = new TimeYear(eAny);
    mNodeHeader = node;
    node = node->setNextNode(new TimeMonth(eAny));
    node = node->setNextNode(flag ? (TimeNode*)(new TimeWeek(eAny)) : (TimeNode*)(new TimeDay(eAny)));
    node = node->setNextNode(new TimeHour(eNull));
    node = node->setNextNode(new TimeMinute(eNull));
    node = node->setNextNode(new TimeSecond(eNull));
}

RuleTimerEvent::~RuleTimerEvent()
{
    LOGTT();
    cancelTimer();
    if (mNodeHeader)
        delete mNodeHeader;
    mNodeHeader = 0;
}

TimeNode* RuleTimerEvent::getTimeNode(TimeNodeType type)
{
    TimeNode *node = mNodeHeader;
    while (node) {
        if (node->type() == type)
            return node;
        node = node->mNextNode;
    }
    return 0;
}

int RuleTimerEvent::startTimer(time_t secs)
{
	timer_t timerid;
	struct sigevent evp;
	memset(&evp, 0, sizeof(struct sigevent));

    cancelTimer();

    /* evp.sigev_value.sival_ptr = (void*)this; */
	evp.sigev_value.sival_int = mEventID;
	evp.sigev_notify = SIGEV_THREAD;
	evp.sigev_notify_function = _TimerThread;

	if (timer_create(CLOCK_REALTIME, &evp, &timerid) == -1) {
        LOGE("create timer error!\n");
        return -1;
	}

    LOGD("startTimer [%ld]\n", secs);

	struct itimerspec it;
	it.it_interval.tv_sec = secs;
	it.it_interval.tv_nsec = 0;
	it.it_value.tv_sec = secs;
	it.it_value.tv_nsec = 0;
	if (timer_settime(timerid, 0, &it, NULL) == -1) {
        LOGE("set timer error!\n");
        timer_delete(timerid);
        return -1;
	}
    mTimerID = timerid;
    return 0;
}

int RuleTimerEvent::cancelTimer()
{
    if (mTimerID == 0)
        return -1;
    return timer_delete(mTimerID);
}

int RuleTimerEvent::nextDate(SysTime::DateTime &dt, time_t &elapse)
{
    if (!mNodeHeader)
        return -1;
    int ret = mNodeHeader->nextDate(dt, elapse, true);
    if (ret != RET_OK)
        return -1;
    return 0;
}

} /* namespace HB */
