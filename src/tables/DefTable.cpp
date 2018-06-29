/***************************************************************************
 *  DefTable.cpp - Define Table Base Impl
 *
 *  Created: 2018-06-26 12:48:04
 *
 *  Copyright QRS
 ****************************************************************************/

#include "DefTable.h"
#include "SQLiteLog.h"

#include <algorithm>

#define VERSIONS_TABLE_NAME     "versions"
#define VERSIONS_FIELD_NAME     "Name"
#define VERSIONS_FIELD_VERSION  "Version"

#define VERSIONS_TABLE_CREATE "CREATE TABLE "\
    VERSIONS_TABLE_NAME "(" \
        VERSIONS_FIELD_NAME " TEXT UNIQUE NOT NULL PRIMARY KEY, " \
        VERSIONS_FIELD_VERSION " INT)"

#define VERSIONS_TABLE_QUERY_VERSION "SELECT " VERSIONS_FIELD_VERSION " FROM " \
        VERSIONS_TABLE_NAME " WHERE " VERSIONS_FIELD_NAME " = ?"

#define VERSIONS_TABLE_REPLACE_VERSION  "REPLACE INTO "\
    VERSIONS_TABLE_NAME "(" \
        VERSIONS_FIELD_NAME ", " \
        VERSIONS_FIELD_VERSION " ) VALUES(?, ?)"


#define DEF_FIELD_DEFNAME  "DefName"
#define DEF_FIELD_VERSION  "Version"
#define DEF_FIELD_CLPPATH  "FilePath"
#define DEF_FIELD_RAWDATA  "RawData"

namespace HB {

DefTable::DefTable(SQLiteDatabase &db, const char *tabName)
    : mDB(db)
    , mTabName(tabName)
{
    std::string sql("CREATE TABLE ");
    sql.append(mTabName).append("(");
    sql.append(DEF_FIELD_DEFNAME).append(" TEXT UNIQUE NOT NULL PRIMARY KEY, ");
    sql.append(DEF_FIELD_VERSION).append(" TEXT, ");
    sql.append(DEF_FIELD_CLPPATH).append(" TEXT, ");
    sql.append(DEF_FIELD_RAWDATA).append(" TEXT)");
    mUpdateHistoryList.push_back(std::make_pair(0, sql));
}

DefTable::~DefTable()
{
    mUpdateHistoryList.clear();
}

bool DefTable::init()
{
    if (!mDB.tableExists(VERSIONS_TABLE_NAME))
        if (!mDB.exec(VERSIONS_TABLE_CREATE))
            return false;

    sort(mUpdateHistoryList.begin(), mUpdateHistoryList.end());
    if (mUpdateHistoryList.size() > 0) {
        std::pair<int, std::string> elem = mUpdateHistoryList.back();
        if (elem.first > version())
            return _Update(elem.first);
    }
    return true;
}

int DefTable::version()
{
    int ver = -1;
    SQLiteResultSet* rs = mDB.query(VERSIONS_TABLE_QUERY_VERSION, SQLText(name()));
    if (rs) {
        if (rs->next())
            ver = rs->columnInt(0);
        rs->close();
    }
    return ver;
}

bool DefTable::_Update(int latestVersion)
{
    for (size_t i = 0; i < mUpdateHistoryList.size(); ++i) {
        if (!mDB.exec(mUpdateHistoryList[i].second.c_str()))
            return false;
    }
    return mDB.exec(VERSIONS_TABLE_REPLACE_VERSION, SQLText(name()), SQLInt(latestVersion));
}

bool DefTable::updateOrInsert(const DefInfo &info)
{
    LOGTT();
    SQLiteValue values[3];
    values[0] = SQLText(info.mDefName.c_str());
    values[1] = SQLText(info.mVersion.c_str());
    values[2] = SQLText(info.mFilePath.c_str());

    std::string sql("REPLACE INTO ");
    sql.append(mTabName).append("(");
    sql.append(DEF_FIELD_DEFNAME).append(", ");
    sql.append(DEF_FIELD_VERSION).append(", ");
    sql.append(DEF_FIELD_CLPPATH).append(") VALUES(?, ?, ?)");
    return mDB.exec(sql.c_str(), values, sizeof(values) / sizeof(values[0]));
}

std::string DefTable::getVersion(std::string defName)
{
    LOGTT();
    std::string ver("");
    std::string sql("SELECT ");
    sql.append(DEF_FIELD_VERSION).append(" FROM ").append(mTabName);
    sql.append(" WHERE ").append(DEF_FIELD_DEFNAME).append(" = ").append(defName);

    SQLiteResultSet *rs = mDB.query(sql.c_str());
    if (rs) {
        if (rs->next())
            ver = rs->columnText(0);
        rs->close();
    }
    return ver;
}

std::string DefTable::getFilePath(std::string defName)
{
    LOGTT();
    std::string filepath("");
    std::string sql("SELECT ");
    sql.append(DEF_FIELD_CLPPATH).append(" FROM ").append(mTabName);
    sql.append(" WHERE ").append(DEF_FIELD_DEFNAME).append(" = ").append(defName);

    SQLiteResultSet *rs = mDB.query(sql.c_str());
    if (rs) {
        if (rs->next())
            filepath = rs->columnText(0);
        rs->close();
    }
    return filepath;
}

std::vector<DefInfo> DefTable::getDefInfos()
{
    LOGTT();
    std::vector<DefInfo> infos;
    std::string sql("SELECT ");
    sql.append(DEF_FIELD_DEFNAME).append(", ");
    sql.append(DEF_FIELD_VERSION).append(", ");
    sql.append(DEF_FIELD_CLPPATH).append(" FROM ").append(mTabName);

    SQLiteResultSet *rs = mDB.query(sql.c_str());
    if (rs) {
        while (rs->next()) {
            DefInfo info;
            info.mDefName = rs->columnText(0);
            info.mVersion = rs->columnText(1);
            info.mFilePath = rs->columnText(2);
            infos.push_back(info);
        }
        rs->close();
    }
    return infos;
}

std::vector<std::string> DefTable::getFilePaths()
{
    LOGTT();
    std::vector<std::string> files;
    std::string sql("SELECT ");
    sql.append(DEF_FIELD_CLPPATH).append(" FROM ").append(mTabName);

    SQLiteResultSet *rs = mDB.query(sql.c_str());
    if (rs) {
        while (rs->next())
            files.push_back(rs->columnText(0));
        rs->close();
    }
    return files;
}

#ifdef TABLE_DEBUG
void DefTable::showTable()
{
    std::vector<DefInfo> infos = getDefInfos();
    LOGD("Table[%s]:\n", mTabName.c_str());
    for (size_t i = 0; i < infos.size(); ++i) {
        LOGD("\t %s:[%s] %s[%s] %s[%s]\n",
            DEF_FIELD_DEFNAME, infos[i].mDefName.c_str(),
            DEF_FIELD_VERSION, infos[i].mVersion.c_str(),
            DEF_FIELD_CLPPATH, infos[i].mFilePath.c_str());
    }
}
#endif


} /* namespace HB */
