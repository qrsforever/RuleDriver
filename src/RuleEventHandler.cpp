/***************************************************************************
 *  RuleEventHandler.cpp - Rule Engine Service Impl
 *
 *  Created: 2018-06-12 18:59:46
 *
 *  Copyright QRS
 ****************************************************************************/

#include "RuleEventHandler.h"
#include "RuleEventThread.h"

#include "Message.h"
#include "Log.h"

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
    switch(msg->what) {

    }
}

RuleEventHandler& ruleHandler()
{
    if (0 == gRuleHandler) {
        MessageQueue *queue = RuleEventThread::getInstance().getMessageQueue();
        gRuleHandler = new RuleEventHandler(queue);
    }
    return *gRuleHandler;
}

} /* namespace HB */
