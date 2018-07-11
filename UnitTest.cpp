/***************************************************************************
 *  UnitTest.cpp - Unit Test
 *
 *  Created: 2018-06-12 17:23:53
 *
 *  Copyright QRS
 ****************************************************************************/

#include "Log.h"
#include "LogThread.h"
#include "Message.h"
#include "MessageTypes.h"
#include "RuleEngineService.h"
#include "RuleEventHandler.h"
#include "DeviceDataChannel.h"
#include "RuleDataChannel.h"
#include "RuleTimerEvent.h"

#include "MainPublicHandler.h" /* temp use, not rule module */

using namespace HB;

extern "C" int initMainThread();
extern "C" int mainThreadRun();

class InitThread : public Thread {
public:
    InitThread(){}
    ~InitThread(){}
    void run();
};

void InitThread::run()
{
    /*************************
     *  Network module init  *
     *************************/

    /*****************************
     *  Rule Engine module init  *
     *****************************/
    ruleEngine().setServerRoot("clips");
    ruleEngine().setDeviceChannel(std::make_shared<DeviceDataChannel>());
    ruleEngine().setRuleChannel(std::make_shared<ElinkRuleDataChannel>());
    ruleEngine().init();


    /*******************************
     *  Device Manger module init  *
     *******************************/


    /* Simulate Test */
    /* mainHandler().sendEmptyMessage(MT_SIMULATE); */
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    printf("\n-----------Test---------\n");

    /***************************
     *  First init log module  *
     ***************************/
    initLogThread();
    setLogLevel(LOG_LEVEL_TRACE);


    /* Test TimerEvent */
   RuleTimerEvent *event = new RuleTimerEvent(1);
   TimeNode *year = event->getTimeNode(eYear);
   year->setValueType(eSet);
   year->append(2018);
   TimeNode *month = event->getTimeNode(eMonth);
   month->setValueType(eSet);
   month->append(7);
   TimeNode *day = event->getTimeNode(eDay);
   day->setValueType(eSet);
   day->append(11);
   TimeNode *hour = event->getTimeNode(eHour);
   if (hour) {
        hour->setValueType(eSet);
        hour->append(19);
   }
   TimeNode *minute = event->getTimeNode(eMinute);
   if (minute) {
       minute->setValueType(eSet);
       minute->append(33).append(36);
   }

   time_t elapse;
   SysTime::DateTime dt;
   SysTime::GetDateTime(&dt);
   LOGD("%04d%02d%02d %02d:%02d:%02d %lu\n", dt.mYear, dt.mMonth, dt.mDay, dt.mHour, dt.mMinute, dt.mSecond, elapse);
   int secs1 = dateTimeToSeconds(dt);
   event->nextDate(dt, elapse);
   LOGD("%04d%02d%02d %02d:%02d:%02d %lu\n", dt.mYear, dt.mMonth, dt.mDay, dt.mHour, dt.mMinute, dt.mSecond, elapse);
   int secs2 = dateTimeToSeconds(dt);
   event->startTimer(secs2-secs1);

#if 0
   RuleTimerEvent *event2 = new RuleTimerEvent(2, true);
   TimeNode *week = event2->getTimeNode(eWeek);
   if (week) {
       week->setValueType(eSet);
       week->append(4).append(6);
   }
   hour = event2->getTimeNode(eHour);
   if (hour) {
        hour->setValueType(eSet);
        hour->append(20).append(23);
   }
   event2->nextDate(dt, elapse);
#endif


    while(1);
    /*****************************
     *  Second init main module  *
     *****************************/
    initMainThread();

    /*******************************
     *  Third init others modules  *
     *******************************/
    InitThread init;
    init.start();


    /****************************
    *  Main message queue run  *
    ****************************/
    return mainThreadRun();
}
