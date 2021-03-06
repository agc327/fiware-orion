#ifndef RESPONSE_DATA_H
#define RESPONSE_DATA_H

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

#include "ngsi/EntityId.h"
#include "ngsi/ContextRegistrationAttribute.h"
#include "ngsi/Metadata.h"

#include "ngsi9/RegisterContextResponse.h"
#include "ngsi9/DiscoverContextAvailabilityResponse.h"
#include "ngsi9/SubscribeContextAvailabilityResponse.h"
#include "ngsi9/UnsubscribeContextAvailabilityResponse.h"
#include "ngsi9/UpdateContextAvailabilitySubscriptionResponse.h"
#include "ngsi10/SubscribeContextResponse.h"
#include "ngsi10/QueryContextResponse.h"
#include "ngsi10/UnsubscribeContextResponse.h"
#include "ngsi10/UpdateContextResponse.h"
#include "ngsi10/UpdateContextSubscriptionResponse.h"



/* ****************************************************************************
*
* ResponseData - 
*/
typedef struct ResponseData
{
  RegisterContextResponse                         rcr;
  DiscoverContextAvailabilityResponse             dcar;
  QueryContextResponse                            qcr;
  SubscribeContextAvailabilityResponse            scar;
  SubscribeContextResponse                        scr;
  UnsubscribeContextAvailabilityResponse          ucar;
  UnsubscribeContextResponse                      uncr;
  UpdateContextAvailabilitySubscriptionResponse   ucas;
  UpdateContextResponse                           upcr;
  UpdateContextSubscriptionResponse               ucsr;
} ResponseData;

#endif
