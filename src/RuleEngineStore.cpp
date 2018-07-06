/***************************************************************************
 *  RuleEngineStore.cpp - Rule Engine Store Impl
 *
 *  Created: 2018-06-26 11:07:12
 *
 *  Copyright QRS
 ****************************************************************************/

#include "RuleEngineStore.h"
#include "DefTemplateTable.h"
#include "DefClassTable.h"
#include "DefRuleTable.h"
#include "RuleEventHandler.h"

#include "Log.h"

#define DELAY_TIME (5*1000)

namespace HB {

RuleEngineStore::RuleEngineStore(const std::string db)
    : mHandler(ruleHandler())
    , mDBFilePath(db), mDB(0)
    , mDefClassTab(0)
    , mDefRuleTab(0)
{
    LOGTT();
}

RuleEngineStore::~RuleEngineStore()
{
    LOGTT();
    close();
}

bool RuleEngineStore::open()
{
    LOGTT();
    mHandler.removeMessages(RET_STORE_CLOSE);
    mHandler.sendMessageDelayed(
        mHandler.obtainMessage(RET_STORE_CLOSE, shared_from_this()), DELAY_TIME);
    if (isOpen())
        return true;
    mDB = new SQLiteDatabase();
    if (!mDB->open(mDBFilePath.c_str()))
        return false;
    mDefTmplTab = new DefTemplateTable(*mDB);
    mDefClassTab = new DefClassTable(*mDB);
    mDefRuleTab = new DefRuleTable(*mDB);
    return true;
}

bool RuleEngineStore::close()
{
    LOGTT();
    Mutex::Autolock _l(&mDBMutex);
    if (mDefClassTab)
        delete mDefClassTab;
    mDefClassTab = 0;
    if (mDefRuleTab)
        delete mDefRuleTab;
    mDefRuleTab = 0;
    if (mDB)
        mDB->close();
    mDB = 0;
    return true;
}

DefTable* RuleEngineStore::_GetTable(TableType type)
{
    switch (type) {
        case TT_DEFTEMPLATE:
            return mDefTmplTab;
        case TT_DEFCLASS:
            return mDefClassTab;
        case TT_DEFRULE:
            return mDefRuleTab;
    }
    return 0;
}

bool RuleEngineStore::_UpdateDefTable(DefTable *table, const char *defName, const char *version, const char *fileName)
{
    if (!open() || !table)
        return false;

    LOGTT();
    Mutex::Autolock _l(&mDBMutex);
    DefInfo info;
    info.mDefName = defName;
    info.mVersion = version;
    info.mFilePath = fileName;
    if (!table->updateOrInsert(info))
        return false;
    return true;
}

std::vector<std::string> RuleEngineStore::_QueryDefFilePaths(DefTable *table, bool urgent)
{
    if (!open() || !table)
        return std::vector<std::string>();

    if (urgent) {
        /* not used yet */
        return std::vector<std::string>();
    }

    LOGTT();
    Mutex::Autolock _l(&mDBMutex);
    return std::move(table->getFilePaths());
}

bool RuleEngineStore::updateTemplateTable(const char *tmplName, const char *version, const char *fileName)
{
    return _UpdateDefTable(_GetTable(TT_DEFTEMPLATE), tmplName, version, fileName);
}

bool RuleEngineStore::updateClassTable(const char *clsName, const char *version, const char *fileName)
{
    return _UpdateDefTable(_GetTable(TT_DEFCLASS), clsName, version, fileName);
}

bool RuleEngineStore::updateRuleTable(const char *ruleName, const char *version, const char *fileName)
{
    return _UpdateDefTable(_GetTable(TT_DEFRULE), ruleName, version, fileName);
}

std::vector<std::string> RuleEngineStore::queryTemplateFilePaths(bool urgent)
{
    return std::move(_QueryDefFilePaths(_GetTable(TT_DEFTEMPLATE), urgent));
}

std::vector<std::string> RuleEngineStore::queryClassFilePaths(bool urgent)
{
    return std::move(_QueryDefFilePaths(_GetTable(TT_DEFCLASS), urgent));
}

std::vector<std::string> RuleEngineStore::queryRuleFilePaths(bool urgent)
{
    return std::move(_QueryDefFilePaths(_GetTable(TT_DEFRULE), urgent));
}

} /* namespace HB */
