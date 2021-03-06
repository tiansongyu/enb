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

/*! \file PHY/LTE_TRANSPORT/dlsch_decoding.c
* \brief Top-level routines for decoding  Turbo-coded (DLSCH) transport channels from 36-212, V8.6 2009-03
* \author R. Knopp
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr
* \note
* \warning
*/

//#include "defs.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "PHY/CODING/extern.h"
#include "SCHED/extern.h"
#include "SIMULATION/TOOLS/defs.h"
//#define DEBUG_DLSCH_DECODING
 

void free_ue_dlsch(LTE_UE_DLSCH_t *dlsch)
{

  int i,r;

  if (dlsch) {
    for (i=0; i<dlsch->Mdlharq; i++) {
      if (dlsch->harq_processes[i]) {
        if (dlsch->harq_processes[i]->b) {
          free16(dlsch->harq_processes[i]->b,MAX_DLSCH_PAYLOAD_BYTES);
          dlsch->harq_processes[i]->b = NULL;
        }

        for (r=0; r<MAX_NUM_DLSCH_SEGMENTS; r++) {
          free16(dlsch->harq_processes[i]->c[r],((r==0)?8:0) + 3+768);
          dlsch->harq_processes[i]->c[r] = NULL;
        }

        for (r=0; r<MAX_NUM_DLSCH_SEGMENTS; r++)
          if (dlsch->harq_processes[i]->d[r]) {
            free16(dlsch->harq_processes[i]->d[r],((3*8*6144)+12+96)*sizeof(short));
            dlsch->harq_processes[i]->d[r] = NULL;
          }

        free16(dlsch->harq_processes[i],sizeof(LTE_DL_UE_HARQ_t));
        dlsch->harq_processes[i] = NULL;
      }
    }

    free16(dlsch,sizeof(LTE_UE_DLSCH_t));
    dlsch = NULL;
  }
}

LTE_UE_DLSCH_t *new_ue_dlsch(uint8_t Kmimo,uint8_t Mdlharq,uint8_t max_turbo_iterations,uint8_t N_RB_DL, uint8_t abstraction_flag)
{

  LTE_UE_DLSCH_t *dlsch;
  uint8_t exit_flag = 0,i,r;

  unsigned char bw_scaling =1;

  switch (N_RB_DL) {
  case 6:
    bw_scaling =16;
    break;

  case 25:
    bw_scaling =4;
    break;

  case 50:
    bw_scaling =2;
    break;

  default:
    bw_scaling =1;
    break;
  }

  dlsch = (LTE_UE_DLSCH_t *)malloc16(sizeof(LTE_UE_DLSCH_t));

  if (dlsch) {
    memset(dlsch,0,sizeof(LTE_UE_DLSCH_t));
    dlsch->Kmimo = Kmimo;
    dlsch->Mdlharq = Mdlharq;
    dlsch->max_turbo_iterations = max_turbo_iterations;

    for (i=0; i<Mdlharq; i++) {
      //      msg("new_ue_dlsch: Harq process %d\n",i);
      dlsch->harq_processes[i] = (LTE_DL_UE_HARQ_t *)malloc16(sizeof(LTE_DL_UE_HARQ_t));

      if (dlsch->harq_processes[i]) {
        memset(dlsch->harq_processes[i],0,sizeof(LTE_DL_UE_HARQ_t));
        dlsch->harq_processes[i]->first_tx=1;
        dlsch->harq_processes[i]->b = (uint8_t*)malloc16(MAX_DLSCH_PAYLOAD_BYTES/bw_scaling);

        if (dlsch->harq_processes[i]->b)
          memset(dlsch->harq_processes[i]->b,0,MAX_DLSCH_PAYLOAD_BYTES/bw_scaling);
        else
          exit_flag=3;

        if (abstraction_flag == 0) {
          for (r=0; r<MAX_NUM_DLSCH_SEGMENTS/bw_scaling; r++) {
            dlsch->harq_processes[i]->c[r] = (uint8_t*)malloc16(((r==0)?8:0) + 3+ 768);

            if (dlsch->harq_processes[i]->c[r])
              memset(dlsch->harq_processes[i]->c[r],0,((r==0)?8:0) + 3+ 768);
            else
              exit_flag=2;

            dlsch->harq_processes[i]->d[r] = (short*)malloc16(((3*8*6144)+12+96)*sizeof(short));

            if (dlsch->harq_processes[i]->d[r])
              memset(dlsch->harq_processes[i]->d[r],0,((3*8*6144)+12+96)*sizeof(short));
            else
              exit_flag=2;
          }
        }
      } else {
        exit_flag=1;
      }
    }

    if (exit_flag==0)
      return(dlsch);
  }

  msg("new_ue_dlsch with size %zu: exit_flag = %u\n",sizeof(LTE_DL_UE_HARQ_t), exit_flag);
  free_ue_dlsch(dlsch);

  return(NULL);
}
extern void setUlAckInfo(int frame_rx,
                      uint8_t subframe,
                      PHY_VARS_UE *phy_vars_ue);

uint32_t  dlsch_decoding(PHY_VARS_UE *phy_vars_ue,
                         short *dlsch_llr,
                         LTE_DL_FRAME_PARMS *frame_parms,
                         LTE_UE_DLSCH_t *dlsch,
                         LTE_DL_UE_HARQ_t *harq_process,
                         int frame,
                         uint8_t subframe,
                         uint8_t harq_pid,
                         uint8_t is_crnti,
                         uint8_t llr8_flag)
{


  time_stats_t *dlsch_rate_unmatching_stats=&phy_vars_ue->dlsch_rate_unmatching_stats;
  time_stats_t *dlsch_turbo_decoding_stats=&phy_vars_ue->dlsch_turbo_decoding_stats;
  time_stats_t *dlsch_deinterleaving_stats=&phy_vars_ue->dlsch_deinterleaving_stats;
  uint32_t A,E;
  uint32_t G;
  uint32_t ret,offset;
  uint16_t iind;
  //  uint8_t dummy_channel_output[(3*8*block_length)+12];
  short dummy_w[MAX_NUM_DLSCH_SEGMENTS][3*(6144+64)]={0};
  uint32_t r,r_offset=0,Kr,Kr_bytes,err_flag=0;
  uint8_t crc_type;
#ifdef DEBUG_DLSCH_DECODING
  uint16_t i;
#endif
  uint8_t (*tc)(int16_t *y,
                uint8_t *,
                uint16_t,
                uint16_t,
                uint16_t,
                uint8_t,
                uint8_t,
                uint8_t,
                time_stats_t *,
                time_stats_t *,
                time_stats_t *,
                time_stats_t *,
                time_stats_t *,
                time_stats_t *,
                time_stats_t *);

  if (!dlsch_llr) {
    msg("dlsch_decoding.c: NULL dlsch_llr pointer\n");
    return(dlsch->max_turbo_iterations);
  }

  if (!harq_process) {
    msg("dlsch_decoding.c: NULL harq_process pointer\n");
    return(dlsch->max_turbo_iterations);
  }

  if (!frame_parms) {
    msg("dlsch_decoding.c: NULL frame_parms pointer\n");
    return(dlsch->max_turbo_iterations);
  }

  if (subframe>9) {
    msg("dlsch_decoding.c: Illegal subframe index %d\n",subframe);
    return(dlsch->max_turbo_iterations);
  }

  if (llr8_flag == 0)
    tc = phy_threegpplte_turbo_decoder16;
  else
    tc = phy_threegpplte_turbo_decoder8;

  //  nb_rb = dlsch->nb_rb;

  /*
  if (nb_rb > frame_parms->N_RB_DL) {
    msg("dlsch_decoding.c: Illegal nb_rb %d\n",nb_rb);
    return(max_turbo_iterations);
    }*/

  /*harq_pid = dlsch->current_harq_pid;
  if (harq_pid >= 8) {
    msg("dlsch_decoding.c: Illegal harq_pid %d\n",harq_pid);
    return(max_turbo_iterations);
  }
  */
  A = harq_process->TBS; //2072 for QPSK 1/3

  ret = dlsch->max_turbo_iterations;


  G = harq_process->G;
  //get_G(frame_parms,nb_rb,dlsch->rb_alloc,mod_order,num_pdcch_symbols,phy_vars_ue->frame,subframe);

  //  msg("DLSCH Decoding, harq_pid %d Ndi %d\n",harq_pid,harq_process->Ndi);

  int oldC = harq_process->C;
  int newC = -1;
  
  if (harq_process->round == 0) {
    // This is a new packet, so compute quantities regarding segmentation
    harq_process->B = A+24;
    lte_segmentation(NULL,
                     NULL,
                     harq_process->B,
                     &harq_process->C,
                     &harq_process->Cplus,
                     &harq_process->Cminus,
                     &harq_process->Kplus,
                     &harq_process->Kminus,
                     &harq_process->F);
    //  CLEAR LLR's HERE for first packet in process
    newC = harq_process->C;
  }

  /*
  else {
    msg("dlsch_decoding.c: Ndi>0 not checked yet!!\n");
    return(max_turbo_iterations);
  }
  */
  err_flag = 0;
  r_offset = 0;

  unsigned char bw_scaling =1;

  switch (frame_parms->N_RB_DL) {
  case 6:
    bw_scaling =16;
    break;

  case 25:
    bw_scaling =4;
    break;

  case 50:
    bw_scaling =2;
    break;

  default:
    bw_scaling =1;
    break;
  }

  if (harq_process->C >= MAX_NUM_DLSCH_SEGMENTS/bw_scaling) {
    LOG_E(PHY,"Illegal harq_process->C %d > %d harq_process->B=%d Cplus=%d Cminus=%d Kplus=%d Kminus=%d old=%d new=%d round=%d TBS=%d/%d\n",
        harq_process->C,MAX_NUM_DLSCH_SEGMENTS/bw_scaling,
        harq_process->B,harq_process->Cplus,harq_process->Cminus,harq_process->Kplus,harq_process->Kminus,
        oldC,newC,harq_process->round,harq_process->TBS,harq_process->TBS>>3);
    return((1+dlsch->max_turbo_iterations));
  }
  for (r=0; r<harq_process->C; r++) {

    
    // Get Turbo interleaver parameters
    if (r<harq_process->Cminus)
      Kr = harq_process->Kminus;
    else
      Kr = harq_process->Kplus;

    Kr_bytes = Kr>>3;

    if (Kr_bytes<=64)
      iind = (Kr_bytes-5);
    else if (Kr_bytes <=128)
      iind = 59 + ((Kr_bytes-64)>>1);
    else if (Kr_bytes <= 256)
      iind = 91 + ((Kr_bytes-128)>>2);
    else if (Kr_bytes <= 768)
      iind = 123 + ((Kr_bytes-256)>>3);
    else {
      msg("dlsch_decoding: Illegal codeword size %d!!!\n",Kr_bytes);
      return(dlsch->max_turbo_iterations);
    }

#ifdef DEBUG_DLSCH_DECODING
    printf("f1 %d, f2 %d, F %d\n",f1f2mat_old[2*iind],f1f2mat_old[1+(2*iind)],(r==0) ? harq_process->F : 0);
#endif

    start_meas(dlsch_rate_unmatching_stats);
    memset(&dummy_w[r][0],0,3*(6144+64)*sizeof(short));
    harq_process->RTC[r] = generate_dummy_w(4+(Kr_bytes*8),
                                            (uint8_t*) &dummy_w[r][0],
                                            (r==0) ? harq_process->F : 0);

#ifdef DEBUG_DLSCH_DECODING
    LOG_I(PHY,"HARQ_PID %d Rate Matching Segment %d (coded bits %d,unpunctured/repeated bits %d, TBS %d, mod_order %d, nb_rb %d, Nl %d, rv %d, round %d)...\n",
          harq_pid,r, G,
          Kr*3,
          harq_process->TBS,
          harq_process->Qm,
          harq_process->nb_rb,
          harq_process->Nl,
          harq_process->rvidx,
          harq_process->round);
#endif

    if( harq_pid == 4)
    {
        print_info("lte_rate_matching_turbo_rx_ue: frame=%d subframe=%d harq=%d, round=%d rvidx=%d mcs=%d q_m=%d Nl=%d clear=%d\n", 
            frame, subframe, harq_pid, harq_process->round, harq_process->rvidx, 
            harq_process->mcs, harq_process->Qm,harq_process->Nl, (harq_process->round==0)?1:0);
    }

    if (lte_rate_matching_turbo_rx_ue(harq_process->RTC[r],
                                   G,
                                   harq_process->w[r],
                                   (uint8_t*)&dummy_w[r][0],
                                   dlsch_llr+r_offset,
                                   harq_process->C,
                                   NSOFT,
                                   dlsch->Mdlharq,
                                   dlsch->Kmimo,
                                   harq_process->rvidx,
                                   (harq_process->round==0)?1:0,
                                   harq_process->Qm,
                                   harq_process->Nl,
                                   r,
                                   &E)==-1) {
      stop_meas(dlsch_rate_unmatching_stats);
      LOG_E(PHY,"dlsch_decoding.c: Problem in rate_matching\n");
      return(dlsch->max_turbo_iterations);
    } else
      stop_meas(dlsch_rate_unmatching_stats);

    r_offset += E;

    /*
    printf("Subblock deinterleaving, d %p w %p\n",
     harq_process->d[r],
     harq_process->w);
    */
    start_meas(dlsch_deinterleaving_stats);
    sub_block_deinterleaving_turbo(4+Kr,
                                   &harq_process->d[r][96],

                                   harq_process->w[r]);
    stop_meas(dlsch_deinterleaving_stats);

#ifdef DEBUG_DLSCH_DECODING
    /*
    if (r==0) {
              write_output("decoder_llr.m","decllr",dlsch_llr,G,1,0);
              write_output("decoder_in.m","dec",&harq_process->d[0][96],(3*8*Kr_bytes)+12,1,0);
    }

    printf("decoder input(segment %d) :",r);
    int i; for (i=0;i<(3*8*Kr_bytes)+12;i++)
      printf("%d : %d\n",i,harq_process->d[r][96+i]);
      printf("\n");*/
#endif


    //    printf("Clearing c, %p\n",harq_process->c[r]);
    memset(harq_process->c[r],0,Kr_bytes);

    //    printf("done\n");
    if (harq_process->C == 1)
      crc_type = CRC24_A;
    else
      crc_type = CRC24_B;

    /*
    printf("decoder input(segment %d)\n",r);
    for (i=0;i<(3*8*Kr_bytes)+12;i++)
      if ((harq_process->d[r][96+i]>7) ||
    (harq_process->d[r][96+i] < -8))
    printf("%d : %d\n",i,harq_process->d[r][96+i]);
    printf("\n");
    */

    if (err_flag == 0) {

      start_meas(dlsch_turbo_decoding_stats);
#ifdef TURBO_S
      ret = phy_threegpplte_turbo_decoder_scalar
#else

      ret = tc
#endif
            (&harq_process->d[r][96],
             harq_process->c[r],
             Kr,
             f1f2mat_old[iind*2],
             f1f2mat_old[(iind*2)+1],
             dlsch->max_turbo_iterations,
             crc_type,
             (r==0) ? harq_process->F : 0,
             &phy_vars_ue->dlsch_tc_init_stats,
             &phy_vars_ue->dlsch_tc_alpha_stats,
             &phy_vars_ue->dlsch_tc_beta_stats,
             &phy_vars_ue->dlsch_tc_gamma_stats,
             &phy_vars_ue->dlsch_tc_ext_stats,
             &phy_vars_ue->dlsch_tc_intl1_stats,
             &phy_vars_ue->dlsch_tc_intl2_stats); //(is_crnti==0)?harq_pid:harq_pid+1);


      stop_meas(dlsch_turbo_decoding_stats);
    }



    if ((err_flag == 0) && (ret>=(1+dlsch->max_turbo_iterations))) {// a Code segment is in error so break;
      //printf("CRC failed, segment %d\n",r);
      err_flag = 1;
    }

  }

  if (err_flag == 1) {
    dlsch->harq_ack[subframe].ack = 0;
    dlsch->harq_ack[subframe].harq_id = harq_pid;
    dlsch->harq_ack[subframe].send_harq_status = 1;
    harq_process->round++;

    //    LOG_D(PHY,"[UE %d] DLSCH: Setting NACK for subframe %d (pid %d, round %d)\n",phy_vars_ue->Mod_id,subframe,harq_pid,harq_process->round);
    //    printf("Rate: [UE %d] DLSCH: Setting NACK for subframe %d (pid %d, round %d)\n",phy_vars_ue->Mod_id,subframe,harq_pid,harq_process->round);
    if (harq_process->round >= dlsch->Mdlharq) {
      harq_process->status = SCH_IDLE;
    }
    setUlAckInfo(frame,subframe,phy_vars_ue);

    return((1+dlsch->max_turbo_iterations));
  } else {
    harq_process->status = SCH_IDLE;
    harq_process->round  = 0;
    dlsch->harq_ack[subframe].ack = 1;
    dlsch->harq_ack[subframe].harq_id = harq_pid;
    dlsch->harq_ack[subframe].send_harq_status = 1;
    LOG_D(PHY,"[UE %d] DLSCH: Setting ACK for subframe %d (pid %d, round %d)\n",phy_vars_ue->Mod_id,subframe,harq_pid,harq_process->round);
    setUlAckInfo(frame,subframe,phy_vars_ue);

  }

  // Reassembly of Transport block here
  offset = 0;

  /*
  printf("harq_pid %d\n",harq_pid);
  printf("F %d, Fbytes %d\n",harq_process->F,harq_process->F>>3);
  printf("C %d\n",harq_process->C);
  */
  for (r=0; r<harq_process->C; r++) {
    if (r<harq_process->Cminus)
      Kr = harq_process->Kminus;
    else
      Kr = harq_process->Kplus;

    Kr_bytes = Kr>>3;

    //    printf("Segment %d : Kr= %d bytes\n",r,Kr_bytes);
    if (r==0) {
      memcpy(harq_process->b,
             &harq_process->c[0][(harq_process->F>>3)],
             Kr_bytes - (harq_process->F>>3)- ((harq_process->C>1)?3:0));
      offset = Kr_bytes - (harq_process->F>>3) - ((harq_process->C>1)?3:0);
      //            printf("copied %d bytes to b sequence (harq_pid %d)\n",
      //          Kr_bytes - (harq_process->F>>3),harq_pid);
      //          printf("b[0] = %x,c[%d] = %x\n",
      //      harq_process->b[0],
      //      harq_process->F>>3,
      //      harq_process->c[0][(harq_process->F>>3)]);
    } else {
      memcpy(harq_process->b+offset,
             harq_process->c[r],
             Kr_bytes- ((harq_process->C>1)?3:0));
      offset += (Kr_bytes - ((harq_process->C>1)?3:0));
    }
  }

  return(ret);
}

#ifdef PHY_ABSTRACTION
#include "SIMULATION/TOOLS/defs.h"
#ifdef OPENAIR2
#include "LAYER2/MAC/extern.h"
#include "LAYER2/MAC/defs.h"
#endif

int dlsch_abstraction_EESM(double* sinr_dB, uint8_t TM, uint32_t rb_alloc[4], uint8_t mcs, uint8_t dl_power_off)
{

  int ii;
  double sinr_eff = 0;
  int rb_count = 0;
  int offset;
  double bler = 0;

  if(TM==5 && dl_power_off==1) {
    //do nothing -- means there is no second UE and TM 5 is behaving like TM 6 for a singal user
  } else
    TM = TM-1;

  for (offset = 0; offset <= 24; offset++) {
    if (rb_alloc[0] & (1<<offset)) {
      rb_count++;

      for(ii=0; ii<12; ii++) {
        sinr_eff += exp(-(pow(10, 0.1*(sinr_dB[(offset*12)+ii])))/beta1_dlsch[TM][mcs]);
        //printf("sinr_eff1 = %f, power %lf\n",sinr_eff, exp(-pow(10,6.8)));

        //  sinr_eff += exp(-(pow(10, (sinr_dB[offset*2+1])/10))/beta1_dlsch[TM][mcs]);
        //printf("sinr_dB[%d]=%f\n",offset,sinr_dB[offset*2]);
      }
    }
  }

  LOG_D(OCM,"sinr_eff (lin, unweighted) = %f\n",sinr_eff);
  sinr_eff =  -beta2_dlsch[TM][mcs]*log((sinr_eff)/(12*rb_count));
  LOG_D(OCM,"sinr_eff (lin, weighted) = %f\n",sinr_eff);
  sinr_eff = 10 * log10(sinr_eff);
  LOG_D(OCM,"sinr_eff (dB) = %f\n",sinr_eff);

  bler = interp(sinr_eff,&sinr_bler_map[mcs][0][0],&sinr_bler_map[mcs][1][0],table_length[mcs]);

#ifdef USER_MODE // need to be adapted for the emulation in the kernel space 

  if (uniformrandom() < bler) {
    LOG_I(OCM,"abstraction_decoding failed (mcs=%d, sinr_eff=%f, bler=%f, TM %d)\n",mcs,sinr_eff,bler, TM);
    return(1);
  } else {
    LOG_I(OCM,"abstraction_decoding successful (mcs=%d, sinr_eff=%f, bler=%f, TM %d)\n",mcs,sinr_eff,bler, TM);
    return(1);
  }

#endif
}

int dlsch_abstraction_MIESM(double* sinr_dB,uint8_t TM, uint32_t rb_alloc[4], uint8_t mcs,uint8_t dl_power_off)
{
  int ii;
  double sinr_eff = 0;
  double x = 0;
  double I =0;
  double qpsk_max=12.2;
  double qam16_max=19.2;
  double qam64_max=25.2;
  double sinr_min = -20;
  int rb_count = 0;
  int offset=0;
  double bler = 0;

  if(TM==5 && dl_power_off==1) {
    //do nothing -- means there is no second UE and TM 5 is behaving like TM 6 for a singal user
  } else
    TM = TM-1;


  for (offset = 0; offset <= 24; offset++) {
    if (rb_alloc[0] & (1<<offset)) {
      rb_count++;

      for(ii=0; ii<12; ii++) {
        //x is the sinr_dB in dB
        x = sinr_dB[(offset*12)+ii] - 10*log10(beta1_dlsch_MI[TM][mcs]);

        if(x<sinr_min)
          I +=0;
        else {
          if(mcs<10) {
            if(x>qpsk_max)
              I += 1;
            else
              I += (q_qpsk[0]*pow(x,7) + q_qpsk[1]*pow(x,6) + q_qpsk[2]*pow(x,5) + q_qpsk[3]*pow(x,4) + q_qpsk[4]*pow(x,3) + q_qpsk[5]*pow(x,2) + q_qpsk[6]*x + q_qpsk[7]);
          } else if(mcs>9 && mcs<17) {
            if(x>qam16_max)
              I += 1;
            else
              I += (q_qam16[0]*pow(x,7) + q_qam16[1]*pow(x,6) + q_qam16[2]*pow(x,5) + q_qam16[3]*pow(x,4) + q_qam16[4]*pow(x,3) + q_qam16[5]*pow(x,2) + q_qam16[6]*x + q_qam16[7]);
          } else if(mcs>16 && mcs<23) {

            if(x>qam64_max)
              I += 1;
            else
              I += (q_qam64[0]*pow(x,7) + q_qam64[1]*pow(x,6) + q_qam64[2]*pow(x,5) + q_qam64[3]*pow(x,4) + q_qam64[4]*pow(x,3) + q_qam64[5]*pow(x,2) + q_qam64[6]*x + q_qam64[7]);
          }
        }
      }
    }
  }

  // averaging of accumulated MI
  I = I/(12*rb_count);
  //Now  I->SINR_effective Mapping

  if(mcs<10) {
    sinr_eff = (p_qpsk[0]*pow(I,7) + p_qpsk[1]*pow(I,6) + p_qpsk[2]*pow(I,5) + p_qpsk[3]*pow(I,4) + p_qpsk[4]*pow(I,3) + p_qpsk[5]*pow(I,2) + p_qpsk[6]*I + p_qpsk[7]);
  } else if(mcs>9 && mcs<17) {
    sinr_eff = (p_qam16[0]*pow(I,7) + p_qam16[1]*pow(I,6) + p_qam16[2]*pow(I,5) + p_qam16[3]*pow(I,4) + p_qam16[4]*pow(I,3) + p_qam16[5]*pow(I,2) + p_qam16[6]*I + p_qam16[7]);
  } else if(mcs>16 && mcs<23) {
    sinr_eff = (p_qam64[0]*pow(I,7) + p_qam64[1]*pow(I,6) + p_qam64[2]*pow(I,5) + p_qam64[3]*pow(I,4) + p_qam64[4]*pow(I,3) + p_qam64[5]*pow(I,2) + p_qam64[6]*I + p_qam64[7]);
  }

  //sinr_eff = sinr_eff + 10*log10(beta2_dlsch_MI[TM][mcs]);
  LOG_D(OCM,"SINR_Eff = %e\n",sinr_eff);

  bler = interp(sinr_eff,&sinr_bler_map[mcs][0][0],&sinr_bler_map[mcs][1][0],table_length[mcs]);

#ifdef USER_MODE // need to be adapted for the emulation in the kernel space 

  if (uniformrandom() < bler) {
    LOG_N(OCM,"abstraction_decoding failed (mcs=%d, sinr_eff=%f, bler=%f)\n",mcs,sinr_eff,bler);
    return(0);
  } else {
    LOG_I(OCM,"abstraction_decoding successful (mcs=%d, sinr_eff=%f, bler=%f)\n",mcs,sinr_eff,bler);
    return(1);
  }

#endif
}

uint32_t dlsch_decoding_emul(PHY_VARS_UE *phy_vars_ue,
                             uint8_t subframe,
                             uint8_t dlsch_id,
                             uint8_t eNB_id)
{

  LTE_UE_DLSCH_t *dlsch_ue;
  LTE_eNB_DLSCH_t *dlsch_eNB;
  uint8_t harq_pid;
  uint32_t eNB_id2;
  uint32_t ue_id;
#ifdef DEBUG_DLSCH_DECODING
  uint16_t i;
#endif
  uint8_t CC_id = phy_vars_ue->CC_id;

  // may not be necessary for PMCH??
  for (eNB_id2=0; eNB_id2<NB_eNB_INST; eNB_id2++) {
    if (PHY_vars_eNB_g[eNB_id2][CC_id]->lte_frame_parms.Nid_cell == phy_vars_ue->lte_frame_parms.Nid_cell)
      break;
  }

  if (eNB_id2==NB_eNB_INST) {
    LOG_E(PHY,"FATAL : Could not find attached eNB for DLSCH emulation !!!!\n");
    mac_xface->macphy_exit("Could not find attached eNB for DLSCH emulation");
  }

  LOG_D(PHY,"[UE] dlsch_decoding_emul : subframe %d, eNB_id %d, dlsch_id %d\n",subframe,eNB_id2,dlsch_id);

  //  printf("dlsch_eNB_ra->harq_processes[0] %p\n",PHY_vars_eNB_g[eNB_id]->dlsch_eNB_ra->harq_processes[0]);


  switch (dlsch_id) {
  case 0: // SI
    dlsch_ue = phy_vars_ue->dlsch_ue_SI[eNB_id];
    dlsch_eNB = PHY_vars_eNB_g[eNB_id2][CC_id]->dlsch_eNB_SI;
    //    printf("Doing SI: TBS %d\n",dlsch_ue->harq_processes[0]->TBS>>3);
    memcpy(dlsch_ue->harq_processes[0]->b,dlsch_eNB->harq_processes[0]->b,dlsch_ue->harq_processes[0]->TBS>>3);
#ifdef DEBUG_DLSCH_DECODING
    LOG_D(PHY,"SI Decoded\n");

    for (i=0; i<dlsch_ue->harq_processes[0]->TBS>>3; i++)
      LOG_T(PHY,"%x.",dlsch_eNB->harq_processes[0]->b[i]);

    LOG_T(PHY,"\n");
#endif
    return(1);
    break;

  case 1: // RA
    dlsch_ue  = phy_vars_ue->dlsch_ue_ra[eNB_id];
    dlsch_eNB = PHY_vars_eNB_g[eNB_id2][CC_id]->dlsch_eNB_ra;
    memcpy(dlsch_ue->harq_processes[0]->b,dlsch_eNB->harq_processes[0]->b,dlsch_ue->harq_processes[0]->TBS>>3);
#ifdef DEBUG_DLSCH_DECODING
    LOG_D(PHY,"RA Decoded\n");

    for (i=0; i<dlsch_ue->harq_processes[0]->TBS>>3; i++)
      LOG_T(PHY,"%x.",dlsch_eNB->harq_processes[0]->b[i]);

    LOG_T(PHY,"\n");
#endif
    return(1);
    break;

  case 2: // TB0
    dlsch_ue  = phy_vars_ue->dlsch_ue[eNB_id][0];
    harq_pid = dlsch_ue->current_harq_pid[subframe];
    ue_id= (uint32_t)find_ue((int16_t)phy_vars_ue->lte_ue_pdcch_vars[(uint32_t)eNB_id]->crnti,PHY_vars_eNB_g[eNB_id2][CC_id]);
    DevAssert( ue_id != (uint32_t)-1 );
    dlsch_eNB = PHY_vars_eNB_g[eNB_id2][CC_id]->dlsch_eNB[ue_id][0];

#ifdef DEBUG_DLSCH_DECODING

    for (i=0; i<dlsch_ue->harq_processes[harq_pid]->TBS>>3; i++)
      LOG_T(PHY,"%x.",dlsch_eNB->harq_processes[harq_pid]->b[i]);

    LOG_T(PHY,"\n current harq pid is %d ue id %d \n", harq_pid, ue_id);
#endif

    if (dlsch_abstraction_MIESM(phy_vars_ue->sinr_dB,
                                phy_vars_ue->transmission_mode[eNB_id],
                                dlsch_eNB->harq_processes[harq_pid]->rb_alloc,
                                dlsch_eNB->harq_processes[harq_pid]->mcs,
                                PHY_vars_eNB_g[eNB_id][CC_id]->mu_mimo_mode[ue_id].dl_pow_off) == 1) {
      // reset HARQ
      dlsch_ue->harq_processes[harq_pid]->status = SCH_IDLE;
      dlsch_ue->harq_processes[harq_pid]->round  = 0;
      dlsch_ue->harq_ack[subframe].ack = 1;
      dlsch_ue->harq_ack[subframe].harq_id = harq_pid;
      dlsch_ue->harq_ack[subframe].send_harq_status = 1;

      if (dlsch_ue->harq_processes[harq_pid]->round == 0)
        memcpy(dlsch_ue->harq_processes[harq_pid]->b,
               dlsch_eNB->harq_processes[harq_pid]->b,
               dlsch_ue->harq_processes[harq_pid]->TBS>>3);

      return(1);
    } else {
      // retransmission
      dlsch_ue->harq_processes[harq_pid]->status = ACTIVE;
      dlsch_ue->harq_processes[harq_pid]->round++;
      dlsch_ue->harq_ack[subframe].ack = 0;
      dlsch_ue->harq_ack[subframe].harq_id = harq_pid;
      dlsch_ue->harq_ack[subframe].send_harq_status = 1;
      return(1+dlsch_ue->max_turbo_iterations);
    }

    break;

  case 3: { // TB1
    dlsch_ue = phy_vars_ue->dlsch_ue[eNB_id][1];
    harq_pid = dlsch_ue->current_harq_pid[subframe];
    int8_t UE_id = find_ue( phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti, PHY_vars_eNB_g[eNB_id2][CC_id] );
    DevAssert( UE_id != -1 );
    dlsch_eNB = PHY_vars_eNB_g[eNB_id2][CC_id]->dlsch_eNB[UE_id][1];
    // reset HARQ
    dlsch_ue->harq_processes[harq_pid]->status = SCH_IDLE;
    dlsch_ue->harq_processes[harq_pid]->round  = 0;
    dlsch_ue->harq_ack[subframe].ack = 1;
    dlsch_ue->harq_ack[subframe].harq_id = harq_pid;
    dlsch_ue->harq_ack[subframe].send_harq_status = 1;

    if (dlsch_ue->harq_processes[harq_pid]->round == 0)
      memcpy(dlsch_eNB->harq_processes[harq_pid]->b,dlsch_ue->harq_processes[harq_pid]->b,dlsch_ue->harq_processes[harq_pid]->TBS>>3);

    break;
  }

  case 5: // PMCH

    dlsch_ue  = phy_vars_ue->dlsch_ue_MCH[eNB_id];
    dlsch_eNB = PHY_vars_eNB_g[eNB_id2][CC_id]->dlsch_eNB_MCH;

    LOG_D(PHY,"decoding pmch emul (size is %d, enb %d %d)\n",  dlsch_ue->harq_processes[0]->TBS>>3, eNB_id, eNB_id2);
#ifdef DEBUG_DLSCH_DECODING

    for (i=0; i<dlsch_ue->harq_processes[0]->TBS>>3; i++)
      printf("%x.",dlsch_eNB->harq_processes[0]->b[i]);

    printf("\n");
#endif

    /*
      if (dlsch_abstraction_MIESM(phy_vars_ue->sinr_dB, phy_vars_ue->transmission_mode[eNB_id], dlsch_eNB->rb_alloc,
        dlsch_eNB->harq_processes[0]->mcs,PHY_vars_eNB_g[eNB_id]->mu_mimo_mode[ue_id].dl_pow_off) == 1) {
    */
    if (1) {
      // reset HARQ
      dlsch_ue->harq_processes[0]->status = SCH_IDLE;
      dlsch_ue->harq_processes[0]->round  = 0;
      memcpy(dlsch_ue->harq_processes[0]->b,
             dlsch_eNB->harq_processes[0]->b,
             dlsch_ue->harq_processes[0]->TBS>>3);
      return(1);
    } else {
      // retransmission
      return(1+dlsch_ue->max_turbo_iterations);
    }

    break;

  default:
    dlsch_ue = phy_vars_ue->dlsch_ue[eNB_id][0];
    LOG_E(PHY,"dlsch_decoding_emul: FATAL, unknown DLSCH_id %d\n",dlsch_id);
    return(1+dlsch_ue->max_turbo_iterations);
  }

  LOG_E(PHY,"[FATAL] dlsch_decoding.c: Should never exit here ...\n");
  return(0);
}
#endif
