/***************************************************************************
 *  RuleEventHandler.h - Rule Engine Service Header
 *
 *  Created: 2018-06-12 18:07:36
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __RuleEventHandler_H__
#define __RuleEventHandler_H__

#include "MessageHandler.h"

#ifdef __cplusplus

using namespace UTILS;

namespace HB {

class RuleEventHandler : public MessageHandler {
public:
    RuleEventHandler(MessageQueue *queue);
    ~RuleEventHandler();

protected:
    void handleMessage(Message *msg);

}; /* class RuleEventHandler */

RuleEventHandler& ruleHandler();

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __RuleEventHandler_H__ */
