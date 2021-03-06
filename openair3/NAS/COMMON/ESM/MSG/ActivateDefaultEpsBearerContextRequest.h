/*
 * Copyright (c) 2015, EURECOM (www.eurecom.fr)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ProtocolDiscriminator.h"
#include "EpsBearerIdentity.h"
#include "ProcedureTransactionIdentity.h"
#include "MessageType.h"
#include "EpsQualityOfService.h"
#include "AccessPointName.h"
#include "PdnAddress.h"
#include "TransactionIdentifier.h"
#include "QualityOfService.h"
#include "LlcServiceAccessPointIdentifier.h"
#include "RadioPriority.h"
#include "PacketFlowIdentifier.h"
#include "ApnAggregateMaximumBitRate.h"
#include "EsmCause.h"
#include "ProtocolConfigurationOptions.h"

#ifndef ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_H_
#define ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_H_

/* Minimum length macro. Formed by minimum length of each mandatory field */
#define ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_MINIMUM_LENGTH ( \
    EPS_QUALITY_OF_SERVICE_MINIMUM_LENGTH + \
    ACCESS_POINT_NAME_MINIMUM_LENGTH + \
    PDN_ADDRESS_MINIMUM_LENGTH )

/* Maximum length macro. Formed by maximum length of each field */
#define ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_MAXIMUM_LENGTH ( \
    EPS_QUALITY_OF_SERVICE_MAXIMUM_LENGTH + \
    ACCESS_POINT_NAME_MAXIMUM_LENGTH + \
    PDN_ADDRESS_MAXIMUM_LENGTH + \
    TRANSACTION_IDENTIFIER_MAXIMUM_LENGTH + \
    QUALITY_OF_SERVICE_MAXIMUM_LENGTH + \
    LLC_SERVICE_ACCESS_POINT_IDENTIFIER_MAXIMUM_LENGTH + \
    RADIO_PRIORITY_MAXIMUM_LENGTH + \
    PACKET_FLOW_IDENTIFIER_MAXIMUM_LENGTH + \
    APN_AGGREGATE_MAXIMUM_BIT_RATE_MAXIMUM_LENGTH + \
    ESM_CAUSE_MAXIMUM_LENGTH + \
    PROTOCOL_CONFIGURATION_OPTIONS_MAXIMUM_LENGTH )

/* If an optional value is present and should be encoded, the corresponding
 * Bit mask should be set to 1.
 */
# define ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_TRANSACTION_IDENTIFIER_PRESENT         (1<<0)
# define ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_NEGOTIATED_QOS_PRESENT                 (1<<1)
# define ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_NEGOTIATED_LLC_SAPI_PRESENT            (1<<2)
# define ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_RADIO_PRIORITY_PRESENT                 (1<<3)
# define ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_PACKET_FLOW_IDENTIFIER_PRESENT         (1<<4)
# define ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_APNAMBR_PRESENT                        (1<<5)
# define ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_ESM_CAUSE_PRESENT                      (1<<6)
# define ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT (1<<7)

typedef enum activate_default_eps_bearer_context_request_iei_tag {
  ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_TRANSACTION_IDENTIFIER_IEI          = 0x5D, /* 0x5D = 93 */
  ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_NEGOTIATED_QOS_IEI                  = 0x30, /* 0x30 = 48 */
  ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_NEGOTIATED_LLC_SAPI_IEI             = 0x32, /* 0x32 = 50 */
  ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_RADIO_PRIORITY_IEI                  = 0x80, /* 0x80 = 128 */
  ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_PACKET_FLOW_IDENTIFIER_IEI          = 0x34, /* 0x34 = 52 */
  ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_APNAMBR_IEI                         = 0x5E, /* 0x5E = 94 */
  ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_ESM_CAUSE_IEI                       = 0x58, /* 0x58 = 88 */
  ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_PROTOCOL_CONFIGURATION_OPTIONS_IEI  = 0x27, /* 0x27 = 39 */
} activate_default_eps_bearer_context_request_iei;

/*
 * Message name: Activate default EPS bearer context request
 * Description: This message is sent by the network to the UE to request activation of a default EPS bearer context. See table??8.3.6.1.
 * Significance: dual
 * Direction: network to UE
 */

typedef struct activate_default_eps_bearer_context_request_msg_tag {
  /* Mandatory fields */
  ProtocolDiscriminator                                 protocoldiscriminator:4;
  EpsBearerIdentity                                     epsbeareridentity:4;
  ProcedureTransactionIdentity                          proceduretransactionidentity;
  MessageType                                           messagetype;
  EpsQualityOfService                                   epsqos;
  AccessPointName                                       accesspointname;
  PdnAddress                                            pdnaddress;
  /* Optional fields */
  uint32_t                                              presencemask;
  TransactionIdentifier                                 transactionidentifier;
  QualityOfService                                      negotiatedqos;
  LlcServiceAccessPointIdentifier                       negotiatedllcsapi;
  RadioPriority                                         radiopriority;
  PacketFlowIdentifier                                  packetflowidentifier;
  ApnAggregateMaximumBitRate                            apnambr;
  EsmCause                                              esmcause;
  ProtocolConfigurationOptions                          protocolconfigurationoptions;
} activate_default_eps_bearer_context_request_msg;

int decode_activate_default_eps_bearer_context_request(activate_default_eps_bearer_context_request_msg *activatedefaultepsbearercontextrequest, uint8_t *buffer, uint32_t len);

int encode_activate_default_eps_bearer_context_request(activate_default_eps_bearer_context_request_msg *activatedefaultepsbearercontextrequest, uint8_t *buffer, uint32_t len);

#endif /* ! defined(ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_REQUEST_H_) */

