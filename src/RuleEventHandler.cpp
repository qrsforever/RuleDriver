/***************************************************************************
 *  RuleEventHandler.cpp - Rule Engine Service Impl
 *
 *  Created: 2018-06-12 18:59:46
 *
 *  Copyright QRS
 ****************************************************************************/

#include "RuleEventHandler.h"
#include "RuleEventThread.h"
#include "RuleEventTypes.h"
#include "RuleEngineStore.h"
#include "RuleEngineService.h"

#include "Log.h"
#include "Message.h"
#include "MessageTypes.h"

using namespace UTILS;

#define TIMER_MSECONDS 1000 /* 1s */

namespace HB {

static RuleEventHandler *gRuleHandler = 0;
static RuleEventHandler *gUrgentHandler = 0;

RuleEventHandler::RuleEventHandler(MessageQueue *queue, pthread_t id)
    : MessageHandler(queue), mID(id)
{

}

RuleEventHandler::~RuleEventHandler()
{

}

void RuleEventHandler::handleMessage(Message *msg)
{
    if (msg->what == RET_REFRESH_TIMER) {
        sendEmptyMessageDelayed(RET_REFRESH_TIMER, TIMER_MSECONDS);
        return;
    }

    LOGD("msg: [%d] [%d] [%d]\n", msg->what, msg->arg1, msg->arg2);

    switch(msg->what) {
        case RET_STORE_CLOSE:
            ruleEngine().store()->close();
            break;
        case RET_DEBUG:
            ruleEngine().ccore()->debug(msg->arg1);
            break;
        default:
            break;
    }
}

RuleEventHandler& ruleHandler()
{
    if (0 == gRuleHandler) {
        RuleEventThread *ruleThread = new RuleEventThread();
        gRuleHandler = new RuleEventHandler(
            ruleThread->getMessageQueue(),
            ruleThread->id());
        ruleThread->start();
    }
    return *gRuleHandler;
}

RuleEventHandler& urgentHandler()
{
    if (0 == gUrgentHandler) {
        RuleEventThread *ruleThread = new RuleEventThread();
        gUrgentHandler = new RuleEventHandler(
            ruleThread->getMessageQueue(),
            ruleThread->id());
        ruleThread->start();
    }
    return *gUrgentHandler;
}

} /* namespace HB */
