/***************************************************************************
 *  RuleEngineCore.h - Rule Clips Brain
 *
 *  Created: 2018-06-13 09:26:42
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __RuleEngineCore_H__
#define __RuleEngineCore_H__

#include "Environment.h"
#include "Mutex.h"
#include <map>

#ifdef __cplusplus

#define HB_VERSION_MAJOR 1
#define HB_VERSION_MINOR 0
#define HB_VERSION_MICRO 0

namespace HB {

using namespace CLIPS;

class RuleEventHandler;

class RuleEngineCore {
public:
    RuleEngineCore(RuleEventHandler &handler, std::string &rootdir);
    ~RuleEngineCore();

    void init();
    void finalize();

    void handleTimer();
    void handleClassSync(const char *clsName, const char *buildStr);
    void handleRuleSync(const char *ruleName, const char *ruleID, const char *buildStr);
    void handleInstanceAdd(const char *insName, const char *clsName);
    void handleInstanceDel(const char *insName);
    void handleInstancePut(const char *insName, const char *slot, const char *value);

private:

    void _OnClear(void);
    void _OnPeriodic(void);
    void _OnReset(void);
    void _OnRuleFiring(void);

    int _CallGetDebugLevel();
    std::string _CallGetRootDir();
    Values _CallGetClsesFiles();
    Values _CallGetRulesFiles();
    Values _CallNow();

private:
    typedef std::map<std::string, Instance::pointer>::iterator InsesIt;
    UTILS::Mutex mEnvMutex;
    RuleEventHandler &mHandler;
    Environment *mEnv;
    std::string &mRootDir;
    std::map<std::string, Instance::pointer> mInses;
}; /* class RuleEngineCore */


} /* namespace HB */

#endif /* __cplusplus */

#endif /* __RuleEngineCore_H__ */
