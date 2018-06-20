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
#include <memory>

#ifdef __cplusplus

using namespace UTILS;

namespace HB {

class DataChannel;
class RuleEngineCore;

class RuleEngineService : public MessageHandler::Callback {
public:
    RuleEngineService();
    ~RuleEngineService();

    bool handleMessage(Message *msg);

    bool newRuleFound(std::string ruleId);
    void setServerRoot(std::string rootDir) { mServerRoot = rootDir; }
    int init();

    void setRuleChannel(std::shared_ptr<DataChannel> channel);
    void setDeviceChannel(std::shared_ptr<DataChannel> channel);

private:
    RuleEngineCore *mCore;
    std::string mServerRoot;
    std::shared_ptr<DataChannel> mRuleChannel;
    std::shared_ptr<DataChannel> mDeviceChannel;

}; /* class RuleEngineService */

RuleEngineService& ruleEngine();

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __RuleEngineService_H__ */
