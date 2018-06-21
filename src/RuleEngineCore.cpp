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

#include<fstream>
#include <sstream>

#define CLSES_SEARCH_DIR "classes/"
#define RULES_SEARCH_DIR "rules/"

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

RuleEngineCore::RuleEngineCore(RuleEventHandler &handler, std::string &rootdir)
    : mHandler(handler)
    , mEnv(0), mRootDir(rootdir)
{
    LOGTT();
}

RuleEngineCore::~RuleEngineCore()
{
    LOGTT();
    mInses.clear();
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
    mEnv->add_function("get-debug-level", std::make_shared<Functor<int>>(this, &RuleEngineCore::_CallGetDebugLevel));
    mEnv->add_function("get-root-dir", std::make_shared<Functor<std::string>>(this, &RuleEngineCore::_CallGetRootDir));
    mEnv->add_function("get-clses-files", std::make_shared<Functor<Values>>(this, &RuleEngineCore::_CallGetClsesFiles));
    mEnv->add_function("get-rules-files", std::make_shared<Functor<Values>>(this, &RuleEngineCore::_CallGetRulesFiles));
    mEnv->add_function("now", std::make_shared<Functor<Values>>(this, &RuleEngineCore::_CallNow));

    mEnv->batch_evaluate(mRootDir + "/init.clp");
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

void RuleEngineCore::handleClassSync(const char *clsName, const char *buildStr)
{
    LOGT("(%s)\n%s\n", clsName, buildStr);
    if (!clsName || !buildStr)
        return;

    Mutex::Autolock _l(&mEnvMutex);
    Class::pointer cls = mEnv->get_class(clsName);
    if (cls)
        cls->undefine();
    if (!mEnv->build(buildStr)) {
        LOGW("build class [%s] error!\n", clsName);
        return;
    }

    /* write to file */
    std::string filename("");
    std::string fullname("");
    filename.append("dev-").append(clsName).append(".clp");
    fullname.append(mRootDir).append("/").append(CLSES_SEARCH_DIR).append(filename);
    std::ofstream of(fullname);
    if (of.is_open()) {
        of << buildStr << std::endl;
        LOGD("write rule [%s]\n", fullname.c_str());
        of.close();
    }
}

void RuleEngineCore::handleRuleSync(const char *ruleName, const char *ruleID, const char *buildStr)
{
    LOGD("(%s, %s)\n%s\n", ruleName, ruleID, buildStr);
    if (!ruleName || !ruleID || !buildStr)
        return;

    Mutex::Autolock _l(&mEnvMutex);
    Rule::pointer rule = mEnv->get_rule(ruleName);
    if (rule)
        rule->retract();

    if (!mEnv->build(buildStr)) {
        LOGW("build rule [%s] error!\n", ruleName);
        return;
    }

    /* write to file */
    std::string filename("");
    std::string fullname("");
    filename.append("rule-").append(ruleID).append(".clp");
    fullname.append(mRootDir).append("/").append(RULES_SEARCH_DIR).append(filename);
    std::ofstream of(fullname);
    if (of.is_open()) {
        of << buildStr << std::endl;
        LOGD("write rule [%s]\n", fullname.c_str());
        of.close();
    }

    /* update database */
}

void RuleEngineCore::handleInstanceAdd(const char *insName, const char *clsName)
{
    LOGT("(%s %s)\n", insName, clsName);
    if (!insName || !clsName)
        return;

    Mutex::Autolock _l(&mEnvMutex);
    InsesIt it = mInses.find(insName);
    if (it != mInses.end()) {
        LOGW("Instance[%s] already exists!\n", insName);
        return;
    }
    std::stringstream ss;
    ss << "(" << insName << " of " << clsName << ")";
    Instance::pointer ins = mEnv->make_instance(ss.str().c_str());
    if (!ins) {
        LOGW("Make instance[%s] fail!\n", insName);
        return;
    }
    mInses.insert(std::pair<std::string, Instance::pointer>(insName, ins));
    LOGD("Make instance[%s] success!\n", insName);
    mEnv->refresh_agenda();
    mEnv->run();

    /* TODO DEBUG */
    ins->send("print");
    mEnv->assert_fact("(show instances)");
}

void RuleEngineCore::handleInstanceDel(const char *insName)
{
    LOGT("(%s)\n", insName);
    if (!insName)
        return;

    Mutex::Autolock _l(&mEnvMutex);
    InsesIt it = mInses.find(insName);
    if (it == mInses.end()) {
        LOGW("Not found instance[%s]!\n", insName);
        return;
    }
    mInses.erase(it);

    /* TODO DEBUG */
    mEnv->assert_fact("(show instances)");
}

void RuleEngineCore::handleInstancePut(const char *insName, const char *slot, const char *value)
{
    LOGT("(%s %s %s)\n", insName, slot, value);
    if (!insName || !slot || !value)
        return;

    Mutex::Autolock _l(&mEnvMutex);

    InsesIt it = mInses.find(insName);
    if (it == mInses.end()) {
        LOGW("Not found instance[%s]!\n", insName);
        return;
    }
    it->second->send(std::string("put-").append(slot), value);

    mEnv->refresh_agenda();
    mEnv->run();

    /* TODO DEBUG */
    it->second->send("print");
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

int RuleEngineCore::_CallGetDebugLevel()
{
    return getLogLevel();
}

std::string RuleEngineCore::_CallGetRootDir()
{
    LOGTT();
    /* TODO current path: test for debug */
    return mRootDir;
}

Values RuleEngineCore::_CallGetClsesFiles()
{
    LOGTT();
    Values rv;
    /* TODO */
    rv.push_back(std::string(CLSES_SEARCH_DIR "class-001.clp"));
    rv.push_back(std::string(CLSES_SEARCH_DIR "class-002.bat"));
    rv.push_back(std::string(CLSES_SEARCH_DIR "class-test.clp"));
    return rv;
}

Values RuleEngineCore::_CallGetRulesFiles()
{
    LOGTT();
    Values rv;
    /* TODO */
    rv.push_back(std::string(RULES_SEARCH_DIR "rule-001.clp"));
    rv.push_back(std::string(RULES_SEARCH_DIR "rule-002.bat"));
    rv.push_back(std::string(RULES_SEARCH_DIR "rule-test.clp"));
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
