/***************************************************************************
 *  RuleEventThread.cpp - RuleEventThread Impl
 *
 *  Created: 2018-06-12 19:06:29
 *
 *  Copyright QRS
 ****************************************************************************/

#include "RuleEventThread.h"

SINGLETON_STATIC_INSTANCE(HB::RuleEventThread)

namespace HB {

RuleEventThread::RuleEventThread() : MessageLooper()
{

}

RuleEventThread::~RuleEventThread()
{

}

} /* namespace HB */
