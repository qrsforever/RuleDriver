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
#include "RuleEventTypes.h"

#ifdef __cplusplus

namespace HB {

class RuleEventHandler : public ::UTILS::MessageHandler {
public:
    RuleEventHandler(::UTILS::MessageQueue *queue);
    ~RuleEventHandler();

protected:
    void handleMessage(::UTILS::Message *msg);

}; /* class RuleEventHandler */

RuleEventHandler& ruleHandler();

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __RuleEventHandler_H__ */

