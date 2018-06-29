/***************************************************************************
 *  RuleEventThread.h - Rule Event Thread Header
 *
 *  Created: 2018-06-12 19:05:03
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __RuleEventThread_H__
#define __RuleEventThread_H__

#include "MessageLooper.h"
#include "Singleton.h"

#ifdef __cplusplus

using namespace UTILS;

namespace HB {

class RuleEventThread : public MessageLooper, public Singleton<RuleEventThread> {
public:
    RuleEventThread();
    ~RuleEventThread();
    virtual void run();
}; /* class RuleEventThread */

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __RuleEventThread_H__ */
