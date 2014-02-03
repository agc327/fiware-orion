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
#include <vector>

#include "logMsg/logMsg.h"
#include "logMsg/traceLevels.h"

#include "common/globals.h"
#include "mongoBackend/MongoGlobal.h"
#include "ngsi/ParseData.h"
#include "rest/ConnectionInfo.h"
#include "rest/OrionError.h"
#include "rest/rest.h"
#include "serviceRoutines/exitTreat.h"



/* ****************************************************************************
*
* exitTreat - 
*/
std::string exitTreat(ConnectionInfo* ciP, int components, std::vector<std::string> compV, ParseData* parseDataP)
{
   std::string password = "XXX";
   std::string out;

   if (harakiri == false)
   {
     OrionError orionError(SccBadRequest, "Bad Request", "no such service");

     ciP->httpStatusCode = SccOk;
     out = orionError.render(ciP->outFormat, "");
     return out;
   }

   if (components > 1)
    password = compV[1];

   if (components == 1)
   {
      OrionError orionError(SccBadRequest, "Bad Request", "Password requested");
      ciP->httpStatusCode = SccOk;
      out = orionError.render(ciP->outFormat, "");
   }
   else if (password != "harakiri")
   {
      OrionError orionError(SccBadRequest, "Bad Request", "Request denied - password erroneous");
      ciP->httpStatusCode = SccOk;
      out = orionError.render(ciP->outFormat, "");
   }
   else
   {
      // Cleanup for valgrind
      free(ciP->payload);
      delete ciP;
      mongoDisconnect();
      compV.clear();
      return "DIE";
   }

   return out;
}
