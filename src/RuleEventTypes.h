/***************************************************************************
 *  RuleEventTypes.h - RuleEventTypes Header
 *
 *  Created: 2018-06-13 18:14:49
 *
 *  Copyright QRS
 ****************************************************************************/

/****************
*  Rule Event  *
****************/
#define RET_REFRESH_TIMER 1           /* Rule Event Type: refresh timer */
#define RET_INSTANCE_ADD  2           /* Rule Event Type: device online */
#define RET_INSTANCE_DEL  3           /* Rule Event Type: device offline */
#define RET_INSTANCE_PUT  4           /* Rule Event Type: device put attr */
#define RET_RULE_SYNC     5           /* Rule Event Type: rule sync from cloud */
#define RET_CLASS_SYNC    6           /* Rule Event Type: class profile sync */
#define RET_STORE_CLOSE   7           /* Rule Event Type: close store database */


#define DEBUG_SHOW_ALL          0
#define DEBUG_SHOW_CLASSES      1
#define DEBUG_SHOW_RULES        2
#define DEBUG_SHOW_INSTANCES    3
#define DEBUG_SHOW_FACTS        4
#define DEBUG_SHOW_AGENDA       5
#define DEBUG_SHOW_MEMORY       6
