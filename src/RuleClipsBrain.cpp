/***************************************************************************
 *  RuleClipsBrain.cpp - Rule Clips Brain Impl
 *
 *  Created: 2018-06-13 09:28:22
 *
 *  Copyright QRS
 ****************************************************************************/

#include "RuleClipsBrain.h"
#include "Router.h"

namespace HB {

RuleClipsBrain::RuleClipsBrain()
    : mClipsEnv(0)
{
    mClipsEnv = new Environment();
}

RuleClipsBrain::~RuleClipsBrain()
{
    if (mClipsEnv) {
        finalize_clips_logger(mClipsEnv->cobj());
        delete mClipsEnv;
    }
    mClipsEnv = 0;
}

void RuleClipsBrain::setupClips()
{
    init_clips_logger(mClipsEnv->cobj());
}

void RuleClipsBrain::startClips()
{
}

} /* namespace HB */
