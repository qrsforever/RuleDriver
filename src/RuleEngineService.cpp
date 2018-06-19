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
#include "DataChannel.h"
#include "StringArray.h"
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
    mRuleChannel->init();
    mDeviceChannel->init();
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

    if (!mCore)
        return false;

    switch(msg->what) {
        case RET_REFRESH_TIMER:
            mCore->handleTimer();
            return false;
        case RET_INSTANCE_ADD:
            if (msg->obj) {
                std::shared_ptr<StringArray> arr(std::dynamic_pointer_cast<StringArray>(msg->obj));
                if (arr && arr->size() == 2)
                    mCore->handleInstanceAdd((*arr)[0], (*arr)[1]);
            }
            return true;
        case RET_INSTANCE_DEL:
            if (msg->obj) {
                std::shared_ptr<StringArray> arr(std::dynamic_pointer_cast<StringArray>(msg->obj));
                if (arr && arr->size() == 1)
                    mCore->handleInstanceDel((*arr)[0]);
            }
            return true;
        case RET_INSTANCE_PUT:
            if (msg->obj) {
                std::shared_ptr<StringArray> arr(std::dynamic_pointer_cast<StringArray>(msg->obj));
                if (arr && arr->size() == 3)
                    mCore->handleInstancePut((*arr)[0], (*arr)[1], (*arr)[2]);
            }
            return true;
        default:
            return false;
    }
    return false;
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

