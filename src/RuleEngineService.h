/***************************************************************************
 *  RuleEngineService.h - Rule Engine Service Header
 *
 *  Created: 2018-06-13 09:24:58
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __RuleEngineService_H__
#define __RuleEngineService_H__


#ifdef __cplusplus

namespace HB {

class RuleEngineService {
public:
    RuleEngineService();
    ~RuleEngineService();

    void start();

private:

}; /* class RuleEngineService */

RuleEngineService& ruleEngine();

} /* namespace HB */

#endif /* __cplusplus */

#endif /* __RuleEngineService_H__ */
