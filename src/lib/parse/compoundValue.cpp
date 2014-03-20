/*
*
* Copyright 2014 Telefonica Investigacion y Desarrollo, S.A.U
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
#include "logMsg/traceLevels.h"

#include "ngsi/ParseData.h"
#include "parse/CompoundValueNode.h"
#include "parse/compoundValue.h"
#include "rest/ConnectionInfo.h"


namespace orion
{

/* ****************************************************************************
*
* compoundValueStart - 
*/
void compoundValueStart(ConnectionInfo* ciP, std::string path, std::string name, std::string value, std::string root, std::string rest, orion::CompoundValueNode::Type type)
{
  LM_T(LmtCompoundValue, ("Compound START: PATH:'%s', NAME:'%s' (VALUE:%s)", path.c_str(), name.c_str(), value.c_str()));
  ciP->inCompoundValue = true;

  ciP->compoundValueP = new orion::CompoundValueNode(root);
  LM_T(LmtCompoundValueContainer, ("Set current container to '%s' (%s)", ciP->compoundValueP->path.c_str(), ciP->compoundValueP->name.c_str()));
  ciP->compoundValueRoot = ciP->compoundValueP;

  if (ciP->parseDataP->lastContextAttribute == NULL)
     LM_X(1, ("No pointer to last ContextAttribute"));

  if (ciP->parseDataP->lastContextAttribute->typeAttribute == "vector")
    ciP->compoundValueP->type = orion::CompoundValueNode::Vector;

  ciP->compoundValueVector.push_back(ciP->compoundValueP);
  LM_T(LmtCompoundValueAdd, ("Created new toplevel element"));
  compoundValueMiddle(ciP, rest, name, value, type);
}



/* ****************************************************************************
*
* compoundValueMiddle - 
*
* containerType: vector/struct/leaf
*/
void compoundValueMiddle(ConnectionInfo* ciP, std::string relPath, std::string name, std::string value, orion::CompoundValueNode::Type type)
{
  LM_T(LmtCompoundValue, ("Compound MIDDLE %s: %s: NAME: '%s', VALUE: '%s'", relPath.c_str(), CompoundValueNode::typeName(type), name.c_str(), value.c_str()));

  if ((type == orion::CompoundValueNode::Vector) || (type == orion::CompoundValueNode::Struct))
  {
    ciP->compoundValueP = ciP->compoundValueP->add(type, name);
    LM_T(LmtCompoundValueContainer, ("Set current container to '%s' (%s)", ciP->compoundValueP->path.c_str(), ciP->compoundValueP->name.c_str()));
  }
  else
  {
    ciP->compoundValueP->add(type, name, value);
  }
} 



/* ****************************************************************************
*
* compoundValueEnd - 
*/
void compoundValueEnd(ConnectionInfo* ciP, std::string path, std::string name, std::string value, std::string fatherPath, ParseData* parseDataP)
{
  LM_T(LmtCompoundValue, ("Compound END:    '%s'", path.c_str()));

  // Finish the compound value - error check included
  std::string status = ciP->compoundValueRoot->finish();

  // Any problems in 'finish'?
  // If so, mark as erroneous
  if (status != "OK")
  {
    ciP->httpStatusCode = SccBadRequest;
    ciP->answer = std::string("compound value error: ") + status;
    LM_W(("ERROR: '%s', PATH: '%s'   ", ciP->answer.c_str(), fatherPath.c_str()));
  }
  else
  {
    ciP->compoundValueRoot->show("");
    ciP->compoundValueRoot->shortShow("");
  }

  // Now, give the root pointer of this Compound to the active ContextAttribute
  parseDataP->lastContextAttribute->compoundValueP = ciP->compoundValueRoot;
  ciP->compoundValueRoot = NULL;
  ciP->compoundValueP    = NULL;
  LM_T(LmtCompoundValueContainer, ("Set current container to NULL"));
  ciP->inCompoundValue   = false;
} 

}
