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
#include <string.h>


#include "TLVEncoder.h"
#include "TLVDecoder.h"
#include "UeSecurityCapability.h"

int decode_ue_security_capability(UeSecurityCapability *uesecuritycapability, uint8_t iei, uint8_t *buffer, uint32_t len)
{
  int decoded = 0;
  uint8_t ielen = 0;

  if (iei > 0) {
    CHECK_IEI_DECODER(iei, *buffer);
    decoded++;
  }

  memset(uesecuritycapability, 0, sizeof(UeSecurityCapability));
  ielen = *(buffer + decoded);
  decoded++;
  CHECK_LENGTH_DECODER(len - decoded, ielen);
  uesecuritycapability->eea = *(buffer + decoded);
  decoded++;
  uesecuritycapability->eia = *(buffer + decoded);
  decoded++;

  if (len >= decoded + 3) {
    uesecuritycapability->umts_present = 1;
    uesecuritycapability->uea = *(buffer + decoded);
    decoded++;
    uesecuritycapability->uia = *(buffer + decoded) & 0x7f;
    decoded++;

    if (len == decoded + 4) {
      uesecuritycapability->gprs_present = 1;
      uesecuritycapability->gea = *(buffer + decoded) & 0x7f;
      decoded++;
    }
  }

#if defined (NAS_DEBUG)
  dump_ue_security_capability_xml(uesecuritycapability, iei);
#endif
  return decoded;
}
int encode_ue_security_capability(UeSecurityCapability *uesecuritycapability, uint8_t iei, uint8_t *buffer, uint32_t len)
{
  uint8_t *lenPtr;
  uint32_t encoded = 0;
  /* Checking IEI and pointer */
  CHECK_PDU_POINTER_AND_LENGTH_ENCODER(buffer, UE_SECURITY_CAPABILITY_MINIMUM_LENGTH, len);
#if defined (NAS_DEBUG)
  dump_ue_security_capability_xml(uesecuritycapability, iei);
#endif

  if (iei > 0) {
    *buffer = iei;
    encoded++;
  }

  lenPtr  = (buffer + encoded);
  encoded ++;
  *(buffer + encoded) = uesecuritycapability->eea;
  encoded++;
  *(buffer + encoded) =  uesecuritycapability->eia;
  encoded++;

  // From ETSI TS 124 301 V10.15.0 (2014-10) 9.9.3.36 Security capability:
  // Octets 5, 6, and 7 are optional. If octet 5 is included, then also octet 6 shall be included and octet 7 may be included.
  // If a UE did not indicate support of any security algorithm for Gb mode, octet 7 shall not be included. If the UE did not
  // indicate support of any security algorithm for Iu mode and Gb mode, octets 5, 6, and 7 shall not be included.
  // If the UE did not indicate support of any security algorithm for Iu mode but indicated support of a security algorithm for
  // Gb mode, octets 5, 6, and 7 shall be included. In this case octets 5 and 6 are filled with the value of zeroes.
  if (uesecuritycapability->umts_present) {
    *(buffer + encoded) = uesecuritycapability->uea;
    encoded++;
    *(buffer + encoded) = 0x00 |
                          (uesecuritycapability->uia & 0x7f);
    encoded++;

    if (uesecuritycapability->gprs_present) {
      *(buffer + encoded) = 0x00 | (uesecuritycapability->gea & 0x7f);
      encoded++;
    }
  } else {
    if (uesecuritycapability->gprs_present) {
      *(buffer + encoded) = 0x00;
      encoded++;
      *(buffer + encoded) = 0x00;
      encoded++;
      *(buffer + encoded) = 0x00 | (uesecuritycapability->gea & 0x7f);
      encoded++;
    }
  }

  *lenPtr = encoded - 1 - ((iei > 0) ? 1 : 0);
  return encoded;
}

void dump_ue_security_capability_xml(UeSecurityCapability *uesecuritycapability, uint8_t iei)
{
  printf("<Ue Security Capability>\n");

  if (iei > 0)
    /* Don't display IEI if = 0 */
    printf("    <IEI>0x%X</IEI>\n", iei);

  printf("    <EEA>%u</EEA>\n", uesecuritycapability->eea);
  printf("    <EIA>%u</EIA>\n", uesecuritycapability->eia);

  if (uesecuritycapability->umts_present == 1) {
    printf("    <UEA>%u</UEA>\n", uesecuritycapability->uea);
    printf("    <UIA>%u</UIA>\n", uesecuritycapability->uia);
  }

  if (uesecuritycapability->gprs_present == 1) {
    printf("    <GEA>%u</GEA>\n", uesecuritycapability->gea);
  }

  printf("</Ue Security Capability>\n");
}

