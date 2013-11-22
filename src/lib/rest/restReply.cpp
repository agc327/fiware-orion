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

#include "logMsg/logMsg.h"

#include "common/Format.h"
#include "ngsi/ErrorCode.h"

#include "ngsi9/DiscoverContextAvailabilityResponse.h"
#include "ngsi9/RegisterContextResponse.h"
#include "ngsi9/SubscribeContextAvailabilityResponse.h"
#include "ngsi9/UnsubscribeContextAvailabilityResponse.h"
#include "ngsi9/UpdateContextAvailabilitySubscriptionResponse.h"
#include "ngsi9/NotifyContextAvailabilityResponse.h"

#include "ngsi10/QueryContextResponse.h"
#include "ngsi10/SubscribeContextResponse.h"
#include "ngsi10/UnsubscribeContextResponse.h"
#include "ngsi10/UpdateContextResponse.h"
#include "ngsi10/UpdateContextSubscriptionResponse.h"
#include "ngsi10/NotifyContextResponse.h"

#include "rest/ConnectionInfo.h"
#include "rest/HttpStatusCode.h"
#include "rest/mhd.h"
#include "rest/OrionError.h"
#include "rest/restReply.h"
#include "logMsg/traceLevels.h"


/* ****************************************************************************
*
* formatedAnswer - 
*/
static std::string formatedAnswer
(
  Format       format,
  std::string  header,
  std::string  tag1,
  std::string  value1,
  std::string  tag2,
  std::string  value2
)
{
   std::string answer;

   if (format == XML)
   {
      answer  = std::string("<")   + header + ">\n";
      answer += std::string("  <") + tag1   + ">" + value1 + "</" + tag1 + ">\n";
      answer += std::string("  <") + tag2   + ">" + value2 + "</" + tag2 + ">\n";
      answer += std::string("</")  + header + ">";
   }
   else if (format == JSON)
   {
      answer  = std::string("{\n");
      answer += std::string("  \"") + header + "\":\n";
      answer += std::string("  {\n");
      answer += std::string("  \"") + tag1   + "\": \"" + value1 + "\",\n";
      answer += std::string("  \"") + tag2   + "\": \"" + value2 + "\"\n";
      answer += std::string("  }\n");
      answer += std::string("}");
   }
   else
      answer = header + ": " + tag1 + "=" + value1 + ", " + tag2 + "=" + tag2;

   return answer;
}



char savedResponse[2 * 1024 * 1024];
static int replyIx = 0;
/* ****************************************************************************
*
* restReply - 
*/
int restReply(ConnectionInfo* ciP, std::string answer)
{
  int            ret;
  MHD_Response*  response;

  ++replyIx;
  LM_T(LmtRestReply, ("Response %d: responding with %d bytes, Status Code %d", replyIx, answer.length(), ciP->httpStatusCode));

  if (answer == "")
    response = MHD_create_response_from_data(answer.length(), (void*) answer.c_str(), MHD_NO, MHD_NO);
  else
    response = MHD_create_response_from_data(answer.length(), (void*) answer.c_str(), MHD_YES, MHD_YES);

  if (!response)
    LM_RE(MHD_NO, ("MHD_create_response_from_buffer FAILED"));

  if (ciP->httpHeader.size() != 0)
  {
     for (unsigned int hIx = 0; hIx < ciP->httpHeader.size(); ++hIx)
        MHD_add_response_header(response, ciP->httpHeader[hIx].c_str(), ciP->httpHeaderValue[hIx].c_str());
  }

  if (answer != "")
  {
    if (ciP->outFormat == XML)
      MHD_add_response_header(response, "Content-Type", "application/xml");
    else if (ciP->outFormat == JSON)
      MHD_add_response_header(response, "Content-Type", "application/json");
  }

  ret = MHD_queue_response(ciP->connection, ciP->httpStatusCode, response);
  MHD_destroy_response(response);

  if (ret != MHD_YES)
  {
    if (strlen(answer.c_str()) > sizeof(savedResponse))
    {
       std::string errorAnswer = restErrorReplyGet(ciP, ciP->outFormat, "", ciP->payloadWord, 500, "response too large", "Maximum size of responses has been set to 2Mb");
       LM_W(("answer too large: %d bytes (max allowed is %d bytes", strlen(answer.c_str()), sizeof(savedResponse)));
       savedResponse[0] = 0;
       restReply(ciP, errorAnswer);
       return MHD_NO;
    }
    else
    {
       strcpy(savedResponse, answer.c_str());
       LM_T(LmtSavedResponse, ("MHD_queue_response failed - saved the answer for later"));
    }
    return MHD_YES;
  }

  return MHD_NO;
}



/* ****************************************************************************
*
* restReply - 
*/
int restReply(ConnectionInfo* ciP, MHD_Response* response)
{
  int ret;

  ret = MHD_queue_response(ciP->connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);

  if (ret != MHD_YES)
    LM_E(("MHD_queue_response failed: %d", ret));

  return MHD_NO;
}



/* ****************************************************************************
*
* restReply - 
*/
int restReply(ConnectionInfo* ciP, std::string reason, std::string detail)
{
  std::string  answer;

  answer = formatedAnswer(ciP->outFormat, "orionError", "reason", reason, "detail", detail);

  return restReply(ciP, answer);
}



/* ****************************************************************************
*
* tagGet - return a tag (request type) depending on the incoming request string
*
* This function is called only from restErrorReplyGet, but as the parameter 
* 'request' is simply 'forwarded' from restErrorReplyGet, the 'request' can
* have various contents - for that the different strings of 'request'. 
*/
static std::string tagGet(std::string request)
{
  if ((request == "registerContext") || (request == "/ngsi9/registerContext") || (request == "/NGSI9/registerContext") || (request == "registerContextRequest"))
    return "registerContextResponse";
  else if ((request == "discoverContextAvailability") || (request == "/ngsi9/discoverContextAvailability") || (request == "/NGSI9/discoverContextAvailability") || (request == "discoverContextAvailabilityRequest"))
    return "discoverContextAvailabilityResponse";
  else if ((request == "subscribeContextAvailability") || (request == "/ngsi9/subscribeContextAvailability") || (request == "/NGSI9/subscribeContextAvailability") || (request == "subscribeContextAvailabilityRequest"))
    return "subscribeContextAvailabilityResponse";
  else if ((request == "updateContextAvailabilitySubscription") || (request == "/ngsi9/updateContextAvailabilitySubscription") || (request == "/NGSI9/updateContextAvailabilitySubscription") || (request == "updateContextAvailabilitySubscriptionRequest"))
    return "updateContextAvailabilitySubscriptionResponse";
  else if ((request == "unsubscribeContextAvailability") || (request == "/ngsi9/unsubscribeContextAvailability") || (request == "/NGSI9/unsubscribeContextAvailability") || (request == "unsubscribeContextAvailabilityRequest"))
    return "unsubscribeContextAvailabilityResponse";
  else if ((request == "notifyContextAvailability") || (request == "/ngsi9/notifyContextAvailability") || (request == "/NGSI9/notifyContextAvailability") || (request == "notifyContextAvailabilityRequest"))
    return "notifyContextAvailabilityResponse";

  else if ((request == "queryContext") || (request == "/ngsi10/queryContext") || (request == "/NGSI10/queryContext") || (request == "queryContextRequest"))
    return "queryContextResponse";
  else if ((request == "subscribeContext") || (request == "/ngsi10/subscribeContext") || (request == "/NGSI10/subscribeContext") || (request == "subscribeContextRequest"))
    return "subscribeContextResponse";
  else if ((request == "updateContextSubscription") || (request == "/ngsi10/updateContextSubscription") || (request == "/NGSI10/updateContextSubscription") || (request == "updateContextSubscriptionRequest"))
    return "updateContextSubscriptionResponse";
  else if ((request == "unsubscribeContext") || (request == "/ngsi10/unsubscribeContext") || (request == "/NGSI10/unsubscribeContext") || (request == "unsubscribeContextRequest"))
    return "unsubscribeContextResponse";
  else if ((request == "updateContext") || (request == "/ngsi10/updateContext") || (request == "/NGSI10/updateContext") || (request == "updateContextRequest"))
    return "updateContextResponse";
  else if ((request == "notifyContext") || (request == "/ngsi10/notifyContext") || (request == "/NGSI10/notifyContext") || (request == "notifyContextRequest"))
    return "notifyContextResponse";

  return "UnknownTag";
}



/* ****************************************************************************
*
* restErrorReplyGet - 
*
* This function renders an error reply depending on the 'request' type.
* Many responses have different syntax and especially the tag in the reply
* differs (registerContextResponse, discoverContextAvailabilityResponse etc).
*
* Also, the function is called from more than one place, especially from 
* restErrorReply, but also from where the payload type is matched against the request URL.
* Where the payload type is matched against the request URL, the incoming 'request' is a
* request and not a response.
*/
std::string restErrorReplyGet(ConnectionInfo* ciP, Format format, std::string indent, std::string request, int code, std::string reasonPhrase, std::string detail)
{
   std::string   tag = tagGet(request);
   ErrorCode     errorCode(code, reasonPhrase, detail);
   std::string   reply;

   ciP->httpStatusCode = SccOk;

   if (tag == "registerContextResponse")
   {
      RegisterContextResponse rcr("0", errorCode);
      reply =  rcr.render(RegisterContext, format, indent);
   }
   else if (tag == "discoverContextAvailabilityResponse")
   {
      DiscoverContextAvailabilityResponse dcar(errorCode);
      reply =  dcar.render(DiscoverContextAvailability, format, indent);
   }
   else if (tag == "subscribeContextAvailabilityResponse")
   {
      SubscribeContextAvailabilityResponse scar("0", errorCode);
      reply =  scar.render(SubscribeContextAvailability, format, indent);
   }
   else if (tag == "updateContextAvailabilitySubscriptionResponse")
   {
      UpdateContextAvailabilitySubscriptionResponse ucas(errorCode);
      reply =  ucas.render(UpdateContextAvailabilitySubscription, format, indent, 0);
   }
   else if (tag == "unsubscribeContextAvailabilityResponse")
   {
      UnsubscribeContextAvailabilityResponse ucar(errorCode);
      reply =  ucar.render(UnsubscribeContextAvailability, format, indent);
   }
   else if (tag == "notifyContextAvailabilityResponse")
   {
      NotifyContextAvailabilityResponse ncar(errorCode);
      reply =  ncar.render(NotifyContextAvailability, format, indent);
   }

   else if (tag == "queryContextResponse")
   {
      QueryContextResponse qcr(errorCode);
      reply =  qcr.render(QueryContext, format, indent);
   }
   else if (tag == "subscribeContextResponse")
   {
      SubscribeContextResponse scr(errorCode);
      reply =  scr.render(SubscribeContext, format, indent);
   }
   else if (tag == "updateContextSubscriptionResponse")
   {
      UpdateContextSubscriptionResponse ucsr(errorCode);
      reply =  ucsr.render(UpdateContextSubscription, format, indent);
   }
   else if (tag == "unsubscribeContextResponse")
   {
      UnsubscribeContextResponse uncr(errorCode);
      reply =  uncr.render(UnsubscribeContext, format, indent);
   }
   else if (tag == "updateContextResponse")
   {
      UpdateContextResponse ucr(errorCode);
      reply = ucr.render(UpdateContext, format, indent);
   }
   else if (tag == "notifyContextResponse")
   {
      NotifyContextResponse ncr(errorCode);
      reply =  ncr.render(NotifyContext, format, indent);
   }
   else
   {
      OrionError orionError(errorCode);

      LM_E(("Unknown tag: '%s', request == '%s'", tag.c_str(), request.c_str()));
      
      reply = orionError.render(format, indent);
   }

   return reply;
}


/* ****************************************************************************
*
* restErrorReply - 
*/
void restErrorReply(ConnectionInfo* ciP, Format format, std::string indent, std::string request, int code, std::string reasonPhrase, std::string detail)
{
  std::string reply = restErrorReplyGet(ciP, format, indent, request, code, reasonPhrase, detail);

  restReply(ciP, reply);
}



/* ****************************************************************************
*
* restErrorReply - 
*/
void restErrorReply(ConnectionInfo* ciP, Format format, std::string indent, HttpStatusCode code, std::string reasonPhrase, std::string detail)
{
  ErrorCode     errorCode(code, reasonPhrase, detail);
  std::string   reply;

  ciP->httpStatusCode = SccOk;

  reply = errorCode.render(format, indent);
  restReply(ciP, reply);
}
