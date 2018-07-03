/***************************************************************************
 *  RuleEngineService.cpp - Rule Engine Service
 *
 *  Created: 2018-06-13 09:46:25
 *
 *  Copyright QRS
 ****************************************************************************/

#include "RuleEngineService.h"
#include "RuleEventHandler.h"
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
    : mServerRoot("clips")
    , mCore(0), mStore(0)
    , mRuleChannel(0), mClassChannel(0)
{
    LOGTT();
}

RuleEngineService::~RuleEngineService()
{
    LOGTT();
    mCore.reset();
    mStore.reset();
}

void RuleEngineService::setRuleChannel(std::shared_ptr<DataChannel> channel)
{
    mRuleChannel = channel;
}

void RuleEngineService::setDeviceChannel(std::shared_ptr<DataChannel> channel)
{
    mClassChannel = channel;
}

int RuleEngineService::init()
{
    LOGTT();

    if (!(mStore = std::make_shared<RuleEngineStore>(ruleHandler(), mServerRoot + "/ruleengine.db")))
        return -1;

    if (!(mCore = std::make_shared<RuleEngineCore>(ruleHandler())))
        return -1;

    /* install rule driver interface */
    mCore->driver().add_function(
        "msg-push",
        std::make_shared<Functor<bool, int, int, std::string, std::string>>(
            this,
            &RuleEngineService::callMessagePush));
    mCore->driver().add_function(
        "ins-push",
        std::make_shared<Functor<bool, std::string, std::string, std::string>>(
            this,
            &RuleEngineService::callInstancePush));
    mCore->driver().add_function(
        "txt-push",
        std::make_shared<Functor<bool, std::string, std::string, std::string>>(
            this,
            &RuleEngineService::callContentPush));
    mCore->start();
    /* mCore->debugShow(); */

    if (mRuleChannel)
        mRuleChannel->init();
    if (mClassChannel)
        mClassChannel->init();
    return 0;
}

bool RuleEngineService::handleMessage(Message *msg)
{
    if (!mCore)
        return false;

    if (msg->what == RET_REFRESH_TIMER)
        return mCore->handleTimer();
    else
        LOGD("msg: [%d] [%d] [%d]\n", msg->what, msg->arg1, msg->arg2);

    switch(msg->what) {
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
                if (PT_CLASS_PAYLOAD != payload->type()) {
                    LOGW("payload type not match!\n");
                    return false;
                }
                std::string path = mCore->handleClassSync(
                    payload->mClsName.c_str(),
                    payload->mVersion.c_str(),
                    payload->toString().c_str());
                if (!path.empty()) {
                    mStore->updateClassTable(
                        payload->mClsName.c_str(),
                        payload->mVersion.c_str(), path.c_str());
                }
            }
            return true;
        case RET_RULE_SYNC:
            if (msg->obj) {
                std::shared_ptr<RulePayload> payload(std::dynamic_pointer_cast<RulePayload>(msg->obj));
                if (PT_RULE_PAYLOAD != payload->type()) {
                    LOGW("payload type not match!\n");
                    return false;
                }
                std::string path = mCore->handleRuleSync(
                    payload->mRuleName.c_str(),
                    payload->mVersion.c_str(),
                    payload->toString().c_str());
                if (!path.empty()) {
                    mStore->updateRuleTable(
                        payload->mRuleName.c_str(),
                        payload->mVersion.c_str(), path.c_str());
                }
            }
            return true;
        default:
            return false;
    }
    return false;
}

bool RuleEngineService::callMessagePush(int what, int arg1, std::string arg2, std::string message)
{
    LOGD("(%d, %d, %s, %s)\n", what, arg1, arg2.c_str(), message.c_str());
    return false;
}

bool RuleEngineService::callInstancePush(std::string insName, std::string slot, std::string value)
{
    if ('#' == value[0])
        value = value.substr(1);
    LOGD("(%s, %s, %s)\n", insName.c_str(), slot.c_str(), value.c_str());

    std::shared_ptr<InstancePayload> payload = std::make_shared<InstancePayload>();
    payload->mInsName = insName;
    payload->mSlots.push_back(InstancePayload::SlotInfo(slot, value));
    mClassChannel->send(PT_INSTANCE_PAYLOAD, payload);
    return false; /* asynchronous */
}

bool RuleEngineService::callContentPush(std::string id, std::string title, std::string content)
{
    LOGD("(%s, %s, %s)\n", id.c_str(), title.c_str(), content.c_str());
    return true; /* synchronous */
}

bool RuleEngineService::triggerRule(std::string ruleId)
{
    LOGD("ruleId: %s\n", ruleId.c_str());
    if (!mCore)
        return false;
    std::string assert("");
    assert.append("(scene ").append(ruleId).append(")");
    int count = mCore->assertRun(assert);
    LOGD("count --------> %d\n", count);
    return count > 0 ? true : false;
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

