/***************************************************************************
 *  RuleEngineService.h - Rule Engine Service Header
 *
 *  Created: 2018-06-13 09:24:58
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __RuleEngineService_H__
#define __RuleEngineService_H__

#include "MessageHandler.h"
#include "DataChannel.h"
#include "RuleEventThread.h"
#include "RuleEngineStore.h"
#include "RuleEngineCore.h"
#include "RuleEventHandler.h"

#include <map>
#include <set>

#ifdef __cplusplus

using namespace UTILS;

namespace HB {

class RuleEngineService : public MessageHandler::Callback {
public:
    RuleEngineService();
    ~RuleEngineService();

    bool handleMessage(Message *msg);

    void setServerRoot(std::string rootDir) { mServerRoot = rootDir; }
    std::string& getServerRoot() { return mServerRoot; }
    int init(bool urgent = false);

    void setRuleChannel(std::shared_ptr<DataChannel> channel);
    void setDeviceChannel(std::shared_ptr<DataChannel> channel);

    bool callMessagePush(int what, int arg1, std::string arg2, std::string message);
    bool callInstancePush(std::string insName, std::string slot, std::string value);
    bool callContentPush(std::string id, std::string title, std::string content);
    bool triggerRule(std::string ruleId);

    std::vector<std::string> callGetFiles(int fileType, bool urgent);

    RuleEngineStore::pointer store() { return mStore; }
    RuleEngineCore::pointer core() { return mCore; }
    RuleEngineCore::pointer coreForUrgent() { return mCoreForUrgent; }
    RuleEventHandler::pointer urgentHandler() { return mUrgentHandler; }

    class RuleUrgentThread : public RuleEventThread {
    public:
        RuleUrgentThread(RuleEngineService &service) : mService(service) {}
        ~RuleUrgentThread(){}
        void run();
    private:
        RuleEngineService &mService;
    };

private:
    bool _OfflineInstanceCalledByRHS(std::string &insName, std::string &rulName);
    bool _OnlineInstanceRefreshRules(std::string &insName);

private:
    friend class RuleEventThread;
    std::string mServerRoot;
    RuleEngineCore::pointer mCore;
    RuleEngineCore::pointer mCoreForUrgent;
    RuleEventHandler::pointer mUrgentHandler;

    RuleEngineStore::pointer mStore;
    DataChannel::pointer mRuleChannel;
    DataChannel::pointer mClassChannel;

    std::map<std::string, std::set<std::string>> mOfflineInsesCalled;

}; /* class RuleEngineService */

RuleEngineService& ruleEngine();

RuleEventHandler& urgentHandler();

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __RuleEngineService_H__ */
