/***************************************************************************
 *  DefRuleTable.h - Define Rule Table Header
 *
 *  Created: 2018-06-26 15:51:56
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __DefRuleTable_H__
#define __DefRuleTable_H__

#include "DefTable.h"

#ifdef __cplusplus

namespace HB {

class DefRuleTable : public DefTable {
public:
    DefRuleTable(SQLiteDatabase &db);
    ~DefRuleTable();

    TableType type() { return TT_DEFRULE; }

private:

}; /* class DefRuleTable */

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __DefRuleTable_H__ */
