/***************************************************************************
 *  RuleEngineService.cpp - Rule Engine Service
 *
 *  Created: 2018-06-13 09:46:25
 *
 *  Copyright QRS
 ****************************************************************************/

#include "RuleEngineService.h"
#include "RuleEventHandler.h"
#include "RuleClipsBrain.h"
#include "Log.h"

namespace HB {

using namespace UTILS;

static RuleEngineService *gRuleEngine = 0;

RuleEngineService::RuleEngineService()
{

}

RuleEngineService::~RuleEngineService()
{

}

void RuleEngineService::start()
{
    LOGD("start...\n");
}

RuleEngineService& ruleEngine()
{
    if (0 == gRuleEngine) {
        gRuleEngine = new RuleEngineService();
    }

    return *gRuleEngine;
}

} /* namespace HB */
