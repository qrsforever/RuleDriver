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

#include "Log.h"
#include "Message.h"
#include "MessageTypes.h"

using namespace UTILS;

namespace HB {

static RuleEventHandler *gRuleHandler = 0;

RuleEventHandler::RuleEventHandler(MessageQueue *queue)
    : MessageHandler(queue)
{

}

RuleEventHandler::~RuleEventHandler()
{

}

void RuleEventHandler::handleMessage(Message *msg)
{
    if (msg->what != RET_REFRESH_TIMER)
        LOGD("msg: [%d] [%d] [%d]\n", msg->what, msg->arg1, msg->arg2);

    switch(msg->what) {
        case RET_REFRESH_TIMER:
            sendEmptyMessageDelayed(RET_REFRESH_TIMER, 1000);
            break;
        case RET_STORE_CLOSE:
            if (msg->obj) {
                std::shared_ptr<RuleEngineStore> store(std::dynamic_pointer_cast<RuleEngineStore>(msg->obj));
                store->close();
            }
            break;
        default:
            break;
    }
}

RuleEventHandler& ruleHandler()
{
    if (0 == gRuleHandler) {
        RuleEventThread &ruleThread = RuleEventThread::getInstance();
        gRuleHandler = new RuleEventHandler(ruleThread.getMessageQueue());
        ruleThread.start();
    }
    return *gRuleHandler;
}

} /* namespace HB */
