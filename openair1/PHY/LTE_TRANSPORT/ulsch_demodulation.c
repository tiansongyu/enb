/*******************************************************************************
    OpenAirInterface
    Copyright(c) 1999 - 2014 Eurecom

    OpenAirInterface is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.


    OpenAirInterface is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenAirInterface.The full GNU General Public License is
   included in this distribution in the file called "COPYING". If not,
   see <http://www.gnu.org/licenses/>.

  Contact Information
  OpenAirInterface Admin: openair_admin@eurecom.fr
  OpenAirInterface Tech : openair_tech@eurecom.fr
  OpenAirInterface Dev  : openair4g-devel@lists.eurecom.fr

  Address      : Eurecom, Campus SophiaTech, 450 Route des Chappes, CS 50193 - 06904 Biot Sophia Antipolis cedex, FRANCE

 *******************************************************************************/

/*! \file PHY/LTE_TRANSPORT/ulsch_demodulation.c
* \brief Top-level routines for demodulating the PUSCH physical channel from 36.211 V8.6 2009-03
* \author R. Knopp
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr, florian.kaltenberger@eurecom.fr, ankit.bhamri@eurecom.fr
* \note
* \warning
*/

#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "defs.h"
#include "extern.h"
//#define DEBUG_ULSCH
#include "PHY/sse_intrin.h"

//extern char* namepointer_chMag ;
//eren
//extern int **ulchmag_eren;
//eren

static short jitter[8]  __attribute__ ((aligned(16))) = {1,0,0,1,0,1,1,0};
static short jitterc[8] __attribute__ ((aligned(16))) = {0,1,1,0,1,0,0,1};

#ifndef OFDMA_ULSCH
void lte_idft(LTE_DL_FRAME_PARMS *frame_parms,uint32_t *z, uint16_t Msc_PUSCH)
{
#if defined(__x86_64__) || defined(__i386__)
  __m128i idft_in128[3][1200],idft_out128[3][1200];
  __m128i norm128;
#elif defined(__arm__)
  int16x8_t idft_in128[3][1200],idft_out128[3][1200];
  int16x8_t norm128;
#endif
  int16_t *idft_in0=(int16_t*)idft_in128[0],*idft_out0=(int16_t*)idft_out128[0];
  int16_t *idft_in1=(int16_t*)idft_in128[1],*idft_out1=(int16_t*)idft_out128[1];
  int16_t *idft_in2=(int16_t*)idft_in128[2],*idft_out2=(int16_t*)idft_out128[2];

  uint32_t *z0,*z1,*z2,*z3,*z4,*z5,*z6,*z7,*z8,*z9,*z10=NULL,*z11=NULL;
  int i,ip;



  //  printf("Doing lte_idft for Msc_PUSCH %d\n",Msc_PUSCH);

  if (frame_parms->Ncp == 0) { // Normal prefix
    z0 = z;
    z1 = z0+(frame_parms->N_RB_DL*12);
    z2 = z1+(frame_parms->N_RB_DL*12);
    //pilot
    z3 = z2+(2*frame_parms->N_RB_DL*12);
    z4 = z3+(frame_parms->N_RB_DL*12);
    z5 = z4+(frame_parms->N_RB_DL*12);

    z6 = z5+(frame_parms->N_RB_DL*12);
    z7 = z6+(frame_parms->N_RB_DL*12);
    z8 = z7+(frame_parms->N_RB_DL*12);
    //pilot
    z9 = z8+(2*frame_parms->N_RB_DL*12);
    z10 = z9+(frame_parms->N_RB_DL*12);
    // srs
    z11 = z10+(frame_parms->N_RB_DL*12);
  } else { // extended prefix
    z0 = z;
    z1 = z0+(frame_parms->N_RB_DL*12);
    //pilot
    z2 = z1+(2*frame_parms->N_RB_DL*12);
    z3 = z2+(frame_parms->N_RB_DL*12);
    z4 = z3+(frame_parms->N_RB_DL*12);

    z5 = z4+(frame_parms->N_RB_DL*12);
    z6 = z5+(frame_parms->N_RB_DL*12);
    //pilot
    z7 = z6+(2*frame_parms->N_RB_DL*12);
    z8 = z7+(frame_parms->N_RB_DL*12);
    // srs
    z9 = z8+(frame_parms->N_RB_DL*12);
  }

  // conjugate input
  for (i=0; i<(Msc_PUSCH>>2); i++) {
#if defined(__x86_64__)||defined(__i386__)
    *&(((__m128i*)z0)[i])=_mm_sign_epi16(*&(((__m128i*)z0)[i]),*(__m128i*)&conjugate2[0]);
    *&(((__m128i*)z1)[i])=_mm_sign_epi16(*&(((__m128i*)z1)[i]),*(__m128i*)&conjugate2[0]);
    *&(((__m128i*)z2)[i])=_mm_sign_epi16(*&(((__m128i*)z2)[i]),*(__m128i*)&conjugate2[0]);
    *&(((__m128i*)z3)[i])=_mm_sign_epi16(*&(((__m128i*)z3)[i]),*(__m128i*)&conjugate2[0]);
    *&(((__m128i*)z4)[i])=_mm_sign_epi16(*&(((__m128i*)z4)[i]),*(__m128i*)&conjugate2[0]);
    *&(((__m128i*)z5)[i])=_mm_sign_epi16(*&(((__m128i*)z5)[i]),*(__m128i*)&conjugate2[0]);
    *&(((__m128i*)z6)[i])=_mm_sign_epi16(*&(((__m128i*)z6)[i]),*(__m128i*)&conjugate2[0]);
    *&(((__m128i*)z7)[i])=_mm_sign_epi16(*&(((__m128i*)z7)[i]),*(__m128i*)&conjugate2[0]);
    *&(((__m128i*)z8)[i])=_mm_sign_epi16(*&(((__m128i*)z8)[i]),*(__m128i*)&conjugate2[0]);
    *&(((__m128i*)z9)[i])=_mm_sign_epi16(*&(((__m128i*)z9)[i]),*(__m128i*)&conjugate2[0]);

    if (frame_parms->Ncp==NORMAL) {
      *&(((__m128i*)z10)[i])=_mm_sign_epi16(*&(((__m128i*)z10)[i]),*(__m128i*)&conjugate2[0]);
      *&(((__m128i*)z11)[i])=_mm_sign_epi16(*&(((__m128i*)z11)[i]),*(__m128i*)&conjugate2[0]);
    }
#elif defined(__arm__)
    *&(((int16x8_t*)z0)[i])=vmulq_s16(*&(((int16x8_t*)z0)[i]),*(int16x8_t*)&conjugate2[0]);
    *&(((int16x8_t*)z1)[i])=vmulq_s16(*&(((int16x8_t*)z1)[i]),*(int16x8_t*)&conjugate2[0]);
    *&(((int16x8_t*)z2)[i])=vmulq_s16(*&(((int16x8_t*)z2)[i]),*(int16x8_t*)&conjugate2[0]);
    *&(((int16x8_t*)z3)[i])=vmulq_s16(*&(((int16x8_t*)z3)[i]),*(int16x8_t*)&conjugate2[0]);
    *&(((int16x8_t*)z4)[i])=vmulq_s16(*&(((int16x8_t*)z4)[i]),*(int16x8_t*)&conjugate2[0]);
    *&(((int16x8_t*)z5)[i])=vmulq_s16(*&(((int16x8_t*)z5)[i]),*(int16x8_t*)&conjugate2[0]);
    *&(((int16x8_t*)z6)[i])=vmulq_s16(*&(((int16x8_t*)z6)[i]),*(int16x8_t*)&conjugate2[0]);
    *&(((int16x8_t*)z7)[i])=vmulq_s16(*&(((int16x8_t*)z7)[i]),*(int16x8_t*)&conjugate2[0]);
    *&(((int16x8_t*)z8)[i])=vmulq_s16(*&(((int16x8_t*)z8)[i]),*(int16x8_t*)&conjugate2[0]);
    *&(((int16x8_t*)z9)[i])=vmulq_s16(*&(((int16x8_t*)z9)[i]),*(int16x8_t*)&conjugate2[0]);


    if (frame_parms->Ncp==NORMAL) {
      *&(((int16x8_t*)z10)[i])=vmulq_s16(*&(((int16x8_t*)z10)[i]),*(int16x8_t*)&conjugate2[0]);
      *&(((int16x8_t*)z11)[i])=vmulq_s16(*&(((int16x8_t*)z11)[i]),*(int16x8_t*)&conjugate2[0]);
    }

#endif
  }

  for (i=0,ip=0; i<Msc_PUSCH; i++,ip+=4) {
    ((uint32_t*)idft_in0)[ip+0] =  z0[i];
    ((uint32_t*)idft_in0)[ip+1] =  z1[i];
    ((uint32_t*)idft_in0)[ip+2] =  z2[i];
    ((uint32_t*)idft_in0)[ip+3] =  z3[i];
    ((uint32_t*)idft_in1)[ip+0] =  z4[i];
    ((uint32_t*)idft_in1)[ip+1] =  z5[i];
    ((uint32_t*)idft_in1)[ip+2] =  z6[i];
    ((uint32_t*)idft_in1)[ip+3] =  z7[i];
    ((uint32_t*)idft_in2)[ip+0] =  z8[i];
    ((uint32_t*)idft_in2)[ip+1] =  z9[i];

    if (frame_parms->Ncp==0) {
      ((uint32_t*)idft_in2)[ip+2] =  z10[i];
      ((uint32_t*)idft_in2)[ip+3] =  z11[i];
    }
  }


  switch (Msc_PUSCH) {
  case 12:
    dft12((int16_t *)idft_in0,(int16_t *)idft_out0);
    dft12((int16_t *)idft_in1,(int16_t *)idft_out1);
    dft12((int16_t *)idft_in2,(int16_t *)idft_out2);

#if defined(__x86_64__)||defined(__i386__)
    norm128 = _mm_set1_epi16(9459);
#elif defined(__arm__)
    norm128 = vdupq_n_s16(9459);
#endif
    for (i=0; i<12; i++) {
#if defined(__x86_64__)||defined(__i386__)
      ((__m128i*)idft_out0)[i] = _mm_slli_epi16(_mm_mulhi_epi16(((__m128i*)idft_out0)[i],norm128),1);
      ((__m128i*)idft_out1)[i] = _mm_slli_epi16(_mm_mulhi_epi16(((__m128i*)idft_out1)[i],norm128),1);
      ((__m128i*)idft_out2)[i] = _mm_slli_epi16(_mm_mulhi_epi16(((__m128i*)idft_out2)[i],norm128),1);
#elif defined(__arm__)
      ((int16x8_t*)idft_out0)[i] = vqdmulhq_s16(((int16x8_t*)idft_out0)[i],norm128);
      ((int16x8_t*)idft_out1)[i] = vqdmulhq_s16(((int16x8_t*)idft_out1)[i],norm128);
      ((int16x8_t*)idft_out2)[i] = vqdmulhq_s16(((int16x8_t*)idft_out2)[i],norm128);
#endif
    }

    break;

  case 24:
    dft24(idft_in0,idft_out0,1);
    dft24(idft_in1,idft_out1,1);
    dft24(idft_in2,idft_out2,1);
    break;

  case 36:
    dft36(idft_in0,idft_out0,1);
    dft36(idft_in1,idft_out1,1);
    dft36(idft_in2,idft_out2,1);
    break;

  case 48:
    dft48(idft_in0,idft_out0,1);
    dft48(idft_in1,idft_out1,1);
    dft48(idft_in2,idft_out2,1);
    break;

  case 60:
    dft60(idft_in0,idft_out0,1);
    dft60(idft_in1,idft_out1,1);
    dft60(idft_in2,idft_out2,1);
    break;

  case 72:
    dft72(idft_in0,idft_out0,1);
    dft72(idft_in1,idft_out1,1);
    dft72(idft_in2,idft_out2,1);
    break;

  case 96:
    dft96(idft_in0,idft_out0,1);
    dft96(idft_in1,idft_out1,1);
    dft96(idft_in2,idft_out2,1);
    break;

  case 108:
    dft108(idft_in0,idft_out0,1);
    dft108(idft_in1,idft_out1,1);
    dft108(idft_in2,idft_out2,1);
    break;

  case 120:
    dft120(idft_in0,idft_out0,1);
    dft120(idft_in1,idft_out1,1);
    dft120(idft_in2,idft_out2,1);
    break;

  case 144:
    dft144(idft_in0,idft_out0,1);
    dft144(idft_in1,idft_out1,1);
    dft144(idft_in2,idft_out2,1);
    break;

  case 180:
    dft180(idft_in0,idft_out0,1);
    dft180(idft_in1,idft_out1,1);
    dft180(idft_in2,idft_out2,1);
    break;

  case 192:
    dft192(idft_in0,idft_out0,1);
    dft192(idft_in1,idft_out1,1);
    dft192(idft_in2,idft_out2,1);
    break;

  case 216:
    dft216(idft_in0,idft_out0,1);
    dft216(idft_in1,idft_out1,1);
    dft216(idft_in2,idft_out2,1);
    break;

  case 240:
    dft240(idft_in0,idft_out0,1);
    dft240(idft_in1,idft_out1,1);
    dft240(idft_in2,idft_out2,1);
    break;

  case 288:
    dft288(idft_in0,idft_out0,1);
    dft288(idft_in1,idft_out1,1);
    dft288(idft_in2,idft_out2,1);
    break;

  case 300:
    dft300(idft_in0,idft_out0,1);
    dft300(idft_in1,idft_out1,1);
    dft300(idft_in2,idft_out2,1);
    break;

  case 324:
    dft324((int16_t*)idft_in0,(int16_t*)idft_out0,1);
    dft324((int16_t*)idft_in1,(int16_t*)idft_out1,1);
    dft324((int16_t*)idft_in2,(int16_t*)idft_out2,1);
    break;

  case 360:
    dft360((int16_t*)idft_in0,(int16_t*)idft_out0,1);
    dft360((int16_t*)idft_in1,(int16_t*)idft_out1,1);
    dft360((int16_t*)idft_in2,(int16_t*)idft_out2,1);
    break;

  case 384:
    dft384((int16_t*)idft_in0,(int16_t*)idft_out0,1);
    dft384((int16_t*)idft_in1,(int16_t*)idft_out1,1);
    dft384((int16_t*)idft_in2,(int16_t*)idft_out2,1);
    break;

  case 432:
    dft432((int16_t*)idft_in0,(int16_t*)idft_out0,1);
    dft432((int16_t*)idft_in1,(int16_t*)idft_out1,1);
    dft432((int16_t*)idft_in2,(int16_t*)idft_out2,1);
    break;

  case 480:
    dft480((int16_t*)idft_in0,(int16_t*)idft_out0,1);
    dft480((int16_t*)idft_in1,(int16_t*)idft_out1,1);
    dft480((int16_t*)idft_in2,(int16_t*)idft_out2,1);
    break;

  case 540:
    dft540((int16_t*)idft_in0,(int16_t*)idft_out0,1);
    dft540((int16_t*)idft_in1,(int16_t*)idft_out1,1);
    dft540((int16_t*)idft_in2,(int16_t*)idft_out2,1);
    break;

  case 576:
    dft576((int16_t*)idft_in0,(int16_t*)idft_out0,1);
    dft576((int16_t*)idft_in1,(int16_t*)idft_out1,1);
    dft576((int16_t*)idft_in2,(int16_t*)idft_out2,1);
    break;

  case 600:
    dft600((int16_t*)idft_in0,(int16_t*)idft_out0,1);
    dft600((int16_t*)idft_in1,(int16_t*)idft_out1,1);
    dft600((int16_t*)idft_in2,(int16_t*)idft_out2,1);
    break;

  case 648:
    dft648((int16_t*)idft_in0,(int16_t*)idft_out0,1);
    dft648((int16_t*)idft_in1,(int16_t*)idft_out1,1);
    dft648((int16_t*)idft_in2,(int16_t*)idft_out2,1);
    break;

  case 720:
    dft720((int16_t*)idft_in0,(int16_t*)idft_out0,1);
    dft720((int16_t*)idft_in1,(int16_t*)idft_out1,1);
    dft720((int16_t*)idft_in2,(int16_t*)idft_out2,1);
    break;

  case 864:
    dft864((int16_t*)idft_in0,(int16_t*)idft_out0,1);
    dft864((int16_t*)idft_in1,(int16_t*)idft_out1,1);
    dft864((int16_t*)idft_in2,(int16_t*)idft_out2,1);
    break;

  case 900:
    dft900((int16_t*)idft_in0,(int16_t*)idft_out0,1);
    dft900((int16_t*)idft_in1,(int16_t*)idft_out1,1);
    dft900((int16_t*)idft_in2,(int16_t*)idft_out2,1);
    break;

  case 960:
    dft960((int16_t*)idft_in0,(int16_t*)idft_out0,1);
    dft960((int16_t*)idft_in1,(int16_t*)idft_out1,1);
    dft960((int16_t*)idft_in2,(int16_t*)idft_out2,1);
    break;

  case 972:
    dft972((int16_t*)idft_in0,(int16_t*)idft_out0,1);
    dft972((int16_t*)idft_in1,(int16_t*)idft_out1,1);
    dft972((int16_t*)idft_in2,(int16_t*)idft_out2,1);
    break;

  case 1080:
    dft1080((int16_t*)idft_in0,(int16_t*)idft_out0,1);
    dft1080((int16_t*)idft_in1,(int16_t*)idft_out1,1);
    dft1080((int16_t*)idft_in2,(int16_t*)idft_out2,1);
    break;

  case 1152:
    dft1152((int16_t*)idft_in0,(int16_t*)idft_out0,1);
    dft1152((int16_t*)idft_in1,(int16_t*)idft_out1,1);
    dft1152((int16_t*)idft_in2,(int16_t*)idft_out2,1);
    break;

  case 1200:
    dft1200(idft_in0,idft_out0,1);
    dft1200(idft_in1,idft_out1,1);
    dft1200(idft_in2,idft_out2,1);
    break;

  default:
    // should not be reached
    LOG_E( PHY, "Unsupported Msc_PUSCH value of %"PRIu16"\n", Msc_PUSCH );
    return;
  }



  for (i=0,ip=0; i<Msc_PUSCH; i++,ip+=4) {
    z0[i]     = ((uint32_t*)idft_out0)[ip];
    /*
      printf("out0 (%d,%d),(%d,%d),(%d,%d),(%d,%d)\n",
      ((int16_t*)&idft_out0[ip])[0],((int16_t*)&idft_out0[ip])[1],
      ((int16_t*)&idft_out0[ip+1])[0],((int16_t*)&idft_out0[ip+1])[1],
      ((int16_t*)&idft_out0[ip+2])[0],((int16_t*)&idft_out0[ip+2])[1],
      ((int16_t*)&idft_out0[ip+3])[0],((int16_t*)&idft_out0[ip+3])[1]);
    */
    z1[i]     = ((uint32_t*)idft_out0)[ip+1];
    z2[i]     = ((uint32_t*)idft_out0)[ip+2];
    z3[i]     = ((uint32_t*)idft_out0)[ip+3];
    z4[i]     = ((uint32_t*)idft_out1)[ip+0];
    z5[i]     = ((uint32_t*)idft_out1)[ip+1];
    z6[i]     = ((uint32_t*)idft_out1)[ip+2];
    z7[i]     = ((uint32_t*)idft_out1)[ip+3];
    z8[i]     = ((uint32_t*)idft_out2)[ip];
    z9[i]     = ((uint32_t*)idft_out2)[ip+1];

    if (frame_parms->Ncp==0) {
      z10[i]    = ((uint32_t*)idft_out2)[ip+2];
      z11[i]    = ((uint32_t*)idft_out2)[ip+3];
    }
  }

  // conjugate output
  for (i=0; i<(Msc_PUSCH>>2); i++) {
#if defined(__x86_64__) || defined(__i386__)
    ((__m128i*)z0)[i]=_mm_sign_epi16(((__m128i*)z0)[i],*(__m128i*)&conjugate2[0]);
    ((__m128i*)z1)[i]=_mm_sign_epi16(((__m128i*)z1)[i],*(__m128i*)&conjugate2[0]);
    ((__m128i*)z2)[i]=_mm_sign_epi16(((__m128i*)z2)[i],*(__m128i*)&conjugate2[0]);
    ((__m128i*)z3)[i]=_mm_sign_epi16(((__m128i*)z3)[i],*(__m128i*)&conjugate2[0]);
    ((__m128i*)z4)[i]=_mm_sign_epi16(((__m128i*)z4)[i],*(__m128i*)&conjugate2[0]);
    ((__m128i*)z5)[i]=_mm_sign_epi16(((__m128i*)z5)[i],*(__m128i*)&conjugate2[0]);
    ((__m128i*)z6)[i]=_mm_sign_epi16(((__m128i*)z6)[i],*(__m128i*)&conjugate2[0]);
    ((__m128i*)z7)[i]=_mm_sign_epi16(((__m128i*)z7)[i],*(__m128i*)&conjugate2[0]);
    ((__m128i*)z8)[i]=_mm_sign_epi16(((__m128i*)z8)[i],*(__m128i*)&conjugate2[0]);
    ((__m128i*)z9)[i]=_mm_sign_epi16(((__m128i*)z9)[i],*(__m128i*)&conjugate2[0]);

    if (frame_parms->Ncp==NORMAL) {
      ((__m128i*)z10)[i]=_mm_sign_epi16(((__m128i*)z10)[i],*(__m128i*)&conjugate2[0]);
      ((__m128i*)z11)[i]=_mm_sign_epi16(((__m128i*)z11)[i],*(__m128i*)&conjugate2[0]);
    }
#elif defined(__arm__)
    *&(((int16x8_t*)z0)[i])=vmulq_s16(*&(((int16x8_t*)z0)[i]),*(int16x8_t*)&conjugate2[0]);
    *&(((int16x8_t*)z1)[i])=vmulq_s16(*&(((int16x8_t*)z1)[i]),*(int16x8_t*)&conjugate2[0]);
    *&(((int16x8_t*)z2)[i])=vmulq_s16(*&(((int16x8_t*)z2)[i]),*(int16x8_t*)&conjugate2[0]);
    *&(((int16x8_t*)z3)[i])=vmulq_s16(*&(((int16x8_t*)z3)[i]),*(int16x8_t*)&conjugate2[0]);
    *&(((int16x8_t*)z4)[i])=vmulq_s16(*&(((int16x8_t*)z4)[i]),*(int16x8_t*)&conjugate2[0]);
    *&(((int16x8_t*)z5)[i])=vmulq_s16(*&(((int16x8_t*)z5)[i]),*(int16x8_t*)&conjugate2[0]);
    *&(((int16x8_t*)z6)[i])=vmulq_s16(*&(((int16x8_t*)z6)[i]),*(int16x8_t*)&conjugate2[0]);
    *&(((int16x8_t*)z7)[i])=vmulq_s16(*&(((int16x8_t*)z7)[i]),*(int16x8_t*)&conjugate2[0]);
    *&(((int16x8_t*)z8)[i])=vmulq_s16(*&(((int16x8_t*)z8)[i]),*(int16x8_t*)&conjugate2[0]);
    *&(((int16x8_t*)z9)[i])=vmulq_s16(*&(((int16x8_t*)z9)[i]),*(int16x8_t*)&conjugate2[0]);


    if (frame_parms->Ncp==NORMAL) {
      *&(((int16x8_t*)z10)[i])=vmulq_s16(*&(((int16x8_t*)z10)[i]),*(int16x8_t*)&conjugate2[0]);
      *&(((int16x8_t*)z11)[i])=vmulq_s16(*&(((int16x8_t*)z11)[i]),*(int16x8_t*)&conjugate2[0]);
    }

#endif
  }

#if defined(__x86_64__) || defined(__i386__)
  _mm_empty();
  _m_empty();
#endif

}
#endif





int32_t ulsch_qpsk_llr(LTE_DL_FRAME_PARMS *frame_parms,
                       int32_t **rxdataF_comp,
                       int16_t *ulsch_llr,
                       uint8_t symbol,
                       uint16_t nb_rb,
                       int16_t **llrp)
{
#if defined(__x86_64__) || defined(__i386__)
  __m128i *rxF=(__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  __m128i **llrp128 = (__m128i **)llrp;
#elif defined(__arm__)
  int16x8_t *rxF= (int16x8_t*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  int16x8_t **llrp128 = (int16x8_t **)llrp;
#endif

  int i;

  //  printf("qpsk llr for symbol %d (pos %d), llr offset %d\n",symbol,(symbol*frame_parms->N_RB_DL*12),llr128U-(__m128i*)ulsch_llr);

  for (i=0; i<(nb_rb*3); i++) {
    //printf("%d,%d,%d,%d,%d,%d,%d,%d\n",((int16_t *)rxF)[0],((int16_t *)rxF)[1],((int16_t *)rxF)[2],((int16_t *)rxF)[3],((int16_t *)rxF)[4],((int16_t *)rxF)[5],((int16_t *)rxF)[6],((int16_t *)rxF)[7]);
    *(*llrp128) = *rxF;
    rxF++;
    (*llrp128)++;
  }

#if defined(__x86_64__) || defined(__i386__)
  _mm_empty();
  _m_empty();
#endif

  return(0);

}

void ulsch_16qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
                     int32_t **rxdataF_comp,
                     int16_t *ulsch_llr,
                     int32_t **ul_ch_mag,
                     uint8_t symbol,
                     uint16_t nb_rb,
                     int16_t **llrp)
{
int i;

#if defined(__x86_64__) || defined(__i386__)
  __m128i *rxF=(__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  __m128i *ch_mag;
  __m128i mmtmpU0;
  __m128i **llrp128=(__m128i **)llrp;
  ch_mag =(__m128i*)&ul_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
#elif defined(__arm__)
  int16x8_t *rxF=(int16x8_t*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  int16x8_t *ch_mag;
  int16x8_t xmm0;
  int16_t **llrp16=llrp;
  ch_mag =(int16x8_t*)&ul_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
#endif

  for (i=0; i<(nb_rb*3); i++) {

#if defined(__x86_64__) || defined(__i386__)
    mmtmpU0 = _mm_abs_epi16(rxF[i]);
    //    print_shorts("tmp0",&tmp0);

    mmtmpU0 = _mm_subs_epi16(ch_mag[i],mmtmpU0);

    (*llrp128)[0] = _mm_unpacklo_epi32(rxF[i],mmtmpU0);
    (*llrp128)[1] = _mm_unpackhi_epi32(rxF[i],mmtmpU0);
    (*llrp128)+=2;
#elif defined(__arm__)
    xmm0 = vabsq_s16(rxF[i]);
    xmm0 = vqsubq_s16(ch_mag[i],xmm0);
    (*llrp16)[0] = vgetq_lane_s16(rxF[i],0);
    (*llrp16)[1] = vgetq_lane_s16(xmm0,0);
    (*llrp16)[2] = vgetq_lane_s16(rxF[i],1);
    (*llrp16)[3] = vgetq_lane_s16(xmm0,1);
    (*llrp16)[4] = vgetq_lane_s16(rxF[i],2);
    (*llrp16)[5] = vgetq_lane_s16(xmm0,2);
    (*llrp16)[6] = vgetq_lane_s16(rxF[i],2);
    (*llrp16)[7] = vgetq_lane_s16(xmm0,3);
    (*llrp16)[8] = vgetq_lane_s16(rxF[i],4);
    (*llrp16)[9] = vgetq_lane_s16(xmm0,4);
    (*llrp16)[10] = vgetq_lane_s16(rxF[i],5);
    (*llrp16)[11] = vgetq_lane_s16(xmm0,5);
    (*llrp16)[12] = vgetq_lane_s16(rxF[i],6);
    (*llrp16)[13] = vgetq_lane_s16(xmm0,6);
    (*llrp16)[14] = vgetq_lane_s16(rxF[i],7);
    (*llrp16)[15] = vgetq_lane_s16(xmm0,7);
    (*llrp16)+=16;
#endif


    //    print_bytes("rxF[i]",&rxF[i]);
    //    print_bytes("rxF[i+1]",&rxF[i+1]);
  }

#if defined(__x86_64__) || defined(__i386__)
  _mm_empty();
  _m_empty();
#endif

}

void ulsch_64qam_llr(LTE_DL_FRAME_PARMS *frame_parms,
                     int32_t **rxdataF_comp,
                     int16_t *ulsch_llr,
                     int32_t **ul_ch_mag,
                     int32_t **ul_ch_magb,
                     uint8_t symbol,
                     uint16_t nb_rb,
                     int16_t **llrp)
{
  int i;
  int32_t **llrp32=(int32_t **)llrp;

#if defined(__x86_64__) || defined(__i386)
  __m128i *rxF=(__m128i*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  __m128i *ch_mag,*ch_magb;
  __m128i mmtmpU1,mmtmpU2;

  ch_mag =(__m128i*)&ul_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
  ch_magb =(__m128i*)&ul_ch_magb[0][(symbol*frame_parms->N_RB_DL*12)];
#elif defined(__arm__)
  int16x8_t *rxF=(int16x8_t*)&rxdataF_comp[0][(symbol*frame_parms->N_RB_DL*12)];
  int16x8_t *ch_mag,*ch_magb;
  int16x8_t mmtmpU1,mmtmpU2;

  ch_mag =(int16x8_t*)&ul_ch_mag[0][(symbol*frame_parms->N_RB_DL*12)];
  ch_magb =(int16x8_t*)&ul_ch_magb[0][(symbol*frame_parms->N_RB_DL*12)];
#endif
  //  printf("symbol %d: mag %d, magb %d\n",symbol,_mm_extract_epi16(ch_mag[0],0),_mm_extract_epi16(ch_magb[0],0));
  for (i=0; i<(nb_rb*3); i++) {


#if defined(__x86_64__) || defined(__i386__)
    mmtmpU1 = _mm_abs_epi16(rxF[i]);

    mmtmpU1  = _mm_subs_epi16(ch_mag[i],mmtmpU1);

    mmtmpU2 = _mm_abs_epi16(mmtmpU1);
    mmtmpU2 = _mm_subs_epi16(ch_magb[i],mmtmpU2);

    (*llrp32)[0]  = _mm_extract_epi32(rxF[i],0);
    (*llrp32)[1]  = _mm_extract_epi32(mmtmpU1,0);
    (*llrp32)[2]  = _mm_extract_epi32(mmtmpU2,0);
    (*llrp32)[3]  = _mm_extract_epi32(rxF[i],1);
    (*llrp32)[4]  = _mm_extract_epi32(mmtmpU1,1);
    (*llrp32)[5]  = _mm_extract_epi32(mmtmpU2,1);
    (*llrp32)[6]  = _mm_extract_epi32(rxF[i],2);
    (*llrp32)[7]  = _mm_extract_epi32(mmtmpU1,2);
    (*llrp32)[8]  = _mm_extract_epi32(mmtmpU2,2);
    (*llrp32)[9]  = _mm_extract_epi32(rxF[i],3);
    (*llrp32)[10] = _mm_extract_epi32(mmtmpU1,3);
    (*llrp32)[11] = _mm_extract_epi32(mmtmpU2,3);
#elif defined(__arm__)
    mmtmpU1 = vabsq_s16(rxF[i]);

    mmtmpU1 = vqsubq_s16(ch_mag[i],mmtmpU1);

    mmtmpU2 = vabsq_s16(mmtmpU1);
    mmtmpU2 = vqsubq_s16(ch_magb[i],mmtmpU2);

    (*llrp32)[0]  = vgetq_lane_s32((int32x4_t)rxF[i],0);
    (*llrp32)[1]  = vgetq_lane_s32((int32x4_t)mmtmpU1,0);
    (*llrp32)[2]  = vgetq_lane_s32((int32x4_t)mmtmpU2,0);
    (*llrp32)[3]  = vgetq_lane_s32((int32x4_t)rxF[i],1);
    (*llrp32)[4]  = vgetq_lane_s32((int32x4_t)mmtmpU1,1);
    (*llrp32)[5]  = vgetq_lane_s32((int32x4_t)mmtmpU2,1);
    (*llrp32)[6]  = vgetq_lane_s32((int32x4_t)rxF[i],2);
    (*llrp32)[7]  = vgetq_lane_s32((int32x4_t)mmtmpU1,2);
    (*llrp32)[8]  = vgetq_lane_s32((int32x4_t)mmtmpU2,2);
    (*llrp32)[9]  = vgetq_lane_s32((int32x4_t)rxF[i],3);
    (*llrp32)[10] = vgetq_lane_s32((int32x4_t)mmtmpU1,3);
    (*llrp32)[11] = vgetq_lane_s32((int32x4_t)mmtmpU2,3);

#endif
    (*llrp32)+=12;
  }
#if defined(__x86_64__) || defined(__i386__)
  _mm_empty();
  _m_empty();
#endif
}

void ulsch_detection_mrc(LTE_DL_FRAME_PARMS *frame_parms,
                         int32_t **rxdataF_comp,
                         int32_t **ul_ch_mag,
                         int32_t **ul_ch_magb,
                         uint8_t symbol,
                         uint16_t nb_rb)
{


#if defined(__x86_64__) || defined(__i386__)

  __m128i *rxdataF_comp128_0,*ul_ch_mag128_0,*ul_ch_mag128_0b;
  __m128i *rxdataF_comp128_1,*ul_ch_mag128_1,*ul_ch_mag128_1b;
#elif defined(__arm__)

  int16x8_t *rxdataF_comp128_0,*ul_ch_mag128_0,*ul_ch_mag128_0b;
  int16x8_t *rxdataF_comp128_1,*ul_ch_mag128_1,*ul_ch_mag128_1b;

#endif
  int32_t i;

  if (frame_parms->nb_antennas_rx>1) {
#if defined(__x86_64__) || defined(__i386__)
    rxdataF_comp128_0   = (__m128i *)&rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12];
    rxdataF_comp128_1   = (__m128i *)&rxdataF_comp[1][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128_0      = (__m128i *)&ul_ch_mag[0][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128_1      = (__m128i *)&ul_ch_mag[1][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128_0b     = (__m128i *)&ul_ch_magb[0][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128_1b     = (__m128i *)&ul_ch_magb[1][symbol*frame_parms->N_RB_DL*12];

    // MRC on each re of rb, both on MF output and magnitude (for 16QAM/64QAM llr computation)
    for (i=0; i<nb_rb*3; i++) {
      rxdataF_comp128_0[i] = _mm_adds_epi16(_mm_srai_epi16(rxdataF_comp128_0[i],1),_mm_srai_epi16(rxdataF_comp128_1[i],1));
      ul_ch_mag128_0[i]    = _mm_adds_epi16(_mm_srai_epi16(ul_ch_mag128_0[i],1),_mm_srai_epi16(ul_ch_mag128_1[i],1));
      ul_ch_mag128_0b[i]   = _mm_adds_epi16(_mm_srai_epi16(ul_ch_mag128_0b[i],1),_mm_srai_epi16(ul_ch_mag128_1b[i],1));
      rxdataF_comp128_0[i] = _mm_add_epi16(rxdataF_comp128_0[i],(*(__m128i*)&jitterc[0]));

#elif defined(__arm__)
    rxdataF_comp128_0   = (int16x8_t *)&rxdataF_comp[0][symbol*frame_parms->N_RB_DL*12];
    rxdataF_comp128_1   = (int16x8_t *)&rxdataF_comp[1][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128_0      = (int16x8_t *)&ul_ch_mag[0][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128_1      = (int16x8_t *)&ul_ch_mag[1][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128_0b     = (int16x8_t *)&ul_ch_magb[0][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128_1b     = (int16x8_t *)&ul_ch_magb[1][symbol*frame_parms->N_RB_DL*12];

    // MRC on each re of rb, both on MF output and magnitude (for 16QAM/64QAM llr computation)
    for (i=0; i<nb_rb*3; i++) {
      rxdataF_comp128_0[i] = vhaddq_s16(rxdataF_comp128_0[i],rxdataF_comp128_1[i]);
      ul_ch_mag128_0[i]    = vhaddq_s16(ul_ch_mag128_0[i],ul_ch_mag128_1[i]);
      ul_ch_mag128_0b[i]   = vhaddq_s16(ul_ch_mag128_0b[i],ul_ch_mag128_1b[i]);
      rxdataF_comp128_0[i] = vqaddq_s16(rxdataF_comp128_0[i],(*(int16x8_t*)&jitterc[0]));


#endif
    }
  }

#if defined(__x86_64__) || defined(__i386__)
  _mm_empty();
  _m_empty();
#endif
}

void ulsch_extract_rbs_single(int32_t **rxdataF,
                              int32_t **rxdataF_ext,
                              uint32_t first_rb,
                              uint32_t nb_rb,
                              uint8_t l,
                              uint8_t Ns,
                              LTE_DL_FRAME_PARMS *frame_parms)
{


  uint16_t nb_rb1,nb_rb2;
  uint8_t aarx;
  int32_t *rxF,*rxF_ext;

  //uint8_t symbol = l+Ns*frame_parms->symbols_per_tti/2;
  uint8_t symbol = l+((7-frame_parms->Ncp)*(Ns&1)); ///symbol within sub-frame

  for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {


    nb_rb1 = cmin(cmax((int)(frame_parms->N_RB_UL) - (int)(2*first_rb),(int)0),(int)(2*nb_rb));    // 2 times no. RBs before the DC
    nb_rb2 = 2*nb_rb - nb_rb1;                                   // 2 times no. RBs after the DC

#ifdef DEBUG_ULSCH
    msg("ulsch_extract_rbs_single: 2*nb_rb1 = %d, 2*nb_rb2 = %d\n",nb_rb1,nb_rb2);
#endif

    rxF_ext   = &rxdataF_ext[aarx][(symbol*frame_parms->N_RB_UL*12)];

    if (nb_rb1) {
      rxF = &rxdataF[aarx][(first_rb*12 + frame_parms->first_carrier_offset + symbol*frame_parms->ofdm_symbol_size)];
      memcpy(rxF_ext, rxF, nb_rb1*6*sizeof(int));
      rxF_ext += nb_rb1*6;

      if (nb_rb2)  {
        //#ifdef OFDMA_ULSCH
        //  rxF = &rxdataF[aarx][(1 + symbol*frame_parms->ofdm_symbol_size)*2];
        //#else
        rxF = &rxdataF[aarx][(symbol*frame_parms->ofdm_symbol_size)];
        //#endif
        memcpy(rxF_ext, rxF, nb_rb2*6*sizeof(int));
        rxF_ext += nb_rb2*6;
      }
    } else { //there is only data in the second half
      //#ifdef OFDMA_ULSCH
      //      rxF = &rxdataF[aarx][(1 + 6*(2*first_rb - frame_parms->N_RB_UL) + symbol*frame_parms->ofdm_symbol_size)*2];
      //#else
      rxF = &rxdataF[aarx][(6*(2*first_rb - frame_parms->N_RB_UL) + symbol*frame_parms->ofdm_symbol_size)];
      //#endif
      memcpy(rxF_ext, rxF, nb_rb2*6*sizeof(int));
      rxF_ext += nb_rb2*6;
    }
  }

}

void ulsch_correct_ext(int32_t **rxdataF_ext,
                       int32_t **rxdataF_ext2,
                       uint16_t symbol,
                       LTE_DL_FRAME_PARMS *frame_parms,
                       uint16_t nb_rb)
{

  int32_t i,j,aarx;
  int32_t *rxF_ext2,*rxF_ext;

  for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {
    rxF_ext2 = &rxdataF_ext2[aarx][symbol*12*frame_parms->N_RB_UL];
    rxF_ext  = &rxdataF_ext[aarx][2*symbol*12*frame_parms->N_RB_UL];

    for (i=0,j=0; i<12*nb_rb; i++,j+=2) {
      rxF_ext2[i] = rxF_ext[j];
    }
  }
}

uint8_t gTableWidth[33]={
    0,
    0,
    0,
    0,
    0,
    0, //5
    0,
    1,
    1,
    2,
    3, //10 
    3,
    4,
    4,
    5,
    6, //15
    7,
    7,
    8,
    9,
    10, //20 
    11,
    12,
    13,
    14,
    15, //25
    16,
    17,
    17,
    18,
    18, //30 
    19, //31
    19 //32
 };   

void ulsch_channel_compensation_comp(int **rxdataF_ext,
                                int **dl_ch_estimates_ext,
                                int **dl_ch_mag,
                                int **dl_ch_magb,
                                int **rxdataF_comp,
                                LTE_DL_FRAME_PARMS *frame_parms,
                                unsigned char symbol,
                                unsigned char mod_order,
                                unsigned short nb_rb)
{


  unsigned short rb;
  unsigned char aatx,aarx,symbol_mod,pilots=0;
  __m128i *dl_ch128,*dl_ch128_2,*rxdataF128,*rxdataF_comp128,*dl_ch_mag128,*dl_ch_mag128b;
  __m128i mmtmpD0,mmtmpD1,mmtmpD2,mmtmpD3,QAM_amp128,QAM_amp128b;

  __m128i avg128U;

  uint32_t avg;

  uint8_t output_shift[3]={0};
  uint8_t ucWidth[3]={0};
  int16_t sAmp;
  int16_t *pAmp;
  int16_t *pComp;

  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;


  
  if (mod_order == 4) 
  {
    QAM_amp128 = _mm_set1_epi16(QAM16_n1);  // 2/sqrt(10)
  } 
  else if (mod_order == 6) 
  {
    QAM_amp128  = _mm_set1_epi16(QAM64_n1); //
  }
  

  for (aatx=0; aatx<frame_parms->nb_antennas_tx_eNB; aatx++) 
  {


    for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) 
    {

      dl_ch128          = (__m128i *)&dl_ch_estimates_ext[(aatx<<1)+aarx][symbol*frame_parms->N_RB_DL*12];
      rxdataF128        = (__m128i *)&rxdataF_ext[aarx][symbol*frame_parms->N_RB_DL*12];
      rxdataF_comp128   = (__m128i *)&rxdataF_comp[(aatx<<1)+aarx][symbol*frame_parms->N_RB_DL*12];
      dl_ch_mag128      = (__m128i *)&dl_ch_mag[(aatx<<1)+aarx][symbol*frame_parms->N_RB_DL*12];
      dl_ch_mag128b     = (__m128i *)&dl_ch_magb[(aatx<<1)+aarx][symbol*frame_parms->N_RB_DL*12];


      for (rb=0; rb<nb_rb; rb++) 
      {
        

        //4????????????
        for( int group = 0; group < 3; group++)
        {
            //??????????????
            avg128U = _mm_madd_epi16(dl_ch128[0],dl_ch128[0]);
            avg = (((unsigned int*)&avg128U)[0] +
                         ((unsigned int*)&avg128U)[1] +
                         ((unsigned int*)&avg128U)[2] +
                         ((unsigned int*)&avg128U)[3])/4;
            
            ucWidth[group] = log2_approx(avg);
            output_shift[group] = gTableWidth[ucWidth[group]];

            //????????
            // multiply by conjugated channel
            mmtmpD0 = _mm_madd_epi16(dl_ch128[0],rxdataF128[0]);
            //  print_ints("re",&mmtmpD0);
            
            // mmtmpD0 contains real part of 4 consecutive outputs (32-bit)
            mmtmpD1 = _mm_shufflelo_epi16(dl_ch128[0],_MM_SHUFFLE(2,3,0,1));
            mmtmpD1 = _mm_shufflehi_epi16(mmtmpD1,_MM_SHUFFLE(2,3,0,1));
            mmtmpD1 = _mm_sign_epi16(mmtmpD1,*(__m128i*)&conjugate[0]);
            //  print_ints("im",&mmtmpD1);
            mmtmpD1 = _mm_madd_epi16(mmtmpD1,rxdataF128[0]);
            // mmtmpD1 contains imag part of 4 consecutive outputs (32-bit)
            mmtmpD0 = _mm_srai_epi32(mmtmpD0,output_shift[group]);
            //  print_ints("re(shift)",&mmtmpD0);
            mmtmpD1 = _mm_srai_epi32(mmtmpD1,output_shift[group]);
            //  print_ints("im(shift)",&mmtmpD1);
            mmtmpD2 = _mm_unpacklo_epi32(mmtmpD0,mmtmpD1);
            mmtmpD3 = _mm_unpackhi_epi32(mmtmpD0,mmtmpD1);
            //        print_ints("c0",&mmtmpD2);
            //  print_ints("c1",&mmtmpD3);
            rxdataF_comp128[0] = _mm_packs_epi32(mmtmpD2,mmtmpD3);

            //????MAG
            if (mod_order > 2) 
            {
                mmtmpD0 = _mm_srai_epi32(avg128U,output_shift[group]);
                mmtmpD1 = _mm_packs_epi32(mmtmpD0,mmtmpD0);

                dl_ch_mag128[0] = _mm_unpacklo_epi16(mmtmpD1,mmtmpD1);

                dl_ch_mag128[0] = _mm_mulhi_epi16(dl_ch_mag128[0],QAM_amp128);

                //dl_ch_mag128[0] = _mm_slli_epi16(dl_ch_mag128b[0],1);


                for(int i = 0; i < 4; i++)
                {
                    pAmp= (unsigned short*)&((unsigned int*)dl_ch_mag128)[i];
                    pComp= (unsigned short*)&((unsigned int*)rxdataF_comp128)[i];
                        /*
                        if(*pAmp > 2000)
                        {
                            printf("ucWidth=%d output_shift=%d *pAmp=%d\n",ucWidth,output_shift, *pAmp);
                            sAmp = 1;
                        }
                        else if( *pAmp!=0)
                        {
                            sAmp = 2000/(*pAmp);
                        }*/
                    if( (*pAmp) != 0)
                    {
                        *pComp = *pComp*2000/(*pAmp); //_mm_set1_epi16(amp); //
                    }
                    
                    pAmp++;pComp++;
                    if( (*pAmp) != 0)
                    {
                        *pComp = *pComp*2000/(*pAmp);
                    }
                }
                //     printf("freq_eq: symbol %d re %d => %d,%d,%d, (%d) (%d,%d) => ",symbol,re,*((int16_t*)(&ul_ch_mag128[re])),amp,inv_ch[8*amp],*((int16_t*)(&ul_ch_mag128[re]))*inv_ch[8*amp],*(int16_t*)&(rxdataF_comp128[re]),*(1+(int16_t*)&(rxdataF_comp128[re])));
                //rxdataF_comp128[0] = _mm_mullo_epi16(rxdataF_comp128[0],dl_ch_mag128[0]);
                
                if (mod_order==4)
                {
                  dl_ch_mag128[0]  = _mm_set1_epi16(4000/*324*/);  // this is 512*2/sqrt(10)
                }
                else 
                {
                  dl_ch_mag128[0]  = _mm_set1_epi16(4000/*316*/);  // this is 512*4/sqrt(42)
                  dl_ch_mag128b[0] = _mm_set1_epi16(2000/*158*/);  // this is 512*2/sqrt(42)
                }

            }


            dl_ch128++;
            rxdataF128++;
            rxdataF_comp128++;
            dl_ch_mag128++;
            dl_ch_mag128b++;

        }
        /*
        print_info("ulsch_channel_compensation_comp:rb=%d width=%d %d %d shift=%d %d %d\n",
            rb,
            ucWidth[0],ucWidth[1],ucWidth[2],
            output_shift[0],output_shift[1],output_shift[2]);*/

      }
    }
  }

  _mm_empty();
  _m_empty();


}

void ulsch_channel_compensation(int32_t **rxdataF_ext,
                                int32_t **ul_ch_estimates_ext,
                                int32_t **ul_ch_mag,
                                int32_t **ul_ch_magb,
                                int32_t **rxdataF_comp,
                                LTE_DL_FRAME_PARMS *frame_parms,
                                uint8_t symbol,
                                uint8_t Qm,
                                uint16_t nb_rb,
                                uint8_t output_shift)
{

  uint16_t rb;

#if defined(__x86_64__) || defined(__i386__)

  __m128i *ul_ch128,*ul_ch_mag128,*ul_ch_mag128b,*rxdataF128,*rxdataF_comp128;
  uint8_t aarx;//,symbol_mod;
  __m128i mmtmpU0,mmtmpU1,mmtmpU2,mmtmpU3;
#ifdef OFDMA_ULSCH
  __m128i QAM_amp128U,QAM_amp128bU;
#endif

#elif defined(__arm__)

  int16x4_t *ul_ch128,*rxdataF128;
  int16x8_t *ul_ch_mag128,*ul_ch_mag128b,*rxdataF_comp128;

  uint8_t aarx;//,symbol_mod;
  int32x4_t mmtmpU0,mmtmpU1,mmtmpU0b,mmtmpU1b;
#ifdef OFDMA_ULSCH
  int16x8_t mmtmpU2,mmtmpU3;
  int16x8_t QAM_amp128U,QAM_amp128bU;
#endif
  int16_t conj[4]__attribute__((aligned(16))) = {1,-1,1,-1};
  int32x4_t output_shift128 = vmovq_n_s32(-(int32_t)output_shift);



#endif

#ifdef OFDMA_ULSCH

#if defined(__x86_64__) || defined(__i386__)
  if (Qm == 4)
    QAM_amp128U = _mm_set1_epi16(QAM16_n1);
  else if (Qm == 6) {
    QAM_amp128U  = _mm_set1_epi16(QAM64_n1);
    QAM_amp128bU = _mm_set1_epi16(QAM64_n2);
  }
#elif defined(__arm__)
  if (Qm == 4)
    QAM_amp128U = vdupq_n_s16(QAM16_n1);
  else if (Qm == 6) {
    QAM_amp128U  = vdupq_n_s16(QAM64_n1);
    QAM_amp128bU = vdupq_n_s16(QAM64_n2);
  }

#endif
#endif

  for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {

#if defined(__x86_64__) || defined(__i386__)

    ul_ch128          = (__m128i *)&ul_ch_estimates_ext[aarx][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128      = (__m128i *)&ul_ch_mag[aarx][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128b     = (__m128i *)&ul_ch_magb[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF128        = (__m128i *)&rxdataF_ext[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF_comp128   = (__m128i *)&rxdataF_comp[aarx][symbol*frame_parms->N_RB_DL*12];

#elif defined(__arm__)


    ul_ch128          = (int16x4_t *)&ul_ch_estimates_ext[aarx][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128      = (int16x8_t *)&ul_ch_mag[aarx][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128b     = (int16x8_t *)&ul_ch_magb[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF128        = (int16x4_t *)&rxdataF_ext[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF_comp128   = (int16x8_t *)&rxdataF_comp[aarx][symbol*frame_parms->N_RB_DL*12];

#endif
    for (rb=0; rb<nb_rb; rb++) {
      //            printf("comp: symbol %d rb %d\n",symbol,rb);
#ifdef OFDMA_ULSCH
      if (Qm>2) {
        // get channel amplitude if not QPSK

#if defined(__x86_64__) || defined(__i386__)
        mmtmpU0 = _mm_madd_epi16(ul_ch128[0],ul_ch128[0]);

        mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);

        mmtmpU1 = _mm_madd_epi16(ul_ch128[1],ul_ch128[1]);
        mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift);
        mmtmpU0 = _mm_packs_epi32(mmtmpU0,mmtmpU1);

        ul_ch_mag128[0] = _mm_unpacklo_epi16(mmtmpU0,mmtmpU0);
        ul_ch_mag128b[0] = ul_ch_mag128[0];
        ul_ch_mag128[0] = _mm_mulhi_epi16(ul_ch_mag128[0],QAM_amp128U);
        ul_ch_mag128[0] = _mm_slli_epi16(ul_ch_mag128[0],2);  // 2 to compensate the scale channel estimate
        ul_ch_mag128[1] = _mm_unpackhi_epi16(mmtmpU0,mmtmpU0);
        ul_ch_mag128b[1] = ul_ch_mag128[1];
        ul_ch_mag128[1] = _mm_mulhi_epi16(ul_ch_mag128[1],QAM_amp128U);
        ul_ch_mag128[1] = _mm_slli_epi16(ul_ch_mag128[1],2);  // 2 to compensate the scale channel estimate

        mmtmpU0 = _mm_madd_epi16(ul_ch128[2],ul_ch128[2]);
        mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
        mmtmpU1 = _mm_packs_epi32(mmtmpU0,mmtmpU0);

        ul_ch_mag128[2] = _mm_unpacklo_epi16(mmtmpU1,mmtmpU1);
        ul_ch_mag128b[2] = ul_ch_mag128[2];

        ul_ch_mag128[2] = _mm_mulhi_epi16(ul_ch_mag128[2],QAM_amp128U);
        ul_ch_mag128[2] = _mm_slli_epi16(ul_ch_mag128[2],2); // 2 to compensate the scale channel estimate


        ul_ch_mag128b[0] = _mm_mulhi_epi16(ul_ch_mag128b[0],QAM_amp128bU);
        ul_ch_mag128b[0] = _mm_slli_epi16(ul_ch_mag128b[0],2); // 2 to compensate the scale channel estimate


        ul_ch_mag128b[1] = _mm_mulhi_epi16(ul_ch_mag128b[1],QAM_amp128bU);
        ul_ch_mag128b[1] = _mm_slli_epi16(ul_ch_mag128b[1],2); // 2 to compensate the scale channel estimate

        ul_ch_mag128b[2] = _mm_mulhi_epi16(ul_ch_mag128b[2],QAM_amp128bU);
        ul_ch_mag128b[2] = _mm_slli_epi16(ul_ch_mag128b[2],2);// 2 to compensate the scale channel estimate

#elif defined(__arm__)
          mmtmpU0 = vmull_s16(ul_ch128[0], ul_ch128[0]);
          mmtmpU0 = vqshlq_s32(vqaddq_s32(mmtmpU0,vrev64q_s32(mmtmpU0)),-output_shift128);
          mmtmpU1 = vmull_s16(ul_ch128[1], ul_ch128[1]);
          mmtmpU1 = vqshlq_s32(vqaddq_s32(mmtmpU1,vrev64q_s32(mmtmpU1)),-output_shift128);
          mmtmpU2 = vcombine_s16(vmovn_s32(mmtmpU0),vmovn_s32(mmtmpU1));
          mmtmpU0 = vmull_s16(ul_ch128[2], ul_ch128[2]);
          mmtmpU0 = vqshlq_s32(vqaddq_s32(mmtmpU0,vrev64q_s32(mmtmpU0)),-output_shift128);
          mmtmpU1 = vmull_s16(ul_ch128[3], ul_ch128[3]);
          mmtmpU1 = vqshlq_s32(vqaddq_s32(mmtmpU1,vrev64q_s32(mmtmpU1)),-output_shift128);
          mmtmpU3 = vcombine_s16(vmovn_s32(mmtmpU0),vmovn_s32(mmtmpU1));
          mmtmpU0 = vmull_s16(ul_ch128[4], ul_ch128[4]);
          mmtmpU0 = vqshlq_s32(vqaddq_s32(mmtmpU0,vrev64q_s32(mmtmpU0)),-output_shift128);
          mmtmpU1 = vmull_s16(ul_ch128[5], ul_ch128[5]);
          mmtmpU1 = vqshlq_s32(vqaddq_s32(mmtmpU1,vrev64q_s32(mmtmpU1)),-output_shift128);
          mmtmpU4 = vcombine_s16(vmovn_s32(mmtmpU0),vmovn_s32(mmtmpU1));

          ul_ch_mag128b[0] = vqdmulhq_s16(mmtmpU2,QAM_amp128b);
          ul_ch_mag128b[1] = vqdmulhq_s16(mmtmpU3,QAM_amp128b);
          ul_ch_mag128[0] = vqdmulhq_s16(mmtmpU2,QAM_amp128);
          ul_ch_mag128[1] = vqdmulhq_s16(mmtmpU3,QAM_amp128);
          ul_ch_mag128b[2] = vqdmulhq_s16(mmtmpU4,QAM_amp128b);
          ul_ch_mag128[2]  = vqdmulhq_s16(mmtmpU4,QAM_amp128);
#endif
      }

#else // SC-FDMA
// just compute channel magnitude without scaling, this is done after equalization for SC-FDMA

#if defined(__x86_64__) || defined(__i386__)
      mmtmpU0 = _mm_madd_epi16(ul_ch128[0],ul_ch128[0]);

      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
      mmtmpU1 = _mm_madd_epi16(ul_ch128[1],ul_ch128[1]);

      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift);

      mmtmpU0 = _mm_packs_epi32(mmtmpU0,mmtmpU1);

      ul_ch_mag128[0] = _mm_unpacklo_epi16(mmtmpU0,mmtmpU0);
      ul_ch_mag128[1] = _mm_unpackhi_epi16(mmtmpU0,mmtmpU0);

      mmtmpU0 = _mm_madd_epi16(ul_ch128[2],ul_ch128[2]);

      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
      mmtmpU1 = _mm_packs_epi32(mmtmpU0,mmtmpU0);
      ul_ch_mag128[2] = _mm_unpacklo_epi16(mmtmpU1,mmtmpU1);

      // printf("comp: symbol %d rb %d => %d,%d,%d (output_shift %d)\n",symbol,rb,*((int16_t*)&ul_ch_mag128[0]),*((int16_t*)&ul_ch_mag128[1]),*((int16_t*)&ul_ch_mag128[2]),output_shift);


#elif defined(__arm__)
          mmtmpU0 = vmull_s16(ul_ch128[0], ul_ch128[0]);
          mmtmpU0 = vqshlq_s32(vqaddq_s32(mmtmpU0,vrev64q_s32(mmtmpU0)),-output_shift128);
          mmtmpU1 = vmull_s16(ul_ch128[1], ul_ch128[1]);
          mmtmpU1 = vqshlq_s32(vqaddq_s32(mmtmpU1,vrev64q_s32(mmtmpU1)),-output_shift128);
          ul_ch_mag128[0] = vcombine_s16(vmovn_s32(mmtmpU0),vmovn_s32(mmtmpU1));
          mmtmpU0 = vmull_s16(ul_ch128[2], ul_ch128[2]);
          mmtmpU0 = vqshlq_s32(vqaddq_s32(mmtmpU0,vrev64q_s32(mmtmpU0)),-output_shift128);
          mmtmpU1 = vmull_s16(ul_ch128[3], ul_ch128[3]);
          mmtmpU1 = vqshlq_s32(vqaddq_s32(mmtmpU1,vrev64q_s32(mmtmpU1)),-output_shift128);
          ul_ch_mag128[1] = vcombine_s16(vmovn_s32(mmtmpU0),vmovn_s32(mmtmpU1));
          mmtmpU0 = vmull_s16(ul_ch128[4], ul_ch128[4]);
          mmtmpU0 = vqshlq_s32(vqaddq_s32(mmtmpU0,vrev64q_s32(mmtmpU0)),-output_shift128);
          mmtmpU1 = vmull_s16(ul_ch128[5], ul_ch128[5]);
          mmtmpU1 = vqshlq_s32(vqaddq_s32(mmtmpU1,vrev64q_s32(mmtmpU1)),-output_shift128);
          ul_ch_mag128[2] = vcombine_s16(vmovn_s32(mmtmpU0),vmovn_s32(mmtmpU1));

#endif
#endif

#if defined(__x86_64__) || defined(__i386__)
      // multiply by conjugated channel
      mmtmpU0 = _mm_madd_epi16(ul_ch128[0],rxdataF128[0]);
      //        print_ints("re",&mmtmpU0);

      // mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpU1 = _mm_shufflelo_epi16(ul_ch128[0],_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)&conjugate[0]);

      mmtmpU1 = _mm_madd_epi16(mmtmpU1,rxdataF128[0]);
      //      print_ints("im",&mmtmpU1);
      // mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
      //  print_ints("re(shift)",&mmtmpU0);
      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift);
      //  print_ints("im(shift)",&mmtmpU1);
      mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
      mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);
      //        print_ints("c0",&mmtmpU2);
      //  print_ints("c1",&mmtmpU3);
      rxdataF_comp128[0] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
      /*
              print_shorts("rx:",&rxdataF128[0]);
              print_shorts("ch:",&ul_ch128[0]);
              print_shorts("pack:",&rxdataF_comp128[0]);
      */
      // multiply by conjugated channel
      mmtmpU0 = _mm_madd_epi16(ul_ch128[1],rxdataF128[1]);
      // mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpU1 = _mm_shufflelo_epi16(ul_ch128[1],_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)conjugate);
      mmtmpU1 = _mm_madd_epi16(mmtmpU1,rxdataF128[1]);
      // mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift);
      mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
      mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);

      rxdataF_comp128[1] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
      //        print_shorts("rx:",rxdataF128[1]);
      //        print_shorts("ch:",ul_ch128[1]);
      //        print_shorts("pack:",rxdataF_comp128[1]);
      //       multiply by conjugated channel
      mmtmpU0 = _mm_madd_epi16(ul_ch128[2],rxdataF128[2]);
      // mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpU1 = _mm_shufflelo_epi16(ul_ch128[2],_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)conjugate);
      mmtmpU1 = _mm_madd_epi16(mmtmpU1,rxdataF128[2]);
      // mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift);
      mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
      mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);

      rxdataF_comp128[2] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
      //        print_shorts("rx:",rxdataF128[2]);
      //        print_shorts("ch:",ul_ch128[2]);
      //        print_shorts("pack:",rxdataF_comp128[2]);

      // Add a jitter to compensate for the saturation in "packs" resulting in a bias on the DC after IDFT
      rxdataF_comp128[0] = _mm_add_epi16(rxdataF_comp128[0],(*(__m128i*)&jitter[0]));
      rxdataF_comp128[1] = _mm_add_epi16(rxdataF_comp128[1],(*(__m128i*)&jitter[0]));
      rxdataF_comp128[2] = _mm_add_epi16(rxdataF_comp128[2],(*(__m128i*)&jitter[0]));

      ul_ch128+=3;
      ul_ch_mag128+=3;
      ul_ch_mag128b+=3;
      rxdataF128+=3;
      rxdataF_comp128+=3;
#elif defined(__arm__)
        mmtmpU0 = vmull_s16(ul_ch128[0], rxdataF128[0]);
        //mmtmpU0 = [Re(ch[0])Re(rx[0]) Im(ch[0])Im(ch[0]) Re(ch[1])Re(rx[1]) Im(ch[1])Im(ch[1])] 
        mmtmpU1 = vmull_s16(ul_ch128[1], rxdataF128[1]);
        //mmtmpU1 = [Re(ch[2])Re(rx[2]) Im(ch[2])Im(ch[2]) Re(ch[3])Re(rx[3]) Im(ch[3])Im(ch[3])] 
        mmtmpU0 = vcombine_s32(vpadd_s32(vget_low_s32(mmtmpU0),vget_high_s32(mmtmpU0)),
                               vpadd_s32(vget_low_s32(mmtmpU1),vget_high_s32(mmtmpU1)));
        //mmtmpU0 = [Re(ch[0])Re(rx[0])+Im(ch[0])Im(ch[0]) Re(ch[1])Re(rx[1])+Im(ch[1])Im(ch[1]) Re(ch[2])Re(rx[2])+Im(ch[2])Im(ch[2]) Re(ch[3])Re(rx[3])+Im(ch[3])Im(ch[3])] 

        mmtmpU0b = vmull_s16(vrev32_s16(vmul_s16(ul_ch128[0],*(int16x4_t*)conj)), rxdataF128[0]);
        //mmtmpU0 = [-Im(ch[0])Re(rx[0]) Re(ch[0])Im(rx[0]) -Im(ch[1])Re(rx[1]) Re(ch[1])Im(rx[1])]
        mmtmpU1b = vmull_s16(vrev32_s16(vmul_s16(ul_ch128[1],*(int16x4_t*)conj)), rxdataF128[1]);
        //mmtmpU0 = [-Im(ch[2])Re(rx[2]) Re(ch[2])Im(rx[2]) -Im(ch[3])Re(rx[3]) Re(ch[3])Im(rx[3])]
        mmtmpU1 = vcombine_s32(vpadd_s32(vget_low_s32(mmtmpU0b),vget_high_s32(mmtmpU0b)),
                               vpadd_s32(vget_low_s32(mmtmpU1b),vget_high_s32(mmtmpU1b)));
        //mmtmpU1 = [-Im(ch[0])Re(rx[0])+Re(ch[0])Im(rx[0]) -Im(ch[1])Re(rx[1])+Re(ch[1])Im(rx[1]) -Im(ch[2])Re(rx[2])+Re(ch[2])Im(rx[2]) -Im(ch[3])Re(rx[3])+Re(ch[3])Im(rx[3])]

        mmtmpU0 = vqshlq_s32(mmtmpU0,-output_shift128);
        mmtmpU1 = vqshlq_s32(mmtmpU1,-output_shift128);
        rxdataF_comp128[0] = vcombine_s16(vmovn_s32(mmtmpU0),vmovn_s32(mmtmpU1));
        mmtmpU0 = vmull_s16(ul_ch128[2], rxdataF128[2]);
        mmtmpU1 = vmull_s16(ul_ch128[3], rxdataF128[3]);
        mmtmpU0 = vcombine_s32(vpadd_s32(vget_low_s32(mmtmpU0),vget_high_s32(mmtmpU0)),
                               vpadd_s32(vget_low_s32(mmtmpU1),vget_high_s32(mmtmpU1)));
        mmtmpU0b = vmull_s16(vrev32_s16(vmul_s16(ul_ch128[2],*(int16x4_t*)conj)), rxdataF128[2]);
        mmtmpU1b = vmull_s16(vrev32_s16(vmul_s16(ul_ch128[3],*(int16x4_t*)conj)), rxdataF128[3]);
        mmtmpU1 = vcombine_s32(vpadd_s32(vget_low_s32(mmtmpU0b),vget_high_s32(mmtmpU0b)),
                               vpadd_s32(vget_low_s32(mmtmpU1b),vget_high_s32(mmtmpU1b)));
        mmtmpU0 = vqshlq_s32(mmtmpU0,-output_shift128);
        mmtmpU1 = vqshlq_s32(mmtmpU1,-output_shift128);
        rxdataF_comp128[1] = vcombine_s16(vmovn_s32(mmtmpU0),vmovn_s32(mmtmpU1));

        mmtmpU0 = vmull_s16(ul_ch128[4], rxdataF128[4]);
        mmtmpU1 = vmull_s16(ul_ch128[5], rxdataF128[5]);
        mmtmpU0 = vcombine_s32(vpadd_s32(vget_low_s32(mmtmpU0),vget_high_s32(mmtmpU0)),
                               vpadd_s32(vget_low_s32(mmtmpU1),vget_high_s32(mmtmpU1)));

        mmtmpU0b = vmull_s16(vrev32_s16(vmul_s16(ul_ch128[4],*(int16x4_t*)conj)), rxdataF128[4]);
        mmtmpU1b = vmull_s16(vrev32_s16(vmul_s16(ul_ch128[5],*(int16x4_t*)conj)), rxdataF128[5]);
        mmtmpU1 = vcombine_s32(vpadd_s32(vget_low_s32(mmtmpU0b),vget_high_s32(mmtmpU0b)),
                               vpadd_s32(vget_low_s32(mmtmpU1b),vget_high_s32(mmtmpU1b)));

              
        mmtmpU0 = vqshlq_s32(mmtmpU0,-output_shift128);
        mmtmpU1 = vqshlq_s32(mmtmpU1,-output_shift128);
        rxdataF_comp128[2] = vcombine_s16(vmovn_s32(mmtmpU0),vmovn_s32(mmtmpU1));
              
              // Add a jitter to compensate for the saturation in "packs" resulting in a bias on the DC after IDFT
        rxdataF_comp128[0] = vqaddq_s16(rxdataF_comp128[0],(*(int16x8_t*)&jitter[0]));
        rxdataF_comp128[1] = vqaddq_s16(rxdataF_comp128[1],(*(int16x8_t*)&jitter[0]));
        rxdataF_comp128[2] = vqaddq_s16(rxdataF_comp128[2],(*(int16x8_t*)&jitter[0]));

      
        ul_ch128+=6;
        ul_ch_mag128+=3;
        ul_ch_mag128b+=3;
        rxdataF128+=6;
        rxdataF_comp128+=3;
              
#endif
    }
  }

#if defined(__x86_64__) || defined(__i386__)
  _mm_empty();
  _m_empty();
#endif
}




#if defined(__x86_64__) || defined(__i386__)
__m128i QAM_amp128U_0,QAM_amp128bU_0,QAM_amp128U_1,QAM_amp128bU_1;
#endif

void ulsch_channel_compensation_alamouti(int32_t **rxdataF_ext,                 // For Distributed Alamouti Combining
    int32_t **ul_ch_estimates_ext_0,
    int32_t **ul_ch_estimates_ext_1,
    int32_t **ul_ch_mag_0,
    int32_t **ul_ch_magb_0,
    int32_t **ul_ch_mag_1,
    int32_t **ul_ch_magb_1,
    int32_t **rxdataF_comp_0,
    int32_t **rxdataF_comp_1,
    LTE_DL_FRAME_PARMS *frame_parms,
    uint8_t symbol,
    uint8_t Qm,
    uint16_t nb_rb,
    uint8_t output_shift)
{
#if defined(__x86_64__) || defined(__i386__)
  uint16_t rb;
  __m128i *ul_ch128_0,*ul_ch128_1,*ul_ch_mag128_0,*ul_ch_mag128_1,*ul_ch_mag128b_0,*ul_ch_mag128b_1,*rxdataF128,*rxdataF_comp128_0,*rxdataF_comp128_1;
  uint8_t aarx;//,symbol_mod;
  __m128i mmtmpU0,mmtmpU1,mmtmpU2,mmtmpU3;

  //  symbol_mod = (symbol>=(7-frame_parms->Ncp)) ? symbol-(7-frame_parms->Ncp) : symbol;

  //    printf("comp: symbol %d\n",symbol);


  if (Qm == 4) {
    QAM_amp128U_0 = _mm_set1_epi16(QAM16_n1);
    QAM_amp128U_1 = _mm_set1_epi16(QAM16_n1);
  } else if (Qm == 6) {
    QAM_amp128U_0  = _mm_set1_epi16(QAM64_n1);
    QAM_amp128bU_0 = _mm_set1_epi16(QAM64_n2);

    QAM_amp128U_1  = _mm_set1_epi16(QAM64_n1);
    QAM_amp128bU_1 = _mm_set1_epi16(QAM64_n2);
  }

  for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {

    ul_ch128_0          = (__m128i *)&ul_ch_estimates_ext_0[aarx][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128_0      = (__m128i *)&ul_ch_mag_0[aarx][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128b_0     = (__m128i *)&ul_ch_magb_0[aarx][symbol*frame_parms->N_RB_DL*12];
    ul_ch128_1          = (__m128i *)&ul_ch_estimates_ext_1[aarx][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128_1      = (__m128i *)&ul_ch_mag_1[aarx][symbol*frame_parms->N_RB_DL*12];
    ul_ch_mag128b_1     = (__m128i *)&ul_ch_magb_1[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF128        = (__m128i *)&rxdataF_ext[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF_comp128_0   = (__m128i *)&rxdataF_comp_0[aarx][symbol*frame_parms->N_RB_DL*12];
    rxdataF_comp128_1   = (__m128i *)&rxdataF_comp_1[aarx][symbol*frame_parms->N_RB_DL*12];


    for (rb=0; rb<nb_rb; rb++) {
      //      printf("comp: symbol %d rb %d\n",symbol,rb);
      if (Qm>2) {
        // get channel amplitude if not QPSK

        mmtmpU0 = _mm_madd_epi16(ul_ch128_0[0],ul_ch128_0[0]);

        mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);

        mmtmpU1 = _mm_madd_epi16(ul_ch128_0[1],ul_ch128_0[1]);
        mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift);
        mmtmpU0 = _mm_packs_epi32(mmtmpU0,mmtmpU1);

        ul_ch_mag128_0[0] = _mm_unpacklo_epi16(mmtmpU0,mmtmpU0);
        ul_ch_mag128b_0[0] = ul_ch_mag128_0[0];
        ul_ch_mag128_0[0] = _mm_mulhi_epi16(ul_ch_mag128_0[0],QAM_amp128U_0);
        ul_ch_mag128_0[0] = _mm_slli_epi16(ul_ch_mag128_0[0],2); // 2 to compensate the scale channel estimate

        ul_ch_mag128_0[1] = _mm_unpackhi_epi16(mmtmpU0,mmtmpU0);
        ul_ch_mag128b_0[1] = ul_ch_mag128_0[1];
        ul_ch_mag128_0[1] = _mm_mulhi_epi16(ul_ch_mag128_0[1],QAM_amp128U_0);
        ul_ch_mag128_0[1] = _mm_slli_epi16(ul_ch_mag128_0[1],2); // 2 to scale compensate the scale channel estimate

        mmtmpU0 = _mm_madd_epi16(ul_ch128_0[2],ul_ch128_0[2]);
        mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
        mmtmpU1 = _mm_packs_epi32(mmtmpU0,mmtmpU0);

        ul_ch_mag128_0[2] = _mm_unpacklo_epi16(mmtmpU1,mmtmpU1);
        ul_ch_mag128b_0[2] = ul_ch_mag128_0[2];

        ul_ch_mag128_0[2] = _mm_mulhi_epi16(ul_ch_mag128_0[2],QAM_amp128U_0);
        ul_ch_mag128_0[2] = _mm_slli_epi16(ul_ch_mag128_0[2],2);  //  2 to scale compensate the scale channel estimat


        ul_ch_mag128b_0[0] = _mm_mulhi_epi16(ul_ch_mag128b_0[0],QAM_amp128bU_0);
        ul_ch_mag128b_0[0] = _mm_slli_epi16(ul_ch_mag128b_0[0],2);  //  2 to scale compensate the scale channel estima


        ul_ch_mag128b_0[1] = _mm_mulhi_epi16(ul_ch_mag128b_0[1],QAM_amp128bU_0);
        ul_ch_mag128b_0[1] = _mm_slli_epi16(ul_ch_mag128b_0[1],2);   //  2 to scale compensate the scale channel estima

        ul_ch_mag128b_0[2] = _mm_mulhi_epi16(ul_ch_mag128b_0[2],QAM_amp128bU_0);
        ul_ch_mag128b_0[2] = _mm_slli_epi16(ul_ch_mag128b_0[2],2);   //  2 to scale compensate the scale channel estima




        mmtmpU0 = _mm_madd_epi16(ul_ch128_1[0],ul_ch128_1[0]);

        mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);

        mmtmpU1 = _mm_madd_epi16(ul_ch128_1[1],ul_ch128_1[1]);
        mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift);
        mmtmpU0 = _mm_packs_epi32(mmtmpU0,mmtmpU1);

        ul_ch_mag128_1[0] = _mm_unpacklo_epi16(mmtmpU0,mmtmpU0);
        ul_ch_mag128b_1[0] = ul_ch_mag128_1[0];
        ul_ch_mag128_1[0] = _mm_mulhi_epi16(ul_ch_mag128_1[0],QAM_amp128U_1);
        ul_ch_mag128_1[0] = _mm_slli_epi16(ul_ch_mag128_1[0],2); // 2 to compensate the scale channel estimate

        ul_ch_mag128_1[1] = _mm_unpackhi_epi16(mmtmpU0,mmtmpU0);
        ul_ch_mag128b_1[1] = ul_ch_mag128_1[1];
        ul_ch_mag128_1[1] = _mm_mulhi_epi16(ul_ch_mag128_1[1],QAM_amp128U_1);
        ul_ch_mag128_1[1] = _mm_slli_epi16(ul_ch_mag128_1[1],2); // 2 to scale compensate the scale channel estimate

        mmtmpU0 = _mm_madd_epi16(ul_ch128_1[2],ul_ch128_1[2]);
        mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
        mmtmpU1 = _mm_packs_epi32(mmtmpU0,mmtmpU0);

        ul_ch_mag128_1[2] = _mm_unpacklo_epi16(mmtmpU1,mmtmpU1);
        ul_ch_mag128b_1[2] = ul_ch_mag128_1[2];

        ul_ch_mag128_1[2] = _mm_mulhi_epi16(ul_ch_mag128_1[2],QAM_amp128U_0);
        ul_ch_mag128_1[2] = _mm_slli_epi16(ul_ch_mag128_1[2],2);  //  2 to scale compensate the scale channel estimat


        ul_ch_mag128b_1[0] = _mm_mulhi_epi16(ul_ch_mag128b_1[0],QAM_amp128bU_1);
        ul_ch_mag128b_1[0] = _mm_slli_epi16(ul_ch_mag128b_1[0],2);  //  2 to scale compensate the scale channel estima


        ul_ch_mag128b_1[1] = _mm_mulhi_epi16(ul_ch_mag128b_1[1],QAM_amp128bU_1);
        ul_ch_mag128b_1[1] = _mm_slli_epi16(ul_ch_mag128b_1[1],2);   //  2 to scale compensate the scale channel estima

        ul_ch_mag128b_1[2] = _mm_mulhi_epi16(ul_ch_mag128b_1[2],QAM_amp128bU_1);
        ul_ch_mag128b_1[2] = _mm_slli_epi16(ul_ch_mag128b_1[2],2);   //  2 to scale compensate the scale channel estima
      }


      /************************For Computing (y)*(h0*)********************************************/

      // multiply by conjugated channel
      mmtmpU0 = _mm_madd_epi16(ul_ch128_0[0],rxdataF128[0]);
      //  print_ints("re",&mmtmpU0);

      // mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpU1 = _mm_shufflelo_epi16(ul_ch128_0[0],_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)&conjugate[0]);
      //  print_ints("im",&mmtmpU1);
      mmtmpU1 = _mm_madd_epi16(mmtmpU1,rxdataF128[0]);
      // mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
      //  print_ints("re(shift)",&mmtmpU0);
      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift);
      //  print_ints("im(shift)",&mmtmpU1);
      mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
      mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);
      //        print_ints("c0",&mmtmpU2);
      //  print_ints("c1",&mmtmpU3);
      rxdataF_comp128_0[0] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
      //        print_shorts("rx:",rxdataF128[0]);
      //        print_shorts("ch:",ul_ch128_0[0]);
      //        print_shorts("pack:",rxdataF_comp128_0[0]);

      // multiply by conjugated channel
      mmtmpU0 = _mm_madd_epi16(ul_ch128_0[1],rxdataF128[1]);
      // mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpU1 = _mm_shufflelo_epi16(ul_ch128_0[1],_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)conjugate);
      mmtmpU1 = _mm_madd_epi16(mmtmpU1,rxdataF128[1]);
      // mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift);
      mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
      mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);

      rxdataF_comp128_0[1] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
      //        print_shorts("rx:",rxdataF128[1]);
      //        print_shorts("ch:",ul_ch128_0[1]);
      //        print_shorts("pack:",rxdataF_comp128_0[1]);
      //       multiply by conjugated channel
      mmtmpU0 = _mm_madd_epi16(ul_ch128_0[2],rxdataF128[2]);
      // mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpU1 = _mm_shufflelo_epi16(ul_ch128_0[2],_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)conjugate);
      mmtmpU1 = _mm_madd_epi16(mmtmpU1,rxdataF128[2]);
      // mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift);
      mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
      mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);

      rxdataF_comp128_0[2] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
      //        print_shorts("rx:",rxdataF128[2]);
      //        print_shorts("ch:",ul_ch128_0[2]);
      //        print_shorts("pack:",rxdataF_comp128_0[2]);




      /*************************For Computing (y*)*(h1)************************************/
      // multiply by conjugated signal
      mmtmpU0 = _mm_madd_epi16(ul_ch128_1[0],rxdataF128[0]);
      //  print_ints("re",&mmtmpU0);

      // mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpU1 = _mm_shufflelo_epi16(rxdataF128[0],_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)&conjugate[0]);
      //  print_ints("im",&mmtmpU1);
      mmtmpU1 = _mm_madd_epi16(mmtmpU1,ul_ch128_1[0]);
      // mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
      //  print_ints("re(shift)",&mmtmpU0);
      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift);
      //  print_ints("im(shift)",&mmtmpU1);
      mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
      mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);
      //        print_ints("c0",&mmtmpU2);
      //  print_ints("c1",&mmtmpU3);
      rxdataF_comp128_1[0] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
      //        print_shorts("rx:",rxdataF128[0]);
      //        print_shorts("ch_conjugate:",ul_ch128_1[0]);
      //        print_shorts("pack:",rxdataF_comp128_1[0]);


      // multiply by conjugated signal
      mmtmpU0 = _mm_madd_epi16(ul_ch128_1[1],rxdataF128[1]);
      // mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpU1 = _mm_shufflelo_epi16(rxdataF128[1],_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)conjugate);
      mmtmpU1 = _mm_madd_epi16(mmtmpU1,ul_ch128_1[1]);
      // mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift);
      mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
      mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);

      rxdataF_comp128_1[1] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
      //        print_shorts("rx:",rxdataF128[1]);
      //        print_shorts("ch_conjugate:",ul_ch128_1[1]);
      //        print_shorts("pack:",rxdataF_comp128_1[1]);


      //       multiply by conjugated signal
      mmtmpU0 = _mm_madd_epi16(ul_ch128_1[2],rxdataF128[2]);
      // mmtmpU0 contains real part of 4 consecutive outputs (32-bit)
      mmtmpU1 = _mm_shufflelo_epi16(rxdataF128[2],_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_shufflehi_epi16(mmtmpU1,_MM_SHUFFLE(2,3,0,1));
      mmtmpU1 = _mm_sign_epi16(mmtmpU1,*(__m128i*)conjugate);
      mmtmpU1 = _mm_madd_epi16(mmtmpU1,ul_ch128_1[2]);
      // mmtmpU1 contains imag part of 4 consecutive outputs (32-bit)
      mmtmpU0 = _mm_srai_epi32(mmtmpU0,output_shift);
      mmtmpU1 = _mm_srai_epi32(mmtmpU1,output_shift);
      mmtmpU2 = _mm_unpacklo_epi32(mmtmpU0,mmtmpU1);
      mmtmpU3 = _mm_unpackhi_epi32(mmtmpU0,mmtmpU1);

      rxdataF_comp128_1[2] = _mm_packs_epi32(mmtmpU2,mmtmpU3);
      //        print_shorts("rx:",rxdataF128[2]);
      //        print_shorts("ch_conjugate:",ul_ch128_0[2]);
      //        print_shorts("pack:",rxdataF_comp128_1[2]);



      ul_ch128_0+=3;
      ul_ch_mag128_0+=3;
      ul_ch_mag128b_0+=3;
      ul_ch128_1+=3;
      ul_ch_mag128_1+=3;
      ul_ch_mag128b_1+=3;
      rxdataF128+=3;
      rxdataF_comp128_0+=3;
      rxdataF_comp128_1+=3;

    }
  }


  _mm_empty();
  _m_empty();
#endif
}




void ulsch_alamouti(LTE_DL_FRAME_PARMS *frame_parms,// For Distributed Alamouti Receiver Combining
                    int32_t **rxdataF_comp,
                    int32_t **rxdataF_comp_0,
                    int32_t **rxdataF_comp_1,
                    int32_t **ul_ch_mag,
                    int32_t **ul_ch_magb,
                    int32_t **ul_ch_mag_0,
                    int32_t **ul_ch_magb_0,
                    int32_t **ul_ch_mag_1,
                    int32_t **ul_ch_magb_1,
                    uint8_t symbol,
                    uint16_t nb_rb)
{

#if defined(__x86_64__) || defined(__i386__)
  int16_t *rxF,*rxF0,*rxF1;
  __m128i *ch_mag,*ch_magb,*ch_mag0,*ch_mag1,*ch_mag0b,*ch_mag1b;
  uint8_t rb,re,aarx;
  int32_t jj=(symbol*frame_parms->N_RB_DL*12);


  for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {

    rxF      = (int16_t*)&rxdataF_comp[aarx][jj];
    rxF0     = (int16_t*)&rxdataF_comp_0[aarx][jj];   // Contains (y)*(h0*)
    rxF1     = (int16_t*)&rxdataF_comp_1[aarx][jj];   // Contains (y*)*(h1)
    ch_mag   = (__m128i *)&ul_ch_mag[aarx][jj];
    ch_mag0 = (__m128i *)&ul_ch_mag_0[aarx][jj];
    ch_mag1 = (__m128i *)&ul_ch_mag_1[aarx][jj];
    ch_magb = (__m128i *)&ul_ch_magb[aarx][jj];
    ch_mag0b = (__m128i *)&ul_ch_magb_0[aarx][jj];
    ch_mag1b = (__m128i *)&ul_ch_magb_1[aarx][jj];

    for (rb=0; rb<nb_rb; rb++) {

      for (re=0; re<12; re+=2) {

        // Alamouti RX combining

        rxF[0] = rxF0[0] + rxF1[2];                   // re((y0)*(h0*))+ re((y1*)*(h1)) = re(x0)
        rxF[1] = rxF0[1] + rxF1[3];                   // im((y0)*(h0*))+ im((y1*)*(h1)) = im(x0)

        rxF[2] = rxF0[2] - rxF1[0];                   // re((y1)*(h0*))- re((y0*)*(h1)) = re(x1)
        rxF[3] = rxF0[3] - rxF1[1];                   // im((y1)*(h0*))- im((y0*)*(h1)) = im(x1)

        rxF+=4;
        rxF0+=4;
        rxF1+=4;
      }

      // compute levels for 16QAM or 64 QAM llr unit
      ch_mag[0] = _mm_adds_epi16(ch_mag0[0],ch_mag1[0]);
      ch_mag[1] = _mm_adds_epi16(ch_mag0[1],ch_mag1[1]);
      ch_mag[2] = _mm_adds_epi16(ch_mag0[2],ch_mag1[2]);
      ch_magb[0] = _mm_adds_epi16(ch_mag0b[0],ch_mag1b[0]);
      ch_magb[1] = _mm_adds_epi16(ch_mag0b[1],ch_mag1b[1]);
      ch_magb[2] = _mm_adds_epi16(ch_mag0b[2],ch_mag1b[2]);

      ch_mag+=3;
      ch_mag0+=3;
      ch_mag1+=3;
      ch_magb+=3;
      ch_mag0b+=3;
      ch_mag1b+=3;
    }
  }

  _mm_empty();
  _m_empty();

#endif
}







void ulsch_channel_level(int32_t **drs_ch_estimates_ext,
                         LTE_DL_FRAME_PARMS *frame_parms,
                         uint32_t *avg,
                         uint16_t nb_rb)
{

  int16_t rb;
  uint8_t aarx;

#if defined(__x86_64__) || defined(__i386__)
  __m128i avg128U;
#elif defined(__arm__)
  int32x4_t avg128U;
#endif


#if defined(__x86_64__) || defined(__i386__)
  __m128i *ul_ch128;
#elif defined(__arm__)
  int16x4_t *ul_ch128;
#endif
  for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++) {
    //clear average level
#if defined(__x86_64__) || defined(__i386__)
    avg128U = _mm_setzero_si128();
    ul_ch128=(__m128i *)drs_ch_estimates_ext[aarx];

    for (rb=0; rb<nb_rb; rb++) {

      avg128U = _mm_add_epi32(avg128U,_mm_madd_epi16(ul_ch128[0],ul_ch128[0]));
      avg128U = _mm_add_epi32(avg128U,_mm_madd_epi16(ul_ch128[1],ul_ch128[1]));
      avg128U = _mm_add_epi32(avg128U,_mm_madd_epi16(ul_ch128[2],ul_ch128[2]));

      ul_ch128+=3;


    }

#elif defined(__arm__)
    avg128U = vdupq_n_s32(0);
    ul_ch128=(int16x4_t *)drs_ch_estimates_ext[aarx];

    for (rb=0; rb<nb_rb; rb++) {

       avg128U = vqaddq_s32(avg128U,vmull_s16(ul_ch128[0],ul_ch128[0]));
       avg128U = vqaddq_s32(avg128U,vmull_s16(ul_ch128[1],ul_ch128[1]));
       avg128U = vqaddq_s32(avg128U,vmull_s16(ul_ch128[2],ul_ch128[2]));
       avg128U = vqaddq_s32(avg128U,vmull_s16(ul_ch128[3],ul_ch128[3]));
       avg128U = vqaddq_s32(avg128U,vmull_s16(ul_ch128[4],ul_ch128[4]));
       avg128U = vqaddq_s32(avg128U,vmull_s16(ul_ch128[5],ul_ch128[5]));
       ul_ch128+=6;


    }

#endif

    DevAssert( nb_rb );
    avg[aarx] = (((unsigned int*)&avg128U)[0] +
                 ((unsigned int*)&avg128U)[1] +
                 ((unsigned int*)&avg128U)[2] +
                 ((unsigned int*)&avg128U)[3])/(nb_rb*12);

  }

#if defined(__x86_64__) || defined(__i386__)
  _mm_empty();
  _m_empty();
#endif
}


void rx_ulsch(PHY_VARS_eNB *phy_vars_eNB,
              uint32_t sched_subframe,
              uint8_t eNB_id,  // this is the effective sector id
              uint8_t UE_id,
              LTE_eNB_ULSCH_t **ulsch,
              uint8_t cooperation_flag)
{

  // flagMag = 0;
  LTE_eNB_COMMON *eNB_common_vars = &phy_vars_eNB->lte_eNB_common_vars;
  LTE_eNB_PUSCH *eNB_pusch_vars = phy_vars_eNB->lte_eNB_pusch_vars[UE_id][sched_subframe];
  LTE_DL_FRAME_PARMS *frame_parms = &phy_vars_eNB->lte_frame_parms;

  uint32_t l,i;
  uint32_t avgs;
  uint8_t log2_maxh=0,aarx;
  uint8_t ucWidth=0;

  uint32_t avgU[2];
  int32_t avgU_0[2],avgU_1[2]; // For the Distributed Alamouti Scheme

  int32_t avgs_0,avgs_1;
  uint32_t log2_maxh_0=0,log2_maxh_1=0;


  //  uint8_t harq_pid = ( ulsch->RRCConnRequest_flag== 0) ? subframe2harq_pid_tdd(frame_parms->tdd_config,subframe) : 0;
  uint8_t harq_pid;
  uint8_t Qm;
  uint16_t rx_power_correction;
  int16_t *llrp;
  int subframe = phy_vars_eNB->proc[sched_subframe].subframe_rx;
  int frame = phy_vars_eNB->proc[sched_subframe].frame_rx;
  int frame0 = phy_vars_eNB->proc[sched_subframe].frame_rx0;

  harq_pid = subframe2harq_pid(frame_parms,phy_vars_eNB->proc[sched_subframe].frame_rx,subframe);
  Qm = get_Qm_ul(ulsch[UE_id]->harq_processes[harq_pid]->mcs);
#ifdef DEBUG_ULSCH
  msg("rx_ulsch: eNB_id %d, harq_pid %d, nb_rb %d first_rb %d, cooperation %d\n",eNB_id,harq_pid,ulsch[UE_id]->harq_processes[harq_pid]->nb_rb,ulsch[UE_id]->harq_processes[harq_pid]->first_rb,
      cooperation_flag);
#endif //DEBUG_ULSCH

  rx_power_correction = 1;

  if (ulsch[UE_id]->harq_processes[harq_pid]->nb_rb == 0) {
    LOG_E(PHY,"PUSCH (%d/%x) nb_rb=0!\n", harq_pid,ulsch[UE_id]->rnti,harq_pid);
    return;
  }

  for (l=0; l<(frame_parms->symbols_per_tti-ulsch[UE_id]->harq_processes[harq_pid]->srs_active); l++) {

#ifdef DEBUG_ULSCH
    msg("rx_ulsch : symbol %d (first_rb %d,nb_rb %d), rxdataF %p, rxdataF_ext %p\n",l,
        ulsch[UE_id]->harq_processes[harq_pid]->first_rb,
        ulsch[UE_id]->harq_processes[harq_pid]->nb_rb,
        eNB_common_vars->rxdataF[eNB_id][sched_subframe&1],
        eNB_pusch_vars->rxdataF_ext[eNB_id]);
#endif //DEBUG_ULSCH

    ulsch_extract_rbs_single(eNB_common_vars->rxdataF[eNB_id][sched_subframe&1],
                             eNB_pusch_vars->rxdataF_ext[eNB_id],
                             ulsch[UE_id]->harq_processes[harq_pid]->first_rb,
                             ulsch[UE_id]->harq_processes[harq_pid]->nb_rb,
                             l%(frame_parms->symbols_per_tti/2),
                             l/(frame_parms->symbols_per_tti/2),
                             frame_parms);

    lte_ul_channel_estimation(phy_vars_eNB,
                              eNB_id,
                              UE_id,
                              sched_subframe,
                              l%(frame_parms->symbols_per_tti/2),
                              l/(frame_parms->symbols_per_tti/2),
                              cooperation_flag);
  }

  if(cooperation_flag == 2) {
    for (i=0; i<frame_parms->nb_antennas_rx; i++) {
      eNB_pusch_vars->ulsch_power_0[i] = signal_energy(eNB_pusch_vars->drs_ch_estimates_0[eNB_id][i],
                                         ulsch[UE_id]->harq_processes[harq_pid]->nb_rb*12)*rx_power_correction;
      eNB_pusch_vars->ulsch_power_1[i] = signal_energy(eNB_pusch_vars->drs_ch_estimates_1[eNB_id][i],
                                         ulsch[UE_id]->harq_processes[harq_pid]->nb_rb*12)*rx_power_correction;
    }
  } else {
    for (i=0; i<frame_parms->nb_antennas_rx; i++) {
      eNB_pusch_vars->ulsch_power[i] = signal_energy_nodc(eNB_pusch_vars->drs_ch_estimates[eNB_id][i],
                                       ulsch[UE_id]->harq_processes[harq_pid]->nb_rb*12)*rx_power_correction;
#ifdef LOCALIZATION
      eNB_pusch_vars->subcarrier_power = (int32_t *)malloc(ulsch[UE_id]->harq_processes[harq_pid]->nb_rb*12*sizeof(int32_t));
      eNB_pusch_vars->active_subcarrier = subcarrier_energy(eNB_pusch_vars->drs_ch_estimates[eNB_id][i],
                                          ulsch[UE_id]->harq_processes[harq_pid]->nb_rb*12, eNB_pusch_vars->subcarrier_power, rx_power_correction);
#endif
    }
  }

  //write_output("rxdataF_ext.m","rxF_ext",eNB_pusch_vars->rxdataF_ext[eNB_id][0],300*(frame_parms->symbols_per_tti-ulsch[UE_id]->srs_active),1,1);
  //write_output("ulsch_chest.m","drs_est",eNB_pusch_vars->drs_ch_estimates[eNB_id][0],300*(frame_parms->symbols_per_tti-ulsch[UE_id]->srs_active),1,1);


  if(cooperation_flag == 2) {
    ulsch_channel_level(eNB_pusch_vars->drs_ch_estimates_0[eNB_id],
                        frame_parms,
                        avgU_0,
                        ulsch[UE_id]->harq_processes[harq_pid]->nb_rb);

    //  msg("[ULSCH] avg_0[0] %d\n",avgU_0[0]);


    avgs_0 = 0;

    for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++)
      avgs_0 = cmax(avgs_0,avgU_0[(aarx<<1)]);

    log2_maxh_0 = (log2_approx(avgs_0)/2)+ log2_approx(frame_parms->nb_antennas_rx-1)+3;
#ifdef DEBUG_ULSCH
    msg("[ULSCH] log2_maxh_0 = %d (%d,%d)\n",log2_maxh_0,avgU_0[0],avgs_0);
#endif

    ulsch_channel_level(eNB_pusch_vars->drs_ch_estimates_1[eNB_id],
                        frame_parms,
                        avgU_1,
                        ulsch[UE_id]->harq_processes[harq_pid]->nb_rb);

    //  msg("[ULSCH] avg_1[0] %d\n",avgU_1[0]);


    avgs_1 = 0;

    for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++)
      avgs_1 = cmax(avgs_1,avgU_1[(aarx<<1)]);

    log2_maxh_1 = (log2_approx(avgs_1)/2) + log2_approx(frame_parms->nb_antennas_rx-1)+3;
#ifdef DEBUG_ULSCH
    msg("[ULSCH] log2_maxh_1 = %d (%d,%d)\n",log2_maxh_1,avgU_1[0],avgs_1);
#endif
    log2_maxh = max(log2_maxh_0,log2_maxh_1);
  } 
  else 
  {
    ulsch_channel_level(eNB_pusch_vars->drs_ch_estimates[eNB_id],
                        frame_parms,
                        avgU,
                        ulsch[UE_id]->harq_processes[harq_pid]->nb_rb);

    //  msg("[ULSCH] avg[0] %d\n",avgU[0]);


    avgs = 0;

    for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++)
      avgs = cmax(avgs,avgU[(aarx<<1)]);

    //      log2_maxh = 4+(log2_approx(avgs)/2);

    //xhd_oai_debug
    //log2_maxh = (log2_approx(avgs)/2)+ log2_approx(frame_parms->nb_antennas_rx-1)+4;
    ucWidth = log2_approx(avgs);
    //print_info("log2_approx = %d x=%d\n",l2,x);

    if( ucWidth > 7 )
    {
        log2_maxh = ucWidth - 7;
    }
    //log2_maxh = 15;  
    log2_maxh = gTableWidth[ucWidth];
    
    print_buff_excel(10,eNB_pusch_vars->drs_ch_estimates[eNB_id][0],
        ulsch[UE_id]->harq_processes[harq_pid]->nb_rb*12,
     "rx_ulsch: DRS UE_id=%d frame=%d subframe=%d harq_pid=%d first_rb=%d nb_rb=%d size=%d mcs=%d TBS=%d total=%d ucWidth=%d log2_maxh=%d\n",
        UE_id,frame0,subframe,harq_pid,
        ulsch[UE_id]->harq_processes[harq_pid]->first_rb,
        ulsch[UE_id]->harq_processes[harq_pid]->nb_rb,
        ulsch[UE_id]->harq_processes[harq_pid]->nb_rb*12,
        ulsch[UE_id]->harq_processes[harq_pid]->mcs,
        ulsch[UE_id]->harq_processes[harq_pid]->TBS>>3,
        avgs,
        ucWidth,
        log2_maxh
        );

    ulsch[UE_id]->harq_processes[harq_pid]->ucWidth = ucWidth; //xhd_oai_debug
    ulsch[UE_id]->harq_processes[harq_pid]->log2_maxh = log2_maxh; //xhd_oai_debug

#ifdef DEBUG_ULSCH
    msg("[ULSCH] log2_maxh = %d (%d,%d)\n",log2_maxh,avgU[0],avgs);
#endif
  }

  for (l=0; l<frame_parms->symbols_per_tti-ulsch[UE_id]->harq_processes[harq_pid]->srs_active; l++) {

    if (((frame_parms->Ncp == 0) && ((l==3) || (l==10)))||   // skip pilots
        ((frame_parms->Ncp == 1) && ((l==2) || (l==8)))) {
      l++;
    }

    if(cooperation_flag == 2) {

      ulsch_channel_compensation_alamouti(
        eNB_pusch_vars->rxdataF_ext[eNB_id],
        eNB_pusch_vars->drs_ch_estimates_0[eNB_id],
        eNB_pusch_vars->drs_ch_estimates_1[eNB_id],
        eNB_pusch_vars->ul_ch_mag_0[eNB_id],
        eNB_pusch_vars->ul_ch_magb_0[eNB_id],
        eNB_pusch_vars->ul_ch_mag_1[eNB_id],
        eNB_pusch_vars->ul_ch_magb_1[eNB_id],
        eNB_pusch_vars->rxdataF_comp_0[eNB_id],
        eNB_pusch_vars->rxdataF_comp_1[eNB_id],
        frame_parms,
        l,
        Qm,
        ulsch[UE_id]->harq_processes[harq_pid]->nb_rb,
        log2_maxh);

      ulsch_alamouti(frame_parms,
                     eNB_pusch_vars->rxdataF_comp[eNB_id],
                     eNB_pusch_vars->rxdataF_comp_0[eNB_id],
                     eNB_pusch_vars->rxdataF_comp_1[eNB_id],
                     eNB_pusch_vars->ul_ch_mag[eNB_id],
                     eNB_pusch_vars->ul_ch_magb[eNB_id],
                     eNB_pusch_vars->ul_ch_mag_0[eNB_id],
                     eNB_pusch_vars->ul_ch_magb_0[eNB_id],
                     eNB_pusch_vars->ul_ch_mag_1[eNB_id],
                     eNB_pusch_vars->ul_ch_magb_1[eNB_id],
                     l,
                     ulsch[UE_id]->harq_processes[harq_pid]->nb_rb);
    } else {
      
      ulsch_channel_compensation_comp(
        eNB_pusch_vars->rxdataF_ext[eNB_id],
        eNB_pusch_vars->drs_ch_estimates[eNB_id],
        eNB_pusch_vars->ul_ch_mag[eNB_id],
        eNB_pusch_vars->ul_ch_magb[eNB_id],
        eNB_pusch_vars->rxdataF_comp[eNB_id],
        frame_parms,
        l,
        Qm,
        ulsch[UE_id]->harq_processes[harq_pid]->nb_rb//,
        /*log2_maxh*/); // log2_maxh+I0_shift

    }


    //eren
    /* if(flagMag == 0){
    //writing for the first time
    write_output(namepointer_log2,"xxx",log2_maxh,1,1,12);

    write_output(namepointer_chMag,"xxx",eNB_pusch_vars->ul_ch_mag[eNB_id][0],300,1,11);

    //namepointer_chMag = NULL;
    flagMag=1;
    }*/

    if (frame_parms->nb_antennas_rx > 1)
      ulsch_detection_mrc(frame_parms,
                          eNB_pusch_vars->rxdataF_comp[eNB_id],
                          eNB_pusch_vars->ul_ch_mag[eNB_id],
                          eNB_pusch_vars->ul_ch_magb[eNB_id],
                          l,
                          ulsch[UE_id]->harq_processes[harq_pid]->nb_rb);

#ifndef OFDMA_ULSCH
    /*
    if ((phy_vars_eNB->PHY_measurements_eNB->n0_power_dB[0]+3)<eNB_pusch_vars->ulsch_power[0]) {

      freq_equalization(frame_parms,
                        eNB_pusch_vars->rxdataF_comp[eNB_id],
                        eNB_pusch_vars->ul_ch_mag[eNB_id],
                        eNB_pusch_vars->ul_ch_magb[eNB_id],
                        l,
                        ulsch[UE_id]->harq_processes[harq_pid]->nb_rb*12,
                        Qm);
    }*/

#endif
  }

#ifndef OFDMA_ULSCH

  //#ifdef DEBUG_ULSCH
  // Inverse-Transform equalized outputs
  //  msg("Doing IDFTs\n");
  //if( ulsch[UE_id]->harq_processes[harq_pid]->O_ACK ) //xhd_oai_debug eNB rx_ulsch
  int symbol_size = frame_parms->N_RB_DL*12;
  int nb_rb_size = ulsch[UE_id]->harq_processes[harq_pid]->nb_rb*12;
  
  if( (subframe == 4 || subframe == 8) && (frame&0xf)== 0) 
  for(int i = 0; i< 14; i++)
  {
      if(i==3||i==10)
      {
        continue;
      }
      if( i == 0)
      {
          print_buff_excel(0,&eNB_pusch_vars->rxdataF_ext[eNB_id][0][i*symbol_size],nb_rb_size,
            "rx_ulsch: rxdataF_ext UE_id=%d frame=%d subframe=%d harq_pid=%d symbol=%d first_rb=%d nb_rb=%d size=%d mcs=%d TBS=%d Qm=%d\n",
               UE_id,frame0,subframe,harq_pid,i,
               ulsch[UE_id]->harq_processes[harq_pid]->first_rb,
               ulsch[UE_id]->harq_processes[harq_pid]->nb_rb,
               ulsch[UE_id]->harq_processes[harq_pid]->nb_rb*12,
               ulsch[UE_id]->harq_processes[harq_pid]->mcs,
               ulsch[UE_id]->harq_processes[harq_pid]->TBS>>3,
               Qm
               );
      }
      else
      {
          print_buff_excel(10,&eNB_pusch_vars->rxdataF_ext[eNB_id][0][i*symbol_size],nb_rb_size,"symbo=%d\n",i);
      }
  }

  if( (subframe == 4 || subframe == 8) && (frame&0xf)== 0) 
  for(int i = 0; i< 14; i++)
  {
      if(i==3||i==10)
      {
        continue;
      }
      if( i == 0)
      {
          print_buff_excel(0,&eNB_pusch_vars->rxdataF_comp[eNB_id][0][i*symbol_size],nb_rb_size,
            "rx_ulsch: before lte_idft UE_id=%d frame=%d subframe=%d harq_pid=%d symbol=%d first_rb=%d nb_rb=%d size=%d mcs=%d TBS=%d Qm=%d\n",
               UE_id,frame0,subframe,harq_pid,i,
               ulsch[UE_id]->harq_processes[harq_pid]->first_rb,
               ulsch[UE_id]->harq_processes[harq_pid]->nb_rb,
               ulsch[UE_id]->harq_processes[harq_pid]->nb_rb*12,
               ulsch[UE_id]->harq_processes[harq_pid]->mcs,
               ulsch[UE_id]->harq_processes[harq_pid]->TBS>>3,
               Qm
               );
      }
      else
      {
          print_buff_excel(10,&eNB_pusch_vars->rxdataF_comp[eNB_id][0][i*symbol_size],nb_rb_size,"symbo=%d\n",i);
      }
  }
  
  lte_idft(frame_parms,
           (uint32_t*)eNB_pusch_vars->rxdataF_comp[eNB_id][0],
           ulsch[UE_id]->harq_processes[harq_pid]->nb_rb*12);
  //  msg("Done\n");
  //#endif //DEBUG_ULSCH
  if( (subframe == 4 || subframe == 8) && (frame&0xf)== 0) 
  //if( ulsch[UE_id]->harq_processes[harq_pid]->O_ACK ) //xhd_oai_debug eNB rx_ulsch
  for(int i = 0; i< 14; i++)
  {
      if(i==3||i==10)
      {
        continue;
      }
      if( i == 0)
      {
          print_buff_excel(0,&eNB_pusch_vars->rxdataF_comp[eNB_id][0][i*symbol_size],nb_rb_size,
           "rx_ulsch: after lte_idft UE_id=%d frame=%d subframe=%d harq_pid=%d symbol=%d first_rb=%d nb_rb=%d size=%d mcs=%d TBS=%d log2_maxh=%d Qm=%d\n",
              UE_id,frame0,subframe,harq_pid,i,
              ulsch[UE_id]->harq_processes[harq_pid]->first_rb,
              ulsch[UE_id]->harq_processes[harq_pid]->nb_rb,
              ulsch[UE_id]->harq_processes[harq_pid]->nb_rb*12,
              ulsch[UE_id]->harq_processes[harq_pid]->mcs,
              ulsch[UE_id]->harq_processes[harq_pid]->TBS>>3,
              log2_maxh,
              Qm
              );
      }
      else
      {
          print_buff_excel(0,&eNB_pusch_vars->rxdataF_comp[eNB_id][0][i*symbol_size],nb_rb_size,"symbo=%d\n",i);
      }
  }
  
#endif


  llrp = (int16_t*)&eNB_pusch_vars->llr[0];

  for (l=0; l<frame_parms->symbols_per_tti-ulsch[UE_id]->harq_processes[harq_pid]->srs_active; l++) {

    if (((frame_parms->Ncp == 0) && ((l==3) || (l==10)))||   // skip pilots
        ((frame_parms->Ncp == 1) && ((l==2) || (l==8)))) {
      l++;
    }

    switch (Qm) {
    case 2 :
      ulsch_qpsk_llr(frame_parms,
                     eNB_pusch_vars->rxdataF_comp[eNB_id],
                     eNB_pusch_vars->llr,
                     l,
                     ulsch[UE_id]->harq_processes[harq_pid]->nb_rb,
                     &llrp);
      break;

    case 4 :
      ulsch_16qam_llr(frame_parms,
                      eNB_pusch_vars->rxdataF_comp[eNB_id],
                      eNB_pusch_vars->llr,
                      eNB_pusch_vars->ul_ch_mag[eNB_id],
                      l,ulsch[UE_id]->harq_processes[harq_pid]->nb_rb,
                      &llrp);
      break;

    case 6 :
      ulsch_64qam_llr(frame_parms,
                      eNB_pusch_vars->rxdataF_comp[eNB_id],
                      eNB_pusch_vars->llr,
                      eNB_pusch_vars->ul_ch_mag[eNB_id],
                      eNB_pusch_vars->ul_ch_magb[eNB_id],
                      l,ulsch[UE_id]->harq_processes[harq_pid]->nb_rb,
                      &llrp);
      break;

    default:
#ifdef DEBUG_ULSCH
      msg("ulsch_demodulation.c (rx_ulsch): Unknown Qm!!!!\n");
#endif //DEBUG_ULSCH
      break;
    }
  }
  if( (subframe == 4) && (frame&0xf)== 0 && 0) //xhd_oai_debug eNB rx_ulsch
  {
      print_buff_excel(3,eNB_pusch_vars->llr,ulsch[UE_id]->harq_processes[harq_pid]->nb_rb*12,
       "rx_ulsch: after lte_idft UE_id=%d frame=%d subframe=%d harq_pid=%d first_rb=%d nb_rb=%d size=%d mcs=%d TBS=%d\n",
          UE_id,frame,subframe,harq_pid,
          ulsch[UE_id]->harq_processes[harq_pid]->first_rb,
          ulsch[UE_id]->harq_processes[harq_pid]->nb_rb,
          ulsch[UE_id]->harq_processes[harq_pid]->nb_rb*12,
          ulsch[UE_id]->harq_processes[harq_pid]->mcs,
          ulsch[UE_id]->harq_processes[harq_pid]->TBS>>3
          );
  }

}

void rx_ulsch_emul(PHY_VARS_eNB *phy_vars_eNB,
                   uint8_t subframe,
                   uint8_t sect_id,
                   uint8_t UE_index)
{
  msg("[PHY] EMUL eNB %d rx_ulsch_emul : subframe %d, sect_id %d, UE_index %d\n",phy_vars_eNB->Mod_id,subframe,sect_id,UE_index);
  phy_vars_eNB->lte_eNB_pusch_vars[UE_index][subframe]->ulsch_power[0] = 31622; //=45dB;
  phy_vars_eNB->lte_eNB_pusch_vars[UE_index][subframe]->ulsch_power[1] = 31622; //=45dB;

}


void dump_ulsch(PHY_VARS_eNB *PHY_vars_eNB,uint8_t sched_subframe, uint8_t UE_id)
{

  uint32_t nsymb = (PHY_vars_eNB->lte_frame_parms.Ncp == 0) ? 14 : 12;
  uint8_t harq_pid;
  int subframe = PHY_vars_eNB->proc[sched_subframe].subframe_rx;

  harq_pid = subframe2harq_pid(&PHY_vars_eNB->lte_frame_parms,PHY_vars_eNB->proc[sched_subframe].frame_rx,subframe);

  printf("Dumping ULSCH in subframe %d with harq_pid %d, for NB_rb %d, mcs %d, Qm %d, N_symb %d\n", subframe,harq_pid,PHY_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->nb_rb,
         PHY_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->mcs,get_Qm_ul(PHY_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->mcs),
         PHY_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->Nsymb_pusch);
  //#ifndef OAI_EMU
  write_output("/tmp/ulsch_d.m","ulsch_dseq",&PHY_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->d[0][96],
               PHY_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->Kplus*3,1,0);
  write_output("/tmp/rxsig0.m","rxs0", &PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][0][0],PHY_vars_eNB->lte_frame_parms.samples_per_tti*10,1,1);

  if (PHY_vars_eNB->lte_frame_parms.nb_antennas_rx>1)
    write_output("/tmp/rxsig1.m","rxs1", &PHY_vars_eNB->lte_eNB_common_vars.rxdata[0][1][0],PHY_vars_eNB->lte_frame_parms.samples_per_tti*10,1,1);

  write_output("/tmp/rxsigF0.m","rxsF0", &PHY_vars_eNB->lte_eNB_common_vars.rxdataF[0][0][0][0],PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size*nsymb,1,1);

  if (PHY_vars_eNB->lte_frame_parms.nb_antennas_rx>1)
    write_output("/tmp/rxsigF1.m","rxsF1", &PHY_vars_eNB->lte_eNB_common_vars.rxdataF[0][0][1][0],PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size*nsymb,1,1);

  write_output("/tmp/rxsigF0_ext.m","rxsF0_ext", &PHY_vars_eNB->lte_eNB_pusch_vars[UE_id][sched_subframe&1]->rxdataF_ext[0][0][0],PHY_vars_eNB->lte_frame_parms.N_RB_UL*12*nsymb,1,1);

  if (PHY_vars_eNB->lte_frame_parms.nb_antennas_rx>1)
    write_output("/tmp/rxsigF1_ext.m","rxsF1_ext", &PHY_vars_eNB->lte_eNB_pusch_vars[UE_id][sched_subframe&1]->rxdataF_ext[1][0][0],PHY_vars_eNB->lte_frame_parms.N_RB_UL*12*nsymb,1,1);


  write_output("/tmp/srs_est0.m","srsest0",PHY_vars_eNB->lte_eNB_srs_vars[UE_id].srs_ch_estimates[0][0],PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size,1,1);

  if (PHY_vars_eNB->lte_frame_parms.nb_antennas_rx>1)
    write_output("/tmp/srs_est1.m","srsest1",PHY_vars_eNB->lte_eNB_srs_vars[UE_id].srs_ch_estimates[0][1],PHY_vars_eNB->lte_frame_parms.ofdm_symbol_size,1,1);

  write_output("/tmp/drs_est0.m","drsest0",PHY_vars_eNB->lte_eNB_pusch_vars[UE_id][sched_subframe&1]->drs_ch_estimates[0][0],PHY_vars_eNB->lte_frame_parms.N_RB_UL*12*nsymb,1,1);

  if (PHY_vars_eNB->lte_frame_parms.nb_antennas_rx>1)
    write_output("/tmp/drs_est1.m","drsest1",PHY_vars_eNB->lte_eNB_pusch_vars[UE_id][sched_subframe&1]->drs_ch_estimates[0][1],PHY_vars_eNB->lte_frame_parms.N_RB_UL*12*nsymb,1,1);

  write_output("/tmp/ulsch_rxF_comp0.m","ulsch0_rxF_comp0",&PHY_vars_eNB->lte_eNB_pusch_vars[UE_id][sched_subframe&1]->rxdataF_comp[0][0][0],PHY_vars_eNB->lte_frame_parms.N_RB_UL*12*nsymb,1,1);
  //  write_output("ulsch_rxF_comp1.m","ulsch0_rxF_comp1",&PHY_vars_eNB->lte_eNB_pusch_vars[UE_id]->rxdataF_comp[0][1][0],PHY_vars_eNB->lte_frame_parms.N_RB_UL*12*nsymb,1,1);
  write_output("/tmp/ulsch_rxF_llr.m","ulsch_llr",PHY_vars_eNB->lte_eNB_pusch_vars[UE_id][sched_subframe&1]->llr,
               PHY_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->nb_rb*12*get_Qm_ul(PHY_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->mcs)
               *PHY_vars_eNB->ulsch_eNB[UE_id]->harq_processes[harq_pid]->Nsymb_pusch,1,0);
  write_output("/tmp/ulsch_ch_mag.m","ulsch_ch_mag",&PHY_vars_eNB->lte_eNB_pusch_vars[UE_id][sched_subframe&1]->ul_ch_mag[0][0][0],PHY_vars_eNB->lte_frame_parms.N_RB_UL*12*nsymb,1,1);
  //  write_output("ulsch_ch_mag1.m","ulsch_ch_mag1",&PHY_vars_eNB->lte_eNB_pusch_vars[UE_id]->ul_ch_mag[0][1][0],PHY_vars_eNB->lte_frame_parms.N_RB_UL*12*nsymb,1,1);
  //#endif
}

