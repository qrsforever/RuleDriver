/***************************************************************************
 *  DefTable.h - Define Table Base Class Header
 *
 *  Created: 2018-06-26 12:29:15
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __DefTable_H__
#define __DefTable_H__

#include "SQLiteDatabase.h"
#include <string>
#include <vector>

using namespace UTILS;

namespace HB {

typedef enum {
    TT_DEFTEMPLATE = 0,
    TT_DEFCLASS = 1,
    TT_DEFRULE,
} TableType;

struct DefInfo {
    std::string mDefName;
    std::string mVersion;
    std::string mFilePath;
}; /* struct DefInfo */

class DefTable {
public:
    virtual ~DefTable();

    virtual TableType type() = 0;
    bool init();
    int version();
    const char* name() { return mTabName.c_str(); }

    bool updateOrInsert(const DefInfo &info);
    std::string getVersion(std::string defName);
    std::string getFilePath(std::string defName);
    std::vector<DefInfo> getDefInfos();
    std::vector<std::string> getFilePaths();

#ifdef TABLE_DEBUG
    void showTable();
#endif

private:
    bool _Update(int latestVersion);

protected:
    DefTable(SQLiteDatabase &db, const char *tabName);
    SQLiteDatabase& mDB;
	std::vector<std::pair<int, std::string>> mUpdateHistoryList; /* for iterate alter update table */

private:
    std::string mTabName;

}; /* class DefTable */

} /* namespace HB */

#ifdef __cplusplus


#endif /* __cplusplus */

#endif /* __DefTable_H__ */
