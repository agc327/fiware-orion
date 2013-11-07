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
#include <string>

#include "common/globals.h"
#include "common/tag.h"
#include "ngsi/Request.h"
#include "ngsi/StatusCode.h"
#include "ngsi10/SubscribeContextResponse.h"
#include "ngsi10/SubscribeContextRequest.h"



/* ****************************************************************************
*
* SubscribeContextRequest::render - 
*/
std::string SubscribeContextRequest::render(RequestType requestType, Format format, std::string indent)
{
  std::string out      = "";
  std::string tag      = "subscribeContextRequest";
  std::string indent2  = indent + "  ";

  out += startTag(indent, tag, format, false);
  out += entityIdVector.render(format, indent2);
  out += attributeList.render(format, indent2);
  out += reference.render(format, indent2);
  out += duration.render(format, indent2);
  out += restriction.render(format, indent2);
  out += notifyConditionVector.render(format, indent2);
  out += throttling.render(format, indent2);
  out += endTag(indent, tag, format);

  return out;
}



/* ****************************************************************************
*
* SubscribeContextRequest::check - 
*/
std::string SubscribeContextRequest::check(RequestType requestType, Format format, std::string indent, std::string predetectedError, int counter)
{
  SubscribeContextResponse response;
  std::string              res;

  /* First, check optional fields only in the case they are present */
  /* Second, check the other (mandatory) fields */

  if (((res = entityIdVector.check(SubscribeContext, format, indent, predetectedError, counter))        != "OK") ||
      ((res = attributeList.check(SubscribeContext, format, indent, predetectedError, counter))         != "OK") ||      
      ((res = duration.check(SubscribeContext, format, indent, predetectedError, counter))              != "OK") ||
      ((res = restriction.check(SubscribeContext, format, indent, predetectedError, restrictions))      != "OK") ||
      ((res = notifyConditionVector.check(SubscribeContext, format, indent, predetectedError, counter)) != "OK") ||
      ((res = throttling.check(SubscribeContext, format, indent, predetectedError, counter))            != "OK"))
  {
    response.subscribeError.errorCode.fill(SccBadRequest, "invalid payload", res);
    return response.render(SubscribeContext, format, indent);
  }

  return "OK";
}



/* ****************************************************************************
*
* SubscribeContextRequest::present - 
*/
void SubscribeContextRequest::present(std::string indent)
{
  entityIdVector.present(indent + "  ");
  attributeList.present(indent + "  ");
  reference.present(indent + "  ");
  duration.present(indent + "  ");
  restriction.present(indent + "  ");
  notifyConditionVector.present(indent + "  ");
  throttling.present(indent + "  ");
}



/* ****************************************************************************
*
* SubscribeContextRequest::release - 
*/
void SubscribeContextRequest::release(void)
{
  entityIdVector.release();
  attributeList.release();
  restriction.release();
  notifyConditionVector.release();
}
