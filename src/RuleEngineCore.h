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
#include <vector>

#ifdef __cplusplus

#define HB_VERSION_MAJOR 1
#define HB_VERSION_MINOR 0
#define HB_VERSION_MICRO 0

namespace HB {

using namespace CLIPS;

class RuleEventHandler;

typedef std::map<std::string, Instance::pointer> InstancesMap;
typedef std::map<std::string, Rule::pointer> RulesMap;

typedef Functor<bool, int, int, std::string, std::string> MsgPushCall;
typedef Functor<bool, std::string, std::string, std::string> InsPushCall;
typedef Functor<bool, std::string, std::string, std::string> TxtPushCall;

typedef std::shared_ptr<MsgPushCall> MsgPushPointer;
typedef std::shared_ptr<InsPushCall> InsPushPointer;
typedef std::shared_ptr<TxtPushCall> TxtPushPointer;

typedef std::function<std::vector<std::string>(int type)> GetFilesCallback;

class RuleEngineCore {
public:
    typedef std::shared_ptr<RuleEngineCore> pointer;
    RuleEngineCore(RuleEventHandler &handler);
    ~RuleEngineCore();

    Environment& driver() { return *mEnv; }
    void setup(MsgPushPointer msgcall, InsPushPointer inscall, TxtPushPointer txtcall);
    void start(std::string &rootDir, GetFilesCallback callback);
    void finalize();
    long assertRun(std::string assert);

    bool handleTimer();
    std::string handleClassSync(const char *clsName, const char *ver, const char *buildStr);
    std::string handleRuleSync(const char *ruleName, const char *ver, const char *buildStr);
    bool handleInstanceAdd(const char *insName, const char *clsName);
    bool handleInstanceDel(const char *insName);
    bool handleInstancePut(const char *insName, const char *slot, const char *value);

    bool handleRuleAdd(const char *rulName);
    bool handleRuleDel(const char *rulName);
    bool refreshRule(const char *rulName);

    void debug(int show);

private:

    void _OnClear(void);
    void _OnPeriodic(void);
    void _OnReset(void);
    void _OnRuleFiring(void);

    int _CallGetDebugLevel();
    std::string _CallGetRootDir();
    Values _CallGetFiles(int fileType);
    Values _CallNow();
    void _CallInitFinished();

private:
    UTILS::Mutex mEnvMutex;
    Environment *mEnv;
    RuleEventHandler &mHandler;
    InstancesMap mInses;
    RulesMap mRules;
    std::string mRootDir;
    GetFilesCallback mGetFilesCB;
}; /* class RuleEngineCore */


} /* namespace HB */

#endif /* __cplusplus */

#endif /* __RuleEngineCore_H__ */
