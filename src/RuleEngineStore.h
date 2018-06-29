/***************************************************************************
 *  RuleEngineStore.h - Rule Engine Store Header
 *
 *  Created: 2018-06-26 11:06:44
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __RuleEngineStore_H__
#define __RuleEngineStore_H__

#include "SQLiteDatabase.h"
#include "DefTemplateTable.h"
#include "DefClassTable.h"
#include "DefRuleTable.h"
#include "Object.h"
#include "Mutex.h"

#include <string>
#include <memory>

namespace HB {

class RuleEventHandler;

class RuleEngineStore : public ::UTILS::Object
                        , public std::enable_shared_from_this<RuleEngineStore> {
public:
    typedef std::shared_ptr<RuleEngineStore> pointer;
    RuleEngineStore(RuleEventHandler &hander, const std::string db);
    ~RuleEngineStore();

    bool isOpen() { return mDB != 0; }
    bool open();
    bool close();

    bool updateTemplateTable(const char *tmplName, const char *version, const char *fileName);
    bool updateClassTable(const char *clsName, const char *version, const char *fileName);
    bool updateRuleTable(const char *ruleName, const char *version, const char *fileName);

    std::vector<std::string> queryTemplateFilePaths();
    std::vector<std::string> queryClassFilePaths();
    std::vector<std::string> queryRuleFilePaths();

private:
    DefTable* _GetTable(TableType type);
    bool _UpdateDefTable(DefTable *table, const char *defName, const char *version, const char *fileName);
    std::vector<std::string> _QueryDefFilePaths(DefTable *table);

private:
    RuleEventHandler &mHandler;
    std::string mDBFilePath;
    UTILS::Mutex mDBMutex;
    UTILS::SQLiteDatabase *mDB;
    DefTemplateTable *mDefTmplTab;
    DefClassTable *mDefClassTab;
    DefRuleTable *mDefRuleTab;

}; /* class RuleEngineStore */

} /* namespace HB */

#ifdef __cplusplus


#endif /* __cplusplus */

#endif /* __RuleEngineStore_H__ */
