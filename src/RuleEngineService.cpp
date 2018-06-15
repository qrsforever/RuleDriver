/***************************************************************************
 *  RuleEngineService.cpp - Rule Engine Service
 *
 *  Created: 2018-06-13 09:46:25
 *
 *  Copyright QRS
 ****************************************************************************/

#include "RuleEngineService.h"
#include "RuleEngineCore.h"
#include "RuleEventHandler.h"
#include "RuleEventTypes.h"
#include "Message.h"
#include "Log.h"

namespace HB {

using namespace UTILS;

static RuleEngineService *gRuleEngine = 0;

RuleEngineService::RuleEngineService()
    : mCore(0), mRuleChannel(0), mDeviceChannel(0)
{
    LOGTT();
    mCore = new RuleEngineCore(ruleHandler());
}

RuleEngineService::~RuleEngineService()
{
    LOGTT();
    if (mCore) {
        delete mCore;
        mCore = 0;
    }
}

void RuleEngineService::init()
{
    LOGTT();
    mCore->init();
}

void RuleEngineService::setRuleChannel(std::shared_ptr<DataChannel> channel)
{
    mRuleChannel = channel;
}

void RuleEngineService::setDeviceChannel(std::shared_ptr<DataChannel> channel)
{
    mDeviceChannel = channel;
}

bool RuleEngineService::newRuleFound(std::string ruleId)
{
    LOGD("ruleId: %s\n", ruleId.c_str());
    if (!mCore)
        return false;

    return true;
}

bool RuleEngineService::handleMessage(Message *msg)
{
    if (msg->what != RET_REFRESH_TIMER)
        LOGD("msg: [%d] [%d] [%d]\n", msg->what, msg->arg1, msg->arg2);

    switch(msg->what) {
        case RET_REFRESH_TIMER:
            if (mCore)
                mCore->handleTimer();
            return false;
        default:
            return false;
    }
    return true;
}

RuleEngineService& ruleEngine()
{
    if (0 == gRuleEngine) {
        gRuleEngine = new RuleEngineService();
        ruleHandler().mCallback = gRuleEngine;
    }
    return *gRuleEngine;
}

} /* namespace HB */

