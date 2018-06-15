/***************************************************************************
 *  RuleEngineCore.cpp - Rule Clips Brain Impl
 *
 *  Created: 2018-06-13 09:28:22
 *
 *  Copyright QRS
 ****************************************************************************/

#include "RuleEngineCore.h"
#include "Router.h"
#include "RuleEventHandler.h"
#include "RuleEventTypes.h"

#include "SysTime.h"

#define CLIPS_ROOT_DIR "clips"
#define CLSES_SEARCH_DIR "classes"
#define RULES_SEARCH_DIR "rules"

#define SHOW_VALUE(item, tok) \
    do { \
        switch (item.type()) { \
        case TYPE_FLOAT: \
            LOGD(tok"value  = float(%f)\n", item.as_float()); break; \
        case TYPE_INTEGER: \
            LOGD(tok"value = int(%d)\n", item.as_integer()); break; \
        case TYPE_SYMBOL: \
        case TYPE_STRING: \
        case TYPE_INSTANCE_NAME: \
            LOGD(tok"value = string(%s)\n", item.as_string().c_str()); break; \
        case TYPE_EXTERNAL_ADDRESS: \
        case TYPE_INSTANCE_ADDRESS: \
            LOGD(tok"value = address(TODO)\n"); \
        } \
    } while(0)

#define SHOW_VALUES(items, tok) \
    do { \
        for (unsigned int i = 0; i < items.size(); ++i) \
            SHOW_VALUE(items[i], tok); \
    } while(0)

using namespace UTILS;

namespace HB {

RuleEngineCore::RuleEngineCore(RuleEventHandler &handler)
    : mHandler(handler)
    , mEnv(0)
{
    LOGTT();
}

RuleEngineCore::~RuleEngineCore()
{
    LOGTT();
    finalize();
}

void RuleEngineCore::init()
{
    LOGTT();

    Mutex::Autolock _l(&mEnvMutex);

    /* logger */
    mEnv = new Environment();
    init_clips_logger(mEnv->cobj());

    /* build defglobal version */
    char data[256] = { 0 };
    snprintf(data, 255,
        "(defglobal\n"
        "  ?*VERSION-MAJOR* = %u\n"
        "  ?*VERSION-MINOR* = %u\n"
        "  ?*VERSION-MICRO* = %u\n"
        ")\n", HB_VERSION_MAJOR, HB_VERSION_MINOR, HB_VERSION_MICRO);
    mEnv->build(data);

    /* regist [clear, periodic, reset, rulefiring] callback */
    mEnv->regist_clear_callback(std::bind(&RuleEngineCore::_OnClear, this));
    mEnv->regist_periodic_callback(std::bind(&RuleEngineCore::_OnPeriodic, this));
    mEnv->regist_reset_callback(std::bind(&RuleEngineCore::_OnReset, this));
    mEnv->regist_rulefiring_callback(std::bind(&RuleEngineCore::_OnRuleFiring, this));

    /* regist function for clips script */
    mEnv->add_function("get-root-dir", std::make_shared<Functor<std::string>>(this, &RuleEngineCore::_CallGetRootDir));
    mEnv->add_function("get-clses-files", std::make_shared<Functor<Values>>(this, &RuleEngineCore::_CallGetClsesFiles));
    mEnv->add_function("get-rules-files", std::make_shared<Functor<Values>>(this, &RuleEngineCore::_CallGetRulesFiles));
    mEnv->add_function("now", std::make_shared<Functor<Values>>(this, &RuleEngineCore::_CallNow));

    mEnv->batch_evaluate(CLIPS_ROOT_DIR "/init.clp");
    mEnv->assert_fact("(init)");
    mEnv->refresh_agenda();
    mEnv->run();
}

void RuleEngineCore::finalize()
{
    LOGTT();
    if (!mEnv)
        return;

    mEnv->assert_fact("(finalize)");
    mEnv->refresh_agenda();
    mEnv->run();

    finalize_clips_logger(mEnv->cobj());
    delete mEnv;
    mEnv = 0;
}

void RuleEngineCore::handleTimer()
{
    Mutex::Autolock _l(&mEnvMutex);

    mEnv->assert_fact("(time (now))");
    mEnv->refresh_agenda();
    mEnv->run();
}

void RuleEngineCore::_OnClear(void)
{
    LOGTT();
}

void RuleEngineCore::_OnPeriodic(void)
{
    LOGTT();
}

void RuleEngineCore::_OnReset(void)
{
    LOGTT();
    mHandler.removeMessages(RET_REFRESH_TIMER);
    mHandler.sendEmptyMessage(RET_REFRESH_TIMER);
}

void RuleEngineCore::_OnRuleFiring(void)
{
    LOGTT();
}

std::string RuleEngineCore::_CallGetRootDir()
{
    LOGTT();
    /* TODO current path: test for debug */
    return CLIPS_ROOT_DIR;
}

Values RuleEngineCore::_CallGetClsesFiles()
{
    LOGTT();
    Values rv;
    /* TODO */
    rv.push_back(std::string(CLSES_SEARCH_DIR "/class-001.clp"));
    rv.push_back(std::string(CLSES_SEARCH_DIR "/class-002.bat"));
    return rv;
}

Values RuleEngineCore::_CallGetRulesFiles()
{
    LOGTT();
    Values rv;
    /* TODO */
    rv.push_back(std::string(RULES_SEARCH_DIR "/rule-001.clp"));
    rv.push_back(std::string(RULES_SEARCH_DIR "/rule-002.bat"));
    return rv;
}

Values RuleEngineCore::_CallNow()
{
    LOGTT();
    SysTime::DateTime dt;
    SysTime::GetDateTime(&dt);
    Values rv;
    rv.push_back(dt.mYear);
    rv.push_back(dt.mMonth);
    rv.push_back(dt.mDay);
    rv.push_back(dt.mHour);
    rv.push_back(dt.mMinute);
    rv.push_back(dt.mSecond);
    return rv;
}

} /* namespace HB */