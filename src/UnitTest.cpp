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

#include "MainPublicHandler.h" /* temp use, not rule module */

using namespace HB;

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
    ruleEngine().setDeviceChannel(std::make_shared<DeviceDataChannel>());
    ruleEngine().setRuleChannel(std::make_shared<ELinkRuleDataChannel>());
    ruleEngine().init();


    /*******************************
     *  Device Manger module init  *
     *******************************/

    /* Message *msg = mainHandler().obtainMessage(MT_RULE, RULE_EVENT_SYNC, RULE_SYNC_NONE);
     * mainHandler().sendMessage(msg); */
}

int main(int argc, char *argv[])
{
    printf("\n-----------Test---------\n");

    /***************************
     *  First init log module  *
     ***************************/
    initLogThread();
    setLogLevel(LOG_LEVEL_TRACE);

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
