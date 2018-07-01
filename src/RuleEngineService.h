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
#include "RuleEngineStore.h"
#include "RuleEngineCore.h"

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
    int init();

    void setRuleChannel(std::shared_ptr<DataChannel> channel);
    void setDeviceChannel(std::shared_ptr<DataChannel> channel);

    bool callMessagePush(std::string title, std::string message);
    bool callInstancePush(std::string insName, std::string slot, std::string value);
    bool callContentPush(std::string id, std::string title, std::string content);
    bool triggerRule(std::string ruleId);

    RuleEngineStore::pointer store() { return mStore; }
    RuleEngineCore::pointer core() { return mCore; }

private:
    std::string mServerRoot;
    RuleEngineCore::pointer mCore;
    RuleEngineStore::pointer mStore;
    DataChannel::pointer mRuleChannel;
    DataChannel::pointer mClassChannel;

}; /* class RuleEngineService */

RuleEngineService& ruleEngine();

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __RuleEngineService_H__ */
