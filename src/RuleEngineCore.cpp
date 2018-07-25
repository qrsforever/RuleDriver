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
#include "RuleEngineLog.h"

#include<fstream>
#include <sstream>

#define SHOW_VALUE(item, tok) \
    do { \
        switch (item.type()) { \
        case TYPE_FLOAT: \
            RE_LOGD(tok"value  = float(%f)\n", item.as_float()); break; \
        case TYPE_INTEGER: \
            RE_LOGD(tok"value = int(%d)\n", item.as_integer()); break; \
        case TYPE_SYMBOL: \
        case TYPE_STRING: \
        case TYPE_INSTANCE_NAME: \
            RE_LOGD(tok"value = string(%s)\n", item.as_string().c_str()); break; \
        case TYPE_EXTERNAL_ADDRESS: \
        case TYPE_INSTANCE_ADDRESS: \
            RE_LOGD(tok"value = address(TODO)\n"); \
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
    mLogLevel = getModuleLogLevel("clips");
    RE_LOGTT();
}

RuleEngineCore::~RuleEngineCore()
{
    RE_LOGTT();
    mInses.clear();
    finalize();
}

void RuleEngineCore::setup(MsgPushPointer msgcall, InsPushPointer inscall, TxtPushPointer txtcall)
{
    if (!msgcall || !inscall || !txtcall)
        return;

    RE_LOGTT();
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

    mEnv->setCallback(this);

    /* regist function for clips script */
    mEnv->add_function("get-debug-level", std::make_shared<Functor<int>>(this, &RuleEngineCore::onCallGetDebugLevel));
    mEnv->add_function("get-root-dir", std::make_shared<Functor<std::string>>(this, &RuleEngineCore::onCallGetRootDir));
    mEnv->add_function("get-files", std::make_shared<Functor<Values, int>>(this, &RuleEngineCore::onCallGetFiles));
    mEnv->add_function("now", std::make_shared<Functor<Values>>(this, &RuleEngineCore::onCallNow));
    mEnv->add_function("init-finished", std::make_shared<Functor<void>>(this, &RuleEngineCore::onCallInitFinished));

    mEnv->add_function("msg-push", msgcall);
    mEnv->add_function("ins-push", inscall);
    mEnv->add_function("txt-push", txtcall);
}

void RuleEngineCore::start(std::string &rootDir, GetFilesCallback callback)
{
    RE_LOGTT();
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
    RE_LOGTT();
    if (!mEnv)
        return;

    assertRun("(finalize)");

    finalize_clips_logger(mEnv->cobj());
    delete mEnv;
    mEnv = 0;
}

void RuleEngineCore::debug(int show)
{
    RE_LOGI("===== BEGIN ===== \n");
    switch (show) {
        case DEBUG_SHOW_ALL:
        case DEBUG_SHOW_CLASSES:
            RE_LOGI(">> show classes:\n");
            assertRun("(show classes)");
            if (show)
                break;
        case DEBUG_SHOW_RULES:
            RE_LOGI(">> show rules:\n");
            assertRun("(show rules)");
            if (show)
                break;
        case DEBUG_SHOW_INSTANCES:
            RE_LOGI(">> show instances:\n");
            assertRun("(show instances)");
            {
                InstancesMap::iterator it;
                for (it = mInses.begin(); it != mInses.end(); ++it) {
                    RE_LOGI("\tInstance[%s]:\n", it->first.c_str());
                    it->second->send("print");
                }
            }
            if (show)
                break;
        case DEBUG_SHOW_FACTS:
            RE_LOGI(">> show facts:\n");
            assertRun("(show facts)");
            if (show)
                break;
        case DEBUG_SHOW_AGENDA:
            RE_LOGI(">> show agenda:\n");
            assertRun("(show agenda)");
            if (show)
                break;
        case DEBUG_SHOW_MEMORY:
            RE_LOGI(">> show memory:\n");
            assertRun("(show memory)");
            break;
    }
    RE_LOGI("===== END ===== \n");
}

void RuleEngineCore::setLogLevel(int level)
{
    Global::pointer log = 0;
    log = mEnv->get_global("MAIN::LOG-LEVEL");
    if (log) {
        Value v(level);
        log->set_value(v);
        mLogLevel = level;
    }
}

std::vector<std::string> RuleEngineCore::getClassNames(const char *slotName)
{
    Mutex::Autolock _l(&mEnvMutex);

    std::vector<std::string> devices;
    Class::pointer cls = mEnv->get_class_list_head();
    for (; cls != 0; cls = cls->next()) {
        if (slotName && !cls->slot_exists(slotName, true))
            continue;
        devices.push_back(cls->name());
    }
    return std::move(devices);
}

std::vector<std::string> RuleEngineCore::getSlotNames(const char *clsName)
{
    if (!clsName)
        return std::vector<std::string>();

    Mutex::Autolock _l(&mEnvMutex);
    std::vector<std::string> slots;
    Class::pointer cls = mEnv->get_class(clsName);
    if (cls) {
        std::vector<std::string> names = cls->slot_names(false);
        for (uint32_t i = 0; i < names.size(); ++i)
            slots.push_back(names[i]);
    }
    return std::move(slots);
}

std::vector<std::string> RuleEngineCore::getObjectNames(const char *clsName)
{
    if (!clsName)
        return std::vector<std::string>();

    Mutex::Autolock _l(&mEnvMutex);

    std::vector<std::string> names;
    Class::pointer cls;
    InstancesMap::iterator it = mInses.begin();
    for (; it != mInses.end(); ++it) {
        Class::pointer cls = it->second->getClass();
        if (cls && cls->name() == clsName)
            names.push_back(it->first);
    }
    return std::move(names);
}

std::string RuleEngineCore::getObjectValue(const char *insName, const char *slotName)
{
    if (!insName || !slotName)
        return std::string("");

    Mutex::Autolock _l(&mEnvMutex);

    std::string res("");
    InstancesMap::iterator it = mInses.find(insName);
    if (it != mInses.end()) {
        Values rv;
        char value[64] = {0};
        rv = it->second->send(std::string("get-").append(slotName));
        if (rv.size() == 1) {
            switch (rv[0].type()) {
                case TYPE_FLOAT:
                    sprintf(value, "%f", rv[0].as_float());
                    break;
                case TYPE_INTEGER:
                    sprintf(value, "%ld", rv[0].as_integer());
                    break;
                case TYPE_SYMBOL:
                case TYPE_STRING:
                case TYPE_INSTANCE_NAME:
                    snprintf(value, 63, "%s", rv[0].as_string().c_str());
                    break;
                default:
                    break;
            }
            res = value;
        }
    }
    return std::move(res);
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
    RE_LOGI("(%s, %s)\n%s\n", clsName, ver, buildStr);
    if (!clsName || !ver || !buildStr)
        return std::string("");

    Mutex::Autolock _l(&mEnvMutex);
    Class::pointer cls = mEnv->get_class(clsName);
    if (cls) {
        /* TODO: cannot delete? */
        if (!cls->undefine()) {
            RE_LOGW("delete class[%s] fail! possible using in rule LHS.\n", clsName);
            return std::string("");
        }
    }
    if (!mEnv->build(buildStr)) {
        RE_LOGW("build class [%s] error!\n", clsName);
        return std::string("");
    }

    std::string path;
    path.append(mRootDir).append("/").append(CLSES_SEARCH_DIR);
    path.append(clsName).append(".clp");

    std::ofstream of(path);
    if (!of.is_open())
        return std::string("");
    of << buildStr << std::endl;
    of.close();
    RE_LOGTT();
    return std::move(path);
}

std::string RuleEngineCore::handleRuleSync(const char *ruleId, const char *ver, const char *buildStr)
{
    RE_LOGI("(%s, %s)\n%s\n", ruleId, ver, buildStr);
    if (!ruleId || !ver || !buildStr)
        return std::string("");

    Mutex::Autolock _l(&mEnvMutex);
    Rule::pointer rule = mEnv->get_rule(ruleId);
    if (rule)
        rule->retract();
    if (!mEnv->build(buildStr)) {
        RE_LOGW("build rule [%s] error!\n", ruleId);
        return std::string("");
    }

    std::string path;
    path.append(mRootDir).append("/").append(RULES_SEARCH_DIR);
    path.append(ruleId).append(".clp");

    std::ofstream of(path);
    if (!of.is_open())
        return std::string("");
    of << buildStr << std::endl;
    of.close();
    return std::move(path);
}

bool RuleEngineCore::handleInstanceAdd(const char *insName, const char *clsName)
{
    RE_LOGI("(%s %s)\n", insName, clsName);
    if (!insName || !clsName)
        return false;

    Mutex::Autolock _l(&mEnvMutex);
    InstancesMap::iterator it = mInses.find(insName);
    if (it != mInses.end()) {
        RE_LOGW("Instance[%s] already exists!\n", insName);
        return false;
    }
    std::stringstream ss;
    ss << "(" << insName << " of " << clsName << ")";
    Instance::pointer ins = mEnv->make_instance(ss.str().c_str());
    if (!ins) {
        RE_LOGW("Make instance[%s] fail!\n", insName);
        return false;
    }
    mInses.insert(std::pair<std::string, Instance::pointer>(insName, ins));
    RE_LOGD("Make instance[%s] success!\n", insName);
    return true;
}

bool RuleEngineCore::handleInstanceDel(const char *insName)
{
    RE_LOGI("(%s)\n", insName);
    if (!insName)
        return false;

    Mutex::Autolock _l(&mEnvMutex);
    InstancesMap::iterator it = mInses.find(insName);
    if (it == mInses.end()) {
        RE_LOGW("Not found instance[%s]!\n", insName);
        return false;
    }
    mInses.erase(it);
    return true;
}

bool RuleEngineCore::handleInstancePut(const char *insName, const char *slot, const char *value)
{
    RE_LOGI("(%s %s %s)\n", insName, slot, value);
    if (!insName || !slot || !value)
        return false;

    Mutex::Autolock _l(&mEnvMutex);
    InstancesMap::iterator it = mInses.find(insName);
    if (it == mInses.end()) {
        RE_LOGW("Not found instance[%s]!\n", insName);
        return false;
    }
    it->second->send(std::string("put-").append(slot), value);

    mEnv->refresh_agenda();
    mEnv->run();
    return true;
}

bool RuleEngineCore::enableRule(const char *ruleId)
{
    RE_LOGI("enable rule [%s]\n", ruleId);
    if (!ruleId)
        return false;

    std::string path;
    path.append(mRootDir).append("/").append(RULES_SEARCH_DIR);
    path.append(ruleId).append(".clp");

    std::stringstream buildStr;
    std::ifstream in;
    in.open(path, std::ifstream::in);
    if (!in.is_open())
        return false;
    buildStr << in.rdbuf();
    in.close();

    Mutex::Autolock _l(&mEnvMutex);

    Rule::pointer rule = mEnv->get_rule(ruleId);
    if (rule)
        return true;

    if (!mEnv->build(buildStr.str())) {
        RE_LOGW("build rule [%s] error!\n", ruleId);
        return false;
    }
    return true;
}

bool RuleEngineCore::disableRule(const char *ruleId)
{
    RE_LOGI("(disable rule: %s)\n", ruleId);
    if (!ruleId)
        return false;
    Mutex::Autolock _l(&mEnvMutex);

    Rule::pointer rule = mEnv->get_rule(ruleId);
    if (!rule)
        return true;
    return rule->retract();
}

bool RuleEngineCore::refreshRule(const char *ruleId)
{
    RE_LOGI("refreshRule(%s)\n", ruleId);
    if (!ruleId)
        return false;
    Mutex::Autolock _l(mEnvMutex);

    Rule::pointer rule = mEnv->get_rule(ruleId);
    if (!rule)
        return false;
    rule->refresh();
    return true;
}

void RuleEngineCore::onCallClear()
{
    /* RE_LOGTT(); */
}

void RuleEngineCore::onCallReset(void)
{
    RE_LOGTT();
    mHandler.removeMessages(RET_REFRESH_TIMER);
    mHandler.sendEmptyMessage(RET_REFRESH_TIMER);
}

void RuleEngineCore::onPeriodic()
{
    /* RE_LOGTT(); */
}

void RuleEngineCore::onRuleFiring()
{
    /* RE_LOGTT(); */
}

int RuleEngineCore::onCallGetDebugLevel()
{
    return mLogLevel;
}

std::string RuleEngineCore::onCallGetRootDir()
{
    return mRootDir;
}

Values RuleEngineCore::onCallGetFiles(int fileType)
{
    Values rv;
    if (mGetFilesCB) {
        std::vector<std::string> files;
        files = mGetFilesCB(fileType);
        for (size_t i = 0; i < files.size(); ++i) {
            RE_LOGD("clp file: [%s]\n", files[i].c_str());
            rv.push_back(files[i]);
        }
    }
    return rv;
}

Values RuleEngineCore::onCallNow()
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

void RuleEngineCore::onCallInitFinished()
{

}

} /* namespace HB */
