/***************************************************************************
 *  RuleEngineCore.cpp - Rule Clips Brain Impl
 *
 *  Created: 2018-06-13 09:28:22
 *
 *  Copyright QRS
 ****************************************************************************/

#include "RuleEngineCore.h"
#include "RuleEngineService.h"
#include "Router.h"
#include "RuleEventHandler.h"
#include "RuleEventTypes.h"
#include "SysTime.h"

#include<fstream>
#include <sstream>

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
    : mEnv(0)
    , mHandler(handler)
{
    LOGTT();
}

RuleEngineCore::~RuleEngineCore()
{
    LOGTT();
    mInses.clear();
    finalize();
}

void RuleEngineCore::setup(MsgPushPointer msgcall, InsPushPointer inscall, TxtPushPointer txtcall)
{
    if (!msgcall || !inscall || !txtcall)
        return;

    LOGTT();
    Mutex::Autolock _l(&mEnvMutex);

    /* logger */
    mEnv = new Environment();
    init_clips_logger(mEnv->cobj());

    /* build defglobal version */
    char data[512] = { 0 };
    snprintf(data, 511,
        "(defglobal\n"
        "  ?*VERSION-MAJOR* = %u\n  ?*VERSION-MINOR* = %u\n  ?*VERSION-MICRO* = %u\n"
        "  ?*TYPE-TEM-FILE* = %u\n  ?*TYPE-CLS-FILE* = %u\n  ?*TYPE-RUL-FILE* = %u\n"
        "  ?*MSG-RULE-RESPONSE* = %d\n  ?*RUL-SUCCESS* = %d\n  ?*RUL-FAIL* = %d\n  ?*RUL-TIMEOUT* = %d\n"
        "  ?*MSG-RULE-RHS* = %d\n  ?*RHS-INS-NOT-FOUND* = %d\n  ?*RHS-NTF-WRONG-TYPE* = %d\n  ?*RHS-SEE-NOT-FOUND* = %d\n"
        ")\n",
        HB_VERSION_MAJOR, HB_VERSION_MINOR, HB_VERSION_MICRO,
        TYPE_TEM_FILE, TYPE_CLS_FILE, TYPE_RUL_FILE,
        MSG_RULE_RESPONSE, RUL_SUCCESS, RUL_FAIL, RUL_TIMEOUT,
        MSG_RULE_RHS, RHS_INS_NOT_FOUND, RHS_NTF_WRONG_TYPE, RHS_SEE_NOT_FOUND);
    mEnv->build(data);

    /* regist [clear, periodic, reset, rulefiring] callback */
    mEnv->regist_clear_callback(std::bind(&RuleEngineCore::_OnClear, this));
    mEnv->regist_periodic_callback(std::bind(&RuleEngineCore::_OnPeriodic, this));
    mEnv->regist_reset_callback(std::bind(&RuleEngineCore::_OnReset, this));
    mEnv->regist_rulefiring_callback(std::bind(&RuleEngineCore::_OnRuleFiring, this));

    /* regist function for clips script */
    mEnv->add_function("get-debug-level", std::make_shared<Functor<int>>(this, &RuleEngineCore::_CallGetDebugLevel));
    mEnv->add_function("get-root-dir", std::make_shared<Functor<std::string>>(this, &RuleEngineCore::_CallGetRootDir));
    mEnv->add_function("get-files", std::make_shared<Functor<Values, int>>(this, &RuleEngineCore::_CallGetFiles));
    mEnv->add_function("now", std::make_shared<Functor<Values>>(this, &RuleEngineCore::_CallNow));
    mEnv->add_function("init-finished", std::make_shared<Functor<void>>(this, &RuleEngineCore::_CallInitFinished));

    mEnv->add_function("msg-push", msgcall);
    mEnv->add_function("ins-push", inscall);
    mEnv->add_function("txt-push", txtcall);

}

void RuleEngineCore::start(std::string &rootDir, GetFilesCallback callback)
{
    LOGTT();
    mRootDir = rootDir;
    mGetFilesCB = callback;
    mEnv->batch_evaluate(mRootDir + "/init.clp");
    assertRun("(init)");
}

long RuleEngineCore::assertRun(std::string assert)
{
    mEnv->assert_fact(assert);
    mEnv->refresh_agenda();
    return mEnv->run();
}

void RuleEngineCore::finalize()
{
    LOGTT();
    if (!mEnv)
        return;

    assertRun("(finalize)");

    finalize_clips_logger(mEnv->cobj());
    delete mEnv;
    mEnv = 0;
}

void RuleEngineCore::debug(int show)
{
    LOGI("===== BEGIN ===== \n");
    switch (show) {
        case DEBUG_SHOW_ALL:
        case DEBUG_SHOW_CLASSES:
            LOGI(">> show classes:\n");
            assertRun("(show classes)");
            if (show)
                break;
        case DEBUG_SHOW_RULES:
            LOGI(">> show rules:\n");
            assertRun("(show rules)");
            if (show)
                break;
        case DEBUG_SHOW_INSTANCES:
            LOGI(">> show instances:\n");
            assertRun("(show instances)");
            {
                InstancesMap::iterator it;
                for (it = mInses.begin(); it != mInses.end(); ++it) {
                    LOGI("\tInstance[%s]:\n", it->first.c_str());
                    it->second->send("print");
                }
            }
            if (show)
                break;
        case DEBUG_SHOW_FACTS:
            LOGI(">> show facts:\n");
            assertRun("(show facts)");
            if (show)
                break;
        case DEBUG_SHOW_AGENDA:
            LOGI(">> show agenda:\n");
            assertRun("(show agenda)");
            if (show)
                break;
        case DEBUG_SHOW_MEMORY:
            LOGI(">> show memory:\n");
            assertRun("(show memory)");
            break;
    }
    LOGI("===== END ===== \n");
}

bool RuleEngineCore::handleTimer()
{
    Mutex::Autolock _l(&mEnvMutex);

    assertRun("(datetime (now))");

    /* false: again periodicly */
    return false;
}

std::string RuleEngineCore::handleClassSync(const char *clsName, const char *ver, const char *buildStr)
{
    LOGI("(%s, %s)\n%s\n", clsName, ver, buildStr);
    if (!clsName || !ver || !buildStr)
        return std::string("");

    Mutex::Autolock _l(&mEnvMutex);
    Class::pointer cls = mEnv->get_class(clsName);
    if (cls) {
        /* TODO: cannot delete? */
        if (!cls->undefine()) {
            LOGW("delete class[%s] fail! possible using in rule LHS.\n", clsName);
            return std::string("");
        }
    }
    if (!mEnv->build(buildStr)) {
        LOGW("build class [%s] error!\n", clsName);
        return std::string("");
    }

    std::string path(mRootDir + "/");
    path.append(CLSES_SEARCH_DIR).append(clsName).append(".clp");
    std::ofstream of(path);
    if (!of.is_open())
        return std::string("");
    of << buildStr << std::endl;
    of.close();
    return std::move(path);
}

std::string RuleEngineCore::handleRuleSync(const char *ruleName, const char *ver, const char *buildStr)
{
    LOGI("(%s, %s)\n%s\n", ruleName, ver, buildStr);
    if (!ruleName || !ver || !buildStr)
        return std::string("");

    Mutex::Autolock _l(&mEnvMutex);
    Rule::pointer rule = mEnv->get_rule(ruleName);
    if (rule)
        rule->retract();

    if (!mEnv->build(buildStr)) {
        LOGW("build rule [%s] error!\n", ruleName);
        return std::string("");
    }

    std::string path(mRootDir + "/");
    path.append(RULES_SEARCH_DIR).append(ruleName).append(".clp");
    std::ofstream of(path);
    if (!of.is_open())
        return std::string("");
    of << buildStr << std::endl;
    of.close();
    return std::move(path);
}

bool RuleEngineCore::handleInstanceAdd(const char *insName, const char *clsName)
{
    LOGI("(%s %s)\n", insName, clsName);
    if (!insName || !clsName)
        return false;

    Mutex::Autolock _l(&mEnvMutex);
    InstancesMap::iterator it = mInses.find(insName);
    if (it != mInses.end()) {
        LOGW("Instance[%s] already exists!\n", insName);
        return false;
    }
    std::stringstream ss;
    ss << "(" << insName << " of " << clsName << ")";
    Instance::pointer ins = mEnv->make_instance(ss.str().c_str());
    if (!ins) {
        LOGW("Make instance[%s] fail!\n", insName);
        return false;
    }
    mInses.insert(std::pair<std::string, Instance::pointer>(insName, ins));
    LOGD("Make instance[%s] success!\n", insName);
    return true;
}

bool RuleEngineCore::handleInstanceDel(const char *insName)
{
    LOGI("(%s)\n", insName);
    if (!insName)
        return false;

    Mutex::Autolock _l(&mEnvMutex);
    InstancesMap::iterator it = mInses.find(insName);
    if (it == mInses.end()) {
        LOGW("Not found instance[%s]!\n", insName);
        return false;
    }
    mInses.erase(it);
    return true;
}

bool RuleEngineCore::handleInstancePut(const char *insName, const char *slot, const char *value)
{
    LOGI("(%s %s %s)\n", insName, slot, value);
    if (!insName || !slot || !value)
        return false;

    Mutex::Autolock _l(&mEnvMutex);
    InstancesMap::iterator it = mInses.find(insName);
    if (it == mInses.end()) {
        LOGW("Not found instance[%s]!\n", insName);
        return false;
    }
    it->second->send(std::string("put-").append(slot), value);

    mEnv->refresh_agenda();
    mEnv->run();
    return true;
}

bool RuleEngineCore::handleRuleAdd(const char *rulName)
{
    LOGI("(add rule: %s)\n", rulName);
    if (!rulName)
        return false;
    Mutex::Autolock _l(&mEnvMutex);

    RulesMap::iterator it = mRules.find(rulName);
    if (it != mRules.end())
        return false;

    Rule::pointer rul = mEnv->get_rule(rulName);
    if (!rul) {
        LOGW("get_rule(%s) error!\n", rulName);
        return false;
    }
    mRules.insert(std::pair<std::string, Rule::pointer>(rulName,rul));
    return true;
}

bool RuleEngineCore::handleRuleDel(const char *rulName)
{
    LOGI("(del rule: %s)\n", rulName);
    if (!rulName)
        return false;
    Mutex::Autolock _l(&mEnvMutex);

    RulesMap::iterator it = mRules.find(rulName);
    if (it == mRules.end()) {
        LOGW("Not found rule[%s]!\n", rulName);
        return false;
    }
    /* delete rule */
    it->second->retract();
    mRules.erase(it);
    return true;
}

bool RuleEngineCore::refreshRule(const char *rulName)
{
    LOGI("refreshRule(%s)\n", rulName);
    if (!rulName)
        return false;
    Mutex::Autolock _l(&mEnvMutex);

    RulesMap::iterator it = mRules.find(rulName);
    if (it == mRules.end()) {
        LOGW("Not found rule[%s]!\n", rulName);
        return false;
    }
    it->second->refresh();
    return true;
}

void RuleEngineCore::_OnClear(void)
{
    /* LOGTT(); */
}

void RuleEngineCore::_OnPeriodic(void)
{
    /* LOGTT(); */
}

void RuleEngineCore::_OnReset(void)
{
    LOGTT();
    mHandler.removeMessages(RET_REFRESH_TIMER);
    mHandler.sendEmptyMessage(RET_REFRESH_TIMER);
}

void RuleEngineCore::_OnRuleFiring(void)
{
    /* LOGTT(); */
}

int RuleEngineCore::_CallGetDebugLevel()
{
    return getLogLevel();
}

std::string RuleEngineCore::_CallGetRootDir()
{
    return mRootDir;
}

Values RuleEngineCore::_CallGetFiles(int fileType)
{
    Values rv;
    if (mGetFilesCB) {
        std::vector<std::string> files;
        files = mGetFilesCB(fileType);
        for (size_t i = 0; i < files.size(); ++i) {
            LOGD("clp file: [%s]\n", files[i].c_str());
            rv.push_back(files[i]);
        }
    }
    return rv;
}

Values RuleEngineCore::_CallNow()
{
    SysTime::DateTime dt;
    SysTime::GetDateTime(&dt);
    Values rv;
    rv.push_back(SysTime::GetMSecs()); /* clock time */
    rv.push_back(dt.mYear);
    rv.push_back(dt.mMonth);
    rv.push_back(dt.mDay);
    rv.push_back(dt.mHour);
    rv.push_back(dt.mMinute);
    rv.push_back(dt.mSecond);
    rv.push_back(dt.mDayOfWeek); /* week day */
    return rv;
}

void RuleEngineCore::_CallInitFinished()
{
    Module::pointer mod = mEnv->get_module("MAIN");
    if (mod) {
        std::vector<std::string> rules = mEnv->get_rule_names(mod);
        for (size_t i = 0; i < rules.size(); ++i)
            handleRuleAdd(rules[i].c_str());
    }
}

} /* namespace HB */
