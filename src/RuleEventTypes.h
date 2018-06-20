/***************************************************************************
 *  RuleEventTypes.h - RuleEventTypes Header
 *
 *  Created: 2018-06-13 18:14:49
 *
 *  Copyright QRS
 ****************************************************************************/

#define ID_PREFIX "id-"

/****************
*  Rule Event  *
****************/
#define RET_REFRESH_TIMER 1           /* Rule Event Type: refresh timer */
#define RET_INSTANCE_ADD  2           /* Rule Event Type: device online */
#define RET_INSTANCE_DEL  3           /* Rule Event Type: device offline */
#define RET_INSTANCE_PUT  4           /* Rule Event Type: device put attr */
#define RET_RULE_SYNC     5           /* Rule Event Type: rule sync from cloud */
