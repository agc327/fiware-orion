/*
*
* Copyright 2013 Telefonica Investigacion y Desarrollo, S.A.U
*
* This file is part of Orion Context Broker.
*
* Orion Context Broker is free software: you can redistribute it and/or
* modify it under the terms of the GNU Affero General Public License as
* published by the Free Software Foundation, either version 3 of the
* License, or (at your option) any later version.
*
* Orion Context Broker is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero
* General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with Orion Context Broker. If not, see http://www.gnu.org/licenses/.
*
* For those usages not covered by this license please contact with
* fermin at tid dot es
*
* Author: Ken Zangelin
*/
#include <stdio.h>
#include <string>

#include "common/globals.h"
#include "common/tag.h"
#include "ngsi/StatusCode.h"
#include "ngsi10/UpdateContextSubscriptionResponse.h"
#include "ngsi10/UpdateContextSubscriptionRequest.h"

/* ****************************************************************************
*
* UpdateContextSubscriptionRequest::UpdateContextSubscriptionRequest
*
* Explicit constructor needed to initialize primitive types so they don't get
* random values from the stack
*/
UpdateContextSubscriptionRequest::UpdateContextSubscriptionRequest()
{
  restrictions = 0;
}


/* ****************************************************************************
*
* UpdateContextSubscriptionRequest::render - 
*/
std::string UpdateContextSubscriptionRequest::render(RequestType requestType, Format format, std::string indent)
{
  std::string out                             = "";
  std::string tag                             = "updateContextSubscriptionRequest";

  bool        restrictionRendered             = restrictions != 0;
  bool        subscriptionIdRendered          = true; // Mandatory
  bool        notifyConditionVectorRendered   = notifyConditionVector.size() != 0;
  bool        throttlingRendered              = throttling.get() != "";

  bool        commaAfterThrottling            = false; // Last element
  bool        commaAfterNotifyConditionVector = throttlingRendered;
  bool        commaAfterSubscriptionId        = notifyConditionVectorRendered || throttlingRendered;
  bool        commaAfterRestriction           = subscriptionIdRendered || notifyConditionVectorRendered || throttlingRendered;
  bool        commaAfterDuration              = restrictionRendered || subscriptionIdRendered || notifyConditionVectorRendered || throttlingRendered;
  
  out += startTag(indent, tag, format, false);
  out += duration.render(format, indent + "  ", commaAfterDuration);
  out += restriction.render(format, indent + "  ", restrictions, commaAfterRestriction);
  out += subscriptionId.render(UpdateContextSubscription, format, indent + "  ", commaAfterSubscriptionId);
  out += notifyConditionVector.render(format, indent + "  ", commaAfterNotifyConditionVector);
  out += throttling.render(format, indent + "  ", commaAfterThrottling);
  out += endTag(indent, tag, format);

  return out;
}



/* ****************************************************************************
*
* UpdateContextSubscriptionRequest::check - 
*/
std::string UpdateContextSubscriptionRequest::check(RequestType requestType, Format format, std::string indent, std::string predetectedError, int counter)
{
  std::string                       res;
  UpdateContextSubscriptionResponse response;

  if (predetectedError != "")
  {
    response.subscribeError.subscriptionId = subscriptionId;
    response.subscribeError.errorCode.fill(SccBadRequest, predetectedError);
  }
  else if (((res = duration.check(UpdateContextSubscription, format, indent, predetectedError, counter))              != "OK") ||
           ((res = restriction.check(UpdateContextSubscription, format, indent, predetectedError, restrictions))      != "OK") ||
           ((res = subscriptionId.check(UpdateContextSubscription, format, indent, predetectedError, counter))        != "OK") ||
           ((res = notifyConditionVector.check(UpdateContextSubscription, format, indent, predetectedError, counter)) != "OK") ||
           ((res = throttling.check(UpdateContextSubscription, format, indent, predetectedError, counter))            != "OK"))
  {
    response.subscribeError.subscriptionId = subscriptionId;
    response.subscribeError.errorCode.fill(SccBadRequest, res);
  }
  else
    return "OK";

  return response.render(UpdateContextSubscription, format, indent);
}



/* ****************************************************************************
*
* UpdateContextSubscriptionRequest::present - 
*/
void UpdateContextSubscriptionRequest::present(std::string indent)
{
  duration.present(indent);
  restriction.present(indent);
  subscriptionId.present(indent);
  notifyConditionVector.present(indent);
  throttling.present(indent);
}



/* ****************************************************************************
*
* UpdateContextSubscriptionRequest::release - 
*/
void UpdateContextSubscriptionRequest::release(void)
{
  restriction.release();
  notifyConditionVector.release();
}
