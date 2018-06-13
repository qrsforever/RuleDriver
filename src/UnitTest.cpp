/***************************************************************************
 *  UnitTest.cpp - Unit Test
 *
 *  Created: 2018-06-12 17:23:53
 *
 *  Copyright QRS
 ****************************************************************************/

#include "Log.h"
#include "LogThread.h"
#include "MessageLooper.h"
#include "RuleEngineService.h"

using namespace HB;

RuleEngineService *gEngine = 0;

int main(int argc, char *argv[])
{
    printf("\n-----------Test---------\n");

    initLogThread();
    setLogLevel(LOG_LEVEL_INFO);


    ruleEngine().start();


    UTILS::Looper::getDefaultLooper().run();
    return 0;
}
