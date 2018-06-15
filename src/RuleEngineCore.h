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

#ifdef __cplusplus

#define HB_VERSION_MAJOR 1
#define HB_VERSION_MINOR 0
#define HB_VERSION_MICRO 0

namespace HB {

using namespace CLIPS;

class RuleEventHandler;

class RuleEngineCore {
public:
    RuleEngineCore(RuleEventHandler &handler);
    ~RuleEngineCore();

    void init();
    void finalize();
    void handleTimer();

private:

    void _OnClear(void);
    void _OnPeriodic(void);
    void _OnReset(void);
    void _OnRuleFiring(void);

    std::string _CallGetRootDir();
    Values _CallGetClsesFiles();
    Values _CallGetRulesFiles();
    Values _CallNow();

private:
    RuleEventHandler &mHandler;
    Environment *mEnv;
    UTILS::Mutex mEnvMutex;
}; /* class RuleEngineCore */


} /* namespace HB */

#endif /* __cplusplus */

#endif /* __RuleEngineCore_H__ */
