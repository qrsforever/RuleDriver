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
#include "InstancePayload.h"
#include "ClassPayload.h"
#include "RulePayload.h"
#include "Message.h"
#include "Log.h"

namespace HB {

using namespace UTILS;

static RuleEngineService *gRuleEngine = 0;

RuleEngineService::RuleEngineService()
    : mCore(0)
    , mServerRoot("clips")
    , mRuleChannel(0), mDeviceChannel(0)
{
    LOGTT();
}

RuleEngineService::~RuleEngineService()
{
    LOGTT();
    if (mCore) {
        delete mCore;
        mCore = 0;
    }
}

void RuleEngineService::setRuleChannel(std::shared_ptr<DataChannel> channel)
{
    mRuleChannel = channel;
}

void RuleEngineService::setDeviceChannel(std::shared_ptr<DataChannel> channel)
{
    mDeviceChannel = channel;
}

int RuleEngineService::init()
{
    LOGTT();

    mCore = new RuleEngineCore(ruleHandler(), mServerRoot);
    if (!mCore)
        return -1;
    mCore->driver().add_function(
        "ins-push",
        std::make_shared<Functor<void, std::string, std::string, std::string>>(
            this,
            &RuleEngineService::callInstancePush));
    mCore->driver().add_function(
        "msg-push",
        std::make_shared<Functor<void, std::string, std::string>>(
            this,
            &RuleEngineService::callMessagePush));
    mCore->start();

    if (mRuleChannel)
        mRuleChannel->init();
    if (mDeviceChannel)
        mDeviceChannel->init();
    return 0;
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
                std::shared_ptr<InstancePayload> payload(std::dynamic_pointer_cast<InstancePayload>(msg->obj));
                if (PT_INSTANCE_PAYLOAD == payload->type())
                    mCore->handleInstanceAdd(payload->mInsName.c_str(), payload->mClsName.c_str());
            }
            return true;
        case RET_INSTANCE_DEL:
            if (msg->obj) {
                std::shared_ptr<InstancePayload> payload(std::dynamic_pointer_cast<InstancePayload>(msg->obj));
                if (PT_INSTANCE_PAYLOAD == payload->type())
                    mCore->handleInstanceDel(payload->mInsName.c_str());
            }
            return true;
        case RET_INSTANCE_PUT:
            if (msg->obj) {
                std::shared_ptr<InstancePayload> payload(std::dynamic_pointer_cast<InstancePayload>(msg->obj));
                if (PT_INSTANCE_PAYLOAD == payload->type()) {
                    mCore->handleInstancePut(
                        payload->mInsName.c_str(),
                        payload->mSlots[0].nName.c_str(),
                        payload->mSlots[0].nValue.c_str());
                }
            }
            return true;
        case RET_CLASS_SYNC:
            if (msg->obj) {
                std::shared_ptr<ClassPayload> payload(std::dynamic_pointer_cast<ClassPayload>(msg->obj));
                if (PT_CLASS_PAYLOAD == payload->type()) {
                    mCore->handleClassSync(
                        payload->mClsName.c_str(),
                        payload->mVersion.c_str(),
                        payload->toString().c_str());
                }
            }
            return true;
        case RET_RULE_SYNC:
            if (msg->obj) {
                std::shared_ptr<RulePayload> payload(std::dynamic_pointer_cast<RulePayload>(msg->obj));
                if (PT_RULE_PAYLOAD == payload->type()) {
                    mCore->handleRuleSync(
                        payload->mRuleID.c_str(),
                        payload->mVersion.c_str(),
                        payload->toString().c_str());
                }
            }
            return true;
        default:
            return false;
    }
    return false;
}

void RuleEngineService::callInstancePush(std::string insName, std::string slot, std::string value)
{
    if ('#' == value[0])
        value = value.substr(1);
    LOGD("(%s, %s, %s)\n", insName.c_str(), slot.c_str(), value.c_str());

    std::shared_ptr<InstancePayload> payload = std::make_shared<InstancePayload>();
    payload->mInsName = insName;
    payload->mSlots.push_back(InstancePayload::SlotInfo(slot, value));
    mDeviceChannel->send(PT_INSTANCE_PAYLOAD, payload);
}

void RuleEngineService::callMessagePush(std::string title, std::string message)
{
    LOGD("(%s, %s)\n", title.c_str(), message.c_str());
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

