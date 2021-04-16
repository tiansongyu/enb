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

/*! \file phy_procedures_lte_ue.c
 * \brief Implementation of UE procedures from 36.213 LTE specifications
 * \author R. Knopp, F. Kaltenberger, N. Nikaein
 * \date 2011
 * \version 0.1
 * \company Eurecom
 * \email: knopp@eurecom.fr,florian.kaltenberger@eurecom.fr, navid.nikaein@eurecom.fr
 * \note
 * \warning
 */

#include "assertions.h"
#include "defs.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "MAC_INTERFACE/defs.h"
#include "MAC_INTERFACE/extern.h"
#include "SCHED/defs.h"
#include "SCHED/extern.h"

#ifdef EMOS
#include "SCHED/phy_procedures_emos.h"
#endif

#ifdef EXMIMO
#ifdef DRIVER2013
#include "openair0_lib.h"
#include "gain_control.h"
extern int card;
#endif
#endif

//#define DEBUG_PHY_PROC
#define UE_TX_POWER (-10)

//#ifdef OPENAIR2
#ifndef PUCCH
#define PUCCH
#endif
//#endif

//#ifdef OPENAIR2
#include "LAYER2/MAC/extern.h"
#include "LAYER2/MAC/defs.h"
#include "UTIL/LOG/log.h"
//#endif

#ifdef EMOS
fifo_dump_emos_UE emos_dump_UE;
#endif

#include "UTIL/LOG/vcd_signal_dumper.h"

#if defined(ENABLE_ITTI)
# include "intertask_interface.h"
#   if ENABLE_RAL
#     include "timer.h"
#   endif
#endif

#ifndef OPENAIR2
//#define DIAG_PHY
#endif

#define DLSCH_RB_ALLOC 0x1fbf  // skip DC RB (total 23/25 RBs)
#define DLSCH_RB_ALLOC_12 0x0aaa  // skip DC RB (total 23/25 RBs)

#define NS_PER_SLOT 500000

extern int oai_exit;

uint8_t ulsch_input_buffer[8][3072] __attribute__ ((aligned(16)));
uint8_t access_mode;

#ifdef DLSCH_THREAD
extern int dlsch_instance_cnt[8];
extern int dlsch_subframe[8];
extern pthread_mutex_t dlsch_mutex[8];
/// Condition variable for dlsch thread
extern pthread_cond_t dlsch_cond[8];
extern int rx_pdsch_instance_cnt;
extern int rx_pdsch_slot;
extern pthread_mutex_t rx_pdsch_mutex;
/// Condition variable for rx_pdsch thread
extern pthread_cond_t rx_pdsch_cond;
#endif

//DCI_ALLOC_t dci_alloc_rx[8];

#ifdef DIAG_PHY
extern int rx_sig_fifo;
#endif


//#if defined(EXMIMO) || defined(OAI_USRP)
extern uint32_t downlink_frequency[MAX_NUM_CCs][4];
//#endif

#ifdef USER_MODE

void dump_dlsch(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint8_t subframe,uint8_t harq_pid)
{
  unsigned int coded_bits_per_codeword;
  uint8_t nsymb = (phy_vars_ue->lte_frame_parms.Ncp == 0) ? 14 : 12;

  coded_bits_per_codeword = get_G(&phy_vars_ue->lte_frame_parms,
                                  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->nb_rb,
                                  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rb_alloc_even,
                                  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->Qm,
                                  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->Nl,
                                  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
                                  phy_vars_ue->frame_rx,subframe);

  write_output("rxsigF0.m","rxsF0", phy_vars_ue->lte_ue_common_vars.rxdataF[0],2*nsymb*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,2,1);
  write_output("rxsigF0_ext.m","rxsF0_ext", phy_vars_ue->lte_ue_pdsch_vars[0]->rxdataF_ext[0],2*nsymb*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,1,1);
  write_output("dlsch00_ch0_ext.m","dl00_ch0_ext", phy_vars_ue->lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[0],300*nsymb,1,1);
  /*
    write_output("dlsch01_ch0_ext.m","dl01_ch0_ext",lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[1],300*12,1,1);
    write_output("dlsch10_ch0_ext.m","dl10_ch0_ext",lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[2],300*12,1,1);
    write_output("dlsch11_ch0_ext.m","dl11_ch0_ext",lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[3],300*12,1,1);
    write_output("dlsch_rho.m","dl_rho",lte_ue_pdsch_vars[0]->rho[0],300*12,1,1);
  */
  write_output("dlsch_rxF_comp0.m","dlsch0_rxF_comp0", phy_vars_ue->lte_ue_pdsch_vars[0]->rxdataF_comp0[0],300*12,1,1);
  write_output("dlsch_rxF_llr.m","dlsch_llr", phy_vars_ue->lte_ue_pdsch_vars[0]->llr[0],coded_bits_per_codeword,1,0);

  write_output("dlsch_mag1.m","dlschmag1",phy_vars_ue->lte_ue_pdsch_vars[0]->dl_ch_mag0,300*12,1,1);
  write_output("dlsch_mag2.m","dlschmag2",phy_vars_ue->lte_ue_pdsch_vars[0]->dl_ch_magb0,300*12,1,1);
}

void dump_dlsch_SI(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint8_t subframe)
{
  unsigned int coded_bits_per_codeword;
  uint8_t nsymb = ((phy_vars_ue->lte_frame_parms.Ncp == 0) ? 14 : 12);

  coded_bits_per_codeword = get_G(&phy_vars_ue->lte_frame_parms,
                                  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->nb_rb,
                                  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even,
                                  2,
                                  1,
                                  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
                                  phy_vars_ue->frame_rx,subframe);
  LOG_I(PHY,"[UE %d] Dumping dlsch_SI : ofdm_symbol_size %d, nsymb %d, nb_rb %d, mcs %d, nb_rb %d, num_pdcch_symbols %d,G %d\n",
        phy_vars_ue->Mod_id,
	phy_vars_ue->lte_frame_parms.ofdm_symbol_size,
	nsymb,
        phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->nb_rb,
        phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->mcs,
        phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->nb_rb,
        phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
        coded_bits_per_codeword);

  write_output("rxsig0.m","rxs0", &phy_vars_ue->lte_ue_common_vars.rxdata[0][subframe*phy_vars_ue->lte_frame_parms.samples_per_tti],phy_vars_ue->lte_frame_parms.samples_per_tti,1,1);

  write_output("rxsigF0.m","rxsF0", phy_vars_ue->lte_ue_common_vars.rxdataF[0],nsymb*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,1,1);
  write_output("rxsigF0_ext.m","rxsF0_ext", phy_vars_ue->lte_ue_pdsch_vars_SI[0]->rxdataF_ext[0],2*nsymb*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,1,1);
  write_output("dlsch00_ch0_ext.m","dl00_ch0_ext", phy_vars_ue->lte_ue_pdsch_vars_SI[0]->dl_ch_estimates_ext[0],phy_vars_ue->lte_frame_parms.N_RB_DL*12*nsymb,1,1);
  /*
    write_output("dlsch01_ch0_ext.m","dl01_ch0_ext",lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[1],300*12,1,1);
    write_output("dlsch10_ch0_ext.m","dl10_ch0_ext",lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[2],300*12,1,1);
    write_output("dlsch11_ch0_ext.m","dl11_ch0_ext",lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[3],300*12,1,1);
    write_output("dlsch_rho.m","dl_rho",lte_ue_pdsch_vars[0]->rho[0],300*12,1,1);
  */
  write_output("dlsch_rxF_comp0.m","dlsch0_rxF_comp0", phy_vars_ue->lte_ue_pdsch_vars_SI[0]->rxdataF_comp0[0],phy_vars_ue->lte_frame_parms.N_RB_DL*12*nsymb,1,1);
  write_output("dlsch_rxF_llr.m","dlsch_llr", phy_vars_ue->lte_ue_pdsch_vars_SI[0]->llr[0],coded_bits_per_codeword,1,0);

  write_output("dlsch_mag1.m","dlschmag1",phy_vars_ue->lte_ue_pdsch_vars_SI[0]->dl_ch_mag0,300*nsymb,1,1);
  write_output("dlsch_mag2.m","dlschmag2",phy_vars_ue->lte_ue_pdsch_vars_SI[0]->dl_ch_magb0,300*nsymb,1,1);
  exit(-1);
}

//#if defined(EXMIMO) || defined(OAI_USRP)
//unsigned int gain_table[31] = {100,112,126,141,158,178,200,224,251,282,316,359,398,447,501,562,631,708,794,891,1000,1122,1258,1412,1585,1778,1995,2239,2512,2818,3162};
/*
unsigned int get_tx_amp_prach(int power_dBm, int power_max_dBm, int N_RB_UL)
{

  int gain_dB = power_dBm - power_max_dBm;
  int amp_x_100;

  switch (N_RB_UL) {
  case 6:
    amp_x_100 = AMP;      // PRACH is 6 PRBS so no scale
    break;
  case 15:
    amp_x_100 = 158*AMP;  // 158 = 100*sqrt(15/6)
    break;
  case 25:
    amp_x_100 = 204*AMP;  // 204 = 100*sqrt(25/6)
    break;
  case 50:
    amp_x_100 = 286*AMP;  // 286 = 100*sqrt(50/6)
    break;
  case 75:
    amp_x_100 = 354*AMP;  // 354 = 100*sqrt(75/6)
    break;
  case 100:
    amp_x_100 = 408*AMP;  // 408 = 100*sqrt(100/6)
    break;
  default:
    LOG_E(PHY,"Unknown PRB size %d\n",N_RB_UL);
    mac_xface->macphy_exit("");
    break;
  }
  if (gain_dB < -30) {
    return(amp_x_100/3162);
  } else if (gain_dB>0)
    return(amp_x_100);
  else
    return(amp_x_100/gain_table[-gain_dB]);  // 245 corresponds to the factor sqrt(25/6)
}
*/

unsigned int get_tx_amp(int power_dBm, int power_max_dBm, int N_RB_UL, int nb_rb)
{

  int gain_dB = power_dBm - power_max_dBm;
  double gain_lin;

  if (gain_dB < -30) //-20  xhd_oai_debug
    return(AMP/10);

  if( gain_dB > 0)//xhd_oai_debug
  {
    gain_dB = 0;
  }
  gain_lin = pow(10,.1*gain_dB);
  if ((nb_rb >0) && (nb_rb <= N_RB_UL)) {
    return((int)(AMP*sqrt(gain_lin*N_RB_UL/(double)nb_rb)));
  }
  else {
    LOG_E(PHY,"Illegal nb_rb/N_RB_UL combination (%d/%d)\n",nb_rb,N_RB_UL);
    //mac_xface->macphy_exit("");
    return(AMP/10);
  }
  return(0);
}

//#endif

void dump_dlsch_ra(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint8_t subframe)
{
  unsigned int coded_bits_per_codeword;
  uint8_t nsymb = ((phy_vars_ue->lte_frame_parms.Ncp == 0) ? 14 : 12);

  coded_bits_per_codeword = get_G(&phy_vars_ue->lte_frame_parms,
                                  phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->nb_rb,
                                  phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->rb_alloc_even,
                                  2,
                                  1,
                                  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
                                  phy_vars_ue->frame_rx,subframe);
  LOG_D(PHY,"[UE %d] Dumping dlsch_ra : nb_rb %d, mcs %d, nb_rb %d, num_pdcch_symbols %d,G %d\n",
        phy_vars_ue->Mod_id,
        phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->nb_rb,
        phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->mcs,
        phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->nb_rb,
        phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
        coded_bits_per_codeword);

  write_output("rxsigF0.m","rxsF0", phy_vars_ue->lte_ue_common_vars.rxdataF[0],2*12*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,2,1);
  write_output("rxsigF0_ext.m","rxsF0_ext", phy_vars_ue->lte_ue_pdsch_vars_ra[0]->rxdataF_ext[0],2*12*phy_vars_ue->lte_frame_parms.ofdm_symbol_size,1,1);
  write_output("dlsch00_ch0_ext.m","dl00_ch0_ext", phy_vars_ue->lte_ue_pdsch_vars_ra[0]->dl_ch_estimates_ext[0],300*nsymb,1,1);
  /*
    write_output("dlsch01_ch0_ext.m","dl01_ch0_ext",lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[1],300*12,1,1);
    write_output("dlsch10_ch0_ext.m","dl10_ch0_ext",lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[2],300*12,1,1);
    write_output("dlsch11_ch0_ext.m","dl11_ch0_ext",lte_ue_pdsch_vars[0]->dl_ch_estimates_ext[3],300*12,1,1);
    write_output("dlsch_rho.m","dl_rho",lte_ue_pdsch_vars[0]->rho[0],300*12,1,1);
  */
  write_output("dlsch_rxF_comp0.m","dlsch0_rxF_comp0", phy_vars_ue->lte_ue_pdsch_vars_ra[0]->rxdataF_comp0[0],300*nsymb,1,1);
  write_output("dlsch_rxF_llr.m","dlsch_llr", phy_vars_ue->lte_ue_pdsch_vars_ra[0]->llr[0],coded_bits_per_codeword,1,0);

  write_output("dlsch_mag1.m","dlschmag1",phy_vars_ue->lte_ue_pdsch_vars_ra[0]->dl_ch_mag0,300*nsymb,1,1);
  write_output("dlsch_mag2.m","dlschmag2",phy_vars_ue->lte_ue_pdsch_vars_ra[0]->dl_ch_magb0,300*nsymb,1,1);
}
#endif

void phy_reset_ue(uint8_t Mod_id,uint8_t CC_id,uint8_t eNB_index)
{

  // This flushes ALL DLSCH and ULSCH harq buffers of ALL connected eNBs...add the eNB_index later
  // for more flexibility

  uint8_t i,j,k;
  PHY_VARS_UE *phy_vars_ue = PHY_vars_UE_g[Mod_id][CC_id];

  //[NUMBER_OF_CONNECTED_eNB_MAX][2];
  for(i=0; i<NUMBER_OF_CONNECTED_eNB_MAX; i++) {
    for(j=0; j<2; j++) {
      //DL HARQ
      if(phy_vars_ue->dlsch_ue[i][j]) {
        for(k=0; k<NUMBER_OF_HARQ_PID_MAX && phy_vars_ue->dlsch_ue[i][j]->harq_processes[k]; k++) {
          phy_vars_ue->dlsch_ue[i][j]->harq_processes[k]->status = SCH_IDLE;
        }
      }
    }

    //UL HARQ
    if(phy_vars_ue->ulsch_ue[i]) {
      for(k=0; k<NUMBER_OF_HARQ_PID_MAX && phy_vars_ue->ulsch_ue[i]->harq_processes[k]; k++) {
        phy_vars_ue->ulsch_ue[i]->harq_processes[k]->status = SCH_IDLE;
        //Set NDIs for all UL HARQs to 0
        //  phy_vars_ue->ulsch_ue[i]->harq_processes[k]->Ndi = 0;

      }
    }

    // flush Msg3 buffer
    phy_vars_ue->ulsch_ue_Msg3_active[i] = 0;

  }
}

void ra_failed(uint8_t Mod_id,uint8_t CC_id,uint8_t eNB_index)
{

  // if contention resolution fails, go back to PRACH
  PHY_vars_UE_g[Mod_id][CC_id]->UE_mode[eNB_index] = PRACH;
  LOG_E(PHY,"[UE %d] Frame %d Random-access procedure fails, going back to PRACH, setting SIStatus = 0 and State RRC_IDLE\n",Mod_id,PHY_vars_UE_g[Mod_id][CC_id]->frame_rx);
  //mac_xface->macphy_exit("");
}

void ra_succeeded(uint8_t Mod_id,uint8_t CC_id,uint8_t eNB_index)
{

  int i;

  LOG_I(PHY,"[UE %d][RAPROC] Frame %d Random-access procedure succeeded\n",Mod_id,PHY_vars_UE_g[Mod_id][CC_id]->frame_rx);

  PHY_vars_UE_g[Mod_id][CC_id]->ulsch_ue_Msg3_active[eNB_index] = 0;
  PHY_vars_UE_g[Mod_id][CC_id]->UE_mode[eNB_index] = PUSCH;

  for (i=0; i<8; i++) {
    if (PHY_vars_UE_g[Mod_id][CC_id]->ulsch_ue[eNB_index]->harq_processes[i]) {
      PHY_vars_UE_g[Mod_id][CC_id]->ulsch_ue[eNB_index]->harq_processes[i]->status=IDLE;
      PHY_vars_UE_g[Mod_id][CC_id]->dlsch_ue[eNB_index][0]->harq_processes[i]->round=0;
    }
  }


}

UE_MODE_t get_ue_mode(uint8_t Mod_id,uint8_t CC_id,uint8_t eNB_index)
{

  return(PHY_vars_UE_g[Mod_id][CC_id]->UE_mode[eNB_index]);

}
void process_timing_advance_rar(PHY_VARS_UE *phy_vars_ue,uint16_t timing_advance)
{

  /*
  if ((timing_advance>>10) & 1) //it is negative
    timing_advance = timing_advance - (1<<11);
  */

  if (openair_daq_vars.manual_timing_advance == 0) {
    phy_vars_ue->timing_advance = timing_advance*4;

  }

#ifdef DEBUG_PHY_PROC
  LOG_I(PHY,"[UE %d] Frame %d, received (rar) timing_advance %d, HW timing advance %d\n",phy_vars_ue->Mod_id,phy_vars_ue->frame_rx, phy_vars_ue->timing_advance,openair_daq_vars.timing_advance);
#endif

}

void process_timing_advance(uint8_t Mod_id,uint8_t CC_id,int16_t timing_advance)
{

  //  uint32_t frame = PHY_vars_UE_g[Mod_id]->frame;

  // timing advance has Q1.5 format
  timing_advance = timing_advance - 31;

  if (openair_daq_vars.manual_timing_advance == 0) {
    //if ( (frame % 100) == 0) {
    //if ((timing_advance > 3) || (timing_advance < -3) )
    PHY_vars_UE_g[Mod_id][CC_id]->timing_advance = PHY_vars_UE_g[Mod_id][CC_id]->timing_advance+timing_advance*4; //this is for 25RB only!!!
  }

  LOG_D(PHY,"[UE %d] Got timing advance %d from MAC, new value %d\n",Mod_id, timing_advance, PHY_vars_UE_g[Mod_id][CC_id]->timing_advance);


}

uint8_t is_SR_TXOp(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint8_t subframe)
{

  LOG_D(PHY,"[UE %d][SR %x] Frame %d subframe %d Checking for SR TXOp (sr_ConfigIndex %d)\n",
        phy_vars_ue->Mod_id,phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,phy_vars_ue->frame_tx,subframe,
        phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex);

  if (phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex <= 4) {        // 5 ms SR period
    if ((subframe%5) == phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex)
      return(1);
  } else if (phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex <= 14) { // 10 ms SR period
    if (subframe==(phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex-5))
      return(1);
  } else if (phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex <= 34) { // 20 ms SR period
    if ((10*(phy_vars_ue->frame_tx&1)+subframe) == (phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex-15))
      return(1);
  } else if (phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex <= 74) { // 40 ms SR period
    if ((10*(phy_vars_ue->frame_tx&3)+subframe) == (phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex-35))
      return(1);
  } else if (phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex <= 154) { // 80 ms SR period
    if ((10*(phy_vars_ue->frame_tx&7)+subframe) == (phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex-75))
      return(1);
  }

  return(0);
}

uint16_t get_n1_pucch(PHY_VARS_UE *phy_vars_ue,
                      uint8_t eNB_id,
                      uint8_t subframe,
                      uint8_t *b,
                      uint8_t SR)
{

  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_ue->lte_frame_parms;
  uint8_t nCCE0,nCCE1,harq_ack1,harq_ack0;
  ANFBmode_t bundling_flag;
  uint16_t n1_pucch0=0,n1_pucch1=0;
  int subframe_offset;
  int sf;
  int M;
  // clear this, important for case where n1_pucch selection is not used
  int old_b0=b[0];
  int old_b1=b[1];

  phy_vars_ue->pucch_sel[subframe] = 0;

  if (frame_parms->frame_type == FDD ) { // FDD
    sf = (subframe<4)? subframe+6 : subframe-4;
    my_printf("n1_pucch_UE: subframe %d, nCCE %d\n",sf,phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->nCCE[sf]);

    if (SR == 0)
      return(frame_parms->pucch_config_common.n1PUCCH_AN + phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->nCCE[sf]);
    else
      return(phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex);
  } else {

    bundling_flag = phy_vars_ue->pucch_config_dedicated[eNB_id].tdd_AckNackFeedbackMode;
#ifdef DEBUG_PHY_PROC

    if (bundling_flag==bundling) {
      LOG_D(PHY,"[UE%d] Frame %d subframe %d : get_n1_pucch, bundling, SR %d/%d\n",phy_vars_ue->Mod_id,phy_vars_ue->frame_tx,subframe,SR,
            phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex);
    } else {
      LOG_D(PHY,"[UE%d] Frame %d subframe %d : get_n1_pucch, multiplexing, SR %d/%d\n",phy_vars_ue->Mod_id,phy_vars_ue->frame_tx,subframe,SR,
            phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex);
    }

#endif

    switch (frame_parms->tdd_config) {
    case 1:  // DL:S:UL:UL:DL:DL:S:UL:UL:DL

      harq_ack0 = 2; // DTX
      M=1;

      // This is the offset for a particular subframe (2,3,4) => (0,2,4)
      if (subframe == 2) {  // ACK subframes 5 (forget 6)
        subframe_offset = 5;
        M=2;
      } else if (subframe == 3) { // ACK subframe 9
        subframe_offset = 9;
      } else if (subframe == 7) { // ACK subframes 0 (forget 1)
        subframe_offset = 0;
        M=2;
      } else if (subframe == 8) { // ACK subframes 4
        subframe_offset = 4;
      } else {
        LOG_E(PHY,"[UE%d] : Frame %d phy_procedures_lte.c: get_n1pucch, illegal subframe %d for tdd_config %d\n",
              phy_vars_ue->Mod_id,phy_vars_ue->frame_tx,subframe,frame_parms->tdd_config);
        return(0);
      }


      // i=0
      nCCE0 = phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->nCCE[subframe_offset];
      n1_pucch0 = get_Np(frame_parms->N_RB_DL,nCCE0,0) + nCCE0+ frame_parms->pucch_config_common.n1PUCCH_AN;

      // set ACK/NAK to values if not DTX
      if (phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack[subframe_offset].send_harq_status>0)  // n-6 // subframe 5 is to be ACK/NAKed
        harq_ack0 = phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack[subframe_offset].ack;


      if (harq_ack0!=2) {  // DTX
        if (SR == 0) {  // last paragraph pg 68 from 36.213 (v8.6), m=0
          b[0]=(M==2) ? 1-harq_ack0 : harq_ack0;
          b[1]=harq_ack0;   // in case we use pucch format 1b (subframes 2,7)
          phy_vars_ue->pucch_sel[subframe] = 0;
          return(n1_pucch0);
        } else { // SR and only 0 or 1 ACKs (first 2 entries in Table 7.3-1 of 36.213)
          b[0]=harq_ack0;
          return(phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex);
        }
      }


      break;

    case 3:  
      // DL:S:UL:UL:UL:DL:DL:DL:DL:DL
      // in this configuration we have M=2 from pg 68 of 36.213 (v8.6)
      // Note: this doesn't allow using subframe 1 for PDSCH transmission!!! (i.e. SF 1 cannot be acked in SF 2)
      // set ACK/NAKs to DTX
      harq_ack1 = 2; // DTX
      harq_ack0 = 2; // DTX
      // This is the offset for a particular subframe (2,3,4) => (0,2,4)
      subframe_offset = (subframe-2)<<1;
      // i=0
      nCCE0 = phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->nCCE[5+subframe_offset];
      n1_pucch0 = get_Np(frame_parms->N_RB_DL,nCCE0,0) + nCCE0+ frame_parms->pucch_config_common.n1PUCCH_AN;
      // i=1
      nCCE1 = phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->nCCE[(6+subframe_offset)%10];
      n1_pucch1 = get_Np(frame_parms->N_RB_DL,nCCE1,1) + nCCE1 + frame_parms->pucch_config_common.n1PUCCH_AN;

      // set ACK/NAK to values if not DTX
      if (phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack[(6+subframe_offset)%10].send_harq_status>0)  // n-6 // subframe 6 is to be ACK/NAKed
        harq_ack1 = phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack[(6+subframe_offset)%10].ack;

      if (phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack[5+subframe_offset].send_harq_status>0)  // n-6 // subframe 5 is to be ACK/NAKed
        harq_ack0 = phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack[5+subframe_offset].ack;


      if (harq_ack1!=2) 
      { 
        // n-6 // subframe 6,8,0 and maybe 5,7,9 is to be ACK/NAKed

        if ((bundling_flag==bundling)&&(SR == 0)) 
        {  
          // This is for bundling without SR,
          // n1_pucch index takes value of smallest element in set {0,1}
          // i.e. 0 if harq_ack0 is not DTX, otherwise 1
          b[0] = harq_ack1;

          if (harq_ack0!=2)
            b[0]=b[0]&harq_ack0;

          phy_vars_ue->pucch_sel[subframe] = 1;
          
          print_info("get_n1_pucch:A subframe =%d get_n1_pucch:[b0,b1]=[%d %d] harq_ack0=%d harq_ack1=%d bundling_flag=%d SR=%d n1_pucch=%d/%d subframe_offset=%d\n",
            subframe,b[0],b[1],
            harq_ack0,harq_ack1,bundling_flag,SR,
            n1_pucch0,n1_pucch1,
            subframe_offset);

          return(n1_pucch1);

        }
        else if ((bundling_flag==multiplexing)&&(SR==0)) 
        { 
          // Table 10.1
          if (harq_ack0 == 2)
            harq_ack0 = 0;

          b[1] = harq_ack0;
          b[0] = (harq_ack0!=harq_ack1)?0:1;

          if ((harq_ack0 == 1) && (harq_ack1 == 0)) {
            phy_vars_ue->pucch_sel[subframe] = 0;
            
            print_info("get_n1_pucch:B subframe =%d get_n1_pucch:[b0,b1]=[%d %d] harq_ack0=%d harq_ack1=%d bundling_flag=%d SR=%d n1_pucch=%d/%d/%d subframe_offset=%d\n",
              subframe,b[0],b[1],
              harq_ack0,harq_ack1,bundling_flag,SR,
              n1_pucch0,n1_pucch1,phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex,
              subframe_offset);

            return(n1_pucch0);
          } else {
            phy_vars_ue->pucch_sel[subframe] = 1;
            
            print_info("get_n1_pucch:C subframe =%d get_n1_pucch:[b0,b1]=[%d %d] harq_ack0=%d harq_ack1=%d bundling_flag=%d SR=%d n1_pucch=%d/%d/%d subframe_offset=%d\n",
              subframe,b[0],b[1],
              harq_ack0,harq_ack1,bundling_flag,SR,
              n1_pucch0,n1_pucch1,phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex,
              subframe_offset);

            return(n1_pucch1);
          }
        } 
        else if (SR==1) 
        { 
          // SR and 0,1,or 2 ACKS, (first 3 entries in Table 7.3-1 of 36.213)
          // this should be number of ACKs (including
          if (harq_ack0 == 2)
            harq_ack0 = 0;

          b[0]= harq_ack1 | harq_ack0;
          b[1]= harq_ack1 ^ harq_ack0;
          
          print_info("get_n1_pucch:D subframe =%d get_n1_pucch:[b0,b1]=[%d %d] harq_ack0=%d harq_ack1=%d bundling_flag=%d SR=%d n1_pucch=%d/%d subframe_offset=%d\n",
            subframe,b[0],b[1],
            harq_ack0,harq_ack1,bundling_flag,SR,
            n1_pucch0,n1_pucch1,
            subframe_offset);

          return(phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex);
        }
      } 
      else if (harq_ack0!=2) 
      { 
        // n-7  // subframe 5,7,9 only is to be ACK/NAKed
        if ((bundling_flag==bundling)&&(SR == 0)) 
        {  
          // last paragraph pg 68 from 36.213 (v8.6), m=0
          b[0]=harq_ack0;
          phy_vars_ue->pucch_sel[subframe] = 0;
          
          print_info("get_n1_pucch:E subframe =%d get_n1_pucch:[b0,b1]=[%d %d] harq_ack0=%d harq_ack1=%d bundling_flag=%d SR=%d n1_pucch=%d/%d subframe_offset=%d\n",
            subframe,b[0],b[1],
            harq_ack0,harq_ack1,bundling_flag,SR,
            n1_pucch0,n1_pucch1,
            subframe_offset);

          return(n1_pucch0);
        } 
        else if ((bundling_flag==multiplexing)&&(SR==0)) 
        { 
          // Table 10.1 with i=1 set to DTX
          b[0] = 1-harq_ack0; //harq_ack0; //xhd_oai_debug
          b[1] = 1-b[0];
          phy_vars_ue->pucch_sel[subframe] = 0;
          
          print_info("get_n1_pucch:F subframe =%d get_n1_pucch:[b0,b1]=[%d %d] harq_ack0=%d harq_ack1=%d bundling_flag=%d SR=%d n1_pucch=%d/%d subframe_offset=%d\n",
            subframe,b[0],b[1],
            harq_ack0,harq_ack1,bundling_flag,SR,
            n1_pucch0,n1_pucch1,
            subframe_offset);

          return(n1_pucch0);
        } 
        else if (SR==1) 
        { 
          // SR and only 0 or 1 ACKs (first 2 entries in Table 7.3-1 of 36.213)
          b[0]=harq_ack0;
          b[1]=b[0];

          print_info("get_n1_pucch:G subframe =%d get_n1_pucch:[b0,b1]=[%d %d] harq_ack0=%d harq_ack1=%d bundling_flag=%d SR=%d n1_pucch=%d/%d subframe_offset=%d\n",
            subframe,b[0],b[1],
            harq_ack0,harq_ack1,bundling_flag,SR,
            n1_pucch0,n1_pucch1,
            subframe_offset);

          return(phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex);
        }
      }

      break;

    }  // switch tdd_config
  }

  LOG_E(PHY,"[UE%d] : Frame %d phy_procedures_lte.c: get_n1pucch, exit without proper return\n",phy_vars_ue->frame_tx);
  return(-1);
}


#ifdef EMOS
/*
void phy_procedures_emos_UE_TX(uint8_t next_slot,uint8_t eNB_id) {
  uint8_t harq_pid;


  if (next_slot%2==0) {
    // get harq_pid from subframe relationship
    harq_pid = subframe2harq_pid(&phy_vars_ue->lte_frame_parms,phy_vars_ue->frame,(next_slot>>1));
    if (harq_pid==255) {
      LOG_E(PHY,"[UE%d] Frame %d : FATAL ERROR: illegal harq_pid, returning\n",
    0,phy_vars_ue->frame);
      return;
    }

    if (ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_scheduling_flag == 1) {
      emos_dump_UE.uci_cnt[next_slot>>1] = 1;
      memcpy(emos_dump_UE.UCI_data[0][next_slot>>1].o,ulsch_ue[eNB_id]->o,MAX_CQI_BITS*sizeof(char));
      emos_dump_UE.UCI_data[0][next_slot>>1].O = ulsch_ue[eNB_id]->O;
      memcpy(emos_dump_UE.UCI_data[0][next_slot>>1].o_RI,ulsch_ue[eNB_id]->o_RI,2*sizeof(char));
      emos_dump_UE.UCI_data[0][next_slot>>1].O_RI = ulsch_ue[eNB_id]->O_RI;
      memcpy(emos_dump_UE.UCI_data[0][next_slot>>1].o_ACK,ulsch_ue[eNB_id]->o_ACK,4*sizeof(char));
      emos_dump_UE.UCI_data[0][next_slot>>1].O_ACK = ulsch_ue[eNB_id]->harq_processes[harq_pid]->O_ACK;
    }
    else {
      emos_dump_UE.uci_cnt[next_slot>>1] = 0;
    }
  }
}
*/
#endif

int dummy_tx_buffer[10][3840*4] __attribute__((aligned(16)));
int dummy_tx_buffer2[10][3840*4] __attribute__((aligned(16)));
int dummy_tx_buffer3[10][3840*4] __attribute__((aligned(16)));
#ifndef OPENAIR2
PRACH_RESOURCES_t prach_resources_local;
#endif



void phy_procedures_UE_TX_old(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint8_t abstraction_flag,runmode_t mode,relaying_type_t r_type)
{

#ifndef OPENAIR2
  int i;
#endif
  uint16_t first_rb, nb_rb;
  uint8_t harq_pid;
  unsigned int input_buffer_length;
  unsigned int aa;
  uint8_t Msg3_flag=0;
  uint8_t pucch_ack_payload[2];
  uint8_t n1_pucch;
  ANFBmode_t bundling_flag;
  PUCCH_FMT_t format;
  uint8_t SR_payload;
  int32_t prach_power;
  uint8_t nsymb;
  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_ue->lte_frame_parms;
  uint8_t generate_ul_signal = 0;
  uint8_t ack_status=0;
  int8_t Po_PUCCH;
  int32_t ulsch_start=0;
#if defined(EXMIMO) || defined(OAI_USRP)
  int overflow=0;
  int k,l;
#endif
  int slot_tx = phy_vars_ue->slot_tx;
  int subframe_tx = phy_vars_ue->slot_tx>>1;
  int frame_tx = phy_vars_ue->frame_tx;
  int Mod_id = phy_vars_ue->Mod_id;
  int CC_id = phy_vars_ue->CC_id;
  int tx_amp;

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX,VCD_FUNCTION_IN);

  start_meas(&phy_vars_ue->phy_proc_tx);

#ifdef EMOS
  //phy_procedures_emos_UE_TX(next_slot);
#endif

  //if ((slot_tx%2)==0) 
  {
    phy_vars_ue->tx_power_dBm=-127;

    if (abstraction_flag==0) {
      for (aa=0; aa<frame_parms->nb_antennas_tx; aa++) {
        memset(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][subframe_tx*frame_parms->ofdm_symbol_size*frame_parms->symbols_per_tti],
               0,
               frame_parms->ofdm_symbol_size*frame_parms->symbols_per_tti*sizeof(int32_t));
      }
    }

    if (phy_vars_ue->UE_mode[eNB_id] != PRACH) {
      /*
      #ifdef DEBUG_PHY_PROC
        LOG_D(PHY,"[UE  %d] Frame %d, slot %d: Generating SRS\n",Mod_id,phy_vars_ue->frame,slot_tx);
      #endif
        if (abstraction_flag == 0) {
      #ifdef OFDMA_ULSCH
      generate_srs_tx(phy_vars_ue,eNB_id,AMP,subframe_tx);
      #else
      generate_srs_tx(phy_vars_ue,eNB_id,AMP,subframe_tx);
      #endif
        }

      #ifdef PHY_ABSTRACTION
        else {
      generate_srs_tx_emul(phy_vars_ue,subframe_tx);
        }
      #endif
      */
      // get harq_pid from subframe relationship
      harq_pid = subframe2harq_pid(&phy_vars_ue->lte_frame_parms,
                                   frame_tx,
                                   subframe_tx);


#ifdef OPENAIR2

      if ((phy_vars_ue->ulsch_ue_Msg3_active[eNB_id] == 1) &&
          (phy_vars_ue->ulsch_ue_Msg3_frame[eNB_id] == frame_tx) &&
          (phy_vars_ue->ulsch_ue_Msg3_subframe[eNB_id] == subframe_tx)) { // Initial Transmission of Msg3

        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_scheduling_flag = 1;

        if (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round==0)
          generate_ue_ulsch_params_from_rar(phy_vars_ue,
                                            eNB_id);

        phy_vars_ue->ulsch_ue[eNB_id]->power_offset = 14;
        LOG_D(PHY,"[UE  %d][RAPROC] Frame %d: Setting Msg3_flag in subframe %d, for harq_pid %d\n",
              Mod_id,
              frame_tx,
              subframe_tx,
              harq_pid);
        Msg3_flag = 1;
      } else {

        if (harq_pid==255) {
          LOG_E(PHY,"[UE%d] Frame %d ulsch_decoding.c: FATAL ERROR: illegal harq_pid, returning\n",
                Mod_id,frame_tx);
          mac_xface->macphy_exit("Error in ulsch_decoding");
          VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX, VCD_FUNCTION_OUT);
          stop_meas(&phy_vars_ue->phy_proc_tx);
          return;
        }

        Msg3_flag=0;
      }

#endif

      if (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_scheduling_flag == 1) {

        generate_ul_signal = 1;

        // deactivate service request
        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_scheduling_flag = 0;

        ack_status = get_ack(&phy_vars_ue->lte_frame_parms,
                             phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack,
                             subframe_tx,
                             phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->o_ACK);

        first_rb = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->first_rb;
        nb_rb = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->nb_rb;


        //frame_parms->pusch_config_c ommon.ul_ReferenceSignalsPUSCH.cyclicShift = 0;

        //frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.nPRS[20] = 0;



#ifdef DEBUG_PHY_PROC
        LOG_D(PHY,
              "[UE  %d][PUSCH %d] Frame %d subframe %d Generating PUSCH : first_rb %d, nb_rb %d, round %d, mcs %d, rv %d, cyclic_shift %d (cyclic_shift_common %d,n_DMRS2 %d,n_PRS %d), ACK (%d,%d), O_ACK %d\n",
              Mod_id,harq_pid,frame_tx,subframe_tx,
              first_rb,nb_rb,
              phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round,
              phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->mcs,
              phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->rvidx,
              (frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift+
               phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->n_DMRS2+
               frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.nPRS[slot_tx])%12,
              frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift,
              phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->n_DMRS2,
              frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.nPRS[slot_tx],
              phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->o_ACK[0],
              phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->o_ACK[1],
              phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->O_ACK);
#endif

        if (ack_status > 0) {
          LOG_D(PHY,"[UE  %d][PDSCH %x] Frame %d subframe %d Generating ACK (%d,%d) for %d bits on PUSCH\n",
                Mod_id,
                phy_vars_ue->ulsch_ue[eNB_id]->rnti,
                frame_tx,subframe_tx,
                phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->o_ACK[0],
                phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->o_ACK[1],
                phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->O_ACK);
        }





        //#ifdef DEBUG_PHY_PROC
        //  debug_LOG_D(PHY,"[UE  %d] Frame %d, Subframe %d ulsch harq_pid %d : O %d, O_ACK %d, O_RI %d, TBS %d\n",Mod_id,phy_vars_ue->frame,subframe_tx,harq_pid,phy_vars_ue->ulsch_ue[eNB_id]->O,phy_vars_ue->ulsch_ue[eNB_id]->O_ACK,phy_vars_ue->ulsch_ue[eNB_id]->O_RI,phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS);
        //#endif
        if (Msg3_flag == 1) {
          LOG_I(PHY,"[UE  %d][RAPROC] Frame %d, Subframe %d next slot %d Generating (RRCConnectionRequest) Msg3 (nb_rb %d, first_rb %d, round %d, rvidx %d) Msg3: %x.%x.%x|%x.%x.%x.%x.%x.%x\n",Mod_id,frame_tx,
                subframe_tx, slot_tx,
                phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->nb_rb,
                phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->first_rb,
                phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round,
                phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->rvidx,
                phy_vars_ue->prach_resources[eNB_id]->Msg3[0],
                phy_vars_ue->prach_resources[eNB_id]->Msg3[1],
                phy_vars_ue->prach_resources[eNB_id]->Msg3[2],
                phy_vars_ue->prach_resources[eNB_id]->Msg3[3],
                phy_vars_ue->prach_resources[eNB_id]->Msg3[4],
                phy_vars_ue->prach_resources[eNB_id]->Msg3[5],
                phy_vars_ue->prach_resources[eNB_id]->Msg3[6],
                phy_vars_ue->prach_resources[eNB_id]->Msg3[7],
                phy_vars_ue->prach_resources[eNB_id]->Msg3[8]);

          start_meas(&phy_vars_ue->ulsch_encoding_stats);

          if (abstraction_flag==0) {
            if (ulsch_encoding(phy_vars_ue->prach_resources[eNB_id]->Msg3,
                               phy_vars_ue,
                               harq_pid,
                               eNB_id,
                               phy_vars_ue->transmission_mode[eNB_id],0,0)!=0) {
              LOG_E(PHY,"ulsch_coding.c: FATAL ERROR: returning\n");
              mac_xface->macphy_exit("Error in ulsch_coding");
              VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX, VCD_FUNCTION_OUT);
              stop_meas(&phy_vars_ue->phy_proc_tx);
              return;
            }
          }

#ifdef PHY_ABSTRACTION
          else {
            ulsch_encoding_emul(phy_vars_ue->prach_resources[eNB_id]->Msg3,phy_vars_ue,eNB_id,harq_pid,0);
          }

#endif
          stop_meas(&phy_vars_ue->ulsch_encoding_stats);


#ifdef OPENAIR2
          // signal MAC that Msg3 was sent
          mac_xface->Msg3_transmitted(Mod_id,
                                      CC_id,
                                      frame_tx,
                                      eNB_id);
#endif
        } else {
          input_buffer_length = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS/8;

#ifdef OPENAIR2

          //  LOG_D(PHY,"[UE  %d] ULSCH : Searching for MAC SDUs\n",Mod_id);
          if (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round==0) {
            //if (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->calibration_flag == 0) {
            access_mode=SCHEDULED_ACCESS;
            int sdu_len = 0;
            int offset = 0;
            mac_xface->ue_get_sdu(Mod_id,
                                  CC_id,
                                  frame_tx,
                                  subframe_tx,
                                  eNB_id,
                                  ulsch_input_buffer,
                                  input_buffer_length,
                                  &access_mode,
                                  &sdu_len,
                                  &offset);

            //xhd_oai_miss UE ULSCH 
            if( sdu_len != 0 )
            {
                LOG_W(PHY,"[UE] Frame %d, subframe %d : ULSCH SDU (TX harq_pid %d)  (%d-%d bytes) : \n",
                     frame_tx,subframe_tx,harq_pid, 
                     input_buffer_length, sdu_len);
                  for (int i=0; i<input_buffer_length; i++)
                    printf("%x.",ulsch_input_buffer[i]);
                
                  printf("\n");
              }

            //}
            /*
            else {
              // Get calibration information from TDD procedures
              LOG_D(PHY,"[UE %d] Frame %d, subframe %d : ULSCH: Getting TDD Auto-Calibration information\n",
              Mod_id,phy_vars_ue->frame,subframe_tx);
              for (i=0;i<input_buffer_length;i++)
            ulsch_input_buffer[i]= i;

            }
            */
          }

#ifdef DEBUG_PHY_PROC
#ifdef DEBUG_ULSCH
          LOG_D(PHY,"[UE] Frame %d, subframe %d : ULSCH SDU (TX harq_pid %d)  (%d bytes) : \n",frame_tx,subframe_tx,harq_pid, phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS>>3);

          for (i=0; i<phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS>>3; i++)
            LOG_T(PHY,"%x.",ulsch_input_buffer[i]);

          LOG_T(PHY,"\n");
#endif
#endif
#else //OPENAIR2
          // the following lines were necessary for the calibration in CROWN

        /*
          if (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->calibration_flag == 0) {
              for (i=0;i<input_buffer_length;i++)
                  ulsch_input_buffer[i]= (uint8_t)(taus()&0xff);
          }
          else {
              // Get calibration information from TDD procedures
          }
          */

          for (i=0; i<input_buffer_length; i++)
            ulsch_input_buffer[i]= (uint8_t)(taus()&0xff);

          // the following lines were necessary for the collaborative UL in PUCCO
          /*
          memset(phy_vars_ue->ulsch_ue[eNB_id]->o    ,0,MAX_CQI_BYTES*sizeof(uint8_t));
          memset(phy_vars_ue->ulsch_ue[eNB_id]->o_RI ,0,2*sizeof(uint8_t));
          memset(phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->o_ACK,0,4*sizeof(uint8_t));
          for (i=0;i<input_buffer_length;i++)
            ulsch_input_buffer[i]= i;
          */

#endif //OPENAIR2
          start_meas(&phy_vars_ue->ulsch_encoding_stats);

          if (abstraction_flag==0) {
            /*
            if (phy_vars_ue->frame%100==0) {
              LOG_I(PHY,"Encoding ulsch\n");
            }
            */
            if (ulsch_encoding(ulsch_input_buffer,
                               phy_vars_ue,
                               harq_pid,
                               eNB_id,
                               phy_vars_ue->transmission_mode[eNB_id],0,
                               0)!=0) {  //  Nbundled, to be updated!!!!
              LOG_E(PHY,"ulsch_coding.c: FATAL ERROR: returning\n");
              VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX, VCD_FUNCTION_OUT);
              stop_meas(&phy_vars_ue->phy_proc_tx);
              return;
            }
          }

#ifdef PHY_ABSTRACTION
          else {
            ulsch_encoding_emul(ulsch_input_buffer,phy_vars_ue,eNB_id,harq_pid,0);
          }

#endif
          stop_meas(&phy_vars_ue->ulsch_encoding_stats);
        }

        if (abstraction_flag == 0) {
#ifdef OPENAIR2
          pusch_power_cntl(phy_vars_ue,subframe_tx,eNB_id,1, abstraction_flag);
          phy_vars_ue->tx_power_dBm = phy_vars_ue->ulsch_ue[eNB_id]->Po_PUSCH;
#else
          phy_vars_ue->tx_power_dBm = UE_TX_POWER;
#endif
          phy_vars_ue->tx_total_RE = nb_rb*12;
	  
#if defined(EXMIMO) || defined(OAI_USRP)
	  tx_amp = get_tx_amp(phy_vars_ue->tx_power_dBm,
			      phy_vars_ue->tx_power_max_dBm,
			      phy_vars_ue->lte_frame_parms.N_RB_UL,
			      nb_rb);
#else
          tx_amp = AMP;
#endif
      printf("phy_vars_ue->tx_power_dBm:%d UE_TX_POWER:%d nb_rb=%d tx_amp=%d AMP=%d",
        phy_vars_ue->tx_power_dBm,UE_TX_POWER,nb_rb,tx_amp,AMP);

	  LOG_I(PHY,"[UE  %d][PUSCH %d] Frame %d subframe %d, generating PUSCH, Po_PUSCH: %d dBm (max %d dBm), amp %d\n",
		Mod_id,harq_pid,frame_tx,subframe_tx,phy_vars_ue->tx_power_dBm,phy_vars_ue->tx_power_max_dBm, tx_amp);
          start_meas(&phy_vars_ue->ulsch_modulation_stats);
          ulsch_modulation(phy_vars_ue->lte_ue_common_vars.txdataF,
			   tx_amp,
                           frame_tx,
                           subframe_tx,
                           &phy_vars_ue->lte_frame_parms,
                           phy_vars_ue->ulsch_ue[eNB_id],Mod_id);
          for (aa=0; aa<1/*frame_parms->nb_antennas_tx*/; aa++)
            generate_drs_pusch(phy_vars_ue,
			       eNB_id,
			       tx_amp,
			       subframe_tx,
			       first_rb,
			       nb_rb,
			       aa);

          stop_meas(&phy_vars_ue->ulsch_modulation_stats);
        }

        if (abstraction_flag==1) {
          // clear SR
          phy_vars_ue->sr[subframe_tx]=0;
        }
      } // ULSCH is active

#ifdef PUCCH
      else if (phy_vars_ue->UE_mode[eNB_id] == PUSCH) { // check if we need to use PUCCH 1a/1b
        //      debug_LOG_D(PHY,"[UE%d] Frame %d, subframe %d: Checking for PUCCH 1a/1b\n",Mod_id,frame_tx,subframe_tx);
        bundling_flag = phy_vars_ue->pucch_config_dedicated[eNB_id].tdd_AckNackFeedbackMode;

        if ((frame_parms->frame_type==FDD) ||
            (bundling_flag==bundling)    ||
            ((frame_parms->frame_type==TDD)&&(frame_parms->tdd_config==1)&&((slot_tx!=4)||(slot_tx!=14)))) {
          format = pucch_format1a;
          LOG_D(PHY,"[UE] PUCCH 1a\n");
        } else {
          format = pucch_format1b;
          LOG_D(PHY,"[UE] PUCCH 1b\n");
        }

        // Check for SR and do ACK/NACK accordingly
        if (is_SR_TXOp(phy_vars_ue,eNB_id,subframe_tx)==1) {
          LOG_D(PHY,"[UE %d][SR %x] Frame %d subframe %d: got SR_TXOp, Checking for SR for PUSCH from MAC\n",
                Mod_id,phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,frame_tx,subframe_tx);
#ifdef OPENAIR2
          SR_payload = mac_xface->ue_get_SR(Mod_id,
                                            CC_id,
                                            frame_tx,
                                            eNB_id,
                                            phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,
                                            subframe_tx); // subframe used for meas gap
#else
          SR_payload = 1;
#endif

          if (SR_payload>0) {
            generate_ul_signal = 1;
            LOG_D(PHY,"[UE %d][SR %x] Frame %d subframe %d got the SR for PUSCH is %d\n",
                  Mod_id,phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,frame_tx,subframe_tx,SR_payload);
          } else {
            phy_vars_ue->sr[subframe_tx]=0;
          }
        } else
          SR_payload=0;

        if (get_ack(&phy_vars_ue->lte_frame_parms,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack,
                    subframe_tx,pucch_ack_payload) > 0) {
          // we need to transmit ACK/NAK in this subframe

          generate_ul_signal = 1;

          n1_pucch = get_n1_pucch(phy_vars_ue,
                                  eNB_id,
                                  subframe_tx,
                                  pucch_ack_payload,
                                  SR_payload);

#ifdef OPENAIR2
          Po_PUCCH = pucch_power_cntl(phy_vars_ue,subframe_tx,eNB_id,format);
          phy_vars_ue->tx_power_dBm = Po_PUCCH;
#else
          phy_vars_ue->tx_power_dBm = UE_TX_POWER;
#endif
          phy_vars_ue->tx_total_RE = 12;

#if defined(EXMIMO) || defined(OAI_USRP)
	  tx_amp = get_tx_amp(Po_PUCCH,
			      phy_vars_ue->tx_power_max_dBm,
			      phy_vars_ue->lte_frame_parms.N_RB_UL,
			      1);
#else
	  tx_amp = AMP;
#endif
	  
          if (SR_payload>0) {
	     LOG_D(PHY,"[UE  %d][SR %x] Frame %d subframe %d Generating PUCCH 1a/1b payload %d,%d (with SR for PUSCH), n1_pucch %d, Po_PUCCH, amp %d\n",
                  Mod_id,
                  phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
                  frame_tx, subframe_tx,
		  pucch_ack_payload[0],pucch_ack_payload[1],
                  phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex,
	          Po_PUCCH,
		  tx_amp);
          } else {
            LOG_D(PHY,"[UE  %d][PDSCH %x] Frame %d subframe %d Generating PUCCH 1a/1b, n1_pucch %d, b[0]=%d,b[1]=%d (SR_Payload %d), Po_PUCCH %d, amp %d\n",
                  Mod_id,
                  phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
	    frame_tx, subframe_tx,
		  n1_pucch,pucch_ack_payload[0],pucch_ack_payload[1],SR_payload,
                  Po_PUCCH,
		  tx_amp);
	  }

          if (abstraction_flag == 0) {

	  generate_pucch(phy_vars_ue->lte_ue_common_vars.txdataF,
                           &phy_vars_ue->lte_frame_parms,
                           phy_vars_ue->ncs_cell,
                           format,
                           &phy_vars_ue->pucch_config_dedicated[eNB_id],
                           n1_pucch,
                           0,  // n2_pucch
                           1,  // shortened format
                           pucch_ack_payload,
	                   tx_amp,
	                   subframe_tx);

          } else {
#ifdef PHY_ABSTRACTION
            LOG_D(PHY,"Calling generate_pucch_emul ... (ACK %d %d, SR %d)\n",pucch_ack_payload[0],pucch_ack_payload[1],SR_payload);
            generate_pucch_emul(phy_vars_ue,
                                format,
                                phy_vars_ue->lte_frame_parms.pucch_config_common.nCS_AN,
                                pucch_ack_payload,
                                SR_payload,
                                subframe_tx);
#endif
          }
        } else if (SR_payload==1) { // no ACK/NAK but SR is triggered by MAC

#ifdef OPENAIR2
          Po_PUCCH = pucch_power_cntl(phy_vars_ue,subframe_tx,eNB_id,pucch_format1);
          phy_vars_ue->tx_power_dBm = Po_PUCCH;
#else
          phy_vars_ue->tx_power_dBm = UE_TX_POWER;
#endif
          phy_vars_ue->tx_total_RE = 12;

#if defined(EXMIMO) || defined(OAI_USRP)
          tx_amp =  get_tx_amp(Po_PUCCH,
	                       phy_vars_ue->tx_power_max_dBm,
	                       phy_vars_ue->lte_frame_parms.N_RB_UL,
	                       1);
#else
	  tx_amp = AMP;
#endif
          LOG_D(PHY,"[UE  %d][SR %x] Frame %d subframe %d Generating PUCCH 1 (SR for PUSCH), n1_pucch %d, Po_PUCCH %d\n",
                Mod_id,
                phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
                frame_tx, subframe_tx,
                phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex,
                Po_PUCCH);

          if (abstraction_flag == 0) {

            generate_pucch(phy_vars_ue->lte_ue_common_vars.txdataF,
                           &phy_vars_ue->lte_frame_parms,
                           phy_vars_ue->ncs_cell,
                           pucch_format1,
                           &phy_vars_ue->pucch_config_dedicated[eNB_id],
                           phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex,
                           0,  // n2_pucch
                           1,  // shortened format
                           pucch_ack_payload,  // this is ignored anyway, we just need a pointer
	                   tx_amp,
                           subframe_tx);
          } else {
            LOG_D(PHY,"Calling generate_pucch_emul ...\n");
            generate_pucch_emul(phy_vars_ue,
                                pucch_format1,
                                phy_vars_ue->lte_frame_parms.pucch_config_common.nCS_AN,
                                pucch_ack_payload,
                                SR_payload,
                                subframe_tx);
          }
        }
      }

#endif  // PUCCH

#ifdef CBA

      if ((phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_cba_scheduling_flag >= 1) &&
          (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->status == CBA_ACTIVE)) {
        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_scheduling_flag=0; //-=1
        //  phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->status= IDLE;
        first_rb = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->first_rb;
        nb_rb = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->nb_rb;
        //cba_mcs=phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->mcs;
        input_buffer_length = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS/8;
        access_mode=CBA_ACCESS;

        LOG_D(PHY,"[UE %d] Frame %d, subframe %d: CBA num dci %d\n",
              Mod_id,frame_tx,subframe_tx,
              phy_vars_ue->ulsch_ue[eNB_id]->num_cba_dci[subframe_tx]);
        int sdu_len = 0;
        int offset = 0;
        mac_xface->ue_get_sdu(Mod_id,
                              CC_id,
                              frame_tx,
                              subframe_tx,
                              eNB_id,
                              ulsch_input_buffer,
                              input_buffer_length,
                              &access_mode,
                              &sdu_len,
                              &offset);

        phy_vars_ue->ulsch_ue[eNB_id]->num_cba_dci[subframe_tx]=0;

        if (access_mode > UNKNOWN_ACCESS) {

          if (abstraction_flag==0) {
            if (ulsch_encoding(ulsch_input_buffer,
                               phy_vars_ue,
                               harq_pid,
                               eNB_id,
                               phy_vars_ue->transmission_mode[eNB_id],0,
                               0)!=0) {  //  Nbundled, to be updated!!!!
              LOG_E(PHY,"ulsch_coding.c: FATAL ERROR: returning\n");
              return;
            }
          }

#ifdef PHY_ABSTRACTION
          else {
            ulsch_encoding_emul(ulsch_input_buffer,phy_vars_ue,eNB_id,harq_pid,0);
          }

#endif
        } else {
          phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->status= IDLE;
          //reset_cba_uci(phy_vars_ue->ulsch_ue[eNB_id]->o);
          LOG_N(PHY,"[UE %d] Frame %d, subframe %d: CBA transmission cancelled or postponed\n",
                Mod_id, frame_tx,subframe_tx);
        }
      }

#endif // end CBA

      if (abstraction_flag == 0) {
        nsymb = (frame_parms->Ncp == 0) ? 14 : 12;

#if defined(EXMIMO) || defined(OAI_USRP) //this is the EXPRESS MIMO case
        ulsch_start = (phy_vars_ue->rx_offset+subframe_tx*frame_parms->samples_per_tti-
                       openair_daq_vars.timing_advance-
                       phy_vars_ue->timing_advance-
                       phy_vars_ue->N_TA_offset+5)%(LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*frame_parms->samples_per_tti);
#else //this is the normal case
        ulsch_start = (frame_parms->samples_per_tti*subframe_tx)-phy_vars_ue->N_TA_offset; //-phy_vars_ue->timing_advance;
#endif //else EXMIMO
	if ((frame_tx%100) == 0)
	  LOG_D(PHY,"[UE %d] Frame %d, subframe %d: ulsch_start = %d (rxoff %d, HW TA %d, timing advance %d, TA_offset %d\n",
		Mod_id,frame_tx,subframe_tx,
		ulsch_start,
		phy_vars_ue->rx_offset,
		openair_daq_vars.timing_advance,
		phy_vars_ue->timing_advance,
		phy_vars_ue->N_TA_offset);
 

        if (generate_ul_signal == 1 ) {



          start_meas(&phy_vars_ue->ofdm_mod_stats);

          for (aa=0; aa<frame_parms->nb_antennas_tx; aa++) {
            if (frame_parms->Ncp == 1)
              PHY_ofdm_mod(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][subframe_tx*nsymb*frame_parms->ofdm_symbol_size],
#if defined(EXMIMO) || defined(OAI_USRP)
                           dummy_tx_buffer,
#else
                           &phy_vars_ue->lte_ue_common_vars.txdata[aa][ulsch_start],
#endif
                           frame_parms->log2_symbol_size,
                           nsymb,
                           frame_parms->nb_prefix_samples,
                           CYCLIC_PREFIX);
            else
              normal_prefix_mod(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][subframe_tx*nsymb*frame_parms->ofdm_symbol_size],
#if defined(EXMIMO) || defined(OAI_USRP)
                                dummy_tx_buffer,
#else
                                &phy_vars_ue->lte_ue_common_vars.txdata[aa][ulsch_start],
#endif
                                nsymb,
                                &phy_vars_ue->lte_frame_parms);

            /*
              if (subframe_tx == 8) {
              printf("Symbol 0 %p (offset %d) base %p\n",
              &phy_vars_ue->lte_ue_common_vars.txdataF[0][nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*subframe],
              nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*subframe,
              phy_vars_ue->lte_ue_common_vars.txdataF[0]);
              write_output("txsigF8.m","txsF8", &phy_vars_ue->lte_ue_common_vars.txdataF[0][nsymb*OFDM_SYMBOL_SIZE_COMPLEX_SAMPLES_NO_PREFIX*subframe],
              phy_vars_ue->lte_frame_parms.ofdm_symbol_size*nsymb,1,1);
              write_output("txsig8.m","txs8", &phy_vars_ue->lte_ue_common_vars.txdata[0][phy_vars_ue->lte_frame_parms.samples_per_tti*subframe],
              phy_vars_ue->lte_frame_parms.samples_per_tti,1,1);
              }
            */
#ifndef OFDMA_ULSCH
#if defined(EXMIMO) || defined(OAI_USRP)
            apply_7_5_kHz(phy_vars_ue,dummy_tx_buffer,0);
            apply_7_5_kHz(phy_vars_ue,dummy_tx_buffer,1);
#else
            apply_7_5_kHz(phy_vars_ue,&phy_vars_ue->lte_ue_common_vars.txdata[aa][ulsch_start],0);
            apply_7_5_kHz(phy_vars_ue,&phy_vars_ue->lte_ue_common_vars.txdata[aa][ulsch_start],1);
#endif
            /*
              if (subframe_tx == 8) {
              write_output("txsig8_mod.m","txs8_mod", &phy_vars_ue->lte_ue_common_vars.txdata[0][phy_vars_ue->lte_frame_parms.samples_per_tti*subframe],
              phy_vars_ue->lte_frame_parms.samples_per_tti,1,1);
              }
            */
#endif

#if defined(EXMIMO) || defined(OAI_USRP)
            overflow = ulsch_start - 9*frame_parms->samples_per_tti;

            //if ((slot_tx==4) && (aa==0)) printf("ulsch_start %d, overflow %d\n",ulsch_start,overflow);
            for (k=ulsch_start,l=0; k<cmin(frame_parms->samples_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME,ulsch_start+frame_parms->samples_per_tti); k++,l++) {
              ((short*)phy_vars_ue->lte_ue_common_vars.txdata[aa])[2*k] = ((short*)dummy_tx_buffer)[2*l]<<4;
              ((short*)phy_vars_ue->lte_ue_common_vars.txdata[aa])[2*k+1] = ((short*)dummy_tx_buffer)[2*l+1]<<4;
            }

            for (k=0; k<overflow; k++,l++) {
              ((short*)phy_vars_ue->lte_ue_common_vars.txdata[aa])[2*k] = ((short*)dummy_tx_buffer)[2*l]<<4;
              ((short*)phy_vars_ue->lte_ue_common_vars.txdata[aa])[2*k+1] = ((short*)dummy_tx_buffer)[2*l+1]<<4;
            }
#if defined(EXMIMO)
	    // handle switch before 1st TX subframe, guarantee that the slot prior to transmission is switch on
	    for (k=ulsch_start - (frame_parms->samples_per_tti>>1) ; k<ulsch_start ; k++) {
	      if (k<0)
		phy_vars_ue->lte_ue_common_vars.txdata[aa][k+frame_parms->samples_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME] &= 0xFFFEFFFE;
	      else if (k>(frame_parms->samples_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME))
		phy_vars_ue->lte_ue_common_vars.txdata[aa][k-frame_parms->samples_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME] &= 0xFFFEFFFE;
	      else
		phy_vars_ue->lte_ue_common_vars.txdata[aa][k] &= 0xFFFEFFFE;
	    }
#endif
#endif

          } //nb_antennas_tx

          stop_meas(&phy_vars_ue->ofdm_mod_stats);
        } // generate_ul_signal == 1
        else {  // no uplink so clear signal buffer instead
          for (aa=0; aa<frame_parms->nb_antennas_tx; aa++) {
            memset(&phy_vars_ue->lte_ue_common_vars.txdata[aa][ulsch_start],0,frame_parms->samples_per_tti<<2);
          }
        }
      }
    } // mode != PRACH

    //  }// slot_tx is even
    //  else {  // slot_tx is odd, do the PRACH here

#ifdef OPENAIR2

    if ((phy_vars_ue->UE_mode[eNB_id] == PRACH) && (phy_vars_ue->lte_frame_parms.prach_config_common.prach_Config_enabled==1)) {

#else

    if (1) {
#endif

      // check if we have PRACH opportunity
      if (is_prach_subframe(&phy_vars_ue->lte_frame_parms,frame_tx,subframe_tx)) {
        phy_vars_ue->generate_prach=0;
#ifdef OPENAIR2

        // ask L2 for RACH transport
        if ((mode != rx_calib_ue) && (mode != rx_calib_ue_med) && (mode != rx_calib_ue_byp) && (mode != no_L2_connect) ) {
          phy_vars_ue->prach_resources[eNB_id] = mac_xface->ue_get_rach(Mod_id,
                                                 CC_id,
                                                 frame_tx,
                                                 eNB_id,
                                                 subframe_tx);
          //    LOG_I(PHY,"Got prach_resources for eNB %d address %d, RRCCommon %d\n",eNB_id,phy_vars_ue->prach_resources[eNB_id],UE_mac_inst[Mod_id].radioResourceConfigCommon);
        }
#endif

        if (phy_vars_ue->prach_resources[eNB_id]!=NULL) {

          phy_vars_ue->generate_prach=1;
          phy_vars_ue->prach_cnt=0;
#ifdef SMBV
          phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex = 19;
#endif
#ifdef OAI_EMU
          phy_vars_ue->prach_PreambleIndex=phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex;
#endif

          if (abstraction_flag == 0) {
            LOG_D(PHY,"[UE  %d][RAPROC] Frame %d, Subframe %d : Generating PRACH, preamble %d, TARGET_RECEIVED_POWER %d dBm, PRACH TDD Resource index %d, RA-RNTI %d\n",
                  Mod_id,
                  frame_tx,
                  subframe_tx,
                  phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex,
                  phy_vars_ue->prach_resources[eNB_id]->ra_PREAMBLE_RECEIVED_TARGET_POWER,
                  phy_vars_ue->prach_resources[eNB_id]->ra_TDD_map_index,
                  phy_vars_ue->prach_resources[eNB_id]->ra_RNTI);

#ifdef OPENAIR2
	    if (mode != calib_prach_tx)
	      phy_vars_ue->tx_power_dBm = phy_vars_ue->prach_resources[eNB_id]->ra_PREAMBLE_RECEIVED_TARGET_POWER+get_PL(Mod_id,CC_id,eNB_id);
	    else {
	      phy_vars_ue->tx_power_dBm = phy_vars_ue->tx_power_max_dBm;
	      phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex = 19;	      
	    }
#else
            phy_vars_ue->tx_power_dBm = UE_TX_POWER;
#endif

            phy_vars_ue->tx_total_RE = 96;

#if defined(EXMIMO) || defined(OAI_USRP)
            phy_vars_ue->lte_ue_prach_vars[eNB_id]->amp = get_tx_amp(phy_vars_ue->tx_power_dBm,
								     phy_vars_ue->tx_power_max_dBm,
								     phy_vars_ue->lte_frame_parms.N_RB_UL,
								     6);
#else
            phy_vars_ue->lte_ue_prach_vars[eNB_id]->amp = AMP;
#endif
	    if ((mode == calib_prach_tx) && (((phy_vars_ue->frame_tx&0xfffe)%100)==0))
	      LOG_D(PHY,"[UE  %d][RAPROC] Frame %d, Subframe %d : PRACH TX power %d dBm, amp %d\n",Mod_id,phy_vars_ue->frame_rx,phy_vars_ue->slot_tx>>1,phy_vars_ue->tx_power_dBm,
		    phy_vars_ue->lte_ue_prach_vars[eNB_id]->amp);


            //      start_meas(&phy_vars_ue->tx_prach);
            VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GENERATE_PRACH, VCD_FUNCTION_IN);
            prach_power = generate_prach(phy_vars_ue,eNB_id,subframe_tx,frame_tx);
            VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GENERATE_PRACH, VCD_FUNCTION_OUT);
            //      stop_meas(&phy_vars_ue->tx_prach);
            LOG_D(PHY,"[UE  %d][RAPROC] PRACH PL %d dB, power %d dBm, digital power %d dB (amp %d)\n",
                  Mod_id,
                  get_PL(Mod_id,CC_id,eNB_id),
                  phy_vars_ue->tx_power_dBm,
                  dB_fixed(prach_power),
                  phy_vars_ue->lte_ue_prach_vars[eNB_id]->amp);
          } else {
            UE_transport_info[Mod_id][CC_id].cntl.prach_flag=1;
            UE_transport_info[Mod_id][CC_id].cntl.prach_id=phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex;
#ifdef OPENAIR2
            mac_xface->Msg1_transmitted(Mod_id,
                                        CC_id,
                                        frame_tx,
                                        eNB_id);
#endif
          }

          LOG_D(PHY,"[UE  %d][RAPROC] Frame %d, subframe %d: Generating PRACH (eNB %d) preamble index %d for UL, TX power %d dBm (PL %d dB), l3msg \n",
                Mod_id,frame_tx,subframe_tx,eNB_id,
                phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex,
                phy_vars_ue->prach_resources[eNB_id]->ra_PREAMBLE_RECEIVED_TARGET_POWER+get_PL(Mod_id,CC_id,eNB_id),
                get_PL(Mod_id,CC_id,eNB_id));

	}	  

      }
      // if we're calibrating the PRACH kill the pointer to its resources so that the RA protocol doesn't continue
      if (mode == calib_prach_tx)
	phy_vars_ue->prach_resources[eNB_id]=NULL;

      LOG_D(PHY,"[UE %d] frame %d subframe %d : generate_prach %d, prach_cnt %d\n",
            Mod_id,frame_tx,subframe_tx,phy_vars_ue->generate_prach,phy_vars_ue->prach_cnt);

      phy_vars_ue->prach_cnt++;

      if (phy_vars_ue->prach_cnt==3)
        phy_vars_ue->generate_prach=0;
    } // mode is PRACH
    else {
      phy_vars_ue->generate_prach=0;
    }
  } // slot_tx is even

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX, VCD_FUNCTION_OUT);
  stop_meas(&phy_vars_ue->phy_proc_tx);
}
//}

uint32_t ueUlDci[10][9]={0};
uint32_t ueHich[10][9]={0};

void UE_UlDci(uint8_t ucSubframe, uint8_t ucCount)
{
    if( ucCount < 8 && ucSubframe < 10)
    {
        ueUlDci[ucSubframe][ucCount+1]++;
        ueUlDci[ucSubframe][0]++;
    }
}
void UE_Hich(uint8_t ucSubframe, uint8_t ucCount)
{
    if( ucCount < 8 && ucSubframe < 10)
    {
        ueHich[ucSubframe][ucCount+1]++;
        ueHich[ucSubframe][0]++;
    }
}


uint32_t uePuschSched[10][9]={0};
uint32_t uePuschAck[10][9]={0};
uint32_t uePuschNack[10][9]={0};
uint32_t uePuschSchedSum[9]={0};
uint32_t uePuschAckSum[9]={0};
uint32_t uePuschNackSum[9]={0};


void UE_SendCount(uint8_t ucSubframe, uint8_t ucCount)
{
    if( ucCount < 8 && ucSubframe < 10)
    {
        uePuschSched[ucSubframe][ucCount+1]++;
        uePuschSched[ucSubframe][0]++;
        uePuschSchedSum[ucCount+1]++;
        uePuschSchedSum[0]++;
    }
}
void UE_SendCountA(uint8_t ucSubframe, uint8_t ucCount)
{
    if( ucCount < 8 && ucSubframe < 10)
    {
        uePuschAck[ucSubframe][ucCount+1]++;
        uePuschAck[ucSubframe][0]++;
        uePuschAckSum[ucCount+1]++;
        uePuschAckSum[0]++;
    }
}
void UE_SendCountN(uint8_t ucSubframe, uint8_t ucCount)
{
    if( ucCount < 8 && ucSubframe < 10)
    {
        uePuschNack[ucSubframe][ucCount+1]++;
        uePuschNack[ucSubframe][0]++;
        uePuschNackSum[ucCount+1]++;
        uePuschNackSum[0]++;
    }
}
uint32_t uePucchSr[10]={0};
uint32_t uePucchAck[10]={0};
uint32_t uePucchNack[10]={0};

void UE_PucchSr(uint8_t ucSubframe)
{
    if(  ucSubframe < 10)
    {
        uePucchSr[ucSubframe]++;
    }
}
void UE_PucchAck(uint8_t ucSubframe)
{
    if( ucSubframe < 10)
    {
        uePucchAck[ucSubframe]++;
    }
}
void UE_PucchNack(uint8_t ucSubframe)
{
    if( ucSubframe < 10)
    {
        uePucchNack[ucSubframe]++;
    }
}
uint32_t ueDecodeSucc[10][9]={0};
uint32_t ueDecodeFail[10][9]={0};
uint32_t ueDecodeSuccSum[9]={0};
uint32_t ueDecodeFailSum[9]={0};

void UE_DecodeSucc(uint8_t ucSubframe, uint8_t ucCount)
{
    if( ucCount < 8 && ucSubframe < 10)
    {
        ueDecodeSucc[ucSubframe][ucCount+1]++;
        ueDecodeSucc[ucSubframe][0]++;

        ueDecodeSuccSum[ucCount+1]++;
        ueDecodeSuccSum[0]++;
    }
}
void UE_DecodeFail(uint8_t ucSubframe, uint8_t ucCount)
{
    if( ucCount < 8 && ucSubframe < 10)
    {
        ueDecodeFail[ucSubframe][ucCount+1]++;
        ueDecodeFail[ucSubframe][0]++;

        ueDecodeFailSum[ucCount+1]++;
        ueDecodeFailSum[0]++;
    }
}

void UE_SendPrint()
{
    printf("PUCCH COUNT  %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d==%4d %4d %4d %4d %4d %4d %4d %4d %4d %4d==%4d %4d %4d %4d %4d %4d %4d %4d %4d %4d\n",
        uePucchSr[0], uePucchSr[1],uePucchSr[2],uePucchSr[3],uePucchSr[4],
        uePucchSr[5], uePucchSr[6],uePucchSr[7],uePucchSr[8],uePucchSr[9],
        uePucchAck[0], uePucchAck[1],uePucchAck[2],uePucchAck[3],uePucchAck[4],
        uePucchAck[5], uePucchAck[6],uePucchAck[7],uePucchAck[8],uePucchAck[9],
        uePucchNack[0], uePucchNack[1],uePucchNack[2],uePucchNack[3],uePucchNack[4],
        uePucchNack[5], uePucchNack[6],uePucchNack[7],uePucchNack[8],uePucchNack[9]);
    for(uint8_t ucSubframe = 0; ucSubframe < 10; ucSubframe++)
    {
        uePucchSr[ucSubframe] = 0;
        uePucchAck[ucSubframe] = 0;
        uePucchNack[ucSubframe] = 0;
    }
    for(uint8_t ucCount = 0; ucCount<5; ucCount++)
    {
        printf("ULDCI round%1d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d  HICH %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d\n",
            ucCount,
            ueUlDci[0][ucCount], ueUlDci[1][ucCount],ueUlDci[2][ucCount],ueUlDci[3][ucCount],ueUlDci[4][ucCount],
            ueUlDci[5][ucCount], ueUlDci[6][ucCount],ueUlDci[7][ucCount],ueUlDci[8][ucCount],ueUlDci[9][ucCount],
            ueHich[0][ucCount], ueHich[1][ucCount],ueHich[2][ucCount],ueHich[3][ucCount],ueHich[4][ucCount],
            ueHich[5][ucCount], ueHich[6][ucCount],ueHich[7][ucCount],ueHich[8][ucCount],ueHich[9][ucCount]);
        for(uint8_t ucSubframe = 0; ucSubframe < 10; ucSubframe++)
        {
            ueUlDci[ucSubframe][ucCount] = 0;
            ueHich[ucSubframe][ucCount] = 0;
        }
    }

    for(uint8_t ucCount = 0; ucCount<5; ucCount++)
    {
        float rate1 = 0;
        int ulPushSum = 0;
        if( ucCount == 0)
        {
            ulPushSum = uePuschSchedSum[1];
        }
        else
        {
            ulPushSum = uePuschSchedSum[ucCount];
        }
        if(uePuschSchedSum[ucCount]>0)
        {
            rate1 = (uePuschAckSum[ucCount]*100.0)/ulPushSum;
            
        }
        printf("PUSCH round%1d %8d %8d %6.2f%% == %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d==%4d %4d %4d %4d %4d %4d %4d %4d %4d %4d==%4d %4d %4d %4d %4d %4d %4d %4d %4d %4d\n",
            ucCount,ulPushSum,uePuschAckSum[ucCount],rate1,
            uePuschSched[0][ucCount], uePuschSched[1][ucCount],uePuschSched[2][ucCount],uePuschSched[3][ucCount],uePuschSched[4][ucCount],
            uePuschSched[5][ucCount], uePuschSched[6][ucCount],uePuschSched[7][ucCount],uePuschSched[8][ucCount],uePuschSched[9][ucCount],
            uePuschAck[0][ucCount], uePuschAck[1][ucCount],uePuschAck[2][ucCount],uePuschAck[3][ucCount],uePuschAck[4][ucCount],
            uePuschAck[5][ucCount], uePuschAck[6][ucCount],uePuschAck[7][ucCount],uePuschAck[8][ucCount],uePuschAck[9][ucCount],
            uePuschNack[0][ucCount], uePuschNack[1][ucCount],uePuschNack[2][ucCount],uePuschNack[3][ucCount],uePuschNack[4][ucCount],
            uePuschNack[5][ucCount], uePuschNack[6][ucCount],uePuschNack[7][ucCount],uePuschNack[8][ucCount],uePuschNack[9][ucCount]);
        for(uint8_t ucSubframe = 0; ucSubframe < 10; ucSubframe++)
        {
            uePuschSched[ucSubframe][ucCount] = 0;
            uePuschAck[ucSubframe][ucCount] = 0;
            uePuschNack[ucSubframe][ucCount] = 0;
        }
    }
    for(uint8_t ucCount = 0; ucCount<9; ucCount++)
    {
        float rate2 = 0;
        int ulPdshSum = 0;
        if( ucCount == 0)
        {
            ulPdshSum = (ueDecodeSuccSum[1]+ueDecodeFailSum[1]);
        }
        else
        {
            ulPdshSum = (ueDecodeSuccSum[ucCount]+ueDecodeFailSum[ucCount]);
        }

        if(ueDecodeSuccSum[ucCount]+ueDecodeFailSum[ucCount]>0)
        {
             rate2 = (ueDecodeSuccSum[ucCount]*100.0)/ulPdshSum;
        }
        printf("PDSCH round%1d %8d %8d %6.2f%% == %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d==%4d %4d %4d %4d %4d %4d %4d %4d %4d %4d\n",
            ucCount,ulPdshSum,ueDecodeSuccSum[ucCount],rate2,
            ueDecodeSucc[0][ucCount], ueDecodeSucc[1][ucCount],ueDecodeSucc[2][ucCount],ueDecodeSucc[3][ucCount],ueDecodeSucc[4][ucCount],
            ueDecodeSucc[5][ucCount], ueDecodeSucc[6][ucCount],ueDecodeSucc[7][ucCount],ueDecodeSucc[8][ucCount],ueDecodeSucc[9][ucCount],
            ueDecodeFail[0][ucCount], ueDecodeFail[1][ucCount],ueDecodeFail[2][ucCount],ueDecodeFail[3][ucCount],ueDecodeFail[4][ucCount],
            ueDecodeFail[5][ucCount], ueDecodeFail[6][ucCount],ueDecodeFail[7][ucCount],ueDecodeFail[8][ucCount],ueDecodeFail[9][ucCount]);
        for(uint8_t ucSubframe = 0; ucSubframe < 10; ucSubframe++)
        {
            ueDecodeSucc[ucSubframe][ucCount] = 0;
            ueDecodeFail[ucSubframe][ucCount] = 0;
        }
    }

}
extern uint16_t gusUeAmp;

void phy_procedures_UE_TX(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint8_t abstraction_flag,runmode_t mode,relaying_type_t r_type)
{

  int i;
  uint16_t first_rb, nb_rb;
  uint8_t harq_pid;
  unsigned int input_buffer_length;
  unsigned int aa;
  uint8_t Msg3_flag=0;
  uint8_t pucch_ack_payload[2];
  uint8_t n1_pucch;
  ANFBmode_t bundling_flag;
  PUCCH_FMT_t format;
  uint8_t SR_payload;
  int32_t prach_power;
  uint8_t nsymb;
  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_ue->lte_frame_parms;
  uint8_t generate_ul_signal = 0;
  uint8_t ack_status=0;
  int8_t Po_PUCCH;
  int32_t ulsch_start=0;
  int overflow=0;
  int k,l;
  int slot_tx = phy_vars_ue->slot_tx;
  int subframe_tx = phy_vars_ue->slot_tx>>1;
  int frame_tx = phy_vars_ue->frame_tx;
  int Mod_id = phy_vars_ue->Mod_id;
  int CC_id = phy_vars_ue->CC_id;
  int tx_amp;
  uint8_t printf_flag = 0;

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX,VCD_FUNCTION_IN);

  start_meas(&phy_vars_ue->phy_proc_tx);

  LOG_TX_LINE();
  PERF_TIME_LINE(PERF_UE_phy_procedures_UE_TX, PERF_LINE);


  if ((slot_tx%2)==0) 
  {
      LOG_TX_LINE();
      PERF_TIME_LINE(PERF_UE_phy_procedures_UE_TX, PERF_LINE);

    phy_vars_ue->tx_power_dBm=-127;

    if (abstraction_flag==0) {
      for (aa=0; aa<frame_parms->nb_antennas_tx; aa++) {
        memset(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][subframe_tx*frame_parms->ofdm_symbol_size*frame_parms->symbols_per_tti],
               0,
               frame_parms->ofdm_symbol_size*frame_parms->symbols_per_tti*sizeof(int32_t));
      }
    }

    if (phy_vars_ue->UE_mode[eNB_id] != PRACH) 
    {
      // get harq_pid from subframe relationship
      harq_pid = subframe2harq_pid(&phy_vars_ue->lte_frame_parms,
                                   frame_tx,
                                   subframe_tx);

      if( (subframe_tx == 4) && (frame_tx&0xf)== 0) //xhd_oai_debug UE tx
      {
          phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->printf_flag = 1;
      }
      else
      {
          phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->printf_flag = 0;
      }


      PERF_TIME_LINE(PERF_UE_phy_procedures_UE_TX, PERF_LINE);

      if ((phy_vars_ue->ulsch_ue_Msg3_active[eNB_id] == 1) &&
          (phy_vars_ue->ulsch_ue_Msg3_frame[eNB_id] == frame_tx) &&
          (phy_vars_ue->ulsch_ue_Msg3_subframe[eNB_id] == subframe_tx)) 
      { 
        // Initial Transmission of Msg3

        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_scheduling_flag = 1;

        if (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round==0)
          generate_ue_ulsch_params_from_rar(phy_vars_ue,
                                            eNB_id);
        PERF_TIME_LINE(PERF_UE_phy_procedures_UE_TX, PERF_LINE);

        phy_vars_ue->ulsch_ue[eNB_id]->power_offset = 14;
        LOG_I(PHY,"[UE  %d][RAPROC] Frame %d: Setting Msg3_flag in subframe %d, for harq_pid %d\n",
              Mod_id,
              frame_tx,
              subframe_tx,
              harq_pid);
        Msg3_flag = 1;
      } else {

        if (harq_pid==255) {
          LOG_E(PHY,"[UE%d] Frame %d ulsch_decoding.c: FATAL ERROR: illegal harq_pid, returning\n",
                Mod_id,frame_tx);
          mac_xface->macphy_exit("Error in ulsch_decoding");
          VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX, VCD_FUNCTION_OUT);
          stop_meas(&phy_vars_ue->phy_proc_tx);
          return;
        }

        Msg3_flag=0;
      }

      PERF_TIME_LINE(PERF_UE_phy_procedures_UE_TX, PERF_LINE);

      if (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_scheduling_flag == 1) 
      {

        generate_ul_signal = 1;
        PERF_TIME_LINE(PERF_UE_phy_procedures_UE_TX, PERF_LINE);

        // deactivate service request
        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->subframe_scheduling_flag = 0;

        ack_status = get_ack(&phy_vars_ue->lte_frame_parms,
                             phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack,
                             subframe_tx,
                             phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->o_ACK);

        first_rb = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->first_rb;
        nb_rb = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->nb_rb;

        PERF_TIME_LINE(PERF_UE_phy_procedures_UE_TX, PERF_LINE);


        //frame_parms->pusch_config_c ommon.ul_ReferenceSignalsPUSCH.cyclicShift = 0;

        //frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.nPRS[20] = 0;
        LOG_D(PHY,
              "[UE  %d][PUSCH %d] Frame %d subframe %d Generating PUSCH : first_rb %d, nb_rb %d, round %d, mcs %d, rv %d, cyclic_shift %d (cyclic_shift_common %d,n_DMRS2 %d,n_PRS %d), ACK (%d,%d), O_ACK %d\n",
              Mod_id,harq_pid,frame_tx,subframe_tx,
              first_rb,nb_rb,
              phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round,
              phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->mcs,
              phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->rvidx,
              (frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift+
               phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->n_DMRS2+
               frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.nPRS[slot_tx])%12,
              frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.cyclicShift,
              phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->n_DMRS2,
              frame_parms->pusch_config_common.ul_ReferenceSignalsPUSCH.nPRS[slot_tx],
              phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->o_ACK[0],
              phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->o_ACK[1],
              phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->O_ACK);

        if (ack_status > 0) {
          LOG_D(PHY,"[UE  %d][PDSCH %x] Frame %d subframe %d Generating ACK (%d,%d) for %d bits on PUSCH\n",
                Mod_id,
                phy_vars_ue->ulsch_ue[eNB_id]->rnti,
                frame_tx,subframe_tx,
                phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->o_ACK[0],
                phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->o_ACK[1],
                phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->O_ACK);
        }

        LOG_TX_LINE();
        PERF_TIME_LINE(PERF_UE_phy_procedures_UE_TX, PERF_LINE);



        //#ifdef DEBUG_PHY_PROC
        //  debug_LOG_D(PHY,"[UE  %d] Frame %d, Subframe %d ulsch harq_pid %d : O %d, O_ACK %d, O_RI %d, TBS %d\n",Mod_id,phy_vars_ue->frame,subframe_tx,harq_pid,phy_vars_ue->ulsch_ue[eNB_id]->O,phy_vars_ue->ulsch_ue[eNB_id]->O_ACK,phy_vars_ue->ulsch_ue[eNB_id]->O_RI,phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS);
        //#endif
        if (Msg3_flag == 1) 
        {
            //printf_flag = 1;
            LOG_I(PHY,"[UE  %d][RAPROC] Frame %d, Subframe %d, next slot %d  harq_pid=%d Generating (RRCConnectionRequest) Msg3 (nb_rb %d, first_rb %d round %d, rvidx %d %d %d) Msg3: %x.%x.%x|%x.%x.%x.%x.%x.%x\n",
                Mod_id,frame_tx, subframe_tx, slot_tx,harq_pid,
                phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->nb_rb,
                phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->first_rb,
                phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round,
                phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->rvidx,
                phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->DciNew,
                phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->HichNew,
                phy_vars_ue->prach_resources[eNB_id]->Msg3[0],
                phy_vars_ue->prach_resources[eNB_id]->Msg3[1],
                phy_vars_ue->prach_resources[eNB_id]->Msg3[2],
                phy_vars_ue->prach_resources[eNB_id]->Msg3[3],
                phy_vars_ue->prach_resources[eNB_id]->Msg3[4],
                phy_vars_ue->prach_resources[eNB_id]->Msg3[5],
                phy_vars_ue->prach_resources[eNB_id]->Msg3[6],
                phy_vars_ue->prach_resources[eNB_id]->Msg3[7],
                phy_vars_ue->prach_resources[eNB_id]->Msg3[8]);


            if (ulsch_encoding(phy_vars_ue->prach_resources[eNB_id]->Msg3,
                               phy_vars_ue,
                               harq_pid,
                               eNB_id,
                               phy_vars_ue->transmission_mode[eNB_id],0,0)!=0) 
            {
              LOG_E(PHY,"ulsch_coding.c: FATAL ERROR: returning\n");
              mac_xface->macphy_exit("Error in ulsch_coding");
              VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX, VCD_FUNCTION_OUT);
              stop_meas(&phy_vars_ue->phy_proc_tx);
              return;
            }

 
            LOG_TX_LINE();
            PERF_TIME_LINE(PERF_UE_phy_procedures_UE_TX, PERF_LINE);

            // signal MAC that Msg3 was sent
            mac_xface->Msg3_transmitted(Mod_id,
                                      CC_id,
                                      frame_tx,
                                      eNB_id);
            phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->sdu_len = 9;
            phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->msg3_flag = 1;
        }
        else 
        {
          input_buffer_length = phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS/8;


          //  LOG_D(PHY,"[UE  %d] ULSCH : Searching for MAC SDUs\n",Mod_id);
          if (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round==0) 
          {
            //if (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->calibration_flag == 0) {
            access_mode=SCHEDULED_ACCESS;
            mac_xface->ue_get_sdu(Mod_id,
                                  CC_id,
                                  frame_tx,
                                  subframe_tx,
                                  eNB_id,
                                  ulsch_input_buffer[harq_pid],
                                  input_buffer_length,
                                  &access_mode,
                                  &phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->sdu_len,
                                  &phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->offset
                                  );
            phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->msg3_flag = 0;

            my_printf("phy_procedures_UE_TX: frame=%d subframe=%d harq_pid=%d new get sdu len=%d\n", 
                frame_tx, subframe_tx, harq_pid, phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->sdu_len);

          }
          //xhd_oai_miss UE ULSCH 
          UE_SendCount(subframe_tx,phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round);

          if( phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->sdu_len != 0 )
          {
              printf_flag = 1;
              
          }
          else
          {
              ulsch_input_buffer[harq_pid][5]=0;
              ulsch_input_buffer[harq_pid][6]=0;
              ulsch_input_buffer[harq_pid][7]=0;
              #if 0
              ulsch_input_buffer[harq_pid][10]=(frame_tx/1000)%10;
              ulsch_input_buffer[harq_pid][11]=(frame_tx/100)%10;
              ulsch_input_buffer[harq_pid][12]=(frame_tx/10)%10;
              ulsch_input_buffer[harq_pid][13]=(frame_tx)%10;
              ulsch_input_buffer[harq_pid][14]=subframe_tx;
              ulsch_input_buffer[harq_pid][15]=harq_pid;
              ulsch_input_buffer[harq_pid][16]=phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round;
              #endif
          }


          start_meas(&phy_vars_ue->ulsch_encoding_stats);
          
          LOG_TX_LINE();
          PERF_TIME_LINE(PERF_UE_phy_procedures_UE_TX, PERF_LINE);

            if (ulsch_encoding(ulsch_input_buffer[harq_pid],
                               phy_vars_ue,
                               harq_pid,
                               eNB_id,
                               phy_vars_ue->transmission_mode[eNB_id],0,
                               0)!=0) {  //  Nbundled, to be updated!!!!
              LOG_E(PHY,"ulsch_coding.c: FATAL ERROR: returning\n");
              VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX, VCD_FUNCTION_OUT);
              stop_meas(&phy_vars_ue->phy_proc_tx);
              return;
            }
         
            LOG_TX_LINE();
            PERF_TIME_LINE(PERF_UE_phy_procedures_UE_TX, PERF_LINE);

            stop_meas(&phy_vars_ue->ulsch_encoding_stats);
        }
        LOG_TX_LINE();
        PERF_TIME_LINE(PERF_UE_phy_procedures_UE_TX, PERF_LINE);

#ifdef OPENAIR2
          pusch_power_cntl(phy_vars_ue,subframe_tx,eNB_id,1, abstraction_flag);
          phy_vars_ue->tx_power_dBm = phy_vars_ue->ulsch_ue[eNB_id]->Po_PUSCH;
#else
          phy_vars_ue->tx_power_dBm = UE_TX_POWER;
#endif
          phy_vars_ue->tx_total_RE = nb_rb*12;
	  
#if defined(EXMIMO) || defined(OAI_USRP)
	  tx_amp = get_tx_amp(phy_vars_ue->tx_power_dBm,
			      phy_vars_ue->tx_power_max_dBm,
			      phy_vars_ue->lte_frame_parms.N_RB_UL,
			      nb_rb);
#else
          tx_amp = AMP;
#endif
      //printf("phy_vars_ue->tx_power_dBm:%d UE_TX_POWER:%d nb_rb=%d tx_amp=%d AMP=%d",
      //  phy_vars_ue->tx_power_dBm,UE_TX_POWER,nb_rb,tx_amp,AMP);
      //tx_amp = 1024;
      LOG_TX_LINE();
      PERF_TIME_LINE(PERF_UE_phy_procedures_UE_TX, PERF_LINE);

      if (Msg3_flag == 1) 
      {

          /*
          tx_amp = 350;//512;
          if( gusUeAmp != 0)
          {
            tx_amp = gusUeAmp;
          }*/
          
          LOG_W(PHY,"[UE] Frame %d, subframe %d : ULSCH SDU (TX harq_pid %d )  MSG3 tx_power_dBm=%d tx_amp=%d: \n",
               frame_tx,subframe_tx,harq_pid,
               phy_vars_ue->tx_power_dBm,tx_amp); 
          print_buff(phy_vars_ue->prach_resources[eNB_id]->Msg3,9,0,"[UE] Frame %d, subframe %d : ULSCH SDU (TX harq_pid %d )  MSG3 tx_power_dBm=%d/%d tx_amp=%d: \n",
               frame_tx,subframe_tx,harq_pid,
               phy_vars_ue->tx_power_dBm,
               phy_vars_ue->tx_power_max_dBm,
               tx_amp); 
          print_info("[UE] Frame %d, subframe %d : ULSCH SDU (TX harq_pid %d )  MSG3 tx_power_dBm=%d/%d tx_amp=%d: \n",
               frame_tx,subframe_tx,harq_pid,
               phy_vars_ue->tx_power_dBm,
               phy_vars_ue->tx_power_max_dBm,
               tx_amp); 

      }
      else
      {
          /*
          if( phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round == 0)
          {
              tx_amp = 512;
          }
          else if( phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round == 1)
          {
              tx_amp = 512;
          }
          else if( phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round == 2)
          {
              tx_amp = 512;
          }
          else
          {
              tx_amp = 512;
          }
          tx_amp = 350;//
          if( gusUeAmp != 0)
          {
            tx_amp = gusUeAmp;
          }*/
          
          /*RX GAIN=110  TX GAIN最大值为89.75，超过这个值是没有意义的
             AMP    TG=98 TG=99   TG=100  TG=101  TG=102  TG=103
             240                                    58     FAIL
             230           64       59      59      70
             220            9       74      73      62     70
             210                    73      75      60     FAIL
             200                    65      77      73      6
             190                            69      60      4
             */
          //if( printf_flag )
          {
              LOG_D(PHY,"[UE] Frame %d, subframe %d : ULSCH SDU (TX harq_pid %d round%d)  (%d-%d bytes) tx_power_dBm=%d tx_amp=%d rb %d-%d: timing_advance=%d/%d/%d/%d\n",
                   frame_tx,subframe_tx,harq_pid, 
                   phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round,
                   input_buffer_length, 
                   phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->sdu_len,
                   phy_vars_ue->tx_power_dBm,
                   tx_amp,
                   phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->first_rb,
                   phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->nb_rb,
                   phy_vars_ue->timing_advance,
                   openair_daq_vars.timing_advance,
                   phy_vars_ue->N_TA_offset,
                   phy_vars_ue->rx_offset);

                
                //if (phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round==0) 
                {
                    //for (int i=0; i<input_buffer_length; i++)
                    //  printf("%x.",ulsch_input_buffer[harq_pid][i]);
                  
                    //printf("\n");
                }
                //if( (subframe_tx == 4) && (frame_tx&0xf)== 0) //xhd_oai_debug UE TX
                {
                    print_buff(&ulsch_input_buffer[harq_pid][0], input_buffer_length, 0, 
                       "[UE] Frame %d, subframe %d : ULSCH SDU (TX harq_pid %d round%d)  (%d-%d bytes) tx_power_dBm=%d/%d tx_amp=%d rb %d-%d: timing_advance=%d/%d/%d/%d\n",
                       frame_tx,subframe_tx,harq_pid, 
                       phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round,
                       input_buffer_length, 
                       phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->sdu_len,
                       phy_vars_ue->tx_power_dBm,phy_vars_ue->tx_power_max_dBm,
                       tx_amp,
                       phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->first_rb,
                       phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->nb_rb,
                       phy_vars_ue->timing_advance,
                       openair_daq_vars.timing_advance,
                       phy_vars_ue->N_TA_offset,
                       phy_vars_ue->rx_offset );
                    print_info(
                       "[UE] Frame %d, subframe %d : ULSCH SDU (TX harq_pid %d round%d)  (%d-%d bytes) tx_power_dBm=%d/%d tx_amp=%d rb %d-%d: timing_advance=%d/%d/%d/%d\n",
                       frame_tx,subframe_tx,harq_pid, 
                       phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round,
                       input_buffer_length, 
                       phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->sdu_len,
                       phy_vars_ue->tx_power_dBm,phy_vars_ue->tx_power_max_dBm,
                       tx_amp,
                       phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->first_rb,
                       phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->nb_rb,
                       phy_vars_ue->timing_advance,
                       openair_daq_vars.timing_advance,
                       phy_vars_ue->N_TA_offset,
                       phy_vars_ue->rx_offset );
                }
          }
      }
      LOG_TX_LINE();
      PERF_TIME_LINE(PERF_UE_phy_procedures_UE_TX, PERF_LINE);

	  LOG_D(PHY,"[UE  %d][PUSCH %d] Frame %d subframe %d, generating PUSCH, Po_PUSCH: %d dBm (max %d dBm), amp %d\n",
		Mod_id,harq_pid,frame_tx,subframe_tx,phy_vars_ue->tx_power_dBm,phy_vars_ue->tx_power_max_dBm, tx_amp);
          start_meas(&phy_vars_ue->ulsch_modulation_stats);
          ulsch_modulation(phy_vars_ue->lte_ue_common_vars.txdataF,
			   tx_amp,
                           frame_tx,
                           subframe_tx,
                           &phy_vars_ue->lte_frame_parms,
                           phy_vars_ue->ulsch_ue[eNB_id],Mod_id);
          for (aa=0; aa<1/*frame_parms->nb_antennas_tx*/; aa++)
            generate_drs_pusch(phy_vars_ue,
			       eNB_id,
			       tx_amp,
			       subframe_tx,
			       first_rb,
			       nb_rb,
			       aa);

          stop_meas(&phy_vars_ue->ulsch_modulation_stats);

      } // ULSCH is active

#ifdef PUCCH
      else if (phy_vars_ue->UE_mode[eNB_id] == PUSCH) 
      { 
        // check if we need to use PUCCH 1a/1b
        //      debug_LOG_D(PHY,"[UE%d] Frame %d, subframe %d: Checking for PUCCH 1a/1b\n",Mod_id,frame_tx,subframe_tx);
        bundling_flag = phy_vars_ue->pucch_config_dedicated[eNB_id].tdd_AckNackFeedbackMode;

        if ((frame_parms->frame_type==FDD) ||
            (bundling_flag==bundling)    ||
            ((frame_parms->frame_type==TDD)&&(frame_parms->tdd_config==1)&&((slot_tx!=4)||(slot_tx!=14)))) {
          format = pucch_format1a;
          LOG_D(PHY,"[UE] PUCCH 1a\n");
        } else {
          format = pucch_format1b;
          LOG_D(PHY,"[UE] PUCCH 1b\n");
        }

        // Check for SR and do ACK/NACK accordingly
        if (is_SR_TXOp(phy_vars_ue,eNB_id,subframe_tx)==1) 
        {
          LOG_D(PHY,"[UE %d][SR %x] Frame %d subframe %d: got SR_TXOp, Checking for SR for PUSCH from MAC\n",
                Mod_id,phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,frame_tx,subframe_tx);
#ifdef OPENAIR2
          SR_payload = mac_xface->ue_get_SR(Mod_id,
                                            CC_id,
                                            frame_tx,
                                            eNB_id,
                                            phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,
                                            subframe_tx); // subframe used for meas gap
#else
          SR_payload = 1;
#endif

          if (SR_payload>0) {
            generate_ul_signal = 1;
            LOG_D(PHY,"[UE %d][SR %x] Frame %d subframe %d got the SR for PUSCH is %d\n",
                  Mod_id,phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,frame_tx,subframe_tx,SR_payload);
          } else {
            phy_vars_ue->sr[subframe_tx]=0;
          }
        } 
        else
        {
          SR_payload=0;
        }

        if (get_ack(&phy_vars_ue->lte_frame_parms,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack,
                    subframe_tx,pucch_ack_payload) > 0) 
        {
          // we need to transmit ACK/NAK in this subframe

          generate_ul_signal = 1;

       
          n1_pucch = get_n1_pucch(phy_vars_ue,
                                  eNB_id,
                                  subframe_tx,
                                  pucch_ack_payload,
                                  SR_payload);

#ifdef OPENAIR2
          Po_PUCCH = pucch_power_cntl(phy_vars_ue,subframe_tx,eNB_id,format);
          phy_vars_ue->tx_power_dBm = Po_PUCCH;
#else
          phy_vars_ue->tx_power_dBm = UE_TX_POWER;
#endif
          phy_vars_ue->tx_total_RE = 12;

#if defined(EXMIMO) || defined(OAI_USRP)
	      tx_amp = get_tx_amp(Po_PUCCH,
			      phy_vars_ue->tx_power_max_dBm,
			      phy_vars_ue->lte_frame_parms.N_RB_UL,
			      1);
#else
	      tx_amp = AMP;
#endif
         //xhd_oai_ue
          //tx_amp = 512;

          if( gusUeAmp != 0)
          {
            //tx_amp = gusUeAmp;
          }
          if( tx_amp < 200 )
          {
            tx_amp = 200;
          }
          

          if(pucch_ack_payload[0] == 1)
          {
              UE_PucchAck(subframe_tx);
          }
          else
          {
              UE_PucchNack(subframe_tx);
          }
          if(pucch_ack_payload[1] == 1)
          {
              UE_PucchAck(subframe_tx);
          }
          else
          {
              UE_PucchNack(subframe_tx);
          }
          
          if (SR_payload>0) 
          {
             /*ACK同时有SR请求*/
    	     print_info("[UE  %d][SR %x] Frame %d subframe %d Generating PUCCH 1a/1b payload %d,%d (with SR for PUSCH), n1_pucch %d, Po_PUCCH %d, amp %d format=%d sr_ConfigIndex=%d\n",
                  Mod_id,
                  phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
                  frame_tx, subframe_tx,
		          pucch_ack_payload[0],pucch_ack_payload[1],
                  phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex,
    	          Po_PUCCH,
    		      tx_amp,
    		      format,
                  phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex);
          } 
          else 
          {
            /*只有ACK*/
            print_info("[UE  %d][PDSCH %x] Frame %d subframe %d Generating PUCCH 1a/1b, n1_pucch %d, b[0]=%d,b[1]=%d (SR_Payload %d), Po_PUCCH %d, amp %d format=%d sr_ConfigIndex=%d\n",
                  Mod_id,
                  phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
                  frame_tx, subframe_tx,
                  n1_pucch,pucch_ack_payload[0],pucch_ack_payload[1],SR_payload,
                  Po_PUCCH,
        		  tx_amp,
        		  format,
                  phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex);
    	  }


    	  generate_pucch(phy_vars_ue->lte_ue_common_vars.txdataF,
                           &phy_vars_ue->lte_frame_parms,
                           phy_vars_ue->ncs_cell,
                           format,
                           &phy_vars_ue->pucch_config_dedicated[eNB_id],
                           n1_pucch,
                           0,  // n2_pucch
                           1,  // shortened format
                           pucch_ack_payload,
	                       tx_amp,
	                       subframe_tx);

        }
        else if (SR_payload==1) 
        { 
          // no ACK/NAK but SR is triggered by MAC

#ifdef OPENAIR2
          Po_PUCCH = pucch_power_cntl(phy_vars_ue,subframe_tx,eNB_id,pucch_format1);
          phy_vars_ue->tx_power_dBm = Po_PUCCH;
#else
          phy_vars_ue->tx_power_dBm = UE_TX_POWER;
#endif
          phy_vars_ue->tx_total_RE = 12;

#if defined(EXMIMO) || defined(OAI_USRP)
          tx_amp =  get_tx_amp(Po_PUCCH,
	                       phy_vars_ue->tx_power_max_dBm,
	                       phy_vars_ue->lte_frame_parms.N_RB_UL,
	                       1);
#else
	      tx_amp = AMP;
#endif
          //xhd_oai_ue
          //tx_amp = 512;
          
          if( gusUeAmp != 0)
          {
            //tx_amp = gusUeAmp;
          }
          if( tx_amp < 200 )
          {
            tx_amp = 200;
          }
          print_info("[UE  %d][SR %x] Frame %d subframe %d Generating PUCCH 1 (SR for PUSCH), n1_pucch %d, Po_PUCCH %d tx_amp=%d sr_ConfigIndex=%d\n",
                Mod_id,
                phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
                frame_tx, subframe_tx,
                phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex,
                Po_PUCCH,tx_amp,
                phy_vars_ue->scheduling_request_config[eNB_id].sr_ConfigIndex);

            UE_PucchSr(subframe_tx);

            generate_pucch(phy_vars_ue->lte_ue_common_vars.txdataF,
                           &phy_vars_ue->lte_frame_parms,
                           phy_vars_ue->ncs_cell,
                           pucch_format1,
                           &phy_vars_ue->pucch_config_dedicated[eNB_id],
                           phy_vars_ue->scheduling_request_config[eNB_id].sr_PUCCH_ResourceIndex,
                           0,  // n2_pucch
                           1,  // shortened format
                           pucch_ack_payload,  // this is ignored anyway, we just need a pointer
	                       tx_amp,
                           subframe_tx);
        }
      }

#endif  // PUCCH


        nsymb = (frame_parms->Ncp == 0) ? 14 : 12;

#if defined(EXMIMO) || defined(OAI_USRP) //this is the EXPRESS MIMO case
        ulsch_start = (phy_vars_ue->rx_offset+subframe_tx*frame_parms->samples_per_tti-
                       openair_daq_vars.timing_advance-
                       phy_vars_ue->timing_advance-
                       phy_vars_ue->N_TA_offset+5)%(LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*frame_parms->samples_per_tti);
#else //this is the normal case
        ulsch_start = (frame_parms->samples_per_tti*subframe_tx)-phy_vars_ue->N_TA_offset; //-phy_vars_ue->timing_advance;
#endif //else EXMIMO

    	if ((frame_tx%100) == 0)
    	{
    	  LOG_D(PHY,"[UE %d] Frame %d, subframe %d: ulsch_start = %d (rxoff %d, HW TA %d, timing advance %d, TA_offset %d\n",
    		Mod_id,frame_tx,subframe_tx,
    		ulsch_start,
    		phy_vars_ue->rx_offset,
    		openair_daq_vars.timing_advance,
    		phy_vars_ue->timing_advance,
    		phy_vars_ue->N_TA_offset);
    	}

        if (generate_ul_signal == 1 ) 
        {



          start_meas(&phy_vars_ue->ofdm_mod_stats);

          for (aa=0; aa<frame_parms->nb_antennas_tx; aa++) {
            if (frame_parms->Ncp == 1)
              PHY_ofdm_mod(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][subframe_tx*nsymb*frame_parms->ofdm_symbol_size],
                           dummy_tx_buffer[subframe_tx],
                           frame_parms->log2_symbol_size,
                           nsymb,
                           frame_parms->nb_prefix_samples,
                           CYCLIC_PREFIX);
            else
              normal_prefix_mod(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][subframe_tx*nsymb*frame_parms->ofdm_symbol_size],
                                dummy_tx_buffer[subframe_tx],
                                nsymb,
                                &phy_vars_ue->lte_frame_parms);

            for (l=0; l<frame_parms->samples_per_tti; l++) 
            {
              ((short*)dummy_tx_buffer[subframe_tx])[2*l] = ((short*)(dummy_tx_buffer[subframe_tx]))[2*l]<<4;
              ((short*)dummy_tx_buffer[subframe_tx])[2*l+1] = ((short*)(dummy_tx_buffer[subframe_tx]))[2*l+1]<<4;
            }

            apply_7_5_kHz2(phy_vars_ue,dummy_tx_buffer[subframe_tx],dummy_tx_buffer2[subframe_tx],0);
            apply_7_5_kHz2(phy_vars_ue,dummy_tx_buffer[subframe_tx],dummy_tx_buffer2[subframe_tx],1);

            //remove_7_5_kHz2(dummy_tx_buffer2[subframe_tx],dummy_tx_buffer3[subframe_tx],
            //    frame_parms->N_RB_UL,phy_vars_ue->lte_frame_parms.samples_per_tti);
            
            //apply_7_5_kHz(phy_vars_ue,dummy_tx_buffer[subframe_tx],0);
            //apply_7_5_kHz(phy_vars_ue,dummy_tx_buffer[subframe_tx],1);
            if(phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->msg3_flag)
            {
                /*
                    print_frame25FT3(&phy_vars_ue->lte_ue_common_vars.txdataF[aa][subframe_tx*nsymb*frame_parms->ofdm_symbol_size],
                                 dummy_tx_buffer[subframe_tx],dummy_tx_buffer[subframe_tx],frame_tx,subframe_tx);
                    */
                uint8_t subframe_prev = (subframe_tx-1)%10;
                print_info("====>phy_procedures_UE_TX rnti=%x send Msg3=%d frame=%d subframe=%d harq_pid=%d round=%d first_rb=%d nb_rb=%d mcs=%d TBS=%d rv=%d tpc=%d tx_amp=%d\n",
                    phy_vars_ue->ulsch_ue[eNB_id]->rnti,
                    ulsch_start,frame_tx,subframe_tx,harq_pid,
                    phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round,
                    phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->first_rb,
                    phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->nb_rb,
                    phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->mcs,
                    phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS>>3,
                    phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->rvidx,
                    phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TPC,
                    tx_amp);
                //print_frame25FT33(dummy_tx_buffer[subframe_tx],dummy_tx_buffer2[subframe_tx],dummy_tx_buffer3[subframe_tx],frame_tx,subframe_tx);
                //print_frame25FT7(dummy_tx_buffer[subframe_tx],frame_tx,subframe_tx,5,5);
                print_frame25FT7(dummy_tx_buffer2[subframe_tx],frame_tx,subframe_tx,5,5);
                //print_frame25FT7(dummy_tx_buffer3[subframe_tx],frame_tx,subframe_tx,5,5);
                //print_frame25FT8(dummy_tx_buffer[subframe_tx],frame_tx,subframe_tx,5,5);
                
            }
            else if(printf_flag )
            {
                
                //if( (subframe_tx == 4) && (frame_tx&0xf)== 0) //xhd_oai_debug UE TX
                //if(phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->O_ACK)//xhd_oai_debug UE TX
                {
                    print_info("====>phy_procedures_UE_TX rnti=%x send data=%d frame=%d subframe=%d harq_pid=%d round=%d first_rb=%d nb_rb=%d mcs=%d TBS=%d rv=%d tpc=%d tx_amp=%d\n",
                        phy_vars_ue->ulsch_ue[eNB_id]->rnti,
                        ulsch_start,frame_tx,subframe_tx,harq_pid,
                        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round,
                        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->first_rb,
                        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->nb_rb,
                        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->mcs,
                        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS>>3,
                        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->rvidx,
                        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TPC,
                        tx_amp);
                     print_frame25FT7(dummy_tx_buffer2[subframe_tx],frame_tx,subframe_tx,5,5);
                }
            }
            else
            {
                //if( (subframe_tx == 4) && (frame_tx&0xf)== 0) 
                //if(phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->O_ACK)//xhd_oai_debug UE TX
                {
                    print_info("\n====>phy_procedures_UE_TX rnti=%x send null data=%d frame=%d subframe=%d harq_pid=%d round=%d first_rb=%d nb_rb=%d mcs=%d TBS=%d rv=%d tpc=%d tx_amp=%d\n",
                        phy_vars_ue->ulsch_ue[eNB_id]->rnti,
                        ulsch_start,frame_tx,subframe_tx,harq_pid,
                        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round,
                        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->first_rb,
                        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->nb_rb,
                        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->mcs,
                        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TBS>>3,
                        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->rvidx,
                        phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->TPC,
                        tx_amp);
                    print_frame25FT7(dummy_tx_buffer2[subframe_tx],frame_tx,subframe_tx,5,5);
                }
            }
            phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->printf_flag = printf_flag;


            overflow = ulsch_start - 9*frame_parms->samples_per_tti;

            //if ((slot_tx==4) && (aa==0)) printf("ulsch_start %d, overflow %d\n",ulsch_start,overflow);
            #if 0
            
            for (k=ulsch_start,l=0; k<cmin(frame_parms->samples_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME,ulsch_start+frame_parms->samples_per_tti); k++,l++) {
              ((short*)phy_vars_ue->lte_ue_common_vars.txdata[aa])[2*k] = ((short*)(dummy_tx_buffer2[subframe_tx]))[2*l]<<4;
              ((short*)phy_vars_ue->lte_ue_common_vars.txdata[aa])[2*k+1] = ((short*)(dummy_tx_buffer2[subframe_tx]))[2*l+1]<<4;
            }

            for (k=0; k<overflow; k++,l++) {
              ((short*)phy_vars_ue->lte_ue_common_vars.txdata[aa])[2*k] = ((short*)(dummy_tx_buffer2[subframe_tx]))[2*l]<<4;
              ((short*)phy_vars_ue->lte_ue_common_vars.txdata[aa])[2*k+1] = ((short*)(dummy_tx_buffer2[subframe_tx]))[2*l+1]<<4;
            }
            #endif
            for (k=ulsch_start,l=0; k<cmin(frame_parms->samples_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME,ulsch_start+frame_parms->samples_per_tti); k++,l++) {
              ((short*)phy_vars_ue->lte_ue_common_vars.txdata[aa])[2*k] = ((short*)(dummy_tx_buffer2[subframe_tx]))[2*l];
              ((short*)phy_vars_ue->lte_ue_common_vars.txdata[aa])[2*k+1] = ((short*)(dummy_tx_buffer2[subframe_tx]))[2*l+1];
            }

            for (k=0; k<overflow; k++,l++) {
              ((short*)phy_vars_ue->lte_ue_common_vars.txdata[aa])[2*k] = ((short*)(dummy_tx_buffer2[subframe_tx]))[2*l];
              ((short*)phy_vars_ue->lte_ue_common_vars.txdata[aa])[2*k+1] = ((short*)(dummy_tx_buffer2[subframe_tx]))[2*l+1];
            }
            
            if(phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->msg3_flag)
            {
                //print_frame25FT7(&phy_vars_ue->lte_ue_common_vars.txdata[aa][ulsch_start],frame_tx,subframe_tx,5,5);

                /*
                if( ulsch_start < 15000)
                {
                    print_buff(&phy_vars_ue->lte_ue_common_vars.txdata[aa][ulsch_start],frame_parms->samples_per_tti,0,
                        "rx_offset=%d TA=%d-%d-%d ulsch_start=%d overflow=%d samples_per_tti=%d\n",
                        phy_vars_ue->rx_offset,phy_vars_ue->timing_advance,
                       openair_daq_vars.timing_advance,
                       phy_vars_ue->N_TA_offset,ulsch_start,overflow,frame_parms->samples_per_tti);
                }*/
            }
          } //nb_antennas_tx

          stop_meas(&phy_vars_ue->ofdm_mod_stats);
        } // generate_ul_signal == 1
        else 
        {  
          // no uplink so clear signal buffer instead
          overflow = ulsch_start - 9*frame_parms->samples_per_tti;

          for (aa=0; aa<frame_parms->nb_antennas_tx; aa++) 
          {
            //memset(&phy_vars_ue->lte_ue_common_vars.txdata[aa][ulsch_start],0,frame_parms->samples_per_tti<<2);
            uint32_t ulLength = cmin(frame_parms->samples_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME,
                                        ulsch_start+frame_parms->samples_per_tti);
            
            memset(&phy_vars_ue->lte_ue_common_vars.txdata[aa][ulsch_start],0,ulLength<<2);

            if( overflow > 0)
            {
                memset(&phy_vars_ue->lte_ue_common_vars.txdata[aa][0],0,overflow<<2);
            }

          }
          if( subframe_tx == 0)
          my_printf(" phy_procedures_UE_TX rnti=%x no data=%d frame=%d subframe=%d\n",
              phy_vars_ue->ulsch_ue[eNB_id]->rnti,
              ulsch_start,frame_tx,subframe_tx);
          
        }
    } // mode != PRACH

    //  }// slot_tx is even
    //  else {  // slot_tx is odd, do the PRACH here


    else if ((phy_vars_ue->UE_mode[eNB_id] == PRACH) 
        && (phy_vars_ue->lte_frame_parms.prach_config_common.prach_Config_enabled==1)) 
    {



      // check if we have PRACH opportunity
      if (is_prach_subframe(&phy_vars_ue->lte_frame_parms,frame_tx,subframe_tx)) 
      {

        phy_vars_ue->generate_prach=0;

        // ask L2 for RACH transport
        if ((mode != rx_calib_ue) && (mode != rx_calib_ue_med) && (mode != rx_calib_ue_byp) && (mode != no_L2_connect) ) 
        {
          phy_vars_ue->prach_resources[eNB_id] = mac_xface->ue_get_rach(Mod_id,
                                                 CC_id,
                                                 frame_tx,
                                                 eNB_id,
                                                 subframe_tx);
          //    LOG_I(PHY,"Got prach_resources for eNB %d address %d, RRCCommon %d\n",eNB_id,phy_vars_ue->prach_resources[eNB_id],UE_mac_inst[Mod_id].radioResourceConfigCommon);
        }

        if (phy_vars_ue->prach_resources[eNB_id] != NULL) 
        {

            printf(" UE_TX PRACH Generate succ frame=%d subframe=%d\n", frame_tx,subframe_tx);

            phy_vars_ue->generate_prach=1;
            phy_vars_ue->prach_cnt=0;


            LOG_I(PHY,"[UE  %d][RAPROC] Frame %d, Subframe %d : Generating PRACH, preamble %d, TARGET_RECEIVED_POWER %d dBm, PRACH TDD Resource index %d, RA-RNTI %d\n",
                  Mod_id,
                  frame_tx,
                  subframe_tx,
                  phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex,
                  phy_vars_ue->prach_resources[eNB_id]->ra_PREAMBLE_RECEIVED_TARGET_POWER,
                  phy_vars_ue->prach_resources[eNB_id]->ra_TDD_map_index,
                  phy_vars_ue->prach_resources[eNB_id]->ra_RNTI);

	        phy_vars_ue->tx_power_dBm = phy_vars_ue->prach_resources[eNB_id]->ra_PREAMBLE_RECEIVED_TARGET_POWER+get_PL(Mod_id,CC_id,eNB_id);

            phy_vars_ue->tx_total_RE = 96;

#if defined(EXMIMO) || defined(OAI_USRP)
            phy_vars_ue->lte_ue_prach_vars[eNB_id]->amp = get_tx_amp(phy_vars_ue->tx_power_dBm,
								     phy_vars_ue->tx_power_max_dBm,
								     phy_vars_ue->lte_frame_parms.N_RB_UL,
								     6);
#else
            phy_vars_ue->lte_ue_prach_vars[eNB_id]->amp = AMP;
#endif
    	    if ((mode == calib_prach_tx) && (((phy_vars_ue->frame_tx&0xfffe)%100)==0))
    	      LOG_D(PHY,"[UE  %d][RAPROC] Frame %d, Subframe %d : PRACH TX power %d dBm, amp %d\n",Mod_id,phy_vars_ue->frame_rx,phy_vars_ue->slot_tx>>1,phy_vars_ue->tx_power_dBm,
    		    phy_vars_ue->lte_ue_prach_vars[eNB_id]->amp);


            //      start_meas(&phy_vars_ue->tx_prach);
            VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GENERATE_PRACH, VCD_FUNCTION_IN);
            prach_power = generate_prach(phy_vars_ue,eNB_id,subframe_tx,frame_tx);
            VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GENERATE_PRACH, VCD_FUNCTION_OUT);
            //      stop_meas(&phy_vars_ue->tx_prach);
            LOG_D(PHY,"[UE  %d][RAPROC] PRACH PL %d dB, power %d dBm, digital power %d dB (amp %d)\n",
                  Mod_id,
                  get_PL(Mod_id,CC_id,eNB_id),
                  phy_vars_ue->tx_power_dBm,
                  dB_fixed(prach_power),
                  phy_vars_ue->lte_ue_prach_vars[eNB_id]->amp);

            LOG_I(PHY,"[UE  %d][RAPROC] Frame %d, subframe %d: Generating PRACH (eNB %d) preamble index %d for UL, TX power %d dBm (PL %d dB), l3msg \n",
                Mod_id,frame_tx,subframe_tx,eNB_id,
                phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex,
                phy_vars_ue->prach_resources[eNB_id]->ra_PREAMBLE_RECEIVED_TARGET_POWER+get_PL(Mod_id,CC_id,eNB_id),
                get_PL(Mod_id,CC_id,eNB_id));

	     }	  
         else
         {
             printf(" UE_TX PRACH Generate fail frame=%d subframe=%d\n", frame_tx,subframe_tx);
         }

      }
      else
      {
          printf(" UE_TX PRACH none frame=%d subframe=%d\n", frame_tx,subframe_tx);
      }
      // if we're calibrating the PRACH kill the pointer to its resources so that the RA protocol doesn't continue

      LOG_D(PHY,"[UE %d] frame %d subframe %d : generate_prach %d, prach_cnt %d\n",
            Mod_id,frame_tx,subframe_tx,phy_vars_ue->generate_prach,phy_vars_ue->prach_cnt);

      phy_vars_ue->prach_cnt++;

      if (phy_vars_ue->prach_cnt==3)
        phy_vars_ue->generate_prach=0;
    } // mode is PRACH
    else 
    {
      phy_vars_ue->generate_prach=0;
      //printf(" UE_TX OTHERS frame=%d subframe=%d\n",frame_tx,subframe_tx);

    }
  } // slot_tx is even

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX, VCD_FUNCTION_OUT);
  stop_meas(&phy_vars_ue->phy_proc_tx);
  LOG_TX_LINE();
  PERF_TIME_LINE(PERF_UE_phy_procedures_UE_TX, PERF_LINE);

  if( frame_tx % 1024 == 0 && subframe_tx  == 2)
  {
      UE_SendPrint();
  }
}

void phy_procedures_UE_S_TX(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint8_t abstraction_flag,relaying_type_t r_type)
{
  int aa;//i,aa;
  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_ue->lte_frame_parms;

  if (abstraction_flag==0) {

    for (aa=0; aa<frame_parms->nb_antennas_tx; aa++) {
#if defined(EXMIMO) //this is the EXPRESS MIMO case
      int i;
      // set the whole tx buffer to RX
      for (i=0; i<LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*frame_parms->samples_per_tti; i++)
        phy_vars_ue->lte_ue_common_vars.txdata[aa][i] = 0x00010001;

#else //this is the normal case
      memset(&phy_vars_ue->lte_ue_common_vars.txdata[aa][0],0,
             (LTE_NUMBER_OF_SUBFRAMES_PER_FRAME*frame_parms->samples_per_tti)*sizeof(int32_t));
#endif //else EXMIMO

    }
  }
}

void lte_ue_measurement_procedures(uint16_t l, PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint8_t abstraction_flag,runmode_t mode)
{

  LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_ue->lte_frame_parms;
  //  int aa;
#if defined(EXMIMO) && defined(DRIVER2013)
  //  exmimo_config_t *p_exmimo_config = openair0_exmimo_pci[0].exmimo_config_ptr;
  //  int aa;
#endif

  int slot_rx = phy_vars_ue->slot_rx;
  int subframe_rx = slot_rx>>1;

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_MEASUREMENT_PROCEDURES, VCD_FUNCTION_IN);

  if (l==0) {
    // UE measurements on symbol 0
    if (abstraction_flag==0) {
      LOG_D(PHY,"Calling measurements subframe %d, rxdata %p\n",subframe_rx,phy_vars_ue->lte_ue_common_vars.rxdata);

      lte_ue_measurements(phy_vars_ue,
                          (subframe_rx*frame_parms->samples_per_tti+phy_vars_ue->rx_offset)%(frame_parms->samples_per_tti*LTE_NUMBER_OF_SUBFRAMES_PER_FRAME),
                          (slot_rx == 2) ? 1 : 0,
                          0);
    } else {
      lte_ue_measurements(phy_vars_ue,
                          0,
                          0,
                          1);
    }
  }

  if (l==(6-phy_vars_ue->lte_frame_parms.Ncp)) {
	
   // make sure we have signal from PSS/SSS for N0 measurement

    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_RRC_MEASUREMENTS, VCD_FUNCTION_IN);
    ue_rrc_measurements(phy_vars_ue,
                        slot_rx,
                        abstraction_flag);
    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_RRC_MEASUREMENTS, VCD_FUNCTION_OUT);

    if (abstraction_flag==1)
      phy_vars_ue->sinr_eff =  sinr_eff_cqi_calc(phy_vars_ue, 0);

  }

  if ((slot_rx==1) && (l==(4-frame_parms->Ncp))) {

    // AGC

    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GAIN_CONTROL, VCD_FUNCTION_IN);
#if defined EXMIMO

    if ((openair_daq_vars.rx_gain_mode == DAQ_AGC_ON) &&
        (mode != rx_calib_ue) && (mode != rx_calib_ue_med) && (mode != rx_calib_ue_byp) )
      if  (phy_vars_ue->frame_rx%100==0)
        gain_control_all(phy_vars_ue->PHY_measurements.rx_power_avg_dB[eNB_id],0);

#else
#ifndef OAI_USRP
    phy_adjust_gain (phy_vars_ue,0);
#endif
#endif
    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GAIN_CONTROL, VCD_FUNCTION_OUT);
    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ADJUST_SYNCH, VCD_FUNCTION_IN);
    eNB_id = 0;

    if (abstraction_flag == 0)
      lte_adjust_synch(&phy_vars_ue->lte_frame_parms,
                       phy_vars_ue,
                       eNB_id,
                       0,
                       16384);

    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ADJUST_SYNCH, VCD_FUNCTION_OUT);

    /* if (openair_daq_vars.auto_freq_correction == 1) {
      if (frame_rx % 100 == 0) {
    if ((phy_vars_ue->lte_ue_common_vars.freq_offset>100) && (openair_daq_vars.freq_offset < 1000)) {
    openair_daq_vars.freq_offset+=100;
    #if defined(EXMIMO) && defined(DRIVER2013)
    for (aa = 0; aa<4; aa++) {
      p_exmimo_config->rf.rf_freq_rx[aa] = downlink_frequency[aa]+=openair_daq_vars.freq_offset;
      p_exmimo_config->rf.rf_freq_tx[aa] = downlink_frequency[aa]+=openair_daq_vars.freq_offset;
    }
    #endif
    }
    else if ((phy_vars_ue->lte_ue_common_vars.freq_offset<-100) && (openair_daq_vars.freq_offset > -1000)) {
    openair_daq_vars.freq_offset-=100;
    #if defined(EXMIMO) && defined(DRIVER2013)
    for (aa = 0; aa<4; aa++) {
      p_exmimo_config->rf.rf_freq_rx[aa] = downlink_frequency[aa]+=openair_daq_vars.freq_offset;
      p_exmimo_config->rf.rf_freq_tx[aa] = downlink_frequency[aa]+=openair_daq_vars.freq_offset;
    }
    #endif
    }
      }
    }*/

  }

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_MEASUREMENT_PROCEDURES, VCD_FUNCTION_OUT);
}

#ifdef EMOS
void phy_procedures_emos_UE_RX(PHY_VARS_UE *phy_vars_ue,uint8_t last_slot,uint8_t eNB_id)
{

  uint8_t i,j;
  //uint16_t last_slot_emos;
  uint32_t bytes;
  int Mod_id = phy_vars_ue->Mod_id;

  /*
  if (last_slot<2)
    last_slot_emos = last_slot;
  else if (last_slot>9)
    last_slot_emos = last_slot - 8;
  else {
    LOG_E(PHY,"emos rx last_slot_emos %d, last_slot %d\n", last_slot_emos,last_slot);
    mac_xface->macphy_exit("should never happen");
  }
  */

#ifdef EMOS_CHANNEL

  if ((last_slot==10) || (last_slot==11)) {
    for (i=0; i<phy_vars_ue->lte_frame_parms.nb_antennas_rx; i++)
      for (j=0; j<phy_vars_ue->lte_frame_parms.nb_antennas_tx; j++) {
        // first OFDM symbol with pilots
        memcpy(&emos_dump_UE.channel[i][j][(last_slot%2)*2*phy_vars_ue->lte_frame_parms.ofdm_symbol_size],
               &phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[eNB_id][(j<<1) + i][0],
               phy_vars_ue->lte_frame_parms.ofdm_symbol_size*sizeof(int));
        // second OFDM symbol with pilots
        memcpy(&emos_dump_UE.channel[i][j][((last_slot%2)*2+1)*phy_vars_ue->lte_frame_parms.ofdm_symbol_size],
               &phy_vars_ue->lte_ue_common_vars.dl_ch_estimates[eNB_id][(j<<1) + i][(phy_vars_ue->lte_frame_parms.Ncp == 0 ? 4 : 3)*phy_vars_ue->lte_frame_parms.ofdm_symbol_size],
               phy_vars_ue->lte_frame_parms.ofdm_symbol_size*sizeof(int));
      }
  }

#endif

  if (last_slot==0) {
    emos_dump_UE.timestamp = rt_get_time_ns();
    emos_dump_UE.frame_rx = phy_vars_ue->frame_rx;
    emos_dump_UE.UE_mode = phy_vars_ue->UE_mode[eNB_id];
    emos_dump_UE.mimo_mode = phy_vars_ue->transmission_mode[eNB_id];
    emos_dump_UE.freq_offset = phy_vars_ue->lte_ue_common_vars.freq_offset;
    emos_dump_UE.timing_advance = phy_vars_ue->timing_advance;
    emos_dump_UE.timing_offset  = phy_vars_ue->rx_offset;
    emos_dump_UE.rx_total_gain_dB = phy_vars_ue->rx_total_gain_dB;
    emos_dump_UE.eNb_id = eNB_id;
    memcpy(&emos_dump_UE.PHY_measurements,&phy_vars_ue->PHY_measurements,sizeof(PHY_MEASUREMENTS));
  }

  if (last_slot==1) {
    emos_dump_UE.pbch_errors = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors;
    emos_dump_UE.pbch_errors_last = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_last;
    emos_dump_UE.pbch_errors_conseq = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq;
    emos_dump_UE.pbch_fer = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_fer;
  }

  if (last_slot==19) {
    emos_dump_UE.dlsch_errors = phy_vars_ue->dlsch_errors[eNB_id];
    emos_dump_UE.dlsch_errors_last = phy_vars_ue->dlsch_errors_last[eNB_id];
    emos_dump_UE.dlsch_received = phy_vars_ue->dlsch_received[eNB_id];
    emos_dump_UE.dlsch_received_last = phy_vars_ue->dlsch_received_last[eNB_id];
    emos_dump_UE.dlsch_fer = phy_vars_ue->dlsch_fer[eNB_id];
    emos_dump_UE.dlsch_cntl_errors = phy_vars_ue->dlsch_SI_errors[eNB_id];
    emos_dump_UE.dlsch_ra_errors = phy_vars_ue->dlsch_ra_errors[eNB_id];
    emos_dump_UE.total_TBS = phy_vars_ue->total_TBS[eNB_id];
    emos_dump_UE.total_TBS_last = phy_vars_ue->total_TBS_last[eNB_id];
    emos_dump_UE.bitrate = phy_vars_ue->bitrate[eNB_id];
    emos_dump_UE.total_received_bits = phy_vars_ue->total_received_bits[eNB_id];
    emos_dump_UE.pmi_saved = phy_vars_ue->dlsch_ue[eNB_id][0]->pmi_alloc;
    emos_dump_UE.mcs = phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->mcs;
    emos_dump_UE.use_ia_receiver = openair_daq_vars.use_ia_receiver;

    bytes = rtf_put(CHANSOUNDER_FIFO_MINOR, &emos_dump_UE, sizeof(fifo_dump_emos_UE));

    if (bytes!=sizeof(fifo_dump_emos_UE)) {
      LOG_W(PHY,"[UE  %d] frame %d, slot %d, Problem writing EMOS data to FIFO\n",Mod_id,phy_vars_ue->frame_rx, last_slot);
    } else {
      if (phy_vars_ue->frame_rx%100==0) {
        LOG_I(PHY,"[UE  %d] frame %d, slot %d, Writing %d bytes EMOS data to FIFO\n",Mod_id,phy_vars_ue->frame_rx, last_slot, bytes);
      }
    }
  }

}
#endif


void restart_phy(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint8_t abstraction_flag)
{

  //  uint8_t last_slot;
  uint8_t i;
  LOG_I(PHY,"[UE  %d] frame %d, slot %d, restarting PHY!\n",phy_vars_ue->Mod_id,phy_vars_ue->frame_rx,phy_vars_ue->slot_rx);
  mac_xface->macphy_exit("restart_phy called");
  //   first_run = 1;

  if (abstraction_flag ==0 ) {
    openair_daq_vars.mode = openair_NOT_SYNCHED;
    phy_vars_ue->UE_mode[eNB_id] = NOT_SYNCHED;
    openair_daq_vars.sync_state=0;
  } else {
    phy_vars_ue->UE_mode[eNB_id] = PRACH;
    phy_vars_ue->prach_resources[eNB_id]=NULL;
  }

  phy_vars_ue->frame_rx = -1;
  phy_vars_ue->frame_tx = -1;
  openair_daq_vars.synch_wait_cnt=0;
  openair_daq_vars.sched_cnt=-1;

  phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq=0;
  phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors=0;

  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors = 0;
  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_missed = 0;
  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false  = 0;
  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_received = 0;

  phy_vars_ue->dlsch_errors[eNB_id] = 0;
  phy_vars_ue->dlsch_errors_last[eNB_id] = 0;
  phy_vars_ue->dlsch_received[eNB_id] = 0;
  phy_vars_ue->dlsch_received_last[eNB_id] = 0;
  phy_vars_ue->dlsch_fer[eNB_id] = 0;
  phy_vars_ue->dlsch_SI_received[eNB_id] = 0;
  phy_vars_ue->dlsch_ra_received[eNB_id] = 0;
  phy_vars_ue->dlsch_SI_errors[eNB_id] = 0;
  phy_vars_ue->dlsch_ra_errors[eNB_id] = 0;

  phy_vars_ue->dlsch_mch_received[eNB_id] = 0;

  for (i=0; i < MAX_MBSFN_AREA ; i ++) {
    phy_vars_ue->dlsch_mch_received_sf[i][eNB_id] = 0;
    phy_vars_ue->dlsch_mcch_received[i][eNB_id] = 0;
    phy_vars_ue->dlsch_mtch_received[i][eNB_id] = 0;
    phy_vars_ue->dlsch_mcch_errors[i][eNB_id] = 0;
    phy_vars_ue->dlsch_mtch_errors[i][eNB_id] = 0;
    phy_vars_ue->dlsch_mcch_trials[i][eNB_id] = 0;
    phy_vars_ue->dlsch_mtch_trials[i][eNB_id] = 0;
  }

  //phy_vars_ue->total_TBS[eNB_id] = 0;
  //phy_vars_ue->total_TBS_last[eNB_id] = 0;
  //phy_vars_ue->bitrate[eNB_id] = 0;
  //phy_vars_ue->total_received_bits[eNB_id] = 0;
}

int pbch_err_count = 0;
int pbch_err_frame = 0;
int pbch_err_history = 0;
void lte_ue_pbch_procedures(uint8_t eNB_id,PHY_VARS_UE *phy_vars_ue,uint8_t abstraction_flag)
{

  //  int i;
  int pbch_tx_ant=0;
  uint8_t pbch_phase;
  uint16_t frame_tx;
  static uint8_t first_run = 1;
  uint8_t pbch_trials = 0;

  DevAssert(phy_vars_ue);
  int slot_rx = phy_vars_ue->slot_rx;
  int frame_rx = phy_vars_ue->frame_rx;

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PBCH_PROCEDURES, VCD_FUNCTION_IN);

  pbch_phase=(frame_rx%4);

  if (pbch_phase>=4)
    pbch_phase=0;

  for (pbch_trials=0; pbch_trials<4; pbch_trials++) {
    //for (pbch_phase=0;pbch_phase<4;pbch_phase++) {
    //LOG_I(PHY,"[UE  %d] Frame %d, Trying PBCH %d (NidCell %d, eNB_id %d)\n",phy_vars_ue->Mod_id,frame_rx,pbch_phase,phy_vars_ue->lte_frame_parms.Nid_cell,eNB_id);
    if (abstraction_flag == 0) {
      pbch_tx_ant = rx_pbch(&phy_vars_ue->lte_ue_common_vars,
                            phy_vars_ue->lte_ue_pbch_vars[eNB_id],
                            &phy_vars_ue->lte_frame_parms,
                            eNB_id,
                            phy_vars_ue->lte_frame_parms.mode1_flag==1?SISO:ALAMOUTI,
                            phy_vars_ue->high_speed_flag,
                            pbch_phase);



    }

#ifdef PHY_ABSTRACTION
    else {
      pbch_tx_ant = rx_pbch_emul(phy_vars_ue,
                                 eNB_id,
                                 pbch_phase);
    }

#endif

    if ((pbch_tx_ant>0) && (pbch_tx_ant<=4)) {
      break;
    }

    pbch_phase++;

    if (pbch_phase>=4)
      pbch_phase=0;
  }



  if ((pbch_tx_ant>0) && (pbch_tx_ant<=4)) {

    if (pbch_tx_ant>2) {
      LOG_W(PHY,"[openair][SCHED][SYNCH] PBCH decoding: pbch_tx_ant>2 not supported\n");
      VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PBCH_PROCEDURES, VCD_FUNCTION_OUT);
      return;
    }


    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq = 0;
    frame_tx = (((int)(phy_vars_ue->lte_ue_pbch_vars[eNB_id]->decoded_output[2]&0x03))<<8);
    frame_tx += ((int)(phy_vars_ue->lte_ue_pbch_vars[eNB_id]->decoded_output[1]&0xfc));
    frame_tx += pbch_phase;

#ifdef OPENAIR2
    mac_xface->dl_phy_sync_success(phy_vars_ue->Mod_id,frame_rx,eNB_id,
                                   phy_vars_ue->UE_mode[eNB_id]==NOT_SYNCHED ? 1 : 0);
#endif

#ifdef EMOS
    //emos_dump_UE.frame_tx = frame_tx;
    //emos_dump_UE.mimo_mode = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->decoded_output[1];
#endif

    if (first_run) {
      first_run = 0;
      LOG_I(PHY,"[UE %d] frame %d, slot %d: Adjusting frame counter (PBCH ant_tx=%d, frame_tx=%d, phase %d).\n",
            phy_vars_ue->Mod_id,
            frame_rx,
            slot_rx,
            pbch_tx_ant,
            frame_tx,
            pbch_phase);
      phy_vars_ue->frame_rx = (phy_vars_ue->frame_rx & 0xFFFFFC00) | (frame_tx & 0x000003FF);
      phy_vars_ue->frame_tx = phy_vars_ue->frame_rx;
      frame_rx = phy_vars_ue->frame_rx;
    } else if (((frame_tx & 0x03FF) != (phy_vars_ue->frame_rx & 0x03FF))) {
      //(pbch_tx_ant != phy_vars_ue->lte_frame_parms.nb_antennas_tx)) {
      LOG_D(PHY,"[UE %d] frame %d, slot %d: Re-adjusting frame counter (PBCH ant_tx=%d, frame_rx=%d, frame%1024=%d, phase %d).\n",
            phy_vars_ue->Mod_id,
            phy_vars_ue->frame_rx,
            slot_rx,
            pbch_tx_ant,
            frame_tx,
            frame_rx & 0x03FF,
            pbch_phase);

      phy_vars_ue->frame_rx = (phy_vars_ue->frame_rx & 0xFFFFFC00) | (frame_tx & 0x000003FF);
      phy_vars_ue->frame_tx = phy_vars_ue->frame_rx;
      frame_rx = phy_vars_ue->frame_rx;
      /*
      LOG_D(PHY,"[UE  %d] frame %d, slot %d: PBCH PDU does not match, ignoring it (PBCH ant_tx=%d, frame_tx=%d).\n",
          phy_vars_ue->Mod_id,
          phy_vars_ue->frame,
          slot_rx,
          pbch_tx_ant,
          frame_tx);
      */
      //phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq = 21; // this will make it go out of sync
      //phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq += 1; // this will make it go out of sync
    }

//#ifdef DEBUG_PHY_PROC
    LOG_D(PHY,"[UE %d] frame %d, slot %d, Received PBCH (MIB): mode1_flag %d, tx_ant %d, frame_tx %d. N_RB_DL %d, phich_duration %d, phich_resource %d/6! rx_offset=%d\n",
          phy_vars_ue->Mod_id,
          frame_rx,
          slot_rx,
          phy_vars_ue->lte_frame_parms.mode1_flag,
          pbch_tx_ant,
          frame_tx,
          phy_vars_ue->lte_frame_parms.N_RB_DL,
          phy_vars_ue->lte_frame_parms.phich_config_common.phich_duration,
          phy_vars_ue->lte_frame_parms.phich_config_common.phich_resource,
          phy_vars_ue->rx_offset);
    /*
    if (frame_rx%100 == 0) {
      LOG_I(PHY,"[UE %d] frame %d, slot %d, PBCH: mode1_flag %d, tx_ant %d, frame_tx %d, phase %d. N_RB_DL %d, phich_duration %d, phich_resource %d/6,Frequency offset %d Hz (%d)\n",
            phy_vars_ue->Mod_id,
            frame_rx,
            slot_rx,
            phy_vars_ue->lte_frame_parms.mode1_flag,
            pbch_tx_ant,
            frame_tx,
            pbch_phase,
            phy_vars_ue->lte_frame_parms.N_RB_DL,
            phy_vars_ue->lte_frame_parms.phich_config_common.phich_duration,
            phy_vars_ue->lte_frame_parms.phich_config_common.phich_resource,
            phy_vars_ue->lte_ue_common_vars.freq_offset,openair_daq_vars.freq_offset);
      //dump_frame_parms(&phy_vars_ue->lte_frame_parms);

    }
      */
//#endif

  }
  else 
  {
    LOG_E(PHY,"[UE %d] frame %d, slot %d, Error decoding PBCH! rx_offset=%d pbch_err_count=%d pbch_err_history=%d \n",
          phy_vars_ue->Mod_id,frame_rx, slot_rx, phy_vars_ue->rx_offset,pbch_err_count,pbch_err_history);
    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq++;
    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors++;

    if( frame_rx > 1000)
    {
        if( pbch_err_frame+1 == frame_rx)
        {
            pbch_err_count++;
            if( pbch_err_count > 3)
            {
                oai_exit = 1;
            }
        
        }
        else
        {
            pbch_err_history++;
            pbch_err_count = 0;
        }
        pbch_err_frame = frame_rx;
    }
    
#ifdef OPENAIR2
    mac_xface->out_of_sync_ind(phy_vars_ue->Mod_id,frame_rx,eNB_id);
#else

    if (phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq>=100) {
      LOG_E(PHY,"More that 100 consecutive PBCH errors! Exiting!\n");
      mac_xface->macphy_exit("More that 100 consecutive PBCH errors!");
    }

#endif
  }

  if (frame_rx % 100 == 0) {
    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_fer = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors - phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_last;
    phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_last = phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors;
  }

#ifdef DEBUG_PHY_PROC
  LOG_D(PHY,"[UE %d] frame %d, slot %d, PBCH errors = %d, consecutive errors = %d!\n",
        phy_vars_ue->Mod_id,frame_rx, slot_rx,
        phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors,
        phy_vars_ue->lte_ue_pbch_vars[eNB_id]->pdu_errors_conseq);
#endif
  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PBCH_PROCEDURES, VCD_FUNCTION_OUT);
}

extern uint8_t rv_table[4];

int lte_ue_pdcch_procedures(uint8_t eNB_id,PHY_VARS_UE *phy_vars_ue,uint8_t abstraction_flag)
{

  unsigned int dci_cnt=0, i;
  //DCI_PDU *DCI_pdu;
  //uint16_t ra_RNTI;

  DCI_ALLOC_t dci_alloc_rx[8];

  int frame_rx = phy_vars_ue->frame_rx;
  int slot_rx = phy_vars_ue->slot_rx;
  int subframe_rx = slot_rx>>1;

  //xhd_oai_ue
  #if 0
  //uint8_t harq_pid = phich_subframe_to_harq_pid(&phy_vars_ue->lte_frame_parms,phy_vars_ue->frame_rx,subframe_rx);
  uint8_t harq_pid = phich_subframe_to_harq_pid(&phy_vars_ue->lte_frame_parms,phy_vars_ue->frame_rx,subframe_rx);
  LTE_UL_UE_HARQ_t *pHarq = &phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid];
  uint8_t *pDciNew = &pHarq->DciNew;
  uint8_t *pHichNew = &pHarq->HichNew;
  *pDciNew = 0;
  *pHichNew = 0;
   #endif
   
  PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PDCCH_PROCEDURES, VCD_FUNCTION_IN);


    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_RX_PDCCH, VCD_FUNCTION_IN);
    rx_pdcch(&phy_vars_ue->lte_ue_common_vars,
             phy_vars_ue->lte_ue_pdcch_vars,
             &phy_vars_ue->lte_frame_parms,
             subframe_rx,
             eNB_id,
             (phy_vars_ue->lte_frame_parms.mode1_flag == 1) ? SISO : ALAMOUTI,
             phy_vars_ue->high_speed_flag,
             phy_vars_ue->is_secondary_ue);

    PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);

    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_RX_PDCCH, VCD_FUNCTION_OUT);
    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_DCI_DECODING, VCD_FUNCTION_IN);
    dci_cnt = dci_decoding_procedure(phy_vars_ue,
                                     dci_alloc_rx,
                                     (phy_vars_ue->UE_mode[eNB_id] < PUSCH)? 1 : 0,  // if we're in PUSCH don't listen to common search space,
                                     // later when we need paging or RA during connection, update this ...
                                     eNB_id,frame_rx,subframe_rx);
    VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_DCI_DECODING, VCD_FUNCTION_OUT);
    //LOG_D(PHY,"[UE  %d][PUSCH] Frame %d subframe %d PHICH RX\n",phy_vars_ue->Mod_id,frame_rx,subframe_rx);
    PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);

    if (is_phich_subframe(&phy_vars_ue->lte_frame_parms,subframe_rx)) 
    {
        //printf("lte_ue_pdcch_procedures:subframe_rx=%d frame_type=%d tdd_config=%d\n",
         //   subframe_rx, phy_vars_ue->lte_frame_parms.frame_type, phy_vars_ue->lte_frame_parms.tdd_config);
      VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_RX_PHICH, VCD_FUNCTION_IN);
      rx_phich(phy_vars_ue,subframe_rx,eNB_id);
      VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_RX_PHICH, VCD_FUNCTION_OUT);
    }

  LOG_RX_LINE();
  PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);


  LOG_D(PHY,"[UE  %d] Frame %d, subframe %d , slot %d, Mode %s: DCI found %i\n",
    phy_vars_ue->Mod_id,frame_rx,subframe_rx,slot_rx,mode_string[phy_vars_ue->UE_mode[eNB_id]],dci_cnt);

  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_received += dci_cnt;


  // dci_cnt = 0;
  //  ra_RNTI = (phy_vars_ue->prach_resources[eNB_id]) ? phy_vars_ue->prach_resources[eNB_id]->ra_RNTI : 0;
  for (i=0; i<dci_cnt; i++) 
  {


    if (subframe_rx == 9) { //( frame_rx % 100 == 0)   {
      LOG_D(PHY,"frame %d, subframe %d, rnti %x: dci %d/%d\n",frame_rx,subframe_rx,phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,i,dci_cnt);
      //dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
    }
    LOG_RX_LINE();
    PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);


    //if ((phy_vars_ue->UE_mode[eNB_id] != PRACH) &&
    //    (dci_alloc_rx[i].rnti != 0x1234) &&

    if ((phy_vars_ue->UE_mode[eNB_id]>PRACH) &&
    	(dci_alloc_rx[i].rnti == phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti) &&
    	(dci_alloc_rx[i].format != format0)) 
	{
      
      LOG_D(PHY,"[UE  %d][DCI][PDSCH %x] frame %d, subframe %d: format %d, num_pdcch_symbols %d, nCCE %d, total CCEs %d\n",
            phy_vars_ue->Mod_id,dci_alloc_rx[i].rnti,
            frame_rx,subframe_rx,
            dci_alloc_rx[i].format,
            phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
            phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->nCCE[subframe_rx],
            get_nCCE(3,&phy_vars_ue->lte_frame_parms,get_mi(&phy_vars_ue->lte_frame_parms,0)));


#ifdef DIAG_PHY
      
      if (!(((subframe_rx == 7) && (dci_alloc_rx[i].format == format1E_2A_M10PRB)) ||
            ((subframe_rx == 7) && (dci_alloc_rx[i].format == format1)))) {
        LOG_E(PHY,"[UE  %d][DIAG] frame %d, subframe %d: should not have received C_RNTI Format %d!\n",phy_vars_ue->Mod_id,frame_rx,subframe_rx,dci_alloc_rx[i].format);
        phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors++;
        phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false++;
        VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PDCCH_PROCEDURES, VCD_FUNCTION_OUT);
        return(-1);
      }
      
#endif
      
      LOG_RX_LINE();
      PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);
      //      dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
      if ((phy_vars_ue->UE_mode[eNB_id] > PRACH) &&
	      (generate_ue_dlsch_params_from_dci(frame_rx,
					     subframe_rx,
					     (void *)&dci_alloc_rx[i].dci_pdu,
					     phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,
					     dci_alloc_rx[i].format,
					     phy_vars_ue->dlsch_ue[eNB_id],
					     &phy_vars_ue->lte_frame_parms,
					     phy_vars_ue->pdsch_config_dedicated,
					     SI_RNTI,
					     0,
					     P_RNTI)==0)) {

#ifdef DIAG_PHY
	
        if (phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->mcs != (((frame_rx%1024)%28))) {
          LOG_E(PHY,"[UE  %d][DIAG] frame %d, subframe %d: wrong mcs!\n",phy_vars_ue->Mod_id,frame_rx,subframe_rx,
                phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->mcs);
          dump_dci(&phy_vars_ue->lte_frame_parms,(void *)&dci_alloc_rx[i]);
        }
	
#endif
	
	
        phy_vars_ue->dlsch_received[eNB_id]++;
	
        LOG_D(PHY,"[UE  %d] Generated UE DLSCH C_RNTI format %d  L=%d nCCE=%d \n",
            phy_vars_ue->Mod_id,dci_alloc_rx[i].format,
            dci_alloc_rx[i].L,dci_alloc_rx[i].nCCE);
        dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
        LOG_D(PHY,"[UE %d] *********** dlsch->active in subframe %d (%d)=> %d\n",phy_vars_ue->Mod_id,subframe_rx,slot_rx,phy_vars_ue->dlsch_ue[eNB_id][0]->active);
	
        // we received a CRNTI, so we're in PUSCH
        if (phy_vars_ue->UE_mode[eNB_id] != PUSCH) {
          LOG_D(PHY,"[UE  %d] Frame %d, subframe %d: Received DCI with CRNTI %x => Mode PUSCH\n",phy_vars_ue->Mod_id,frame_rx,subframe_rx,phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti);
          //dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
          phy_vars_ue->UE_mode[eNB_id] = PUSCH;
          //mac_xface->macphy_exit("Connected. Exiting\n");
        }
      } 
      else 
      {
        LOG_E(PHY,"[UE  %d] Frame %d, subframe %d: Problem in DCI! line=%d dci_cnt=%d i=%d L=%d nCCE=%d NORMAL \n",
            phy_vars_ue->Mod_id,frame_rx,subframe_rx,__LINE__,dci_cnt,i,
            dci_alloc_rx[i].L,dci_alloc_rx[i].nCCE);
        dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
      }
      
      PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);
    }

    else if ((dci_alloc_rx[i].rnti == SI_RNTI) &&
             ((dci_alloc_rx[i].format == format1A) || (dci_alloc_rx[i].format == format1C))) 
    {

      LOG_D(PHY,"[UE  %d] subframe %d: Found rnti %x, format 1%s, dci_cnt %d\n",phy_vars_ue->Mod_id,subframe_rx,dci_alloc_rx[i].rnti,dci_alloc_rx[i].format==format1A?"A":"C",i);
#ifdef DIAG_PHY

      if ((subframe_rx != 5)) {
        LOG_E(PHY,"[UE  %d][DIAG] frame %d, subframe %d: should not have received SI_RNTI!\n",phy_vars_ue->Mod_id,frame_rx,subframe_rx);
        phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors++;
        phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false++;
        VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PDCCH_PROCEDURES, VCD_FUNCTION_OUT);
        return(-1);
      }

#endif

      if (generate_ue_dlsch_params_from_dci(frame_rx,
					    subframe_rx,
                                            (void *)&dci_alloc_rx[i].dci_pdu,
                                            SI_RNTI,
                                            dci_alloc_rx[i].format,
                                            &phy_vars_ue->dlsch_ue_SI[eNB_id],
                                            &phy_vars_ue->lte_frame_parms,
                                            phy_vars_ue->pdsch_config_dedicated,
                                            SI_RNTI,
                                            0,
                                            P_RNTI)==0) {

        phy_vars_ue->dlsch_SI_received[eNB_id]++;
 

	    LOG_D(PHY,"[UE  %d] Frame %d, subframe %d : Generate UE DLSCH SI_RNTI format 1%s\n",phy_vars_ue->Mod_id,frame_rx,subframe_rx,dci_alloc_rx[i].format==format1A?"A":"C");
        //dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);

      }
      else 
      {
        LOG_E(PHY,"[UE  %d] Frame %d, subframe %d: Problem in DCI! line=%d dci_cnt=%d i=%d  SI\n",
            phy_vars_ue->Mod_id,frame_rx,subframe_rx, __LINE__,dci_cnt,i);
        dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
      }
    }

    else if ((phy_vars_ue->prach_resources[eNB_id]) &&
             (dci_alloc_rx[i].rnti == phy_vars_ue->prach_resources[eNB_id]->ra_RNTI) &&
             (dci_alloc_rx[i].format == format1A)) {

      LOG_I(PHY,"[UE  %d][RAPROC] subframe %d: Found RA rnti %x, format 1A, dci_cnt %d\n",phy_vars_ue->Mod_id,subframe_rx,dci_alloc_rx[i].rnti,i);
#ifdef DIAG_PHY

      if (subframe_rx != 9) {
        LOG_E(PHY,"[UE  %d][DIAG] frame %d, subframe %d: should not have received RA_RNTI!\n",phy_vars_ue->Mod_id,frame_rx,subframe_rx);
        phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors++;
        phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false++;
        VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PDCCH_PROCEDURES, VCD_FUNCTION_OUT);
        return(-1);
      }

#endif
      PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);

      if (generate_ue_dlsch_params_from_dci(frame_rx,
					    subframe_rx,
                                            (DCI1A_5MHz_TDD_1_6_t *)&dci_alloc_rx[i].dci_pdu,
                                            phy_vars_ue->prach_resources[eNB_id]->ra_RNTI,
                                            format1A,
                                            &phy_vars_ue->dlsch_ue_ra[eNB_id],
                                            &phy_vars_ue->lte_frame_parms,
                                            phy_vars_ue->pdsch_config_dedicated,
                                            SI_RNTI,
                                            phy_vars_ue->prach_resources[eNB_id]->ra_RNTI,
                                            P_RNTI)==0) {

        phy_vars_ue->dlsch_ra_received[eNB_id]++;

        LOG_I(PHY,"[UE  %d] Generate UE DLSCH RA_RNTI format 1A, rb_alloc %x, dlsch_ue_ra[eNB_id] %p\n",
              phy_vars_ue->Mod_id,phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->rb_alloc_even[0],phy_vars_ue->dlsch_ue_ra[eNB_id]);
      }
      else 
      {
        LOG_E(PHY,"[UE  %d] Frame %d, subframe %d: Problem in DCI! line=%d dci_cnt=%d i=%d  RA\n",
            phy_vars_ue->Mod_id,frame_rx,subframe_rx,__LINE__,dci_cnt,i);
        dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
      }
      PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);
      
    } 
    else if( (dci_alloc_rx[i].rnti == phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti) &&
               (dci_alloc_rx[i].format == format0)) 
    {
      LOG_D(PHY,"[UE  %d][PUSCH] Frame %d subframe %d: Found rnti %x, format 0, dci_cnt %d AAA\n",
            phy_vars_ue->Mod_id,frame_rx,subframe_rx,dci_alloc_rx[i].rnti,i);
#ifdef DIAG_PHY

      if (subframe_rx != 9) {
        LOG_E(PHY,"[UE  %d][DIAG] frame %d, subframe %d: should not have received C_RNTI Format 0!\n",phy_vars_ue->Mod_id,frame_rx,subframe_rx);
        phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors++;
        phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false++;
        VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PDCCH_PROCEDURES, VCD_FUNCTION_OUT);
        return(-1);
      }

#endif
      PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);

      phy_vars_ue->ulsch_no_allocation_counter[eNB_id] = 0;
      //dump_dci(&phy_vars_ue->lte_frame_parms,&dci_alloc_rx[i]);

      if ((phy_vars_ue->UE_mode[eNB_id] > PRACH) &&
	  (generate_ue_ulsch_params_from_dci((void *)&dci_alloc_rx[i].dci_pdu,
					     phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,
					     frame_rx,
					     subframe_rx,
					     format0,
					     phy_vars_ue,
					     SI_RNTI,
					     0,
					     P_RNTI,
					     CBA_RNTI,
					     eNB_id,
					     0)==0)) {

        LOG_D(PHY,"[UE  %d] Generate UE ULSCH C_RNTI format 0 (subframe %d)\n",phy_vars_ue->Mod_id,subframe_rx);

      }
      
      PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);
    } 
    else if( (dci_alloc_rx[i].rnti == phy_vars_ue->ulsch_ue[eNB_id]->cba_rnti[0]) &&
               (dci_alloc_rx[i].format == format0)) 
    {
      // UE could belong to more than one CBA group
      // phy_vars_ue->Mod_id%phy_vars_ue->ulsch_ue[eNB_id]->num_active_cba_groups]
      LOG_D(PHY,"[UE  %d][PUSCH] Frame %d subframe %d: Found cba rnti %x, format 0, dci_cnt %d\n",
            phy_vars_ue->Mod_id,frame_rx,subframe_rx,dci_alloc_rx[i].rnti,i);
      /*
        if (((frame_rx%100) == 0) || (frame_rx < 20))
        dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
      */

      phy_vars_ue->ulsch_no_allocation_counter[eNB_id] = 0;
      //dump_dci(&phy_vars_ue->lte_frame_parms,&dci_alloc_rx[i]);
      PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);

      if ((phy_vars_ue->UE_mode[eNB_id] > PRACH) &&
	  (generate_ue_ulsch_params_from_dci((void *)&dci_alloc_rx[i].dci_pdu,
					     phy_vars_ue->ulsch_ue[eNB_id]->cba_rnti[0],
					     frame_rx,
					     subframe_rx,
					     format0,
					     phy_vars_ue,
					     SI_RNTI,
					     0,
					     P_RNTI,
					     CBA_RNTI,
					     eNB_id,
					     0)==0)) {

        LOG_D(PHY,"[UE  %d] Generate UE ULSCH CBA_RNTI format 0 (subframe %d)\n",phy_vars_ue->Mod_id,subframe_rx);
        phy_vars_ue->ulsch_ue[eNB_id]->num_cba_dci[(subframe_rx+4)%10]++;
      }
      
      PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);
    }

    else 
    {
      LOG_D(PHY,"[UE  %d] frame %d, subframe %d: received DCI %d with RNTI=%x (C-RNTI:%x, CBA_RNTI %x) and format %d!\n",phy_vars_ue->Mod_id,frame_rx,subframe_rx,i,dci_alloc_rx[i].rnti,
            phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,
            phy_vars_ue->ulsch_ue[eNB_id]->cba_rnti[0],
            dci_alloc_rx[i].format);
      //      dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
#ifdef DIAG_PHY
      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_errors++;
      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->dci_false++;
      VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PDCCH_PROCEDURES, VCD_FUNCTION_OUT);
      return(-1);
#endif
    }

  }
  PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);

#if 0
  if (phy_vars_ue->ulsch_ue_Msg3_active[eNB_id] == 2) 
  {
    phy_vars_ue->ulsch_ue_Msg3_active[eNB_id] == 0;
  }
  else
  {

      pHarq->subframe_scheduling_flag = 0;
      if( *pDciNew == 0 && *pHichNew == 0)
      {
        //nothing to do
      }
      else if ( *pDciNew == 0 && *pHichNew == 1)
      {
        //nothing to do
        pHarq->round = 0;
      }
      else if ( *pDciNew == 0 && *pHichNew == 2)
      {
        //error
        printf("$$$$error at line%d *pDciNew == %d && *pHichNew == %d\n", __LINE__, *pDciNew, *pHichNew);
      }
      else if ( *pDciNew == 1 && *pHichNew == 0)
      {
        //new data
        pHarq->subframe_scheduling_flag = 1;
        pHarq->round = 0;
        pHarq->rvidx = rv_table[pHarq->round&3];
      }
      else if ( *pDciNew == 1 && *pHichNew == 1)
      {
        //new data again
        pHarq->subframe_scheduling_flag = 1;
        pHarq->round = 0;
        pHarq->rvidx = rv_table[pHarq->round&3];
      }
      else if ( *pDciNew == 1 && *pHichNew == 2)
      {
        //error
        printf("$$$$error at line%d *pDciNew == %d && *pHichNew == %d\n", __LINE__, *pDciNew, *pHichNew);
      }
      else if ( *pDciNew == 2 && *pHichNew == 0)
      {
        //error
        printf("$$$$error at line%d *pDciNew == %d && *pHichNew == %d\n", __LINE__, *pDciNew, *pHichNew);
      }
      else if ( *pDciNew == 2 && *pHichNew == 1)
      {
        //error
        printf("$$$$error at line%d *pDciNew == %d && *pHichNew == %d\n", __LINE__, *pDciNew, *pHichNew);
      }
      else if ( *pDciNew == 2 && *pHichNew == 2)
      {
        //old data
        pHarq->subframe_scheduling_flag = 1;
        pHarq->round++;
        pHarq->rvidx = rv_table[pHarq->round&3];
      }
      else
      {
        //error
        printf("$$$$error at line%d *pDciNew == %d && *pHichNew == %d\n", __LINE__, *pDciNew, *pHichNew);
      }
  }
    
  //      ulsch->harq_processes[harq_pid]->Ndi = 0;
  
  //ulsch->harq_processes[harq_pid]->roundHich++;
  //UE_Hich(subframe,ulsch->harq_processes[harq_pid]->roundHich);
#endif

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PDCCH_PROCEDURES, VCD_FUNCTION_OUT);
  return(0);
}



int phy_procedures_UE_RX_old(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint8_t abstraction_flag,runmode_t mode,
                         relaying_type_t r_type,PHY_VARS_RN *phy_vars_rn)
{
 
  uint16_t l,m,n_symb;
  //  int eNB_id = 0,
  int ret=0;
  uint8_t harq_pid = -1;
  int timing_advance;
  uint8_t pilot1,pilot2,pilot3;
#ifndef DLSCH_THREAD
  uint8_t i_mod = 0;
  int eNB_id_i = 1;
  uint8_t dual_stream_UE = 0;
#endif
#ifndef OPENAIR2
  uint8_t *rar;
#endif
  int pmch_flag=0;
  uint8_t sync_area=255;
  int pmch_mcs=-1;
  uint8_t mcch_active=0;
  int frame_rx = phy_vars_ue->frame_rx;
  int slot_rx = phy_vars_ue->slot_rx;
  int subframe_rx = slot_rx>>1;
  int subframe_prev = (subframe_rx+9)%10;
  int frame_prev = (subframe_rx==0)?(frame_rx-1):frame_rx;

#ifdef OPENAIR2
  int CC_id = phy_vars_ue->CC_id;
#endif

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_IN);


  start_meas(&phy_vars_ue->phy_proc_rx);
#ifdef DEBUG_PHY_PROC
  LOG_D(PHY,"[%s %d] Frame %d subframe %d: Doing phy_procedures_UE_RX(%d)\n",
        (r_type == multicast_relay) ? "RN/UE" : "UE",
        phy_vars_ue->Mod_id,frame_rx, subframe_rx, slot_rx);
#endif
#ifdef EMOS

  if ((slot_rx == 0)) {
    if (frame_rx%1024 == 0)
      openair_daq_vars.use_ia_receiver = 0;
    else
      openair_daq_vars.use_ia_receiver = (openair_daq_vars.use_ia_receiver+1)%3;

    LOG_I(PHY,"[MYEMOS] frame %d, IA receiver %d, MCS %d, bitrate %d\n",
          frame_rx,
          openair_daq_vars.use_ia_receiver,
          phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid]->mcs,
          phy_vars_ue->bitrate[eNB_id]);
  }

#endif

  if (phy_vars_ue->lte_frame_parms.Ncp == 0) {  // normal prefix
    pilot1 = 4;
    pilot2 = 7;
    pilot3 = 11;
  } else { // extended prefix
    pilot1 = 3;
    pilot2 = 6;
    pilot3 = 9;
  }

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if (subframe_select(&phy_vars_ue->lte_frame_parms,subframe_rx) == SF_S) {
    if ((slot_rx%2)==0)
      n_symb = 5;//3;
    else
      n_symb = 0;
  } else {
    /*
    if (is_pmch_subframe(frame_rx,subframe_rx,&phy_vars_ue->lte_frame_parms)) {
      if ((slot_rx%2)==0) {
    n_symb=2;
    pmch_flag=1;
      }
      else
    n_symb=0;
    }
    else*/
    n_symb = phy_vars_ue->lte_frame_parms.symbols_per_tti/2;
  }

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // This is normal processing (i.e. not MBSFN)
  // RX processing of symbols in slot_rx

  

  for (l=0; l<n_symb; l++) {
    if (abstraction_flag == 0) {
      start_meas(&phy_vars_ue->ofdm_demod_stats);
      VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_SLOT_FEP, VCD_FUNCTION_IN);

      slot_fep(phy_vars_ue,
               l,
               slot_rx,
               phy_vars_ue->rx_offset,
               0,
	       0);
      VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_SLOT_FEP, VCD_FUNCTION_OUT);
      stop_meas(&phy_vars_ue->ofdm_demod_stats);
    }

    //if (subframe_select(&phy_vars_ue->lte_frame_parms,subframe_rx) == SF_DL)
    lte_ue_measurement_procedures(l,phy_vars_ue,eNB_id,abstraction_flag,mode);


    if ((slot_rx==1) && (l==4-phy_vars_ue->lte_frame_parms.Ncp)) {

      /*
      phy_vars_ue->ulsch_no_allocation_counter[eNB_id]++;

      if (phy_vars_ue->ulsch_no_allocation_counter[eNB_id] == 10) {
      #ifdef DEBUG_PHY_PROC
      msg("[UE  %d] no_allocation : setting mode to PRACH\n",phy_vars_ue->Mod_id);
      #endif
      phy_vars_ue->UE_mode[eNB_id] = PRACH;
      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti = 0x1234;
      }
      */

      lte_ue_pbch_procedures(eNB_id,phy_vars_ue,abstraction_flag);

      /*
      if (phy_vars_ue->UE_mode[eNB_id] == RA_RESPONSE) {
        phy_vars_ue->Msg3_timer[eNB_id]--;
        msg("[UE RAR] frame %d: Msg3_timer %d\n",frame_rx,phy_vars_ue->Msg3_timer);

        if (phy_vars_ue->Msg3_timer[eNB_id] == 0) {
          LOG_D(PHY,"[UE  %d] Frame %d: Msg3_timer = 0 : setting mode to PRACH\n",phy_vars_ue->Mod_id,frame_rx);
      // I guess here we also need to tell the RRC
          phy_vars_ue->UE_mode[eNB_id] = PRACH;
      phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti = 0x1234;
      }
      }
      */
    }

#ifdef DLSCH_THREAD
    if (phy_vars_ue->dlsch_ue[eNB_id][0]->active == 1)  {
      // activate thread since Chest is now done for slot before slot_rx
      if (l==0) {
        LOG_I(PHY,"frame %d, slot_rx %d: Calling rx_pdsch_thread for harq_pid %d\n",frame_rx,slot_rx, phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid);

        if (pthread_mutex_lock (&rx_pdsch_mutex) != 0) {               // Signal MAC_PHY Scheduler
          LOG_E(PHY,"[UE  %d] ERROR pthread_mutex_lock\n",phy_vars_ue->Mod_id);     // lock before accessing shared resource
          VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
        } else {
          rx_pdsch_instance_cnt++;
          (slot_rx == 0) ? (rx_pdsch_slot = 19) : (rx_pdsch_slot = (slot_rx-1));
          pthread_mutex_unlock (&rx_pdsch_mutex);

          if (rx_pdsch_instance_cnt == 0) {
            if (pthread_cond_signal(&rx_pdsch_cond) != 0) {
              LOG_E(PHY,"[UE  %d] ERROR pthread_cond_signal for rx_pdsch_cond\n",phy_vars_ue->Mod_id);
              VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
            }
          } else {
            LOG_W(PHY,"[UE  %d] Frame=%d, Slot=%d, RX_PDSCH thread for rx_pdsch_thread busy!!!\n",phy_vars_ue->Mod_id,frame_rx,slot_rx);
            VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
          }
        }

        // trigger DLSCH decoding thread
        if ((slot_rx%2)==1) // odd slots
          phy_vars_ue->dlsch_ue[eNB_id][0]->active = 0;
      }
    }

#endif

    // process last DLSCH symbols + invoke decoding
    if (((slot_rx%2)==0) && (l==0)) {
      // Regular PDSCH
      LOG_D(PHY,"[UE %d] dlsch->active in subframe %d => %d\n",phy_vars_ue->Mod_id,subframe_prev,phy_vars_ue->dlsch_ue[eNB_id][0]->active);

      if (phy_vars_ue->dlsch_ue[eNB_id][0]->active[subframe_prev] == 1) {
          harq_pid = phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid[subframe_prev];
#ifndef DLSCH_THREAD //USER_MODE
        LOG_D(PHY,"[UE %d] PDSCH active in subframe %d, harq_pid %d\n",phy_vars_ue->Mod_id,subframe_prev,harq_pid);

        if ((phy_vars_ue->transmission_mode[eNB_id] == 5) &&
            (phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->dl_power_off==0) &&
            (openair_daq_vars.use_ia_receiver ==1)) {
          dual_stream_UE = 1;
          eNB_id_i = phy_vars_ue->n_connected_eNB;
          i_mod = phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->Qm;

          if (frame_rx%100==0) {
            LOG_I(PHY,"using IA receiver\n");
          }
        } else {
          dual_stream_UE = 0;
          eNB_id_i = eNB_id+1;
          i_mod = 0;
        }

        // process symbols 10,11,12 and trigger DLSCH decoding
        if (abstraction_flag == 0) {

          start_meas(&phy_vars_ue->dlsch_llr_stats);

          for (m=pilot3; m<phy_vars_ue->lte_frame_parms.symbols_per_tti; m++) {

            rx_pdsch(phy_vars_ue,
                     PDSCH,
                     eNB_id,
                     eNB_id_i,
                     subframe_prev,  // subframe
                     m,                    // symbol
                     0,                    // first_symbol_flag
                     dual_stream_UE,
                     i_mod,
                     harq_pid);
          }

          stop_meas(&phy_vars_ue->dlsch_llr_stats);
        }

        phy_vars_ue->dlsch_ue[eNB_id][0]->active[subframe_prev] = 0;

#ifdef DEBUG_PHY_PROC
        LOG_D(PHY,"[UE  %d][PDSCH %x/%d] Frame %d subframe %d Scheduling DLSCH decoding\n",
              phy_vars_ue->Mod_id,
              phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
              harq_pid,
              (subframe_prev == 9) ? (frame_rx-1) : frame_rx,subframe_prev);
#endif

        if (phy_vars_ue->dlsch_ue[eNB_id][0]) {
          if (abstraction_flag == 0) {
            phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->G = get_G(&phy_vars_ue->lte_frame_parms,
                phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->nb_rb,
                phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rb_alloc_even,
                phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->Qm,
                phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->Nl,
                phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
                frame_rx,subframe_prev);
            start_meas(&phy_vars_ue->dlsch_unscrambling_stats);
            dlsch_unscrambling(&phy_vars_ue->lte_frame_parms,
                               0,
                               phy_vars_ue->dlsch_ue[0][0],
                               phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->G,
                               phy_vars_ue->lte_ue_pdsch_vars[eNB_id]->llr[0],
                               0,
                               subframe_prev<<1);
            stop_meas(&phy_vars_ue->dlsch_unscrambling_stats);

            start_meas(&phy_vars_ue->dlsch_decoding_stats);
            ret = dlsch_decoding(phy_vars_ue,
                                 phy_vars_ue->lte_ue_pdsch_vars[eNB_id]->llr[0],
                                 &phy_vars_ue->lte_frame_parms,
                                 phy_vars_ue->dlsch_ue[eNB_id][0],
                                 phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid],
                                 frame_prev,subframe_prev,
                                 harq_pid,
                                 1,0);
            stop_meas(&phy_vars_ue->dlsch_decoding_stats);
          }

          else {
            LOG_D(PHY,"Calling dlsch_decoding_emul ...\n");
#ifdef PHY_ABSTRACTION
            ret = dlsch_decoding_emul(phy_vars_ue,
                                      subframe_prev,
                                      2,
                                      eNB_id);
#endif
          }

          if (ret == (1+phy_vars_ue->dlsch_ue[eNB_id][0]->max_turbo_iterations)) {
            phy_vars_ue->dlsch_errors[eNB_id]++;

#ifdef DEBUG_PHY_PROC
            LOG_D(PHY,"[UE  %d][PDSCH %x/%d] Frame %d subframe %d DLSCH in error (rv %d,mcs %d)\n",
                  phy_vars_ue->Mod_id,phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
                  harq_pid,frame_rx,subframe_prev,
                  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rvidx,
                  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs);

            //      if (abstraction_flag ==0 )
            //dump_dlsch(phy_vars_ue,eNB_id,subframe_prev,harq_pid);
            //mac_xface->macphy_exit("");
#endif
          } else {
            LOG_D(PHY,"[UE  %d][PDSCH %x/%d] Frame %d subframe %d (slot_rx %d): Received DLSCH (rv %d,mcs %d,TBS %d)\n",
                  phy_vars_ue->Mod_id,phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
                  harq_pid,frame_rx,subframe_prev,slot_rx,
                  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rvidx,
                  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs,
                  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS);
#ifdef DEBUG_PHY_PROC
#ifdef DEBUG_DLSCH
            int j;
            LOG_D(PHY,"dlsch harq_pid %d (rx): \n",harq_pid);

            for (j=0; j<phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS>>3; j++)
              LOG_T(PHY,"%x.",phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->b[j]);

            LOG_T(PHY,"\n");
#endif
#endif
#ifdef OPENAIR2
            mac_xface->ue_send_sdu(phy_vars_ue->Mod_id,
                                   CC_id,
                                   frame_rx,
                                   phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->b,
                                   phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS>>3,
                                   eNB_id);
#endif
            phy_vars_ue->total_TBS[eNB_id] =  phy_vars_ue->total_TBS[eNB_id] +
                                              phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS;
            phy_vars_ue->total_received_bits[eNB_id] = phy_vars_ue->total_TBS[eNB_id] +
                phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS;
          }
        }


#ifdef DEBUG_PHY_PROC
        LOG_D(PHY,"[UE  %d][PDSCH %x/%d] Frame %d subframe %d: PDSCH/DLSCH decoding iter %d (mcs %d, rv %d, TBS %d)\n",
              phy_vars_ue->Mod_id,
              phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,harq_pid,
              frame_rx,subframe_prev,ret,
              phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs,
              phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rvidx,
              phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS);

        if (frame_rx%100==0) {
          LOG_D(PHY,"[UE  %d][PDSCH %x] Frame %d subframe %d dlsch_errors %d, dlsch_received %d, dlsch_fer %d, current_dlsch_cqi %d\n",
                phy_vars_ue->Mod_id,phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
                frame_rx,subframe_prev,
                phy_vars_ue->dlsch_errors[eNB_id],
                phy_vars_ue->dlsch_received[eNB_id],
                phy_vars_ue->dlsch_fer[eNB_id],
                phy_vars_ue->PHY_measurements.wideband_cqi_tot[eNB_id]);
        }

#endif
#endif //DLSCH_THREAD
      } else {
        //  printf("PDSCH inactive in subframe %d\n",subframe_rx-1);
        phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack[subframe_prev].send_harq_status = 0;
      }

      // SI_DLSCH
      if (phy_vars_ue->dlsch_ue_SI[eNB_id]->active[subframe_prev] == 1) {
#ifdef DEBUG_PHY_PROC
        LOG_I(PHY,"SI is active in subframe %d\n",subframe_prev);
#endif

        // process symbols 10,11,12 (13) of last SF and trigger DLSCH decoding
        if (abstraction_flag==0) {
          start_meas(&phy_vars_ue->dlsch_llr_stats);

          for (m=pilot3; m<phy_vars_ue->lte_frame_parms.symbols_per_tti; m++) {
#ifdef DEBUG_PHY_PROC

            LOG_D(PHY,"[UE  %d] Frame %d, slot %d: DLSCH (SI) demod between pilot 3 and 4 (2nd slot), m %d\n",
                  phy_vars_ue->Mod_id,frame_rx,slot_rx,m);

#endif
            rx_pdsch(phy_vars_ue,
                     SI_PDSCH,
                     eNB_id,
                     eNB_id+1,
                     subframe_prev,  // subframe,
                     m,
                     0,
                     0,
                     phy_vars_ue->is_secondary_ue,
                     phy_vars_ue->dlsch_ue_SI[eNB_id]->current_harq_pid[subframe_prev]);
          }

          stop_meas(&phy_vars_ue->dlsch_llr_stats);
        }

        //  write_output("dlsch_ra_llr.m","llr",lte_ue_pdsch_vars_ra[eNB_id]->llr[0],40,1,0);

        phy_vars_ue->dlsch_ue_SI[eNB_id]->active[subframe_prev] = 0;

        if (frame_rx < phy_vars_ue->dlsch_SI_errors[eNB_id])
          phy_vars_ue->dlsch_SI_errors[eNB_id]=0;

        if (phy_vars_ue->dlsch_ue_SI[eNB_id]) {

          if (abstraction_flag==0) {

            //          dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
            phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->G =
              get_G(&phy_vars_ue->lte_frame_parms,
                    phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->nb_rb,
                    phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even,
                    2,
		    1,
                    phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
                    frame_rx,subframe_prev);

#ifdef DEBUG_PHY_PROC
            LOG_I(PHY,"Decoding DLSCH_SI : rb_alloc %x : nb_rb %d G %d TBS %d, num_pdcch_sym %d\n",phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even[0],
                  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->nb_rb,
                  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->G,
                  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->TBS,
                  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols);
#endif


            dlsch_unscrambling(&phy_vars_ue->lte_frame_parms,
                               0,
                               phy_vars_ue->dlsch_ue_SI[eNB_id],
                               phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->G,
                               phy_vars_ue->lte_ue_pdsch_vars_SI[eNB_id]->llr[0],
                               0,
                               subframe_prev<<1);

            ret = dlsch_decoding(phy_vars_ue,
                                 phy_vars_ue->lte_ue_pdsch_vars_SI[eNB_id]->llr[0],
                                 &phy_vars_ue->lte_frame_parms,
                                 phy_vars_ue->dlsch_ue_SI[eNB_id],
                                 phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0],
                                 frame_prev,subframe_prev,
                                 phy_vars_ue->dlsch_ue_SI[eNB_id]->current_harq_pid[subframe_prev],
                                 0,0);

#ifdef DEBUG_PHY_PROC

            for (int i=0; i<11; i++)
              LOG_D(PHY,"dlsch_output_buffer[%d]=%x\n",i,phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->c[0][i]);

#endif

          }

#ifdef PHY_ABSTRACTION
          else {
            LOG_D(PHY,"Calling dlsch_decoding_emul ...\n");
            ret = dlsch_decoding_emul(phy_vars_ue,
                                      subframe_prev,
                                      0,
                                      eNB_id);
          }

#endif

          if (ret == (1+phy_vars_ue->dlsch_ue_SI[eNB_id]->max_turbo_iterations)) {
            phy_vars_ue->dlsch_SI_errors[eNB_id]++;
#ifdef DEBUG_PHY_PROC
            LOG_I(PHY,"[UE  %d] Frame %d, subframe %d, received SI in error (TBS %d, mcs %d, rvidx %d, rballoc %X.%X.%X.%X\n",
		  phy_vars_ue->Mod_id,
		  frame_rx,
		  subframe_prev,
		  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->TBS,
                  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->mcs,
                  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rvidx,
		  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even[0],
                  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even[1],
		  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even[2],
                  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even[3]);
#endif
 
	    //	      dump_dlsch_SI(phy_vars_ue,eNB_id,subframe_prev);
            VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
            stop_meas(&phy_vars_ue->phy_proc_rx);
	    return(-1);
          } else {


#ifdef DEBUG_PHY_PROC
            //if ((frame_rx % 100) == 0)
            LOG_D(PHY,"[UE  %d] Frame %d, subframe %d, received SI for TBS %d, mcs %d, rvidx %d, rballoc %X.%X.%X.%X\n",
                  phy_vars_ue->Mod_id,frame_rx,subframe_prev,
		  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->TBS,
                  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->mcs,
                  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rvidx,
		  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even[0],
                  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even[1],
		  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even[2],
                  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even[3]);
#endif

#ifdef OPENAIR2
            /*
                            printf("\n\n");
                for (i=0;i<phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->TBS>>3;i++)
                  printf("%02x ",phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->b[i]);
                printf("\n");
              */
            mac_xface->ue_decode_si(phy_vars_ue->Mod_id,
                                    CC_id,
                                    frame_rx,
                                    eNB_id,
                                    phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->b,
                                    phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->TBS>>3);
            /*
              if ((frame_rx % 160) < 10)
                printf("sending SI to L2 in frame %d\n",frame_rx);
              */
#endif
          }
        }

        /*
        #ifdef DEBUG_PHY_PROC
        debug_LOG_D(PHY,"[UE  %d] Frame %d, slot %d: dlsch_decoding (SI) ret %d (%d errors)\n",
            phy_vars_ue->Mod_id,frame_rx,slot_rx,ret,phy_vars_ue->dlsch_SI_errors[eNB_id]);
        #endif
        */
      }


      if (phy_vars_ue->dlsch_ue_ra[eNB_id]->active[subframe_prev] == 1) {
#ifdef DEBUG_PHY_PROC
        LOG_D(PHY,"[UE  %d] Frame %d, slot %d: DLSCH (RA) demod symbols 10,11,12\n",phy_vars_ue->Mod_id,frame_rx,slot_rx);
#endif

        // process symbols 10,11,12 and trigger DLSCH decoding
        if (abstraction_flag==0) {
          start_meas(&phy_vars_ue->dlsch_llr_stats);

          for (m=pilot3; m<phy_vars_ue->lte_frame_parms.symbols_per_tti; m++)
            rx_pdsch(phy_vars_ue,
                     RA_PDSCH,
                     eNB_id,
                     eNB_id+1,
                     subframe_prev,  // subframe,
                     m, // symbol
                     0, // first_symbol_flag
                     0,
                     phy_vars_ue->is_secondary_ue,
                     phy_vars_ue->dlsch_ue_ra[eNB_id]->current_harq_pid[subframe_prev]);
        }

        stop_meas(&phy_vars_ue->dlsch_llr_stats);

        phy_vars_ue->dlsch_ue_ra[eNB_id]->active[subframe_prev] = 0;

        if (frame_rx < phy_vars_ue->dlsch_ra_errors[eNB_id])
          phy_vars_ue->dlsch_ra_errors[eNB_id]=0;

        if (phy_vars_ue->prach_resources[eNB_id]!=NULL)
          phy_vars_ue->dlsch_ue_ra[eNB_id]->rnti = phy_vars_ue->prach_resources[eNB_id]->ra_RNTI;
        else {
          LOG_E(PHY,"[UE %d] Frame %d, subframe %d: FATAL, prach_resources is NULL\n",phy_vars_ue->Mod_id,frame_rx,subframe_prev);
          mac_xface->macphy_exit("prach_resources is NULL");
          VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
          stop_meas(&phy_vars_ue->phy_proc_rx);
          return 0;
        }

        if (abstraction_flag==0) {
          phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->G = get_G(&phy_vars_ue->lte_frame_parms,
									 phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->nb_rb,
									 phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->rb_alloc_even,
									 2,
									 1,
									 phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
              frame_rx,
              subframe_prev);

#ifdef DEBUG_PHY_PROC
          LOG_D(PHY,"[UE] decoding RA (subframe %d): G %d,rnti %x\n" ,subframe_prev,
                phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->G,
                phy_vars_ue->dlsch_ue_ra[eNB_id]->rnti);
#endif

          dlsch_unscrambling(&phy_vars_ue->lte_frame_parms,
                             0,
                             phy_vars_ue->dlsch_ue_ra[eNB_id],
                             phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->G,
                             phy_vars_ue->lte_ue_pdsch_vars_ra[eNB_id]->llr[0],
                             0,
                             subframe_prev<<1);

          ret = dlsch_decoding(phy_vars_ue,
                               phy_vars_ue->lte_ue_pdsch_vars_ra[eNB_id]->llr[0],
                               &phy_vars_ue->lte_frame_parms,
                               phy_vars_ue->dlsch_ue_ra[eNB_id],
                               phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0],
                               frame_prev,subframe_prev,  // subframe
			       phy_vars_ue->dlsch_ue_ra[eNB_id]->current_harq_pid[subframe_prev],
                               0,0);
        }

#ifdef PHY_ABSTRACTION
        else {
          LOG_D(PHY,"Calling dlsch_decoding_emul ...\n");
          ret = dlsch_decoding_emul(phy_vars_ue,
                                    subframe_prev,
                                    1,
                                    eNB_id);
        }

#endif

        if (ret == (1+phy_vars_ue->dlsch_ue_ra[eNB_id]->max_turbo_iterations)) {
          phy_vars_ue->dlsch_ra_errors[eNB_id]++;
          LOG_D(PHY,"[UE  %d] Frame %d, subframe %d, received RA in error\n",phy_vars_ue->Mod_id,frame_rx,subframe_prev);
#ifdef USER_MODE
          //dump_dlsch_ra(phy_vars_ue,eNB_id,subframe_prev);
#endif
          //    oai_exit=1;
          VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
          stop_meas(&phy_vars_ue->phy_proc_rx);
          return(-1);

        } else {
#ifdef DEBUG_PHY_PROC
          LOG_D(PHY,"[UE  %d][RAPROC] Frame %d subframe %d Received RAR  mode %d\n",
                phy_vars_ue->Mod_id,
                frame_rx,
                subframe_prev, phy_vars_ue->UE_mode[eNB_id]);
#endif

#ifdef OPENAIR2

          if ((phy_vars_ue->UE_mode[eNB_id] != PUSCH) && (phy_vars_ue->prach_resources[eNB_id]->Msg3!=NULL)) {
            LOG_I(PHY,"[UE  %d][RAPROC] Frame %d subframe %d Invoking MAC for RAR (current preamble %d)\n",
                  phy_vars_ue->Mod_id,frame_rx-((subframe_prev==9) ? 1 : 0),
                  subframe_prev,
                  phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex);

            timing_advance = mac_xface->ue_process_rar(phy_vars_ue->Mod_id,
                             CC_id,
                             frame_rx-((subframe_prev==0) ? 1 : 0),
                             phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->b,
                             &phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,
                             phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex);


            if (timing_advance!=0xffff) 
            {

              LOG_I(PHY,"[UE  %d][RAPROC] Frame %d subframe %d Got rnti %x and timing advance %d from RAR\n",
                    phy_vars_ue->Mod_id,
                    frame_rx-((subframe_prev==9) ? 1 : 0),
                    subframe_prev,
                    phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,
                    timing_advance);

              //timing_advance = 0;
              process_timing_advance_rar(phy_vars_ue,timing_advance);

              if (mode!=debug_prach) {
                phy_vars_ue->ulsch_ue_Msg3_active[eNB_id]=1;
                get_Msg3_alloc(&phy_vars_ue->lte_frame_parms,
                               subframe_prev,
                               frame_rx-((subframe_prev==9) ? 1 : 0),
                               &phy_vars_ue->ulsch_ue_Msg3_frame[eNB_id],
                               &phy_vars_ue->ulsch_ue_Msg3_subframe[eNB_id]);

                LOG_D(PHY,"[UE  %d][RAPROC] Got Msg3_alloc Frame %d subframe %d: Msg3_frame %d, Msg3_subframe %d\n",
                      phy_vars_ue->Mod_id,
                      frame_rx-((subframe_prev==9) ? 1 : 0),
                      subframe_prev,
                      phy_vars_ue->ulsch_ue_Msg3_frame[eNB_id],
                      phy_vars_ue->ulsch_ue_Msg3_subframe[eNB_id]);
                harq_pid = subframe2harq_pid(&phy_vars_ue->lte_frame_parms,
                                             phy_vars_ue->ulsch_ue_Msg3_frame[eNB_id],
                                             phy_vars_ue->ulsch_ue_Msg3_subframe[eNB_id]);
                phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round = 0;

                phy_vars_ue->UE_mode[eNB_id] = RA_RESPONSE;
                //      phy_vars_ue->Msg3_timer[eNB_id] = 10;
                phy_vars_ue->ulsch_ue[eNB_id]->power_offset = 6;
                phy_vars_ue->ulsch_no_allocation_counter[eNB_id] = 0;
              }
            } 
            else 
            { 
              // PRACH preamble doesn't match RAR
              LOG_W(PHY,"[UE  %d][RAPROC] Received RAR preamble (%d) doesn't match !!!\n",
                    phy_vars_ue->Mod_id,
                    phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex);
            }
          } // mode != PUSCH

#else //OPENAIR2

          rar = phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->b+1;
          timing_advance = ((((uint16_t)(rar[0]&0x7f))<<4) + (rar[1]>>4));
          //timing_advance = phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->b[0];
          process_timing_advance_rar(phy_vars_ue,timing_advance);
#endif
        } //ret <= MAX_ITERATIONS

        /*
        #ifdef DEBUG_PHY_PROC
        debug_LOG_D(PHY,"[UE  %d] Frame %d, slot %d: dlsch_decoding (RA) ret %d (%d errors)\n",
            phy_vars_ue->Mod_id,frame_rx,slot_rx,ret,phy_vars_ue->dlsch_ra_errors[eNB_id]);
        #endif
        */
      } // dlsch_ue_ra[eNB_id]->active == 1

    }


    if ((((slot_rx%2)==0) && ((l==pilot1))) ||
        ((pmch_flag==1)&&(l==1)))  {

#ifdef DEBUG_PHY_PROC
      LOG_D(PHY,"[UE  %d] Frame %d, slot %d: Calling pdcch procedures (eNB %d)\n",phy_vars_ue->Mod_id,frame_rx,slot_rx,eNB_id);
#endif

      //      rt_printk("[PDCCH] Frame %d, slot %d, start %llu\n",frame_rx,slot_rx,rt_get_time_ns());
      if (lte_ue_pdcch_procedures(eNB_id,phy_vars_ue,abstraction_flag) == -1) {
#ifdef DEBUG_PHY_PROC
	  LOG_E(PHY,"[UE  %d] Frame %d, slot %d: Error in pdcch procedures\n",phy_vars_ue->Mod_id,frame_rx,slot_rx);
#endif
	  return(-1);
      }
    

      //      rt_printk("[PDCCH] Frame %d, slot %d, stop  %llu\n",frame_rx,slot_rx,rt_get_time_ns());
#ifdef DEBUG_PHY_PROC
      LOG_D(PHY,"num_pdcch_symbols %d\n",phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols);
#endif
    }

    if (abstraction_flag==0) {

      if (((slot_rx%2)==1) && (l==0)) {
        start_meas(&phy_vars_ue->dlsch_llr_stats);

        for (m=phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols;
             m<pilot2;
             m++) {

#ifndef DLSCH_THREAD

          if (phy_vars_ue->dlsch_ue[eNB_id][0]->active[subframe_rx] == 1)  {
            harq_pid = phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid[subframe_rx];
            LOG_D(PHY,"[UE %d] PDSCH active in subframe %d (%d), harq_pid %d\n",phy_vars_ue->Mod_id,subframe_rx,slot_rx,harq_pid);

            if ((phy_vars_ue->transmission_mode[eNB_id] == 5) &&
                (phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->dl_power_off==0) &&
                (openair_daq_vars.use_ia_receiver ==1)) {
              dual_stream_UE = 1;
              eNB_id_i = phy_vars_ue->n_connected_eNB;
              i_mod =  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->Qm;
            } else {
              dual_stream_UE = 0;
              eNB_id_i = eNB_id+1;
              i_mod = 0;
            }

            // process DLSCH received in first slot

            rx_pdsch(phy_vars_ue,
                     PDSCH,
                     eNB_id,
                     eNB_id_i,
                     subframe_rx,  // subframe,
                     m,
                     (m==phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols)?1:0,   // first_symbol_flag
                     dual_stream_UE,
                     i_mod,
                     harq_pid);
          } // CRNTI active

#endif

          if (phy_vars_ue->dlsch_ue_SI[eNB_id]->active[subframe_rx] == 1)  {
            // process SI DLSCH in first slot
            rx_pdsch(phy_vars_ue,
                     SI_PDSCH,
                     eNB_id,
                     eNB_id+1,
                     subframe_rx,  // subframe,
                     m,
                     (m==phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols)?1:0,   // first_symbol_flag
                     0,
                     phy_vars_ue->is_secondary_ue,
                     phy_vars_ue->dlsch_ue_SI[eNB_id]->current_harq_pid[subframe_rx]);
          } // SI active

          if (phy_vars_ue->dlsch_ue_ra[eNB_id]->active[subframe_rx] == 1)  {
            rx_pdsch(phy_vars_ue,
                     RA_PDSCH,
                     eNB_id,
                     eNB_id+1,
                     subframe_rx,  // subframe,
                     m,
                     (m==phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols)?1:0,
                     0,
                     phy_vars_ue->is_secondary_ue,
                     phy_vars_ue->dlsch_ue_ra[eNB_id]->current_harq_pid[subframe_rx]);
          } // RA active
        } // loop from first dlsch symbol to end of slot

        stop_meas(&phy_vars_ue->dlsch_llr_stats);
      } // 2nd quarter

      if (((slot_rx%2)==1) && (l==pilot1)) {
        start_meas(&phy_vars_ue->dlsch_llr_stats);

        for (m=pilot2; m<pilot3; m++) {

#ifndef DLSCH_THREAD

          if (phy_vars_ue->dlsch_ue[eNB_id][0]->active[subframe_rx] == 1) {
            harq_pid = phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid[subframe_rx];

            if ((phy_vars_ue->transmission_mode[eNB_id] == 5) &&
                (phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->dl_power_off==0) &&
                (openair_daq_vars.use_ia_receiver ==1)) {
              dual_stream_UE = 1;
              eNB_id_i = phy_vars_ue->n_connected_eNB;
              i_mod = phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->Qm;
            } else {
              dual_stream_UE = 0;
              eNB_id_i = eNB_id+1;
              i_mod = 0;
            }

            rx_pdsch(phy_vars_ue,
                     PDSCH,
                     eNB_id,
                     eNB_id_i,
                     subframe_rx,  // subframe,
                     m,
                     0,
                     dual_stream_UE,
                     i_mod,
                     phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid[subframe_rx]);
          } // CRNTI active

#endif

          if(phy_vars_ue->dlsch_ue_SI[eNB_id]->active[subframe_rx] == 1) {
            rx_pdsch(phy_vars_ue,
                     SI_PDSCH,
                     eNB_id,
                     eNB_id+1,
                     subframe_rx,  // subframe,
                     m,
                     0,   // first_symbol_flag
                     0,
                     phy_vars_ue->is_secondary_ue,
                     phy_vars_ue->dlsch_ue_SI[eNB_id]->current_harq_pid[subframe_rx]);
          } // SI active

          if (phy_vars_ue->dlsch_ue_ra[eNB_id]->active[subframe_rx] == 1) {
            rx_pdsch(phy_vars_ue,
                     RA_PDSCH,
                     eNB_id,
                     eNB_id+1,
                     subframe_rx,  // subframe,
                     m,
                     0,   // first_symbol_flag
                     0,
                     phy_vars_ue->is_secondary_ue,
                     phy_vars_ue->dlsch_ue_ra[eNB_id]->current_harq_pid[subframe_rx]);
          } // RA active

        } // loop over 3rd quarter

        stop_meas(&phy_vars_ue->dlsch_llr_stats);
      } // 3rd quarter of subframe
    } // abstraction_flag==0
  }// l loop

  // calculate some statistics
  if (slot_rx==19) {
    if (frame_rx % 10 == 0) {
      if ((phy_vars_ue->dlsch_received[eNB_id] - phy_vars_ue->dlsch_received_last[eNB_id]) != 0)
        phy_vars_ue->dlsch_fer[eNB_id] = (100*(phy_vars_ue->dlsch_errors[eNB_id] - phy_vars_ue->dlsch_errors_last[eNB_id]))/(phy_vars_ue->dlsch_received[eNB_id] - phy_vars_ue->dlsch_received_last[eNB_id]);

      phy_vars_ue->dlsch_errors_last[eNB_id] = phy_vars_ue->dlsch_errors[eNB_id];
      phy_vars_ue->dlsch_received_last[eNB_id] = phy_vars_ue->dlsch_received[eNB_id];
    }

    phy_vars_ue->bitrate[eNB_id] = (phy_vars_ue->total_TBS[eNB_id] - phy_vars_ue->total_TBS_last[eNB_id])*100;
    phy_vars_ue->total_TBS_last[eNB_id] = phy_vars_ue->total_TBS[eNB_id];
    LOG_D(PHY,"[UE %d] Calculating bitrate Frame %d: total_TBS = %d, total_TBS_last = %d, bitrate %f kbits\n",
          phy_vars_ue->Mod_id,frame_rx,phy_vars_ue->total_TBS[eNB_id],
          phy_vars_ue->total_TBS_last[eNB_id],(float) phy_vars_ue->bitrate[eNB_id]/1000.0);
    /*
    if ((frame_rx % 100 == 0)) {
      LOG_I(PHY,"Throughput %5.1f kbps\n",(float) phy_vars_ue->bitrate[eNB_id]/1000.0);
    }
    */
  }
  LOG_RX_LINE();

  if (is_pmch_subframe((subframe_rx==9?-1:0)+frame_rx,subframe_rx,&phy_vars_ue->lte_frame_parms)) {
    LOG_I(PHY,"ue calling pmch subframe ..\n ");

    if ((slot_rx%2)==1) {
      LOG_D(PHY,"[UE %d] Frame %d, subframe %d: Querying for PMCH demodulation(%d)\n",
            phy_vars_ue->Mod_id,(subframe_rx==9?-1:0)+frame_rx,subframe_rx,slot_rx);
#ifdef Rel10
      pmch_mcs = mac_xface->ue_query_mch(phy_vars_ue->Mod_id,
                                         CC_id,
                                         (subframe_rx==9?-1:0)+frame_rx,
                                         subframe_rx,
                                         eNB_id,
                                         &sync_area,
                                         &mcch_active);

      if (phy_vars_rn)
        phy_vars_rn->mch_avtive[subframe_rx]=0;

#else
      pmch_mcs=-1;
#endif

      if (pmch_mcs>=0) {
        LOG_D(PHY,"[UE %d] Frame %d, subframe %d: Programming PMCH demodulation for mcs %d\n",phy_vars_ue->Mod_id,(subframe_rx==9?-1:0)+frame_rx,subframe_rx,pmch_mcs);
        fill_UE_dlsch_MCH(phy_vars_ue,pmch_mcs,1,0,0);

        if (abstraction_flag == 0 ) {
          for (l=2; l<12; l++) {

            slot_fep_mbsfn(phy_vars_ue,
                           l,
                           subframe_rx,
                           0,0);//phy_vars_ue->rx_offset,0);
          }

          for (l=2; l<12; l++) {
            rx_pmch(phy_vars_ue,
                    0,
                    subframe_rx,
                    l);


          }

          /*  printf("PMCH decoding, Frame %d, subframe %d, G %d\n",
               (subframe_rx==9?-1:0)+frame_rx,
               subframe_rx,
               phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->G);
          */
          phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->G = get_G(&phy_vars_ue->lte_frame_parms,
              phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->nb_rb,
              phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->rb_alloc_even,
              phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->Qm,
              1,
              2,
              (subframe_rx==9?-1:0)+frame_rx,subframe_rx);

          dlsch_unscrambling(&phy_vars_ue->lte_frame_parms,1,phy_vars_ue->dlsch_ue_MCH[0],
                             phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->G,
                             phy_vars_ue->lte_ue_pdsch_vars_MCH[0]->llr[0],0,subframe_prev<<1);

          ret = dlsch_decoding(phy_vars_ue,
                               phy_vars_ue->lte_ue_pdsch_vars_MCH[0]->llr[0],
                               &phy_vars_ue->lte_frame_parms,
                               phy_vars_ue->dlsch_ue_MCH[0],
                               phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0],
                               frame_prev,subframe_prev,
                               0,
                               0,0);
        } else { // abstraction
#ifdef PHY_ABSTRACTION
          ret = dlsch_decoding_emul(phy_vars_ue,
                                    subframe_rx,
                                    5, // PMCH
                                    eNB_id);
#endif
        }

        if (mcch_active == 1)
          phy_vars_ue->dlsch_mcch_trials[sync_area][0]++;
        else
          phy_vars_ue->dlsch_mtch_trials[sync_area][0]++;

        if (ret == (1+phy_vars_ue->dlsch_ue_MCH[0]->max_turbo_iterations)) {
          if (mcch_active == 1)
            phy_vars_ue->dlsch_mcch_errors[sync_area][0]++;
          else
            phy_vars_ue->dlsch_mtch_errors[sync_area][0]++;

          LOG_D(PHY,"[%s %d] Frame %d, subframe %d: PMCH in error (%d,%d), not passing to L2 (TBS %d, iter %d,G %d)\n",
                (r_type == no_relay)? "UE": "RN/UE", phy_vars_ue->Mod_id,
                (subframe_rx==9?-1:0)+frame_rx,subframe_rx,
                phy_vars_ue->dlsch_mcch_errors[sync_area][0],
                phy_vars_ue->dlsch_mtch_errors[sync_area][0],
                phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->TBS>>3,
                phy_vars_ue->dlsch_ue_MCH[0]->max_turbo_iterations,
                phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->G);
          dump_mch(phy_vars_ue,0,phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->G,subframe_rx);
#ifdef DEBUG_DLSCH

          for (int i=0; i<phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->TBS>>3; i++) {
            LOG_T(PHY,"%02x.",phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->c[0][i]);
          }

          LOG_T(PHY,"\n");
#endif

          if (subframe_rx==9)
            mac_xface->macphy_exit("Why are we exiting here?");
        } else {
#ifdef Rel10

          if ((r_type == no_relay) || (mcch_active == 1)) {
            mac_xface->ue_send_mch_sdu(phy_vars_ue->Mod_id,
                                       CC_id,
                                       frame_rx,
                                       phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->b,
                                       phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->TBS>>3,
                                       eNB_id,// not relevant in eMBMS context
                                       sync_area);
            /*   for (i=0;i<phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->TBS>>3;i++)
              msg("%2x.",phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->b[i]);
            msg("\n");
            */

            if (mcch_active == 1)
              phy_vars_ue->dlsch_mcch_received[sync_area][0]++;
            else
              phy_vars_ue->dlsch_mtch_received[sync_area][0]++;


            if (phy_vars_ue->dlsch_mch_received_sf[subframe_rx%5][0] == 1 ) {
              phy_vars_ue->dlsch_mch_received_sf[subframe_rx%5][0]=0;
            } else {
              phy_vars_ue->dlsch_mch_received[0]+=1;
              phy_vars_ue->dlsch_mch_received_sf[subframe_rx][0]=1;
            }

          } else if (r_type == multicast_relay) { // mcch is not active here
            // only 1 harq process exists
            // Fix me: this could be a pointer copy
            memcpy (phy_vars_rn->dlsch_rn_MCH[subframe_rx]->harq_processes[0]->b,
                    phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->b,
                    phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->TBS>>3);
            // keep the tbs
            phy_vars_rn->mch_avtive[subframe_rx] = 1;
            phy_vars_rn->sync_area[subframe_rx] = sync_area; // this could also go the harq data struct
            phy_vars_rn->dlsch_rn_MCH[subframe_rx]->harq_processes[0]->TBS = phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->TBS;
            phy_vars_rn->dlsch_rn_MCH[subframe_rx]->harq_processes[0]->mcs = phy_vars_ue->dlsch_ue_MCH[0]->harq_processes[0]->mcs;
            LOG_I(PHY,"[RN/UE %d] Frame %d subframe %d: store the MCH PDU for MBSFN sync area %d (MCS %d, TBS %d)\n",
                  phy_vars_ue->Mod_id, frame_rx,subframe_rx,sync_area,
                  phy_vars_rn->dlsch_rn_MCH[subframe_rx]->harq_processes[0]->mcs,
                  phy_vars_rn->dlsch_rn_MCH[subframe_rx]->harq_processes[0]->TBS>>3);
#ifdef DEBUG_PHY

            for (int i=0; i<phy_vars_rn->dlsch_rn_MCH[subframe_rx]->harq_processes[0]->TBS>>3; i++)
              msg("%02x ",phy_vars_rn->dlsch_rn_MCH[subframe_rx]->harq_processes[0]->b[i]);

            msg("\n");
#endif
          } else
            LOG_W(PHY,"[UE %d] Frame %d: not supported option\n",phy_vars_ue->Mod_id, frame_rx);

#endif
        }
      }
    }
  }

#ifdef EMOS
  phy_procedures_emos_UE_RX(phy_vars_ue,slot_rx,eNB_id);
#endif

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
  stop_meas(&phy_vars_ue->phy_proc_rx);
  return (0);
}

int g_dlsch_fail = 0;
int g_dlsch_count = 0;
int g_dlsch_width = 0;

int phy_procedures_UE_RX(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint8_t abstraction_flag,runmode_t mode,
                         relaying_type_t r_type,PHY_VARS_RN *phy_vars_rn)
{
 
  uint16_t l,m,n_symb;
  //  int eNB_id = 0,
  int ret=0;
  uint8_t harq_pid = -1;
  int timing_advance;
  uint8_t pilot1,pilot2,pilot3;
#ifndef DLSCH_THREAD
  uint8_t i_mod = 0;
  int eNB_id_i = 1;
  uint8_t dual_stream_UE = 0;
#endif
#ifndef OPENAIR2
  uint8_t *rar;
#endif
  int pmch_flag=0;
  uint8_t sync_area=255;
  int pmch_mcs=-1;
  uint8_t mcch_active=0;
  int frame_rx = phy_vars_ue->frame_rx;
  int slot_rx = phy_vars_ue->slot_rx;
  int subframe_rx = slot_rx>>1;
  int subframe_prev = (subframe_rx+9)%10;
  int frame_prev = (subframe_rx==0)?(frame_rx-1):frame_rx;
#ifdef OPENAIR2
  int CC_id = phy_vars_ue->CC_id;
#endif

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_IN);


  start_meas(&phy_vars_ue->phy_proc_rx);
  LOG_D(PHY,"[%s %d] Frame %d subframe %d: Doing phy_procedures_UE_RX(%d)\n",
        (r_type == multicast_relay) ? "RN/UE" : "UE",
        phy_vars_ue->Mod_id,frame_rx, subframe_rx, slot_rx);

  if (phy_vars_ue->lte_frame_parms.Ncp == 0) 
  {  
    // normal prefix
    pilot1 = 4;
    pilot2 = 7;
    pilot3 = 11;
  } 
  else 
  { 
    // extended prefix
    pilot1 = 3;
    pilot2 = 6;
    pilot3 = 9;
  }

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if (subframe_select(&phy_vars_ue->lte_frame_parms,subframe_rx) == SF_S) 
  {
    if ((slot_rx%2)==0)
      n_symb = 5;//3;
    else
      n_symb = 0;
  } else {
    n_symb = phy_vars_ue->lte_frame_parms.symbols_per_tti/2;
  }

  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // This is normal processing (i.e. not MBSFN)
  // RX processing of symbols in slot_rx

  
  if( slot_rx%2 == 3 && subframe_rx == 0)
  {
      printf("===>Rx frame %d subframe %d rx_offset=%d cu_offset=%d\n",
        frame_rx,subframe_rx,
        phy_vars_ue->rx_offset,
        (phy_vars_ue->rx_offset+subframe_rx*phy_vars_ue->lte_frame_parms.samples_per_tti)%(phy_vars_ue->lte_frame_parms.samples_per_tti*10));
  }

  LOG_RX_LINE();
  PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);

  for (l=0; l<n_symb; l++) 
  {
      LOG_RX_LINE();
      PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);
      start_meas(&phy_vars_ue->ofdm_demod_stats);
      VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_SLOT_FEP, VCD_FUNCTION_IN);

      slot_fep(phy_vars_ue,
               l,
               slot_rx,
               phy_vars_ue->rx_offset,
               0,
	       0);
      VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_UE_SLOT_FEP, VCD_FUNCTION_OUT);
      stop_meas(&phy_vars_ue->ofdm_demod_stats);

    //if (subframe_select(&phy_vars_ue->lte_frame_parms,subframe_rx) == SF_DL)
    lte_ue_measurement_procedures(l,phy_vars_ue,eNB_id,abstraction_flag,mode);


    /*PBCH位于子帧0第二个时隙的前面四个符号，频域上占据72个中心子载波*/
    if ((slot_rx==1) && (l==4-phy_vars_ue->lte_frame_parms.Ncp)) 
    {

      lte_ue_pbch_procedures(eNB_id,phy_vars_ue,abstraction_flag);

    }

#ifdef DLSCH_THREAD
    if (phy_vars_ue->dlsch_ue[eNB_id][0]->active[subframe_rx] == 1)  {
      // activate thread since Chest is now done for slot before slot_rx
      if (l==0) {
        LOG_I(PHY,"frame %d, slot_rx %d: Calling rx_pdsch_thread for harq_pid %d\n",
            frame_rx,slot_rx, phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid[subframe_rx]);

        if (pthread_mutex_lock (&rx_pdsch_mutex) != 0) {               // Signal MAC_PHY Scheduler
          LOG_E(PHY,"[UE  %d] ERROR pthread_mutex_lock\n",phy_vars_ue->Mod_id);     // lock before accessing shared resource
          VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
        } else {
          rx_pdsch_instance_cnt++;
          (slot_rx == 0) ? (rx_pdsch_slot = 19) : (rx_pdsch_slot = (slot_rx-1));
          pthread_mutex_unlock (&rx_pdsch_mutex);

          if (rx_pdsch_instance_cnt == 0) {
            if (pthread_cond_signal(&rx_pdsch_cond) != 0) {
              LOG_E(PHY,"[UE  %d] ERROR pthread_cond_signal for rx_pdsch_cond\n",phy_vars_ue->Mod_id);
              VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
            }
          } else {
            LOG_W(PHY,"[UE  %d] Frame=%d, Slot=%d, RX_PDSCH thread for rx_pdsch_thread busy!!!\n",phy_vars_ue->Mod_id,frame_rx,slot_rx);
            VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
          }
        }

        // trigger DLSCH decoding thread
        if ((slot_rx%2)==1) // odd slots
          phy_vars_ue->dlsch_ue[eNB_id][0]->active = 0;
      }
    }

#endif

    // process last DLSCH symbols + invoke decoding
    if (((slot_rx%2)==0) && (l==0)) 
    {
        LOG_RX_LINE();
        PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);
      // Regular PDSCH
      LOG_D(PHY,"[UE %d] dlsch->active in subframe %d => %d\n",phy_vars_ue->Mod_id,subframe_prev,phy_vars_ue->dlsch_ue[eNB_id][0]->active);

      if (phy_vars_ue->dlsch_ue[eNB_id][0]->active[subframe_prev] == 1) 
      {
          harq_pid = phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid[subframe_prev];
        
#ifndef DLSCH_THREAD //USER_MODE
        LOG_D(PHY,"[UE %d] PDSCH active in subframe %d, harq_pid %d\n",phy_vars_ue->Mod_id,subframe_prev,harq_pid);

        if ((phy_vars_ue->transmission_mode[eNB_id] == 5) &&
            (phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->dl_power_off==0) &&
            (openair_daq_vars.use_ia_receiver ==1)) {
          dual_stream_UE = 1;
          eNB_id_i = phy_vars_ue->n_connected_eNB;
          i_mod = phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->Qm;

          if (frame_rx%100==0) {
            LOG_I(PHY,"using IA receiver\n");
          }
        } else {
          dual_stream_UE = 0;
          eNB_id_i = eNB_id+1;
          i_mod = 0;
        }

        // process symbols 10,11,12 and trigger DLSCH decoding

          start_meas(&phy_vars_ue->dlsch_llr_stats);

          for (m=pilot3; m<phy_vars_ue->lte_frame_parms.symbols_per_tti; m++) {

            rx_pdsch(phy_vars_ue,
                     PDSCH,
                     eNB_id,
                     eNB_id_i,
                     subframe_prev,  // subframe
                     m,                    // symbol
                     0,                    // first_symbol_flag
                     dual_stream_UE,
                     i_mod,
                     harq_pid);
          }

          stop_meas(&phy_vars_ue->dlsch_llr_stats);

        phy_vars_ue->dlsch_ue[eNB_id][0]->active[subframe_prev] = 0;
        LOG_RX_LINE();
        PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);

        LOG_D(PHY,"[UE  %d][PDSCH %x/%d] Frame %d subframe %d Scheduling DLSCH decoding\n",
              phy_vars_ue->Mod_id,
              phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
              harq_pid,
              (subframe_prev == 9) ? (frame_rx-1) : frame_rx,subframe_prev);

        if (phy_vars_ue->dlsch_ue[eNB_id][0]) 
        {
            phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->G = get_G(&phy_vars_ue->lte_frame_parms,
                phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->nb_rb,
                phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rb_alloc_even,
                phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->Qm,
                phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->Nl,
                phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
                frame_rx,subframe_prev);
            start_meas(&phy_vars_ue->dlsch_unscrambling_stats);
            dlsch_unscrambling(&phy_vars_ue->lte_frame_parms,
                               0,
                               phy_vars_ue->dlsch_ue[0][0],
                               phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->G,
                               phy_vars_ue->lte_ue_pdsch_vars[eNB_id]->llr[0],
                               0,
                               subframe_prev<<1);
            stop_meas(&phy_vars_ue->dlsch_unscrambling_stats);

            start_meas(&phy_vars_ue->dlsch_decoding_stats);
            uint8_t round_old = phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->round;
            ret = dlsch_decoding(phy_vars_ue,
                                 phy_vars_ue->lte_ue_pdsch_vars[eNB_id]->llr[0],
                                 &phy_vars_ue->lte_frame_parms,
                                 phy_vars_ue->dlsch_ue[eNB_id][0],
                                 phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid],
                                 frame_prev,subframe_prev,
                                 harq_pid,
                                 1,0);
            stop_meas(&phy_vars_ue->dlsch_decoding_stats);

          if (ret == (1+phy_vars_ue->dlsch_ue[eNB_id][0]->max_turbo_iterations)) 
          {
            phy_vars_ue->dlsch_errors[eNB_id]++;


            LOG_D(PHY,"[UE  %d][PDSCH %x/%d] Frame %d subframe %d harq=%d round=%d DLSCH in error (rv %d,mcs %d,TBS %d/%d rballoc=%d NPRB=%d/%d TPC=%d flag=%d vrb_type=%d/%d)\n",
                    phy_vars_ue->Mod_id,phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
                    harq_pid,frame_prev,subframe_prev,harq_pid,round_old,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rvidx,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS2,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rballoc,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->NPRB,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->nb_rb,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TPC,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->flag,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->vrb_type,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->vrb_type2
                  );
            print_info("DLSCH decode fail: Frame %d subframe %d harq %d round %d  rv %d (mcs %d,TBS %d/%d rballoc=%d NPRB=%d/%d TPC=%d flag=%d vrb_type=%d/%d)\n",
                    frame_prev,subframe_prev,harq_pid,round_old,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rvidx,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS2,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rballoc,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->NPRB,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->nb_rb,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TPC,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->flag,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->vrb_type,
                    phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->vrb_type2
                  );

            UE_DecodeFail(subframe_prev,round_old);

            g_dlsch_fail ++;
            if(++g_dlsch_count== 100)
            {
                LOG_W(PHY,"phy_procedures_UE_RX: g_dlsch_fail=%d g_dlsch_width=%d\n",g_dlsch_fail,g_dlsch_width);
                g_dlsch_width++;
                g_dlsch_fail = 0;
                g_dlsch_count = 0;
            }

            /*
            print_frame25TSymbol(&phy_vars_ue->lte_ue_common_vars.rxdata[0][0],
               phy_vars_ue->rx_offset,phy_vars_ue->lte_frame_parms.samples_per_tti * 10,
               frame_rx%1024,subframe_rx);
            print_frame25FSymbol(&phy_vars_ue->lte_ue_common_vars.rxdataF[0][0],frame_rx%1024,subframe_rx);
            */

          }
          else 
          {
              print_info("DLSCH decode succ: Frame %d subframe %d harq %d round %d  rv %d (mcs %d,TBS %d/%d rballoc=%d NPRB=%d/%d TPC=%d flag=%d vrb_type=%d/%d)\n",
                      frame_prev,subframe_prev,harq_pid,round_old,
                      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rvidx,
                      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs,
                      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS,
                      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS2,
                      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rballoc,
                      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->NPRB,
                      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->nb_rb,
                      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TPC,
                      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->flag,
                      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->vrb_type,
                      phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->vrb_type2
                    );

            LOG_D(PHY,"[UE  %d][PDSCH %x/%d] Frame %d subframe %d (slot_rx %d): Received DLSCH (rv %d,mcs %d,TBS %d)\n",
                  phy_vars_ue->Mod_id,phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
                  harq_pid,frame_prev,subframe_prev,slot_rx,
                  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rvidx,
                  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs,
                  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS);

            mac_xface->ue_send_sdu(phy_vars_ue->Mod_id,
                                   CC_id,
                                   frame_rx,
                                   phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->b,
                                   phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS>>3,
                                   eNB_id);
            phy_vars_ue->total_TBS[eNB_id] =  phy_vars_ue->total_TBS[eNB_id] +
                                              phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS;
            phy_vars_ue->total_received_bits[eNB_id] = phy_vars_ue->total_TBS[eNB_id] +
                phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS;
            
            UE_DecodeSucc(subframe_prev,round_old);

            if(++g_dlsch_count== 100)
            {
                LOG_W(PHY,"phy_procedures_UE_RX: g_dlsch_fail=%d g_dlsch_width=%d\n",g_dlsch_fail,g_dlsch_width);
                g_dlsch_width++;
                g_dlsch_fail = 0;
                g_dlsch_count = 0;
            }

            /*
            print_frame25TSymbol(&phy_vars_ue->lte_ue_common_vars.rxdata[0][0],
               phy_vars_ue->rx_offset,phy_vars_ue->lte_frame_parms.samples_per_tti * 10,
               frame_rx%1024,subframe_rx);
            print_frame25FSymbol(&phy_vars_ue->lte_ue_common_vars.rxdataF[0][0],frame_rx%1024,subframe_rx);*/

          }
        }
        LOG_RX_LINE();
        PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);


        LOG_D(PHY,"[UE  %d][PDSCH %x/%d] Frame %d subframe %d: PDSCH/DLSCH decoding iter %d (mcs %d, rv %d, TBS %d)\n",
              phy_vars_ue->Mod_id,
              phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,harq_pid,
              frame_rx,subframe_prev,ret,
              phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->mcs,
              phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->rvidx,
              phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->TBS
              );

        if (frame_rx%100==0) {
          LOG_D(PHY,"[UE  %d][PDSCH %x] Frame %d subframe %d dlsch_errors %d, dlsch_received %d, dlsch_fer %d, current_dlsch_cqi %d\n",
                phy_vars_ue->Mod_id,phy_vars_ue->dlsch_ue[eNB_id][0]->rnti,
                frame_rx,subframe_prev,
                phy_vars_ue->dlsch_errors[eNB_id],
                phy_vars_ue->dlsch_received[eNB_id],
                phy_vars_ue->dlsch_fer[eNB_id],
                phy_vars_ue->PHY_measurements.wideband_cqi_tot[eNB_id]);
        }

#endif //DLSCH_THREAD
      }
      else 
      {
        //  printf("PDSCH inactive in subframe %d\n",subframe_rx-1);
        phy_vars_ue->dlsch_ue[eNB_id][0]->harq_ack[subframe_prev].send_harq_status = 0;
        
        ClearUlAckInfo(frame_prev,subframe_prev,phy_vars_ue);
      }
      
      LOG_RX_LINE();
      PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);

      // SI_DLSCH
      if (phy_vars_ue->dlsch_ue_SI[eNB_id]->active[subframe_prev] == 1) 
      {
        LOG_D(PHY,"SI is active in subframe %d\n",subframe_prev);

        // process symbols 10,11,12 (13) of last SF and trigger DLSCH decoding
        if (abstraction_flag==0) {
          start_meas(&phy_vars_ue->dlsch_llr_stats);

          for (m=pilot3; m<phy_vars_ue->lte_frame_parms.symbols_per_tti; m++) {

            LOG_D(PHY,"[UE  %d] Frame %d, slot %d: DLSCH (SI) demod between pilot 3 and 4 (2nd slot), m %d\n",
                  phy_vars_ue->Mod_id,frame_rx,slot_rx,m);

            rx_pdsch(phy_vars_ue,
                     SI_PDSCH,
                     eNB_id,
                     eNB_id+1,
                     subframe_prev,  // subframe,
                     m,
                     0,
                     0,
                     phy_vars_ue->is_secondary_ue,
                     phy_vars_ue->dlsch_ue_SI[eNB_id]->current_harq_pid[subframe_prev]);
          }

          stop_meas(&phy_vars_ue->dlsch_llr_stats);
        }

        //  write_output("dlsch_ra_llr.m","llr",lte_ue_pdsch_vars_ra[eNB_id]->llr[0],40,1,0);

        phy_vars_ue->dlsch_ue_SI[eNB_id]->active[subframe_prev] = 0;

        if (frame_rx < phy_vars_ue->dlsch_SI_errors[eNB_id])
          phy_vars_ue->dlsch_SI_errors[eNB_id]=0;

        if (phy_vars_ue->dlsch_ue_SI[eNB_id])
        {


            //          dump_dci(&phy_vars_ue->lte_frame_parms, &dci_alloc_rx[i]);
            phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->G =
              get_G(&phy_vars_ue->lte_frame_parms,
                    phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->nb_rb,
                    phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even,
                    2,
		            1,
                    phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
                    frame_rx,subframe_prev);

           LOG_D(PHY,"Decoding DLSCH_SI : rb_alloc %x : nb_rb %d G %d TBS %d, num_pdcch_sym %d\n",phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even[0],
                  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->nb_rb,
                  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->G,
                  phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->TBS,
                  phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols);


            dlsch_unscrambling(&phy_vars_ue->lte_frame_parms,
                               0,
                               phy_vars_ue->dlsch_ue_SI[eNB_id],
                               phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->G,
                               phy_vars_ue->lte_ue_pdsch_vars_SI[eNB_id]->llr[0],
                               0,
                               subframe_prev<<1);

            ret = dlsch_decoding(phy_vars_ue,
                                 phy_vars_ue->lte_ue_pdsch_vars_SI[eNB_id]->llr[0],
                                 &phy_vars_ue->lte_frame_parms,
                                 phy_vars_ue->dlsch_ue_SI[eNB_id],
                                 phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0],
                                 frame_prev,subframe_prev,
                                 phy_vars_ue->dlsch_ue_SI[eNB_id]->current_harq_pid[subframe_prev],
                                 0,0);

#ifdef DEBUG_PHY_PROC
            for (int i=0; i<11; i++)
              LOG_D(PHY,"dlsch_output_buffer[%d]=%x\n",i,phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->c[0][i]);

#endif



          if (ret == (1+phy_vars_ue->dlsch_ue_SI[eNB_id]->max_turbo_iterations)) 
          {
                phy_vars_ue->dlsch_SI_errors[eNB_id]++;
                LOG_I(PHY,"[UE  %d] Frame %d, subframe %d, received SI in error (TBS %d/%d, mcs %d, rvidx %d, rballoc %X.%X.%X.%X rballoc=%d NPRB=%d/%d TPC=%d flag=%d vrb_type=%d/%d\n",
                phy_vars_ue->Mod_id,
                frame_rx,
                subframe_prev,
                phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->TBS,
                phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->TBS2,
                phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->mcs,
                phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rvidx,
                phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even[0],
                phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even[1],
                phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even[2],
                phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even[3],
                phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rballoc,
                phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->NPRB,
                phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->nb_rb,
                phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->TPC,
                phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->flag,
                phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->vrb_type,
                phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->vrb_type2
                );

                //	      dump_dlsch_SI(phy_vars_ue,eNB_id,subframe_prev);
                VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
                stop_meas(&phy_vars_ue->phy_proc_rx);
                return(-1);
          } 
          else 
          {


                //if ((frame_rx % 100) == 0)
                LOG_D(PHY,"[UE  %d] Frame %d, subframe %d, received SI for TBS %d, mcs %d, rvidx %d, rballoc %X.%X.%X.%X\n",
                      phy_vars_ue->Mod_id,frame_rx,subframe_prev,
                phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->TBS,
                      phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->mcs,
                      phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rvidx,
                phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even[0],
                      phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even[1],
                phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even[2],
                      phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->rb_alloc_even[3]);

                mac_xface->ue_decode_si(phy_vars_ue->Mod_id,
                                    CC_id,
                                    frame_rx,
                                    eNB_id,
                                    phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->b,
                                    phy_vars_ue->dlsch_ue_SI[eNB_id]->harq_processes[0]->TBS>>3);
          }
        }
      }


      if (phy_vars_ue->dlsch_ue_ra[eNB_id]->active[subframe_prev] == 1) 
      {
            LOG_D(PHY,"[UE  %d] Frame %d, slot %d: DLSCH (RA) demod symbols 10,11,12\n",phy_vars_ue->Mod_id,frame_rx,slot_rx);
            // process symbols 10,11,12 and trigger DLSCH decoding
            start_meas(&phy_vars_ue->dlsch_llr_stats);

            for (m=pilot3; m<phy_vars_ue->lte_frame_parms.symbols_per_tti; m++)
            {
                rx_pdsch(phy_vars_ue,
                     RA_PDSCH,
                     eNB_id,
                     eNB_id+1,
                     subframe_prev,  // subframe,
                     m, // symbol
                     0, // first_symbol_flag
                     0,
                     phy_vars_ue->is_secondary_ue,
                     phy_vars_ue->dlsch_ue_ra[eNB_id]->current_harq_pid[subframe_prev]);
            }
            stop_meas(&phy_vars_ue->dlsch_llr_stats);

            phy_vars_ue->dlsch_ue_ra[eNB_id]->active[subframe_prev] = 0;

            if (frame_rx < phy_vars_ue->dlsch_ra_errors[eNB_id])
              phy_vars_ue->dlsch_ra_errors[eNB_id]=0;

        if (phy_vars_ue->prach_resources[eNB_id]!=NULL)
          phy_vars_ue->dlsch_ue_ra[eNB_id]->rnti = phy_vars_ue->prach_resources[eNB_id]->ra_RNTI;
        else 
        {
          LOG_E(PHY,"[UE %d] Frame %d, subframe %d: FATAL, prach_resources is NULL\n",phy_vars_ue->Mod_id,frame_rx,subframe_prev);
          mac_xface->macphy_exit("prach_resources is NULL");
          VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
          stop_meas(&phy_vars_ue->phy_proc_rx);
          return 0;
        }

          phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->G = get_G(&phy_vars_ue->lte_frame_parms,
									 phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->nb_rb,
									 phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->rb_alloc_even,
									 2,
									 1,
									 phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols,
              frame_rx,
              subframe_prev);

          LOG_D(PHY,"[UE] decoding RA (subframe %d): G %d,rnti %x\n" ,subframe_prev,
                phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->G,
                phy_vars_ue->dlsch_ue_ra[eNB_id]->rnti);

          dlsch_unscrambling(&phy_vars_ue->lte_frame_parms,
                             0,
                             phy_vars_ue->dlsch_ue_ra[eNB_id],
                             phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->G,
                             phy_vars_ue->lte_ue_pdsch_vars_ra[eNB_id]->llr[0],
                             0,
                             subframe_prev<<1);

          ret = dlsch_decoding(phy_vars_ue,
                               phy_vars_ue->lte_ue_pdsch_vars_ra[eNB_id]->llr[0],
                               &phy_vars_ue->lte_frame_parms,
                               phy_vars_ue->dlsch_ue_ra[eNB_id],
                               phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0],
                               frame_prev,subframe_prev,  // subframe
			       phy_vars_ue->dlsch_ue_ra[eNB_id]->current_harq_pid[subframe_prev],
                               0,0);


        if (ret == (1+phy_vars_ue->dlsch_ue_ra[eNB_id]->max_turbo_iterations)) 
        {
          phy_vars_ue->dlsch_ra_errors[eNB_id]++;
          LOG_I(PHY,"[UE  %d] Frame %d, subframe %d, received RA in error\n",phy_vars_ue->Mod_id,frame_rx,subframe_prev);
          //    oai_exit=1;
          VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
          stop_meas(&phy_vars_ue->phy_proc_rx);
          return(-1);

        } 
        else 
        {
          LOG_I(PHY,"[UE  %d][RAPROC] Frame %d subframe %d Received RAR  mode %d\n",
                phy_vars_ue->Mod_id,
                frame_rx,
                subframe_prev, phy_vars_ue->UE_mode[eNB_id]);

#ifdef OPENAIR2

          if ((phy_vars_ue->UE_mode[eNB_id] != PUSCH) && (phy_vars_ue->prach_resources[eNB_id]->Msg3!=NULL)) {
            LOG_I(PHY,"[UE  %d][RAPROC] Frame %d subframe %d Invoking MAC for RAR (current preamble %d)\n",
                  phy_vars_ue->Mod_id,frame_rx-((subframe_prev==9) ? 1 : 0),
                  subframe_prev,
                  phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex);

            timing_advance = mac_xface->ue_process_rar(phy_vars_ue->Mod_id,
                             CC_id,
                             frame_rx-((subframe_prev==0) ? 1 : 0),
                             phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->b,
                             &phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,
                             phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex);


            if (timing_advance!=0xffff) 
            {

              LOG_I(PHY,"[UE  %d][RAPROC] Frame %d subframe %d Got rnti %x and timing advance %d from RAR\n",
                    phy_vars_ue->Mod_id,
                    frame_rx-((subframe_prev==9) ? 1 : 0),
                    subframe_prev,
                    phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->crnti,
                    timing_advance);

              //timing_advance = 0;
              process_timing_advance_rar(phy_vars_ue,timing_advance);

              if (mode!=debug_prach) {
                phy_vars_ue->ulsch_ue_Msg3_active[eNB_id]=1;
                get_Msg3_alloc(&phy_vars_ue->lte_frame_parms,
                               subframe_prev,
                               frame_rx-((subframe_prev==9) ? 1 : 0),
                               &phy_vars_ue->ulsch_ue_Msg3_frame[eNB_id],
                               &phy_vars_ue->ulsch_ue_Msg3_subframe[eNB_id]);

                LOG_D(PHY,"[UE  %d][RAPROC] Got Msg3_alloc Frame %d subframe %d: Msg3_frame %d, Msg3_subframe %d\n",
                      phy_vars_ue->Mod_id,
                      frame_rx-((subframe_prev==9) ? 1 : 0),
                      subframe_prev,
                      phy_vars_ue->ulsch_ue_Msg3_frame[eNB_id],
                      phy_vars_ue->ulsch_ue_Msg3_subframe[eNB_id]);
                harq_pid = subframe2harq_pid(&phy_vars_ue->lte_frame_parms,
                                             phy_vars_ue->ulsch_ue_Msg3_frame[eNB_id],
                                             phy_vars_ue->ulsch_ue_Msg3_subframe[eNB_id]);
                phy_vars_ue->ulsch_ue[eNB_id]->harq_processes[harq_pid]->round = 0;

                phy_vars_ue->UE_mode[eNB_id] = RA_RESPONSE;
                //      phy_vars_ue->Msg3_timer[eNB_id] = 10;
                phy_vars_ue->ulsch_ue[eNB_id]->power_offset = 6;
                phy_vars_ue->ulsch_no_allocation_counter[eNB_id] = 0;
              }
            } 
            else 
            { 
              // PRACH preamble doesn't match RAR
              LOG_W(PHY,"[UE  %d][RAPROC] Received RAR preamble (%d) doesn't match !!!\n",
                    phy_vars_ue->Mod_id,
                    phy_vars_ue->prach_resources[eNB_id]->ra_PreambleIndex);
            }
          } // mode != PUSCH

#else //OPENAIR2

          rar = phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->b+1;
          timing_advance = ((((uint16_t)(rar[0]&0x7f))<<4) + (rar[1]>>4));
          //timing_advance = phy_vars_ue->dlsch_ue_ra[eNB_id]->harq_processes[0]->b[0];
          process_timing_advance_rar(phy_vars_ue,timing_advance);
#endif
        } //ret <= MAX_ITERATIONS

      } // dlsch_ue_ra[eNB_id]->active == 1
      LOG_RX_LINE();
      PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);

    }

    LOG_RX_LINE();
    PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);

    if (((slot_rx%2)==0) && ((l==pilot1))) 
    {

      LOG_D(PHY,"[UE  %d] Frame %d, subframe %d,slot %d: Calling pdcch procedures (eNB %d)\n",
        phy_vars_ue->Mod_id,frame_rx,slot_rx>>1,slot_rx,eNB_id);

      if(subframe_rx == 41 && ((frame_rx & 0xff) == 0))//It's so strange that if modified to 41 that would be error!
      {
        printf("print_frame25TSymbol\n");
         print_frame25TSymbol(&phy_vars_ue->lte_ue_common_vars.rxdata[0][0],
            phy_vars_ue->rx_offset,phy_vars_ue->lte_frame_parms.samples_per_tti * 10,
            frame_rx%1024,subframe_rx);
         print_frame25FSymbol(&phy_vars_ue->lte_ue_common_vars.rxdataF[0][0],frame_rx%1024,subframe_rx);
      }

      LOG_RX_LINE();
      PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);
      //      rt_printk("[PDCCH] Frame %d, slot %d, start %llu\n",frame_rx,slot_rx,rt_get_time_ns());
      if (lte_ue_pdcch_procedures(eNB_id,phy_vars_ue,abstraction_flag) == -1) 
      {
    	  LOG_E(PHY,"[UE  %d] Frame %d, slot %d: Error in pdcch procedures\n",phy_vars_ue->Mod_id,frame_rx,slot_rx);
    	  return(-1);
      }
    

      LOG_D(PHY,"num_pdcch_symbols %d\n",phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols);
    }
    LOG_RX_LINE();
    PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);


      if (((slot_rx%2)==1) && (l==0)) 
      {
        start_meas(&phy_vars_ue->dlsch_llr_stats);

        for (m=phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols;
             m<pilot2;
             m++) {

#ifndef DLSCH_THREAD

          if (phy_vars_ue->dlsch_ue[eNB_id][0]->active[subframe_rx] == 1)  {
            harq_pid = phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid[subframe_rx];
            LOG_D(PHY,"[UE %d] PDSCH active in subframe %d (%d), harq_pid %d\n",phy_vars_ue->Mod_id,subframe_rx,slot_rx,harq_pid);

            if ((phy_vars_ue->transmission_mode[eNB_id] == 5) &&
                (phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->dl_power_off==0) &&
                (openair_daq_vars.use_ia_receiver ==1)) {
              dual_stream_UE = 1;
              eNB_id_i = phy_vars_ue->n_connected_eNB;
              i_mod =  phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->Qm;
            } else {
              dual_stream_UE = 0;
              eNB_id_i = eNB_id+1;
              i_mod = 0;
            }

            // process DLSCH received in first slot

            rx_pdsch(phy_vars_ue,
                     PDSCH,
                     eNB_id,
                     eNB_id_i,
                     subframe_rx,  // subframe,
                     m,
                     (m==phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols)?1:0,   // first_symbol_flag
                     dual_stream_UE,
                     i_mod,
                     phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid[subframe_rx]);
          } // CRNTI active

#endif

          if (phy_vars_ue->dlsch_ue_SI[eNB_id]->active[subframe_rx] == 1)  {
            // process SI DLSCH in first slot
            rx_pdsch(phy_vars_ue,
                     SI_PDSCH,
                     eNB_id,
                     eNB_id+1,
                     subframe_rx,  // subframe,
                     m,
                     (m==phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols)?1:0,   // first_symbol_flag
                     0,
                     phy_vars_ue->is_secondary_ue,
                     phy_vars_ue->dlsch_ue_SI[eNB_id]->current_harq_pid[subframe_rx]);
          } // SI active

          if (phy_vars_ue->dlsch_ue_ra[eNB_id]->active[subframe_rx] == 1)  {
            rx_pdsch(phy_vars_ue,
                     RA_PDSCH,
                     eNB_id,
                     eNB_id+1,
                     subframe_rx,  // subframe,
                     m,
                     (m==phy_vars_ue->lte_ue_pdcch_vars[eNB_id]->num_pdcch_symbols)?1:0,
                     0,
                     phy_vars_ue->is_secondary_ue,
                     phy_vars_ue->dlsch_ue_ra[eNB_id]->current_harq_pid[subframe_rx]);
          } // RA active
        } // loop from first dlsch symbol to end of slot

        stop_meas(&phy_vars_ue->dlsch_llr_stats);
      } // 2nd quarter

      if (((slot_rx%2)==1) && (l==pilot1)) 
      {
        start_meas(&phy_vars_ue->dlsch_llr_stats);

        for (m=pilot2; m<pilot3; m++) {

#ifndef DLSCH_THREAD

          if (phy_vars_ue->dlsch_ue[eNB_id][0]->active[subframe_rx] == 1) {
            harq_pid = phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid[subframe_rx];

            if ((phy_vars_ue->transmission_mode[eNB_id] == 5) &&
                (phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->dl_power_off==0) &&
                (openair_daq_vars.use_ia_receiver ==1)) {
              dual_stream_UE = 1;
              eNB_id_i = phy_vars_ue->n_connected_eNB;
              i_mod = phy_vars_ue->dlsch_ue[eNB_id][0]->harq_processes[harq_pid]->Qm;
            } else {
              dual_stream_UE = 0;
              eNB_id_i = eNB_id+1;
              i_mod = 0;
            }

            rx_pdsch(phy_vars_ue,
                     PDSCH,
                     eNB_id,
                     eNB_id_i,
                     subframe_rx,  // subframe,
                     m,
                     0,
                     dual_stream_UE,
                     i_mod,
                     phy_vars_ue->dlsch_ue[eNB_id][0]->current_harq_pid[subframe_rx]);
          } // CRNTI active

#endif

          if(phy_vars_ue->dlsch_ue_SI[eNB_id]->active[subframe_rx] == 1) {
            rx_pdsch(phy_vars_ue,
                     SI_PDSCH,
                     eNB_id,
                     eNB_id+1,
                     subframe_rx,  // subframe,
                     m,
                     0,   // first_symbol_flag
                     0,
                     phy_vars_ue->is_secondary_ue,
                     phy_vars_ue->dlsch_ue_SI[eNB_id]->current_harq_pid[subframe_rx]);
          } // SI active

          if (phy_vars_ue->dlsch_ue_ra[eNB_id]->active[subframe_rx] == 1) {
            rx_pdsch(phy_vars_ue,
                     RA_PDSCH,
                     eNB_id,
                     eNB_id+1,
                     subframe_rx,  // subframe,
                     m,
                     0,   // first_symbol_flag
                     0,
                     phy_vars_ue->is_secondary_ue,
                     phy_vars_ue->dlsch_ue_ra[eNB_id]->current_harq_pid[subframe_rx]);
          } // RA active

        } // loop over 3rd quarter

        stop_meas(&phy_vars_ue->dlsch_llr_stats);
      } // 3rd quarter of subframe
  }// l loop
  LOG_RX_LINE();
  PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);

  // calculate some statistics
  if (slot_rx==19) {
    if (frame_rx % 10 == 0) {
      if ((phy_vars_ue->dlsch_received[eNB_id] - phy_vars_ue->dlsch_received_last[eNB_id]) != 0)
        phy_vars_ue->dlsch_fer[eNB_id] = (100*(phy_vars_ue->dlsch_errors[eNB_id] - phy_vars_ue->dlsch_errors_last[eNB_id]))/(phy_vars_ue->dlsch_received[eNB_id] - phy_vars_ue->dlsch_received_last[eNB_id]);

      phy_vars_ue->dlsch_errors_last[eNB_id] = phy_vars_ue->dlsch_errors[eNB_id];
      phy_vars_ue->dlsch_received_last[eNB_id] = phy_vars_ue->dlsch_received[eNB_id];
    }

    phy_vars_ue->bitrate[eNB_id] = (phy_vars_ue->total_TBS[eNB_id] - phy_vars_ue->total_TBS_last[eNB_id])*100;
    phy_vars_ue->total_TBS_last[eNB_id] = phy_vars_ue->total_TBS[eNB_id];
    LOG_D(PHY,"[UE %d] Calculating bitrate Frame %d: total_TBS = %d, total_TBS_last = %d, bitrate %f kbits\n",
          phy_vars_ue->Mod_id,frame_rx,phy_vars_ue->total_TBS[eNB_id],
          phy_vars_ue->total_TBS_last[eNB_id],(float) phy_vars_ue->bitrate[eNB_id]/1000.0);
    /*
    if ((frame_rx % 100 == 0)) {
      LOG_I(PHY,"Throughput %5.1f kbps\n",(float) phy_vars_ue->bitrate[eNB_id]/1000.0);
    }
    */
  }



  LOG_RX_LINE();
  PERF_TIME_LINE(PERF_UE_phy_procedures_UE_RX, PERF_LINE);

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX, VCD_FUNCTION_OUT);
  stop_meas(&phy_vars_ue->phy_proc_rx);
  return (0);
}

#ifdef Rel10
int phy_procedures_RN_UE_RX(uint8_t slot_rx, uint8_t next_slot, relaying_type_t r_type)
{

  int do_proc =0; // do nothing by default

  switch(r_type) {
  case no_relay:
    do_proc=no_relay; // perform the normal UE operation
    break;

  case multicast_relay:
    if (slot_rx > 12)
      do_proc = 0; // do nothing
    else // SF#1, SF#2, SF3, SF#3, SF#4, SF#5, SF#6(do rx slot 12)
      do_proc = multicast_relay ; // do PHY procedures UE RX

    break;

  default: // should'not be here
    LOG_W(PHY,"Not supported relay type %d, do nothing \n", r_type);
    do_proc= 0;
    break;
  }

  return do_proc;
}
#endif


#if defined(ENABLE_ITTI)
#   if ENABLE_RAL
extern PHY_MEASUREMENTS PHY_measurements;

void phy_UE_lte_measurement_thresholds_test_and_report(instance_t instanceP, ral_threshold_phy_t* threshold_phy_pP, uint16_t valP)
{
  MessageDef *message_p = NULL;

  if (
    (
      (((short) threshold_phy_pP->threshold.threshold_val <  (short) valP) && (threshold_phy_pP->threshold.threshold_xdir == RAL_ABOVE_THRESHOLD)) ||
      (((short)threshold_phy_pP->threshold.threshold_val >  (short)valP) && (threshold_phy_pP->threshold.threshold_xdir == RAL_BELOW_THRESHOLD))
    )  ||
    (threshold_phy_pP->threshold.threshold_xdir == RAL_NO_THRESHOLD)
  ) {
    message_p = itti_alloc_new_message(TASK_PHY_UE , PHY_MEAS_REPORT_IND);
    memset(&PHY_MEAS_REPORT_IND(message_p), 0, sizeof(PHY_MEAS_REPORT_IND(message_p)));

    memcpy(&PHY_MEAS_REPORT_IND (message_p).threshold,
           &threshold_phy_pP->threshold,
           sizeof(PHY_MEAS_REPORT_IND (message_p).threshold));

    memcpy(&PHY_MEAS_REPORT_IND (message_p).link_param,
           &threshold_phy_pP->link_param,
           sizeof(PHY_MEAS_REPORT_IND (message_p).link_param));

    switch (threshold_phy_pP->link_param.choice) {
    case RAL_LINK_PARAM_CHOICE_LINK_PARAM_VAL:
      PHY_MEAS_REPORT_IND (message_p).link_param._union.link_param_val = valP;
      break;

    case RAL_LINK_PARAM_CHOICE_QOS_PARAM_VAL:
      //PHY_MEAS_REPORT_IND (message_p).link_param._union.qos_param_val.
      AssertFatal (1 == 0, "TO DO RAL_LINK_PARAM_CHOICE_QOS_PARAM_VAL\n");
      break;
    }

    //LOG_I(PHY,"[XXX] Sending link parameters report msg message to RRC\n");
    itti_send_msg_to_task(TASK_RRC_UE, instanceP, message_p);
  }
}

void phy_UE_lte_check_measurement_thresholds(instance_t instanceP, ral_threshold_phy_t* threshold_phy_pP)
{
  unsigned int  mod_id;
  int CC_id  = 0; // this should become the function argument, requested by the upper layers.

  mod_id = instanceP - NB_eNB_INST;

  switch (threshold_phy_pP->link_param.link_param_type.choice) {

  case RAL_LINK_PARAM_TYPE_CHOICE_GEN:
    switch (threshold_phy_pP->link_param.link_param_type._union.link_param_gen) {
    case RAL_LINK_PARAM_GEN_DATA_RATE:
      phy_UE_lte_measurement_thresholds_test_and_report(instanceP, threshold_phy_pP, 0);
      break;

    case RAL_LINK_PARAM_GEN_SIGNAL_STRENGTH:
      phy_UE_lte_measurement_thresholds_test_and_report(instanceP, threshold_phy_pP, 0);
      break;

    case RAL_LINK_PARAM_GEN_SINR:
      phy_UE_lte_measurement_thresholds_test_and_report(instanceP, threshold_phy_pP, 0);
      break;

    case RAL_LINK_PARAM_GEN_THROUGHPUT:
      break;

    default:
      ;
    }

    break;

  case RAL_LINK_PARAM_TYPE_CHOICE_LTE:
    switch (threshold_phy_pP->link_param.link_param_type._union.link_param_gen) {
    case RAL_LINK_PARAM_LTE_UE_RSRP:
      //                     phy_UE_lte_measurement_thresholds_test_and_report(instanceP, threshold_phy_pP, PHY_vars_UE_g[mod_id]->PHY_measurements.rx_rssi_dBm[0]);
      phy_UE_lte_measurement_thresholds_test_and_report(instanceP, threshold_phy_pP, PHY_vars_UE_g[mod_id][CC_id]->PHY_measurements.rsrp[0]);
      break;

    case RAL_LINK_PARAM_LTE_UE_RSRQ:
      phy_UE_lte_measurement_thresholds_test_and_report(instanceP, threshold_phy_pP, PHY_vars_UE_g[mod_id][CC_id]->PHY_measurements.rsrq[0]);
      break;

    case RAL_LINK_PARAM_LTE_UE_CQI:
      phy_UE_lte_measurement_thresholds_test_and_report(instanceP, threshold_phy_pP, PHY_vars_UE_g[mod_id][CC_id]->PHY_measurements.wideband_cqi_avg[0]);
      break;

    case RAL_LINK_PARAM_LTE_AVAILABLE_BW:
      break;

    case RAL_LINK_PARAM_LTE_PACKET_DELAY:
      break;

    case RAL_LINK_PARAM_LTE_PACKET_LOSS_RATE:
      break;

    case RAL_LINK_PARAM_LTE_L2_BUFFER_STATUS:
      break;

    case RAL_LINK_PARAM_LTE_MOBILE_NODE_CAPABILITIES:
      break;

    case RAL_LINK_PARAM_LTE_EMBMS_CAPABILITY:
      break;

    case RAL_LINK_PARAM_LTE_JUMBO_FEASIBILITY:
      break;

    case RAL_LINK_PARAM_LTE_JUMBO_SETUP_STATUS:
      break;

    case RAL_LINK_PARAM_LTE_NUM_ACTIVE_EMBMS_RECEIVERS_PER_FLOW:
      break;

    default:
      LOG_W(PHY,"unknown message %d\n", threshold_phy_pP->link_param.link_param_type._union.link_param_gen);
    }

    break;

  default:
    LOG_W(PHY,"unknown message %d\n", threshold_phy_pP->link_param.link_param_type.choice);
  }
}
#   endif
#endif

void phy_procedures_UE_lte(PHY_VARS_UE *phy_vars_ue,uint8_t eNB_id,uint8_t abstraction_flag,runmode_t mode,
                           relaying_type_t r_type, PHY_VARS_RN *phy_vars_rn)
{
#if defined(ENABLE_ITTI)
  MessageDef   *msg_p;
  const char   *msg_name;
  instance_t    instance;
  unsigned int  Mod_id;
  int           result;
#endif
#   if ENABLE_RAL
  int           CC_id =0;
#endif
  int           frame_rx = phy_vars_ue->frame_rx;
#ifdef OPENAIR2
  int           frame_tx = phy_vars_ue->frame_tx;
#endif
  int           slot_rx  = phy_vars_ue->slot_rx;
  int           slot_tx  = phy_vars_ue->slot_tx;
  int           subframe_tx = slot_tx>>1;
  int           subframe_rx = slot_rx>>1;
#undef DEBUG_PHY_PROC

#ifdef OPENAIR2
  UE_L2_STATE_t ret;
#endif
#ifndef OPENAIR2
  phy_vars_ue->UE_mode[eNB_id]=PUSCH;
  phy_vars_ue->prach_resources[eNB_id] = &prach_resources_local;
  prach_resources_local.ra_RNTI = 0xbeef;
  prach_resources_local.ra_PreambleIndex = 0;
#endif


  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_LTE,1);
#if defined(EXMIMO)
#ifndef OAI_USRP
  VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME(VCD_SIGNAL_DUMPER_VARIABLES_DAQ_MBOX, *((volatile unsigned int *) openair0_exmimo_pci[0].rxcnt_ptr[0]));
#endif
#endif
  start_meas(&phy_vars_ue->phy_proc);
#if defined(ENABLE_ITTI)

  do {
    // Checks if a message has been sent to PHY sub-task
    itti_poll_msg (TASK_PHY_UE, &msg_p);

    if (msg_p != NULL) {
      msg_name = ITTI_MSG_NAME (msg_p);
      instance = ITTI_MSG_INSTANCE (msg_p);
      Mod_id = instance - NB_eNB_INST;

      switch (ITTI_MSG_ID(msg_p)) {
      case PHY_FIND_CELL_REQ:
        LOG_I(PHY, "[UE %d] Received %s\n", Mod_id, msg_name);

        /* TODO process the message */
#   if ENABLE_RAL
        {
          MessageDef *message_p;

          message_p = itti_alloc_new_message(TASK_PHY_UE , PHY_FIND_CELL_IND);
          memset(&PHY_FIND_CELL_IND(message_p), 0, sizeof(PHY_FIND_CELL_IND(message_p)));
          PHY_FIND_CELL_IND (message_p).transaction_id   = PHY_FIND_CELL_REQ(msg_p).transaction_id;
          PHY_FIND_CELL_IND (message_p).cell_nb          = 1;
          PHY_FIND_CELL_IND (message_p).cells[0].earfcn  = 1;
          PHY_FIND_CELL_IND (message_p).cells[0].cell_id = 06;
          PHY_FIND_CELL_IND (message_p).cells[0].rsrp    = 39;
          PHY_FIND_CELL_IND (message_p).cells[0].rsrq    = 39;

          itti_send_msg_to_task(TASK_RRC_UE, instance, message_p);

        }
#   endif
        break;

#   if ENABLE_RAL

      case TIMER_HAS_EXPIRED:
        // check if it is a measurement timer
      {
        hashtable_rc_t       hashtable_rc;
        hashtable_rc = hashtable_is_key_exists(PHY_vars_UE_g[Mod_id][CC_id]->ral_thresholds_timed, (uint64_t)(TIMER_HAS_EXPIRED(msg_p).timer_id));
        LOG_I(PHY, "[UE %d] Received TIMER HAS EXPIRED: (hash_rc %d, HASH_TABLE_OK %d)\n", Mod_id, hashtable_rc, HASH_TABLE_OK);

        if (hashtable_rc == HASH_TABLE_OK) {
          phy_UE_lte_check_measurement_thresholds(instance, (ral_threshold_phy_t*)TIMER_HAS_EXPIRED(msg_p).arg);
        }
      }
      break;


      case PHY_MEAS_THRESHOLD_REQ:
#warning "TO DO LIST OF THRESHOLDS"
        LOG_I(PHY, "[UE %d] Received %s\n", Mod_id, msg_name);
        {
          ral_threshold_phy_t* threshold_phy_p  = NULL;
          int                  index, res;
          long                 timer_id;
          hashtable_rc_t       hashtable_rc;

          switch (PHY_MEAS_THRESHOLD_REQ(msg_p).cfg_param.th_action) {

          case RAL_TH_ACTION_CANCEL_THRESHOLD:
            break;

          case RAL_TH_ACTION_SET_NORMAL_THRESHOLD:
          case RAL_TH_ACTION_SET_ONE_SHOT_THRESHOLD:
            for (index = 0; index < PHY_MEAS_THRESHOLD_REQ(msg_p).cfg_param.num_thresholds; index++) {
              threshold_phy_p                  = calloc(1, sizeof(ral_threshold_phy_t));
              threshold_phy_p->th_action       = PHY_MEAS_THRESHOLD_REQ(msg_p).cfg_param.th_action;
              memcpy(&threshold_phy_p->link_param.link_param_type,
                     &PHY_MEAS_THRESHOLD_REQ(msg_p).cfg_param.link_param_type,
                     sizeof(ral_link_param_type_t));

              memcpy(&threshold_phy_p->threshold,
                     &PHY_MEAS_THRESHOLD_REQ(msg_p).cfg_param.thresholds[index],
                     sizeof(ral_threshold_t));

              switch (PHY_MEAS_THRESHOLD_REQ(msg_p).cfg_param.union_choice) {

              case RAL_LINK_CFG_PARAM_CHOICE_TIMER_NULL:
                switch (PHY_MEAS_THRESHOLD_REQ(msg_p).cfg_param.link_param_type.choice) {
                case RAL_LINK_PARAM_TYPE_CHOICE_GEN:
                  SLIST_INSERT_HEAD(
                    &PHY_vars_UE_g[Mod_id][CC_id]->ral_thresholds_gen_polled[PHY_MEAS_THRESHOLD_REQ(msg_p).cfg_param.link_param_type._union.link_param_gen],
                    threshold_phy_p,
                    ral_thresholds);
                  break;

                case RAL_LINK_PARAM_TYPE_CHOICE_LTE:
                  SLIST_INSERT_HEAD(
                    &PHY_vars_UE_g[Mod_id][CC_id]->ral_thresholds_lte_polled[PHY_MEAS_THRESHOLD_REQ(msg_p).cfg_param.link_param_type._union.link_param_lte],
                    threshold_phy_p,
                    ral_thresholds);
                  //LOG_E(PHY, "[UE %d] NORMAL/ONE SHOT - TIMER NULL - type LTE in %s\n", Mod_id, msg_name);
                  break;

                default:
                  LOG_E(PHY, "[UE %d] BAD PARAMETER cfg_param.link_param_type.choice %d in %s\n",
                        Mod_id, PHY_MEAS_THRESHOLD_REQ(msg_p).cfg_param.link_param_type.choice, msg_name);
                }

                break;

              case RAL_LINK_CFG_PARAM_CHOICE_TIMER:
                res = timer_setup(
                        (uint32_t)(PHY_MEAS_THRESHOLD_REQ(msg_p).cfg_param._union.timer_interval/1000),//uint32_t      interval_sec,
                        (uint32_t)(PHY_MEAS_THRESHOLD_REQ(msg_p).cfg_param._union.timer_interval%1000),//uint32_t      interval_us,
                        TASK_PHY_UE,
                        instance,
                        TIMER_PERIODIC,
                        threshold_phy_p,
                        &timer_id);

                if (res == 0) {
                  hashtable_rc = hashtable_insert(PHY_vars_UE_g[Mod_id][CC_id]->ral_thresholds_timed, (uint64_t )timer_id, (void*)threshold_phy_p);

                  if (hashtable_rc == HASH_TABLE_OK) {
                    threshold_phy_p->timer_id = timer_id;
                    LOG_I(PHY, "[UE %d] NORMAL/ONE SHOT - TIMER CHOICE - OK - in Hash %s\n", Mod_id, msg_name);
                  } else {
                    LOG_E(PHY, "[UE %d]  %s: Error in hashtable. Could not configure threshold index %d \n",
                          Mod_id, msg_name, index);
                  }

                } else {
                  LOG_E(PHY, "[UE %d]  %s: Could not configure threshold index %d because of timer initialization failure\n",
                        Mod_id, msg_name, index);
                }

                break;

              default: // already checked in RRC, should not happen here
                LOG_E(PHY, "[UE %d] BAD PARAMETER cfg_param.union_choice %d in %s\n",
                      Mod_id, PHY_MEAS_THRESHOLD_REQ(msg_p).cfg_param.union_choice, msg_name);
              }
            }

            break;

          default:
            LOG_E(PHY, "[UE %d] BAD PARAMETER th_action value %d in %s\n",
                  Mod_id, PHY_MEAS_THRESHOLD_REQ(msg_p).cfg_param.th_action, msg_name);
          }

        }
        break;
#   endif

      default:
        LOG_E(PHY, "[UE %d] Received unexpected message %s\n", Mod_id, msg_name);
        break;
      }

      result = itti_free (ITTI_MSG_ORIGIN_ID(msg_p), msg_p);
      AssertFatal (result == EXIT_SUCCESS, "Failed to free memory (%d)!\n", result);
    }
  } while(msg_p != NULL);

#endif

  if ((subframe_select(&phy_vars_ue->lte_frame_parms,subframe_tx)==SF_UL)||
      (phy_vars_ue->lte_frame_parms.frame_type == FDD)) {
    phy_procedures_UE_TX(phy_vars_ue,eNB_id,abstraction_flag,mode,r_type);
  }

  if ((subframe_select(&phy_vars_ue->lte_frame_parms,subframe_rx)==SF_DL) ||
      (phy_vars_ue->lte_frame_parms.frame_type == FDD)) {
#ifdef Rel10

    if (phy_procedures_RN_UE_RX(slot_rx, slot_tx, r_type) != 0 )
#endif
      phy_procedures_UE_RX(phy_vars_ue,eNB_id,abstraction_flag,mode,r_type,phy_vars_rn);
  }

  if ((subframe_select(&phy_vars_ue->lte_frame_parms,subframe_tx)==SF_S) &&
      ((slot_tx&1)==1)) {
    phy_procedures_UE_S_TX(phy_vars_ue,eNB_id,abstraction_flag,r_type);
  }

  if ((subframe_select(&phy_vars_ue->lte_frame_parms,subframe_rx)==SF_S) &&
      ((slot_rx&1)==0)) {
#ifdef Rel10

    if (phy_procedures_RN_UE_RX(slot_rx, slot_tx, r_type) != 0 )
#endif
      phy_procedures_UE_RX(phy_vars_ue,eNB_id,abstraction_flag,mode,r_type,phy_vars_rn);
  }

#ifdef OPENAIR2

  if (slot_rx%2==0) {

    printf("phy_procedures_UE_lte: call ue_scheduler\n");
    ret = mac_xface->ue_scheduler(phy_vars_ue->Mod_id,
                                  frame_tx,
                                  subframe_rx,
                                  subframe_select(&phy_vars_ue->lte_frame_parms,subframe_tx),
                                  eNB_id,
                                  0/*FIXME CC_id*/);

    if (ret == CONNECTION_LOST) {
      LOG_E(PHY,"[UE %d] Frame %d, subframe %d RRC Connection lost, returning to PRACH\n",phy_vars_ue->Mod_id,
            frame_rx,subframe_tx);
      phy_vars_ue->UE_mode[eNB_id] = PRACH;
      //      mac_xface->macphy_exit("Connection lost");
    } else if (ret == PHY_RESYNCH) {
      LOG_E(PHY,"[UE %d] Frame %d, subframe %d RRC Connection lost, trying to resynch\n",
            phy_vars_ue->Mod_id,
            frame_rx,subframe_tx);
      phy_vars_ue->UE_mode[eNB_id] = RESYNCH;
      //     mac_xface->macphy_exit("Connection lost");
    } else if (ret == PHY_HO_PRACH) {
      LOG_I(PHY,"[UE %d] Frame %d, subframe %d, return to PRACH and perform a contention-free access\n",
            phy_vars_ue->Mod_id,frame_rx,subframe_tx);
      phy_vars_ue->UE_mode[eNB_id] = PRACH;
    }
  }

#endif

  //  if (last_slot == 19)
  //    phy_vars_ue->frame++;

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_LTE,0);
  stop_meas(&phy_vars_ue->phy_proc);
}
