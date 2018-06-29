/***************************************************************************
 *  RuleEventThread.cpp - RuleEventThread Impl
 *
 *  Created: 2018-06-12 19:06:29
 *
 *  Copyright QRS
 ****************************************************************************/

#include "RuleEventThread.h"
#include "Log.h"

SINGLETON_STATIC_INSTANCE(HB::RuleEventThread)

namespace HB {

RuleEventThread::RuleEventThread() : MessageLooper()
{

}

RuleEventThread::~RuleEventThread()
{

}

void RuleEventThread::run()
{
    LOGI("Rule EventThread:[%u]\n", id());
    return MessageLooper::run();
}

} /* namespace HB */
