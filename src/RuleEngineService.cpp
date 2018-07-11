/***************************************************************************
 *  RuleEngineService.cpp - Rule Engine Service
 *
 *  Created: 2018-06-13 09:46:25
 *
 *  Copyright QRS
 ****************************************************************************/

#include "RuleEngineService.h"
#include "DataChannel.h"
#include "InstancePayload.h"
#include "ClassPayload.h"
#include "RulePayload.h"
#include "StringData.h"
#include "Message.h"
#include "Log.h"
#include "RuleTimerEvent.h"

#define RULE_DB_NAME "ruleengine.db"

namespace HB {

static RuleEngineService::RuleUrgentThread *gUrgentThread = 0;

using namespace UTILS;

static RuleEngineService *gRuleEngine = 0;

RuleEngineService::RuleEngineService()
    : mServerRoot("clips")
    , mCore(0), mCoreForUrgent(0), mStore(0)
    , mRuleChannel(0), mClassChannel(0)
{
    LOGTT();
}

RuleEngineService::~RuleEngineService()
{
    LOGTT();
    std::map<std::string, std::set<std::string>>::iterator it;
    for (it = mOfflineInsesCalled.begin(); it != mOfflineInsesCalled.end(); ++it)
        it->second.clear();
    mOfflineInsesCalled.clear();
    mCore.reset();
    mCoreForUrgent.reset();
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

int RuleEngineService::init(bool urgent)
{
    if (!(mStore = std::make_shared<RuleEngineStore>(mServerRoot + "/" + RULE_DB_NAME)))
        return -1;

    /*{{{ normal rule trigger */
    if (!(mCore = std::make_shared<RuleEngineCore>(ruleHandler())))
        return -1;
    mCore->setup(
        std::make_shared<MsgPushCall>(this, &RuleEngineService::callMessagePush),
        std::make_shared<InsPushCall>(this, &RuleEngineService::callInstancePush),
        std::make_shared<TxtPushCall>(this, &RuleEngineService::callContentPush)
        );
    mCore->start(mServerRoot, std::bind(&RuleEngineService::callGetFiles, this, std::placeholders::_1, false));
    /*}}}*/

    /*{{{ urgent rule trigger */
    if (urgent) {
        if (!gUrgentThread) {
            gUrgentThread = new RuleUrgentThread(*this);
            gUrgentThread->start();
        }
        if (!(mCoreForUrgent = std::make_shared<RuleEngineCore>(*mUrgentHandler)))
            return -1;

        mCoreForUrgent->setup(
            std::make_shared<MsgPushCall>(this, &RuleEngineService::callMessagePush),
            std::make_shared<InsPushCall>(this, &RuleEngineService::callInstancePush),
            std::make_shared<TxtPushCall>(this, &RuleEngineService::callContentPush)
            );
        mCoreForUrgent->start(mServerRoot, std::bind(&RuleEngineService::callGetFiles, this, std::placeholders::_1, true));
    }
    /*}}}*/

    if (mRuleChannel)
        mRuleChannel->init();
    if (mClassChannel)
        mClassChannel->init();
    return 0;
}

bool RuleEngineService::handleMessage(Message *msg)
{
    RuleEngineCore *driver = 0;
    if (gUrgentThread && gUrgentThread == Thread::currentThread())
        driver = mCoreForUrgent.get();
    else
        driver = mCore.get();

    if (msg->what == RET_REFRESH_TIMER)
        return driver->handleTimer();
    else
        LOGD("msg: [%d] [%d] [%d]\n", msg->what, msg->arg1, msg->arg2);

    switch(msg->what) {
        case RET_INSTANCE_ADD:
            if (msg->obj) {/*{{{*/
                std::shared_ptr<InstancePayload> payload(std::dynamic_pointer_cast<InstancePayload>(msg->obj));
                if (PT_INSTANCE_PAYLOAD == payload->type()) {
                    if (driver->handleInstanceAdd(payload->mInsName.c_str(), payload->mClsName.c_str()))
                        _OnlineInstanceRefreshRules(payload->mInsName);
                }
            }/*}}}*/
            return true;
        case RET_INSTANCE_DEL:
            if (msg->obj) {/*{{{*/
                std::shared_ptr<InstancePayload> payload(std::dynamic_pointer_cast<InstancePayload>(msg->obj));
                if (PT_INSTANCE_PAYLOAD == payload->type())
                    driver->handleInstanceDel(payload->mInsName.c_str());
            }/*}}}*/
            return true;
        case RET_INSTANCE_PUT:
            if (msg->obj) {/*{{{*/
                std::shared_ptr<InstancePayload> payload(std::dynamic_pointer_cast<InstancePayload>(msg->obj));
                if (PT_INSTANCE_PAYLOAD == payload->type()) {
                    driver->handleInstancePut(
                        payload->mInsName.c_str(),
                        payload->mSlots[0].nName.c_str(),
                        payload->mSlots[0].nValue.c_str());
                }
            }/*}}}*/
            return true;
        case RET_CLASS_SYNC:
            if (msg->obj) {/*{{{*/
                std::shared_ptr<ClassPayload> payload(std::dynamic_pointer_cast<ClassPayload>(msg->obj));
                if (PT_CLASS_PAYLOAD != payload->type()) {
                    LOGW("payload type not match!\n");
                    return false;
                }
                std::string path = driver->handleClassSync(
                    payload->mClsName.c_str(),
                    payload->mVersion.c_str(),
                    payload->toString().c_str());
                if (!path.empty()) {
                    mStore->updateClassTable(
                        payload->mClsName.c_str(),
                        payload->mVersion.c_str(), path.c_str());
                }
            }/*}}}*/
            return true;
        case RET_RULE_SYNC:
            if (msg->obj) {/*{{{*/
                std::shared_ptr<RulePayload> payload(std::dynamic_pointer_cast<RulePayload>(msg->obj));
                if (PT_RULE_PAYLOAD != payload->type()) {
                    LOGW("payload type not match!\n");
                    return false;
                }
                std::string path = driver->handleRuleSync(
                    payload->mRuleName.c_str(),
                    payload->mVersion.c_str(),
                    payload->toString().c_str());
                if (!path.empty()) {
                    mStore->updateRuleTable(
                        payload->mRuleName.c_str(),
                        payload->mVersion.c_str(), path.c_str());
                    driver->handleRuleAdd(payload->mRuleName.c_str());
                }
            }/*}}}*/
            return true;
        case RET_TRIGGER_RULE:
            if (msg->obj) {/*{{{*/
                std::shared_ptr<StringData> assert(std::dynamic_pointer_cast<StringData>(msg->obj));
                int count = driver->assertRun(assert->getData());
                if (count > 0)
                    LOGD("trigger rule agenda [%d]\n", count);
            }/*}}}*/
            return true;
        default:
            return false;
    }
    return false;
}

void RuleEngineService::RuleUrgentThread::run()
{
    LOGI("Rule RuleUrgentThread:[%u]\n", id());
    mService.mUrgentHandler = std::make_shared<RuleEventHandler>();
    mService.mUrgentHandler->mCallback = &mService;
    return RuleEventThread::run();
}

bool RuleEngineService::_OfflineInstanceCalledByRHS(std::string &insName, std::string &rulName)
{
    LOGTT();
    std::map<std::string, std::set<std::string>>::iterator it = mOfflineInsesCalled.find(insName);
    if (it != mOfflineInsesCalled.end()) {
        it->second.insert(rulName);
        return true;
    }
    std::set<std::string> rules;
    rules.insert(rulName);
    mOfflineInsesCalled.insert(std::pair<std::string, std::set<std::string>>(insName, rules));
    return true;
}

bool RuleEngineService::_OnlineInstanceRefreshRules(std::string &insName)
{
    LOGTT();
    std::map<std::string, std::set<std::string>>::iterator it = mOfflineInsesCalled.find(insName);
    if (it == mOfflineInsesCalled.end())
        return true;
    std::set<std::string>::iterator si;
    for ( si = it->second.begin(); si != it->second.end(); ++si)
        mCore->refreshRule((*si).c_str());
    return true;
}

bool RuleEngineService::callMessagePush(int what, int arg1, std::string arg2, std::string message)
{
    LOGD("(%d, %d, %s, %s)\n", what, arg1, arg2.c_str(), message.c_str());
    switch (what) {
        case MSG_RULE_RESPONSE:
            switch (arg1) {/*{{{ arg2: ruleid, message: detail info */
                case RUL_SUCCESS:
                    break;
                case RUL_FAIL:
                    break;
                case RUL_TIMEOUT:
                    break;
            }/*}}}*/
            break;
        case MSG_RULE_RHS:
            switch (arg1) {/*{{{ arg2: ruleid, message: instanceid */
                case RHS_INS_NOT_FOUND:
                    return _OfflineInstanceCalledByRHS(message, arg2);
                case RHS_NTF_WRONG_TYPE:
                    break;
                case RHS_SEE_NOT_FOUND:
                    break;
            }/*}}}*/
            break;
    }
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
    std::shared_ptr<StringData> data = std::make_shared<StringData>(assert.c_str());
    return ruleHandler().sendMessage(ruleHandler().obtainMessage(RET_TRIGGER_RULE, data));
}

std::vector<std::string> RuleEngineService::callGetFiles(int fileType, bool urgent)
{
    std::vector<std::string> files;
    switch (fileType) {
        case TYPE_TEM_FILE:
            files = mStore->queryTemplateFilePaths(urgent);
            break;
        case TYPE_CLS_FILE:
            files = mStore->queryClassFilePaths(urgent);
            break;
        case TYPE_RUL_FILE:
            files = mStore->queryRuleFilePaths(urgent);
            break;
        default:
            break;
    }
    return std::move(files);
}

RuleEngineService& ruleEngine()
{
    if (0 == gRuleEngine) {
        gRuleEngine = new RuleEngineService();
        ruleHandler().mCallback = gRuleEngine;
    }
    return *gRuleEngine;
}

RuleEventHandler& urgentHandler()
{
    /* TODO Dangerous API, must call this after RuleEngineService::init() */
    return *(ruleEngine().urgentHandler().get());
}

} /* namespace HB */

