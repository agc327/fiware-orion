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
#include "gtest/gtest.h"

#include "logMsg/logMsg.h"
#include "logMsg/traceLevels.h"

#include "common/Format.h"
#include "convenience/ContextAttributeResponseVector.h"
#include "convenience/ContextAttributeResponse.h"
#include "ngsi/ContextAttribute.h"



/* ****************************************************************************
*
* render - 
*/
TEST(ContextAttributeResponseVector, render)
{
  ContextAttributeResponseVector  carV;
  ContextAttribute                ca("caName", "caType", "caValue");
  ContextAttributeResponse        car;  
  std::string                     out;
  std::string                     expected = "<contextResponseList>\n  <contextAttributeResponse>\n    <contextAttributeList>\n      <contextAttribute>\n        <name>caName</name>\n        <type>caType</type>\n        <contextValue>caValue</contextValue>\n      </contextAttribute>\n    </contextAttributeList>\n    <statusCode>\n      <code>0</code>\n      <reasonPhrase></reasonPhrase>\n    </statusCode>\n  </contextAttributeResponse>\n</contextResponseList>\n";

  // 1. empty vector
  out = carV.render(XML, "");
  EXPECT_STREQ("", out.c_str());

  // 2. normal case
  car.contextAttributeVector.push_back(&ca);
  carV.push_back(&car);

  out = carV.render(XML, "");
  EXPECT_STREQ(expected.c_str(), out.c_str());
}



/* ****************************************************************************
*
* check - 
*/
TEST(ContextAttributeResponseVector, check)
{
  ContextAttributeResponseVector  carV;
  ContextAttribute                ca("caName", "caType", "caValue");
  ContextAttributeResponse        car;  
  std::string                     out;
  std::string                     expected1 = "OK";
  std::string                     expected2 = "<contextAttributeResponse>\n  <statusCode>\n    <code>400</code>\n    <reasonPhrase>PRE ERROR</reasonPhrase>\n  </statusCode>\n</contextAttributeResponse>\n";
  std::string                     expected3 = "<contextAttributeResponse>\n  <statusCode>\n    <code>400</code>\n    <reasonPhrase>missing attribute name</reasonPhrase>\n  </statusCode>\n</contextAttributeResponse>\n";

  // 1. ok
  car.contextAttributeVector.push_back(&ca);
  carV.push_back(&car);
  out = carV.check(UpdateContextAttribute, XML, "", "", 0);
  EXPECT_STREQ(expected1.c_str(), out.c_str());

  // 2. Predetected Error
  out = carV.check(UpdateContextAttribute, XML, "", "PRE ERROR", 0);
  EXPECT_STREQ(expected2.c_str(), out.c_str());


  // 3. Bad ContextAttribute
  ContextAttribute                ca2("", "caType", "caValue");

  car.contextAttributeVector.push_back(&ca2);
  out = carV.check(UpdateContextAttribute, XML, "", "", 0);
  EXPECT_STREQ(expected3.c_str(), out.c_str());
}



/* ****************************************************************************
*
* present - just exercise the code
*/
TEST(ContextAttributeResponseVector, present)
{
  ContextAttributeResponseVector  carV;
  ContextAttribute                ca("caName", "caType", "caValue");
  ContextAttributeResponse        car;

  car.contextAttributeVector.push_back(&ca);
  carV.push_back(&car);

  carV.present("");
}



/* ****************************************************************************
*
* getAndSize - 
*/
TEST(ContextAttributeResponseVector, getAndSize)
{
  ContextAttributeResponse         car;
  ContextAttribute                 ca("caName", "caType", "caValue");
  ContextAttributeResponseVector   carV;

  car.contextAttributeVector.push_back(&ca);

  EXPECT_EQ(0, carV.size());
  carV.push_back(&car);
  ASSERT_EQ(1, carV.size());

  ContextAttributeResponse* carP = carV.get(0);
  EXPECT_TRUE(carP != NULL);
}



/* ****************************************************************************
*
* release - just exercise the code
*/
TEST(ContextAttributeResponseVector, release)
{
  ContextAttribute*               caP  = new ContextAttribute("caName", "caType", "caValue");
  ContextAttributeResponse*       carP = new ContextAttributeResponse();
  ContextAttributeResponseVector  carV;

  carP->contextAttributeVector.push_back(caP);
  carP->statusCode.fill(SccOk, "OK");

  carV.push_back(carP);

  carV.release();
}

