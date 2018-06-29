/***************************************************************************
 *  DefRuleTable.cpp - Define Rule Table Impl
 *
 *  Created: 2018-06-26 15:57:07
 *
 *  Copyright QRS
 ****************************************************************************/

#include "DefRuleTable.h"
#include "SQLiteLog.h"

#define DEFRULE_TABLE_NAME     "defrule"

namespace HB {

DefRuleTable::DefRuleTable(SQLiteDatabase &db)
    : DefTable(db, DEFRULE_TABLE_NAME)
{
    init();
}

DefRuleTable::~DefRuleTable()
{

}

} /* namespace HB */
