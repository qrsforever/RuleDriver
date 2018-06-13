/***************************************************************************
 *  RuleClipsBrain.h - Rule Clips Brain
 *
 *  Created: 2018-06-13 09:26:42
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __RuleClipsBrain_H__
#define __RuleClipsBrain_H__

#include "Environment.h"

#ifdef __cplusplus

namespace HB {

using namespace CLIPS;

class RuleClipsBrain {
public:
    RuleClipsBrain();
    ~RuleClipsBrain();

    void setupClips();
    void startClips();

private:
    Environment *mClipsEnv;
}; /* class RuleClipsBrain */


} /* namespace HB */

#endif /* __cplusplus */

#endif /* __RuleClipsBrain_H__ */
