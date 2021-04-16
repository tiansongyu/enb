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

/*! \file PHY/LTE_TRANSPORT/initial_sync.c
* \brief Routines for initial UE synchronization procedure (PSS,SSS,PBCH and frame format detection)
* \author R. Knopp, F. Kaltenberger
* \date 2011
* \version 0.1
* \company Eurecom
* \email: knopp@eurecom.fr,kaltenberger@eurecom.fr
* \note
* \warning
*/
#include "PHY/types.h"
#include "PHY/defs.h"
#include "PHY/extern.h"
#include "SCHED/defs.h"
#include "SCHED/extern.h"
#include "defs.h"
#include "extern.h"
#ifdef EXMIMO
#include "gain_control.h"
#endif

#if defined(OAI_USRP) || defined(EXMIMO) || defined(ETHERNET)
#include "common_lib.h"
extern openair0_config_t openair0_cfg[];
#endif
#define DEBUG_INITIAL_SYNCH 

int pbch_detection(PHY_VARS_UE *phy_vars_ue, runmode_t mode) 
{

uint8_t l,pbch_decoded,frame_mod4,pbch_tx_ant,dummy;
LTE_DL_FRAME_PARMS *frame_parms=&phy_vars_ue->lte_frame_parms;
char phich_resource[6];

#ifdef DEBUG_INITIAL_SYNCH
  LOG_D(PHY,"[UE%d] Initial sync: starting PBCH detection (rx_offset %d) symbols_per_tti=%d mode=%d frame_type:%s\n",phy_vars_ue->Mod_id,
        phy_vars_ue->rx_offset, frame_parms->symbols_per_tti,mode,
        (phy_vars_ue->lte_frame_parms.frame_type==TDD)?"TDD":"FDD");
#endif

  //处理第1个时隙的符号
  for (l=0; l<frame_parms->symbols_per_tti/2; l++) {

    slot_fep_pbch(phy_vars_ue,
	     l,
	     0,
	     phy_vars_ue->rx_offset,
	     0,
	     1);
  }  
  //处理第2个时隙的符号
  for (l=0; l<frame_parms->symbols_per_tti/2; l++) {

    slot_fep_pbch(phy_vars_ue,
	     l,
	     1,
	     phy_vars_ue->rx_offset,
	     0,
	     1);
  }  
  //处理第3个时隙的符号0?
  slot_fep_pbch(phy_vars_ue,
	   0,
	   2,
	   phy_vars_ue->rx_offset,
	   0,
	   1);

  lte_ue_measurements(phy_vars_ue,
		      phy_vars_ue->rx_offset,
		      0,
		      0);
  
  
  if (phy_vars_ue->lte_frame_parms.frame_type == TDD) {
    ue_rrc_measurements(phy_vars_ue,
			1,
			0);
  }
  else {
    ue_rrc_measurements(phy_vars_ue,
			0,
			0);
  }
#ifdef DEBUG_INITIAL_SYNCH
  LOG_D(PHY,"[UE %d] RX RSSI %d dBm, digital (%d, %d) dB, linear (%d, %d), avg rx power %d dB (%d lin), RX gain %d dB\n",
        phy_vars_ue->Mod_id,
        phy_vars_ue->PHY_measurements.rx_rssi_dBm[0] - ((phy_vars_ue->lte_frame_parms.nb_antennas_rx==2) ? 3 : 0),
        phy_vars_ue->PHY_measurements.rx_power_dB[0][0],
        phy_vars_ue->PHY_measurements.rx_power_dB[0][1],
        phy_vars_ue->PHY_measurements.rx_power[0][0],
        phy_vars_ue->PHY_measurements.rx_power[0][1],
        phy_vars_ue->PHY_measurements.rx_power_avg_dB[0],
        phy_vars_ue->PHY_measurements.rx_power_avg[0],
        phy_vars_ue->rx_total_gain_dB);

  LOG_D(PHY,"[UE %d] N0 %d dBm digital (%d, %d) dB, linear (%d, %d), avg noise power %d dB (%d lin)\n",
        phy_vars_ue->Mod_id,
        phy_vars_ue->PHY_measurements.n0_power_tot_dBm,
        phy_vars_ue->PHY_measurements.n0_power_dB[0],
        phy_vars_ue->PHY_measurements.n0_power_dB[1],
        phy_vars_ue->PHY_measurements.n0_power[0],
        phy_vars_ue->PHY_measurements.n0_power[1],
        phy_vars_ue->PHY_measurements.n0_power_avg_dB,
        phy_vars_ue->PHY_measurements.n0_power_avg);
#endif

  pbch_decoded = 0;

  for (frame_mod4=0; frame_mod4<4; frame_mod4++) 
  {
    
    pbch_tx_ant = rx_pbch(&phy_vars_ue->lte_ue_common_vars,
                          phy_vars_ue->lte_ue_pbch_vars[0],
                          frame_parms,
                          0,
                          SISO,
                          phy_vars_ue->high_speed_flag,
                          frame_mod4);

    //printf("pbch_detection: SISO frame_mod4=%d pbch_tx_ant=%d\n",frame_mod4,pbch_tx_ant);

    if ((pbch_tx_ant>0) && (pbch_tx_ant<=2)) {
      pbch_decoded = 1;
      //break;
    }
    else
    {
    
        pbch_tx_ant = rx_pbch(&phy_vars_ue->lte_ue_common_vars,
                              phy_vars_ue->lte_ue_pbch_vars[0],
                              frame_parms,
                              0,
                              ALAMOUTI,
                              phy_vars_ue->high_speed_flag,
                              frame_mod4);

        //printf("pbch_detection: ALAMOUTI frame_mod4=%d pbch_tx_ant=%d\n",frame_mod4,pbch_tx_ant);

        if ((pbch_tx_ant>0) && (pbch_tx_ant<=2)) {
          pbch_decoded = 1;
          //break;
        }
    }


  if (pbch_decoded) 
  {

    frame_parms->nb_antennas_tx_eNB = pbch_tx_ant;

    // set initial transmission mode to 1 or 2 depending on number of detected TX antennas
    frame_parms->mode1_flag = (pbch_tx_ant==1);
    // openair_daq_vars.dlsch_transmission_mode = (pbch_tx_ant>1) ? 2 : 1;


    // flip byte endian on 24-bits for MIB
    //    dummy = phy_vars_ue->lte_ue_pbch_vars[0]->decoded_output[0];
    //    phy_vars_ue->lte_ue_pbch_vars[0]->decoded_output[0] = phy_vars_ue->lte_ue_pbch_vars[0]->decoded_output[2];
    //    phy_vars_ue->lte_ue_pbch_vars[0]->decoded_output[2] = dummy;

    // now check for Bandwidth of Cell
    dummy = (phy_vars_ue->lte_ue_pbch_vars[0]->decoded_output[2]>>5)&7;

    switch (dummy) {

    case 0 :
      frame_parms->N_RB_DL = 6;
      break;

    case 1 :
      frame_parms->N_RB_DL = 15;
      break;

    case 2 :
      frame_parms->N_RB_DL = 25;
      break;

    case 3 :
      frame_parms->N_RB_DL = 50;
      break;

    case 4 :
      frame_parms->N_RB_DL = 75;
      break;

    case 5:
      frame_parms->N_RB_DL = 100;
      break;

    default:
      LOG_E(PHY,"[UE%d] Initial sync: PBCH decoding: Unknown N_RB_DL\n",phy_vars_ue->Mod_id);
      return -1;
      break;
    }


    // now check for PHICH parameters
    frame_parms->phich_config_common.phich_duration = (PHICH_DURATION_t)((phy_vars_ue->lte_ue_pbch_vars[0]->decoded_output[2]>>4)&1);
    dummy = (phy_vars_ue->lte_ue_pbch_vars[0]->decoded_output[2]>>2)&3;

    switch (dummy) {
    case 0:
      frame_parms->phich_config_common.phich_resource = oneSixth;
      sprintf(phich_resource,"1/6");
      break;

    case 1:
      frame_parms->phich_config_common.phich_resource = half;
      sprintf(phich_resource,"1/2");
      break;

    case 2:
      frame_parms->phich_config_common.phich_resource = one;
      sprintf(phich_resource,"1");
      break;

    case 3:
      frame_parms->phich_config_common.phich_resource = two;
      sprintf(phich_resource,"2");
      break;

    default:
      LOG_E(PHY,"[UE%d] Initial sync: Unknown PHICH_DURATION\n",phy_vars_ue->Mod_id);
      return -1;
      break;
    }

    phy_vars_ue->frame_rx =   (((phy_vars_ue->lte_ue_pbch_vars[0]->decoded_output[2]&3)<<6) + (phy_vars_ue->lte_ue_pbch_vars[0]->decoded_output[1]>>2))<<2;
    phy_vars_ue->frame_rx += frame_mod4;

#ifndef USER_MODE
    // one frame delay
    phy_vars_ue->frame_rx ++;
#endif
    phy_vars_ue->frame_tx = phy_vars_ue->frame_rx;
#ifdef DEBUG_INITIAL_SYNCH
    LOG_D(PHY,"[UE%d] Initial sync: pbch decoded sucessfully mode1_flag %d, tx_ant %d, frame %d, N_RB_DL %d, phich_duration %d, phich_resource %s!\n",
          phy_vars_ue->Mod_id,
          frame_parms->mode1_flag,
          pbch_tx_ant,
          phy_vars_ue->frame_rx,
          frame_parms->N_RB_DL,
          frame_parms->phich_config_common.phich_duration,
          phich_resource);  //frame_parms->phich_config_common.phich_resource);
#endif
    printf("pbch_detection: SUCCESS!!! frame_mod4=%d\n", frame_mod4);

    return(0);
  }
  else 
  {
    printf("pbch_detection: FAIL!!! frame_mod4=%d\n",frame_mod4);
    return(-1);
  }
  }
}

char phich_string[13][4] = {"","1/6","","1/2","","","one","","","","","","two"};
char duplex_string[2][4] = {"FDD","TDD"};
char prefix_string[2][9] = {"NORMAL","EXTENDED"};
FILE *fp=NULL;
FILE *fp_syn=NULL;
void print_frame_syn(uint32_t *pData, char *msg)
{
    //return;
    print_info("%s", msg);
    for( int k = 0; k < LTE_NUMBER_OF_SUBFRAMES_PER_FRAME; k++)
    {
        print_frame25(pData, 960*8, 960*8*k, 0, k);
    }
}
void print_frame25(uint32_t *pData, int len, int start, int frame, int hw_subframe)
{
     int cur_pos = start;
     int cp_begin = 0;
     int sym_begin = 0;
     int sym_end = 0;
     int len1 = 0;
     for( int i = 0; i<14; i++)
     {
         sym_begin = cp_begin + 36;
         if( i == 0 || i == 7 )
         {
             sym_begin += 4;
         }
         sym_end = sym_begin + 512;
         if( cur_pos >= cp_begin && cur_pos <= sym_begin)
         {
             if( sym_begin < start+len)
             {
                 len1 = sym_begin-cur_pos;
             }
             else
             {
                 len1 = start+len-cur_pos;
             }
             print_buff(pData+cur_pos-start, len1, 0,"CP of frame%d subframe%d symbol %d  start=%d  len=%d\n", frame, hw_subframe, i, cur_pos-cp_begin, len1);
             cur_pos += len1;
             if( cur_pos >= len + start )
             {
                 return;
             }
         }
         if( cur_pos >= sym_begin && cur_pos <= sym_end)
         {
             if( sym_end < start+len)
             {
                 len1 = sym_end-cur_pos;
             }
             else
             {
                 len1 = start+len-cur_pos;
             }
             print_buff(pData+cur_pos-start, len1, 0,"Data of frame%d subframe%d symbol %d start=%d  len=%d\n", frame, hw_subframe, i, cur_pos-sym_begin, len1);
             cur_pos += len1;
             if( cur_pos >= len + start )
             {
                 return;
             }
         }
         cp_begin = sym_end;
     }
}
void print_frame25F(uint32_t *pData, int len, int start, int frame, int hw_subframe)
{
     int cur_pos = start;
     int sym_begin = 0;
     int sym_end = 0;
     int len1 = 0;
     return;
     for( int i = 0; i<14; i++)
     {
         sym_end = sym_begin + 512;

         if( cur_pos >= sym_begin && cur_pos <= sym_end)
         {
             if( sym_end < start+len)
             {
                 len1 = sym_end-cur_pos;
             }
             else
             {
                 len1 = start+len-cur_pos;
             }
             print_buff(pData+cur_pos-start, len1, 0,"DataF of frame%d subframe%d symbol %d start=%d  len=%d\n", frame, hw_subframe, i, cur_pos-sym_begin, len1);
             cur_pos += len1;
             if( cur_pos >= len + start )
             {
                 return;
             }
         }
         sym_begin = sym_end;
     }
}
void print_frame25FT(uint32_t *pDataF, uint32_t *pDataT, int frame, int hw_subframe)
{
     int cur_pos = 0;
     int len_cp = 0;
     return;
     for( int i = 0; i<14; i++)
     {
         print_buff(pDataF+512*i, 512, 0, "DataF of frame%d subframe%d symbol%d len=%d\n", frame, hw_subframe, i, 512);

         len_cp = ( i == 0 || i == 7 )?40:36;
         print_buff(pDataT+cur_pos, len_cp, 48-len_cp,"CP of frame%d subframe%d symbol%d len=%d\n", frame, hw_subframe, i, len_cp);
         cur_pos += len_cp;
         
         print_buff(pDataT+cur_pos, 512, 0,"Data of frame%d subframe%d symbol%d len=%d\n", frame, hw_subframe, i, 512);
         cur_pos += 512;


     }
}
void print_frame25FT2(uint32_t *pDataF, uint32_t *pDataT, uint32_t *pDataT2, int frame, int hw_subframe)
{
     int cur_pos = 0;
     int len_cp = 0;
     //return;
     for( int i = 0; i<14; i++)
     {
         print_buff(pDataF+512*i, 512, 0, "DataF of frame%d subframe%d symbol%d len=%d\n", frame, hw_subframe, i, 512);

         len_cp = ( i == 0 || i == 7 )?40:36;
         print_buff(pDataT+cur_pos, len_cp, 64-len_cp,"CP of frame%d subframe%d symbol%d len=%d\n", frame, hw_subframe, i, len_cp);
         print_buff(pDataT2+cur_pos, len_cp, 64-len_cp,"CP of frame%d subframe%d symbol%d len=%d\n", frame, hw_subframe, i, len_cp);
         cur_pos += len_cp;
         
         print_buff(pDataT+cur_pos, 512, 0,"Data of frame%d subframe%d symbol%d len=%d\n", frame, hw_subframe, i, 512);
         print_buff(pDataT2+cur_pos, 512, 0,"Data of frame%d subframe%d symbol%d len=%d\n", frame, hw_subframe, i, 512);
         cur_pos += 512;


     }
}

void print_frame25FT3(uint32_t *pDataF, uint32_t *pDataT, uint32_t *pDataT2, int frame, int hw_subframe)
{
     int cur_pos = 0;
     int len_cp = 0;

     
     //return;
     print_info("##################Data of frame%d subframe%d begin################\n", frame, hw_subframe);
     for( int i = 0; i<14; i++)
     {
         print_buff(pDataF+512*i, 512, 0, "DataF of frame%d subframe%d symbol%d len=%d\n", frame, hw_subframe, i, 512);
     
         len_cp = ( i == 0 || i == 7 )?40:36;
         print_buff(pDataT+cur_pos, len_cp, 64-len_cp,"CP of frame%d subframe%d symbol%d len=%d\n", frame, hw_subframe, i, len_cp);
         cur_pos += len_cp;
         
         print_buff(pDataT+cur_pos, 512, 0,"Data of frame%d subframe%d symbol%d len=%d\n", frame, hw_subframe, i, 512);
         cur_pos += 512;
     
     }
     //960*8=7680
     /*
     for( int i = 0; i<960; i++)
     {
         print_info("%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\n", 
            ((int16_t *)pDataT2)[2*i],((int16_t *)pDataT2)[2*i+1],
             ((int16_t *)pDataT2)[2*(960+i)],((int16_t *)pDataT2)[2*(960+i)+1],
             ((int16_t *)pDataT2)[2*(960*2+i)],((int16_t *)pDataT2)[2*(960*2+i)+1],
             ((int16_t *)pDataT2)[2*(960*3+i)],((int16_t *)pDataT2)[2*(960*3+i)+1],
             ((int16_t *)pDataT2)[2*(960*4+i)],((int16_t *)pDataT2)[2*(960*4+i)+1],
             ((int16_t *)pDataT2)[2*(960*5+i)],((int16_t *)pDataT2)[2*(960*5+i)+1],
             ((int16_t *)pDataT2)[2*(960*6+i)],((int16_t *)pDataT2)[2*(960*6+i)+1],
             ((int16_t *)pDataT2)[2*(960*7+i)],((int16_t *)pDataT2)[2*(960*7+i)+1]
            );
     }
     */

     print_info("##################Data of frame%d subframe%d end################\n", frame, hw_subframe);
}
void print_frame25FT33(uint32_t *pDataT1, uint32_t *pDataT2, uint32_t *pDataT3, int frame, int hw_subframe)
{
     int cur_pos = 0;
     int len_cp = 0;

     
     //return;
     print_info("##################Data of frame%d subframe%d begin################\n", frame, hw_subframe);
     for( int i = 0; i<14; i++)
     {
     
         len_cp = ( i == 0 || i == 7 )?40:36;
         print_buff(pDataT1+cur_pos, len_cp, 0,"CP of frame%d subframe%d symbol%d len=%d\n", frame, hw_subframe, i, len_cp);
         print_buff(pDataT3+cur_pos, len_cp, 0,"CP of frame%d subframe%d symbol%d len=%d\n", frame, hw_subframe, i, len_cp);
         print_buff(pDataT2+cur_pos, len_cp, 0,"CP of frame%d subframe%d symbol%d len=%d\n", frame, hw_subframe, i, len_cp);
         cur_pos += len_cp;
         
         print_buff(pDataT1+cur_pos, 512, 0,"Data of frame%d subframe%d symbol%d len=%d\n", frame, hw_subframe, i, 512);
         print_buff(pDataT3+cur_pos, 512, 0,"Data of frame%d subframe%d symbol%d len=%d\n", frame, hw_subframe, i, 512);
         print_buff(pDataT2+cur_pos, 512, 0,"Data of frame%d subframe%d symbol%d len=%d\n", frame, hw_subframe, i, 512);
         cur_pos += 512;

         break;
     }

     print_info("##################Data of frame%d subframe%d end################\n", frame, hw_subframe);
}

void print_frame25FT4( uint32_t *pDataT2, int frame, int hw_subframe)
{
     int cur_pos = 0;
     int len_cp = 0;

     
     //960*8=7680
     //print_buff(pDataT2, 32, 0,"Data of frame%d subframe%d head\n", frame, hw_subframe, 0);
     //print_buff(pDataT2+3840, 32, 0,"Data of frame%d subframe%d body\n", frame, hw_subframe, 0);
     //print_buff(pDataT2+7680-32, 32, 0,"Data of frame%d subframe%d tail\n", frame, hw_subframe, 0);

     print_info("##################Data of frame%d subframe%d begin################\n", frame, hw_subframe);
     for( int i = 0; i<960; i++)
     {
        if(i <=20 || i>=940)
         print_info("%3d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\n", 
             i,
            ((int16_t *)pDataT2)[2*i],((int16_t *)pDataT2)[2*i+1],
             ((int16_t *)pDataT2)[2*(960+i)],((int16_t *)pDataT2)[2*(960+i)+1],
             ((int16_t *)pDataT2)[2*(960*2+i)],((int16_t *)pDataT2)[2*(960*2+i)+1],
             ((int16_t *)pDataT2)[2*(960*3+i)],((int16_t *)pDataT2)[2*(960*3+i)+1],
             ((int16_t *)pDataT2)[2*(960*4+i)],((int16_t *)pDataT2)[2*(960*4+i)+1],
             ((int16_t *)pDataT2)[2*(960*5+i)],((int16_t *)pDataT2)[2*(960*5+i)+1],
             ((int16_t *)pDataT2)[2*(960*6+i)],((int16_t *)pDataT2)[2*(960*6+i)+1],
             ((int16_t *)pDataT2)[2*(960*7+i)],((int16_t *)pDataT2)[2*(960*7+i)+1]
            );
     }

     print_info("##################Data of frame%d subframe%d end################\n", frame, hw_subframe);
}
void print_frame25FT5( uint32_t *pDataT2, int frame, int hw_subframe, int head_line, int tail_line)
{
     int cur_pos = 0;
     int len_cp = 0;

     
     //960*8=7680
     //print_buff(pDataT2, 32, 0,"Data of frame%d subframe%d head\n", frame, hw_subframe, 0);
     //print_buff(pDataT2+3840, 32, 0,"Data of frame%d subframe%d body\n", frame, hw_subframe, 0);
     //print_buff(pDataT2+7680-32, 32, 0,"Data of frame%d subframe%d tail\n", frame, hw_subframe, 0);

     print_info("##################Data of frame%d subframe%d begin################\n", frame, hw_subframe);
     for( int i = 0; i<480; i++)
     {
        if(i < head_line || i>= 480 - tail_line)
        {
         print_info("%3d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d == %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d\n", 
             i,
            ((int16_t *)pDataT2)[2*i],((int16_t *)pDataT2)[2*i+2],((int16_t *)pDataT2)[2*i+4],((int16_t *)pDataT2)[2*i+6],
            ((int16_t *)pDataT2)[2*i+8],((int16_t *)pDataT2)[2*i+10],((int16_t *)pDataT2)[2*i+12],((int16_t *)pDataT2)[2*i+14],
            ((int16_t *)pDataT2)[2*i+16],((int16_t *)pDataT2)[2*i+18],((int16_t *)pDataT2)[2*i+20],((int16_t *)pDataT2)[2*i+22],
            ((int16_t *)pDataT2)[2*i+24],((int16_t *)pDataT2)[2*i+26],((int16_t *)pDataT2)[2*i+28],((int16_t *)pDataT2)[2*i+30],
            ((int16_t *)pDataT2)[2*i+1],((int16_t *)pDataT2)[2*i+3],((int16_t *)pDataT2)[2*i+5],((int16_t *)pDataT2)[2*i+7],
            ((int16_t *)pDataT2)[2*i+9],((int16_t *)pDataT2)[2*i+11],((int16_t *)pDataT2)[2*i+13],((int16_t *)pDataT2)[2*i+15],
            ((int16_t *)pDataT2)[2*i+17],((int16_t *)pDataT2)[2*i+19],((int16_t *)pDataT2)[2*i+21],((int16_t *)pDataT2)[2*i+23],
            ((int16_t *)pDataT2)[2*i+25],((int16_t *)pDataT2)[2*i+27],((int16_t *)pDataT2)[2*i+29],((int16_t *)pDataT2)[2*i+31]
            );
        }
     }

     print_info("##################Data of frame%d subframe%d end################\n", frame, hw_subframe);
}
void print_frame25FT6( uint32_t *pDataT2, int frame, int hw_subframe, int head_line, int tail_line)
{
     int cur_pos = 0;
     int len_cp = 0;

     
     //960*8=7680
     //print_buff(pDataT2, 32, 0,"Data of frame%d subframe%d head\n", frame, hw_subframe, 0);
     //print_buff(pDataT2+3840, 32, 0,"Data of frame%d subframe%d body\n", frame, hw_subframe, 0);
     //print_buff(pDataT2+7680-32, 32, 0,"Data of frame%d subframe%d tail\n", frame, hw_subframe, 0);

     print_info("##################Data of frame%d subframe%d begin################\n", frame, hw_subframe);
     for( int i = 0; i<240; i++)
     {
        if(i < head_line || i>= 240 - tail_line)
        {
         print_info("%3d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\n", 
             i,
            ((int16_t *)pDataT2)[64*i],((int16_t *)pDataT2)[64*i+2],((int16_t *)pDataT2)[64*i+4],((int16_t *)pDataT2)[64*i+6],
            ((int16_t *)pDataT2)[64*i+8],((int16_t *)pDataT2)[64*i+10],((int16_t *)pDataT2)[64*i+12],((int16_t *)pDataT2)[64*i+14],
            ((int16_t *)pDataT2)[64*i+16],((int16_t *)pDataT2)[64*i+18],((int16_t *)pDataT2)[64*i+20],((int16_t *)pDataT2)[64*i+22],
            ((int16_t *)pDataT2)[64*i+24],((int16_t *)pDataT2)[64*i+26],((int16_t *)pDataT2)[64*i+28],((int16_t *)pDataT2)[64*i+30],
             ((int16_t *)pDataT2)[64*i+32],((int16_t *)pDataT2)[64*i+34],((int16_t *)pDataT2)[64*i+36],((int16_t *)pDataT2)[64*i+38],
             ((int16_t *)pDataT2)[64*i+40],((int16_t *)pDataT2)[64*i+42],((int16_t *)pDataT2)[64*i+44],((int16_t *)pDataT2)[64*i+46],
             ((int16_t *)pDataT2)[64*i+48],((int16_t *)pDataT2)[64*i+50],((int16_t *)pDataT2)[64*i+52],((int16_t *)pDataT2)[64*i+54],
             ((int16_t *)pDataT2)[64*i+56],((int16_t *)pDataT2)[64*i+58],((int16_t *)pDataT2)[64*i+60],((int16_t *)pDataT2)[64*i+62]
            );
        }
     }

     print_info("##################Data of frame%d subframe%d end################\n", frame, hw_subframe);
}
void print_frame25FT7( uint32_t *pDataT2, int frame, int hw_subframe, int head_line, int tail_line)
{
    int i = 0;

     //if( frame > 2000 )
     {
        return;
     }
     //960*8=7680
     //print_buff(pDataT2, 32, 0,"Data of frame%d subframe%d head\n", frame, hw_subframe, 0);
     //print_buff(pDataT2+3840, 32, 0,"Data of frame%d subframe%d body\n", frame, hw_subframe, 0);
     //print_buff(pDataT2+7680-32, 32, 0,"Data of frame%d subframe%d tail\n", frame, hw_subframe, 0);

        print_info("##################Data of frame%d subframe%d begin (first symbol)################\n", frame, hw_subframe);
        for(i=0;i<69;i++)
        {
            print_info("%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t", 
            ((int16_t *)pDataT2)[16*i],((int16_t *)pDataT2)[16*i+2],((int16_t *)pDataT2)[16*i+4],((int16_t *)pDataT2)[16*i+6],
            ((int16_t *)pDataT2)[16*i+8],((int16_t *)pDataT2)[16*i+10],((int16_t *)pDataT2)[16*i+12],((int16_t *)pDataT2)[16*i+14]
            );
        }
        print_info("\n");
        for(i=69;i<77;i++)
        {
            print_info("%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t", 
            ((int16_t *)pDataT2)[16*i],((int16_t *)pDataT2)[16*i+2],((int16_t *)pDataT2)[16*i+4],((int16_t *)pDataT2)[16*i+6],
            ((int16_t *)pDataT2)[16*i+8],((int16_t *)pDataT2)[16*i+10],((int16_t *)pDataT2)[16*i+12],((int16_t *)pDataT2)[16*i+14]
            );
        }
        
        i=891;
        print_info("\n%5d\t%5d\t%5d\t%5d\t", 
        ((int16_t *)pDataT2)[16*i+8],((int16_t *)pDataT2)[16*i+10],((int16_t *)pDataT2)[16*i+12],((int16_t *)pDataT2)[16*i+14]);
        
        for(i=892;i<960;i++)
        {
            print_info("%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t", 
            ((int16_t *)pDataT2)[16*i],((int16_t *)pDataT2)[16*i+2],((int16_t *)pDataT2)[16*i+4],((int16_t *)pDataT2)[16*i+6],
            ((int16_t *)pDataT2)[16*i+8],((int16_t *)pDataT2)[16*i+10],((int16_t *)pDataT2)[16*i+12],((int16_t *)pDataT2)[16*i+14]
            );
        }

        print_info("\n##################Data of frame%d subframe%d end (last symbol)################\n", frame, hw_subframe);
        
}
void print_frame25FT8( uint32_t *pDataT2, int frame, int hw_subframe, int head_line, int tail_line)
{
    int i = 0;
     
     //960*8=7680
     //print_buff(pDataT2, 32, 0,"Data of frame%d subframe%d head\n", frame, hw_subframe, 0);
     //print_buff(pDataT2+3840, 32, 0,"Data of frame%d subframe%d body\n", frame, hw_subframe, 0);
     //print_buff(pDataT2+7680-32, 32, 0,"Data of frame%d subframe%d tail\n", frame, hw_subframe, 0);

        print_info("##################Data of frame%d subframe%d begin (first symbol QQQQ)################\n", frame, hw_subframe);
        for(i=0;i<69;i++)
        {
            print_info("%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t", 
            ((int16_t *)pDataT2)[16*i+1],((int16_t *)pDataT2)[16*i+3],((int16_t *)pDataT2)[16*i+5],((int16_t *)pDataT2)[16*i+7],
            ((int16_t *)pDataT2)[16*i+9],((int16_t *)pDataT2)[16*i+11],((int16_t *)pDataT2)[16*i+13],((int16_t *)pDataT2)[16*i+15]
            );
        }
      

        print_info("\n##################Data of frame%d subframe%d end (last symbol QQQQ)################\n", frame, hw_subframe);
        
}
void print_frame25FSymbol( uint32_t *pDataT, int frame, int hw_subframe)
{
    int i = 0;
     
     //960*8=7680
     //print_buff(pDataT2, 32, 0,"Data of frame%d subframe%d head\n", frame, hw_subframe, 0);
     //print_buff(pDataT2+3840, 32, 0,"Data of frame%d subframe%d body\n", frame, hw_subframe, 0);
     //print_buff(pDataT2+7680-32, 32, 0,"Data of frame%d subframe%d tail\n", frame, hw_subframe, 0);

        print_info("##################DataF of frame%d subframe%d begin ################\n", frame, hw_subframe);
        for(i=0;i<64;i++)
        {
            print_info("%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t", 
            ((int16_t *)pDataT)[16*i+1],((int16_t *)pDataT)[16*i+3],((int16_t *)pDataT)[16*i+5],((int16_t *)pDataT)[16*i+7],
            ((int16_t *)pDataT)[16*i+9],((int16_t *)pDataT)[16*i+11],((int16_t *)pDataT)[16*i+13],((int16_t *)pDataT)[16*i+15]
            );
        }
      
        print_info("\n");
        for(i=64;i<128;i++)
        {
          print_info("%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t", 
          ((int16_t *)pDataT)[16*i+1],((int16_t *)pDataT)[16*i+3],((int16_t *)pDataT)[16*i+5],((int16_t *)pDataT)[16*i+7],
          ((int16_t *)pDataT)[16*i+9],((int16_t *)pDataT)[16*i+11],((int16_t *)pDataT)[16*i+13],((int16_t *)pDataT)[16*i+15]
          );
        }

        print_info("\n##################DataF of frame%d subframe%d end ################\n", frame, hw_subframe);
        
}
void print_frame25TSymbol( uint32_t *pDataT, int pos, int size, int frame, int hw_subframe)
{
    int i = 0;
     
     //960*8=7680
     //print_buff(pDataT2, 32, 0,"Data of frame%d subframe%d head\n", frame, hw_subframe, 0);
     //print_buff(pDataT2+3840, 32, 0,"Data of frame%d subframe%d body\n", frame, hw_subframe, 0);
     //print_buff(pDataT2+7680-32, 32, 0,"Data of frame%d subframe%d tail\n", frame, hw_subframe, 0);
     uint32_t *pDataT2 ;
     int offset = (pos + hw_subframe*(7680))%size;

        print_info("############# Downlink Data of frame%d subframe%d begin pos=%d offset=%d size=%d################\n", 
            frame, hw_subframe,pos,offset,size);

        //print_info(" pos=%d offset=%d ################\n",pos,offset);

        
        for(i=0;i<69;i++)
        {
            pDataT2 = &pDataT[offset%size];
            print_info("%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t", 
            ((int16_t *)pDataT2)[0],((int16_t *)pDataT2)[2],((int16_t *)pDataT2)[4],((int16_t *)pDataT2)[6],
            ((int16_t *)pDataT2)[8],((int16_t *)pDataT2)[10],((int16_t *)pDataT2)[12],((int16_t *)pDataT2)[14]
            );
            offset+=8;
        }
        print_info("\n");
        for(i=69;i<138;i++)
        {
            pDataT2 = &pDataT[offset%size];
            print_info("%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t", 
            ((int16_t *)pDataT2)[0],((int16_t *)pDataT2)[2],((int16_t *)pDataT2)[4],((int16_t *)pDataT2)[6],
            ((int16_t *)pDataT2)[8],((int16_t *)pDataT2)[10],((int16_t *)pDataT2)[12],((int16_t *)pDataT2)[14]
            );
            offset+=8;
        }
        

        print_info("\n#############Downlink Data of frame%d subframe%d end ################\n", frame, hw_subframe);
        
}

#define PRINT_BUFF_SIMPLE 0
#define PRINT_BUFF_DETAIL 1
#define PRINT_BUFF_HEADTAIL 2
#define PRINT_BUFF_EXCEL 1

void printf_buf_all(FILE *fp, uint32_t *pData, int len,int flag,int line_size)
{
    if( flag == PRINT_BUFF_SIMPLE )
    {
        for( int kk = 0; kk < (len+255)/256; kk++)
        {
            fprintf(fp, "#%02d: ", kk*16);
            for( int kkk = 0; kkk < 16; kkk++)
            {  
                if( kk*256+kkk*16 >= len )
                {
                    break;
                }
                fprintf(fp, "%08x ", pData[kk*256+kkk*16]);
            }
            fprintf(fp, "\n");
        }
    }
    else if( flag == PRINT_BUFF_EXCEL )
    {
        for( int kk = 0; kk < (len+8)/8; kk++)
        {
            fprintf(fp, "%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t", 
                ((int16_t *)pData)[kk*16],((int16_t *)pData)[kk*16+2],((int16_t *)pData)[kk*16+4],((int16_t *)pData)[kk*16+6],
                ((int16_t *)pData)[kk*16+8],((int16_t *)pData)[kk*16+10],((int16_t *)pData)[kk*16+12],((int16_t *)pData)[kk*16+14]);
        }
        fprintf(fp, "\n");
        for( int kk = 0; kk < (len+8)/8; kk++)
        {
            fprintf(fp, "%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t", 
                ((int16_t *)pData)[kk*16+1],((int16_t *)pData)[kk*16+3],((int16_t *)pData)[kk*16+5],((int16_t *)pData)[kk*16+7],
                ((int16_t *)pData)[kk*16+9],((int16_t *)pData)[kk*16+11],((int16_t *)pData)[kk*16+13],((int16_t *)pData)[kk*16+15]);
        }
        fprintf(fp, "\n");
    }
    else
    {
        uint32_t ulTotalLine = (len+31+line_size)/32;
        for( int kk = 0; kk < ulTotalLine; kk++)
        {
            fprintf(fp, "#%02d: ", kk);
            for( int kkk = 0; kkk < 32; kkk++)
            {  
                if( kk*32+kkk >= len+line_size )
                {
                    break;
                }
                if(kk*32+kkk<line_size)
                {
                    fprintf(fp, "         ", pData[kk*32+kkk]);
                }
                else
                {
                    fprintf(fp, "%08x ", pData[kk*32+kkk-line_size]);
                }
            }
            fprintf(fp, "\n");
        }
    }
    fflush(fp);
    return;
}
extern uint8_t UE_flag;
uint8_t print_flag=1;
void print_buff(uint32_t *pData, int len,int line_size, const char* msg, ...)
{
    if( 0 == print_flag)return;
    
    if( fp == NULL )
    {
        if (UE_flag == 1) 
        {
            fp = fopen("./ue_buff.txt", "w");
        }
        else
        {
            fp = fopen("./enb_buff.txt", "w");
        }
        if( fp == NULL )
        {
           return;
        }
    }

	va_list argp;

	va_start(argp, msg);
	vfprintf(fp, msg, argp);

	va_end(argp);

	if(len == 0) return;
	
    //printf_buf_all(fp, pData, len,PRINT_BUFF_DETAIL,line_size);
    
}
void print_buff_excel(int type, void *pData_t, int len, const char* msg, ...)
{
    if( 0 == print_flag)return;

    if(type >= 10)return;
    
    if( fp == NULL )
    {
        if (UE_flag == 1) 
        {
            fp = fopen("./ue_buff.txt", "w");
        }
        else
        {
            fp = fopen("./enb_buff.txt", "w");
        }
        if( fp == NULL )
        {
           return;
        }
    }

	va_list argp;

	va_start(argp, msg);
	vfprintf(fp, msg, argp);

	va_end(argp);

	if(len == 0) 
        return;

    if( 0 == type )
    {
        int16_t *pData =  (int16_t *)pData_t;
        fprintf(fp, "I:\t");
        for( int kk = 0; kk < (len+8)/8; kk++)
        {
            fprintf(fp, "%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t", 
                (pData)[kk*16],(pData)[kk*16+2],(pData)[kk*16+4],(pData)[kk*16+6],
                (pData)[kk*16+8],(pData)[kk*16+10],(pData)[kk*16+12],(pData)[kk*16+14]);
        }
        fprintf(fp, "\nQ:\t");
        for( int kk = 0; kk < (len+8)/8; kk++)
        {
            fprintf(fp, "%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t", 
                (pData)[kk*16+1],(pData)[kk*16+3],(pData)[kk*16+5],(pData)[kk*16+7],
                (pData)[kk*16+9],(pData)[kk*16+11],(pData)[kk*16+13],(pData)[kk*16+15]);
        }
        fprintf(fp, "\n");
    }
    else if( 1 == type )
    {
        int16_t *pData =  (int16_t *)pData_t;
        for( int kk = 0; kk < (len+8)/8; kk++)
        {
            fprintf(fp, "%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t", 
                (pData)[kk*8],(pData)[kk*8+1],(pData)[kk*8+2],(pData)[kk*8+3],
                (pData)[kk*8+4],(pData)[kk*8+5],(pData)[kk*8+6],(pData)[kk*8+7]);
        }
        fprintf(fp, "\n");
    }
    else if( 2 == type )
    {
        int8_t *pData =  (int8_t *)pData_t;
        for( int kk = 0; kk < (len+8)/8; kk++)
        {
            fprintf(fp, "%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t", 
                (pData)[kk*8],(pData)[kk*8+1],(pData)[kk*8+2],(pData)[kk*8+3],
                (pData)[kk*8+4],(pData)[kk*8+5],(pData)[kk*8+6],(pData)[kk*8+7]);
        }
        fprintf(fp, "\n");
    }
    else if( 3 == type )
    {
        int16_t *pData =  (int16_t *)pData_t;
        /*打印前面6个符号*/
        for( int m = 0; m < (len)*12; m+=12)
        {
            fprintf(fp, "%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t", 
                pData[m]*2+pData[m+1],
                pData[m+2]*2+pData[m+3],
                pData[m+4]*2+pData[m+5],
                pData[m+6]*2+pData[m+7],
                pData[m+8]*2+pData[m+9],
                pData[m+10]*2+pData[m+11]
                );
        }
        fprintf(fp, "\n");
        /*打印最后6个符号*/
        pData +=  6*len*2;
        for( int m = 0; m < (len)*12; m+=12)
        {
            fprintf(fp, "%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t", 
                pData[m]*2+pData[m+1],
                pData[m+2]*2+pData[m+3],
                pData[m+4]*2+pData[m+5],
                pData[m+6]*2+pData[m+7],
                pData[m+8]*2+pData[m+9],
                pData[m+10]*2+pData[m+11]
                );
        }
        fprintf(fp, "\n");

    }
    else if(  4 == type)
    {
        int16_t *pData =  (int16_t *)pData_t;
        for( int symbol = 0; symbol < 12; symbol++)
        {
            for( int m = 0; m < (len)*2; m+=12)
            {
                fprintf(fp, "%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t", 
                    pData[m]*2+pData[m+1],
                    pData[m+2]*2+pData[m+3],
                    pData[m+4]*2+pData[m+5],
                    pData[m+6]*2+pData[m+7],
                    pData[m+8]*2+pData[m+9],
                    pData[m+10]*2+pData[m+11]
                    );
            }
            pData +=  25*12*2;
            if( symbol == 2 || symbol == 8)
            {
                //跳过导频
                pData +=  25*12*2;
            }
            if( symbol == 5 || symbol == 11)
            {
                fprintf(fp, "\n");
            }

        }


    }
    else if( 5 == type )
    {
        int8_t *pData =  (int8_t *)pData_t;
        for( int kk = 0; kk < (len); kk+=12)
        {
            fprintf(fp, "%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t%5d\t", 
                (pData)[kk],(pData)[kk+1],(pData)[kk+2],(pData)[kk+3],
                (pData)[kk+4],(pData)[kk+5],(pData)[kk+6],(pData)[kk+7],
                (pData)[kk+8],(pData)[kk+9],(pData)[kk+10],(pData)[kk+11]
                );
        }
        fprintf(fp, "\n");
    }
        
    
}


void print_info(const char* msg, ...)
{
    if( 0 == print_flag)return;
    
    //return;
    
    if( fp == NULL )
    {
        if (UE_flag == 1) 
        {
            fp = fopen("./ue_buff.txt", "w");
        }
        else
        {
            fp = fopen("./enb_buff.txt", "w");
        }
        if( fp == NULL )
        {
           return;
        }
    }

	va_list argp;

	va_start(argp, msg);
	vfprintf(fp, msg, argp);

	va_end(argp);
	
    fflush(fp);
    
}

//xhd_oai_perf
#define PERF_MAX_THREAD_NUM 30 



char gname[PERF_MAX_ITEM_NUM][100];
int gthread[PERF_MAX_THREAD_NUM];

typedef struct
{
    struct timespec start;
    struct timespec start2;
    struct timespec total;
    int count;
}run_time;
run_time gtotal[PERF_MAX_THREAD_NUM][PERF_MAX_ITEM_NUM];

void TIME_INIT()
{
    memset(gtotal, 0x00, sizeof(run_time)*PERF_MAX_THREAD_NUM*PERF_MAX_ITEM_NUM);
    memset(gthread, 0x00, sizeof(int)* PERF_MAX_THREAD_NUM);
}
#include <unistd.h>
#include <sys/syscall.h>

#define gettid() syscall(__NR_gettid)

run_time *TIME_find(int count, char *name)
{
   int tid = gettid();
   int i = 0;
   for( i = 0; i < PERF_MAX_THREAD_NUM; i++)
   {
       if( gthread[i] == 0 )
       {
           gthread[i] = tid;
           break;
       }
       if( gthread[i] == tid )
       {  
           break;
       }
   }
   if( i >= PERF_MAX_THREAD_NUM )
   {
       return NULL;
   }
   if( gname[count][0] == 0)
   {
      strncpy(gname[count], name, 90);
   }
   return &gtotal[i][count];
}

void TIME_PERF_START(int count, char *name)
{
    //return;
    
    run_time *pTemp = TIME_find(count, name);

    if( pTemp != NULL )
    {
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &pTemp->start); 
        clock_gettime(CLOCK_MONOTONIC, &pTemp->start2); 
    }
}

#define TIME_INC(total, start, curr, duration){\
    if( curr.tv_nsec > start.tv_nsec)\
    {\
        duration = (curr.tv_nsec - start.tv_nsec)/1000;\
    }\
    else\
    {\
        duration = 1000000 - ( start.tv_nsec - curr.tv_nsec )/1000;\
    }\
    if( total.tv_nsec + duration >= 1000000 )\
    {\
        total.tv_nsec -= (1000000-duration);\
        total.tv_sec++;\
    }\
    else\
    {\
        total.tv_nsec += duration;\
    }\
}
#define TIME_DURE(start, curr, duration){\
    if( curr.tv_nsec > start.tv_nsec)\
    {\
        duration = (curr.tv_nsec - start.tv_nsec)/1000;\
    }\
    else\
    {\
        duration = 1000000 - ( start.tv_nsec - curr.tv_nsec )/1000;\
    }\
}

void TIME_PERF_FINISH(int count, char *name)
{
    //return;

    run_time *pTemp = TIME_find(count, name);

    if( pTemp != NULL )
    {
        struct timespec current = {0,0};
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &current);

        int duration = 0;
        TIME_INC(pTemp->total, pTemp->start, current, duration);
        pTemp->count ++;
    }
}
void TIME_PERF_ONCE(int count, char *name)
{
    //return;

    run_time *pTemp = TIME_find(count, name);

    if( pTemp != NULL )
    {
        struct timespec current1 = {0,0};
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &current1);
        int duration1 = 0;
        TIME_DURE(pTemp->start, current1, duration1);

        struct timespec current2 = {0,0};
        clock_gettime(CLOCK_MONOTONIC, &current2);
        int duration2 = 0;
        TIME_DURE(pTemp->start2, current2, duration2);

        if( duration1 < 2000 && duration2 < 2000)
        {
            return;
        }
        
        printf("#####[%s]: %3llu.%03llu ms  %3llu.%03llu ms\n",
                    name,
                    duration1/1000,
                    duration1%1000,
                    duration2/1000,
                    duration2%1000);
    }
}
void TIME_PERF_LINE(int count, char *name, int line)
{
    //return;

    run_time *pTemp = TIME_find(count, name);

    if( pTemp != NULL )
    {
        struct timespec current1 = {0,0};
        clock_gettime(CLOCK_THREAD_CPUTIME_ID, &current1);
        int duration1 = 0;
        TIME_DURE(pTemp->start, current1, duration1);

        struct timespec current2 = {0,0};
        clock_gettime(CLOCK_MONOTONIC, &current2);
        int duration2 = 0;
        TIME_DURE(pTemp->start2, current2, duration2);

        if( duration1 < 2000 && duration2 < 2000)
        {
            return;
        }
        
        printf("#####[%s line=%d]: %3u.%03u ms  %3d.%03d ms \n",
                    name,line,
                    duration1/1000,
                    duration1%1000,
                    (duration2/1000)&0xffff,
                    (duration2%1000)&0xffff);
        /*
        printf("#####[%s line=%d]: ",
                    name,line);
        printf(" %3llu.%03llu ms  %3llu.%03llu ms \n",
                    duration1/1000,
                    duration1%1000,
                    duration2/1000,
                    duration2%1000);*/
    }
}

void TIME_PERF_PERIOD(int count, char *name)
{
    //return;

    run_time *pTemp = TIME_find(count, name);

    if( pTemp != NULL )
    {
        struct timespec current = {0,0};
        clock_gettime(CLOCK_MONOTONIC, &current);
        if( 0 != pTemp->start.tv_nsec )
        {
            int duration = 0;
            TIME_INC(pTemp->total, pTemp->start, current, duration);
            pTemp->count ++;
        }
        pTemp->start.tv_sec = current.tv_sec;
        pTemp->start.tv_nsec = current.tv_nsec;
    }
}
void TIME_PERF_PERIOD_WAIT(int count, char *name)
{
    //return;

    run_time *pTemp = TIME_find(count, name);

    if( pTemp != NULL )
    {
        struct timespec current = {0,0};
        clock_gettime(CLOCK_MONOTONIC, &current);
        if( 0 != pTemp->start.tv_nsec )
        {
            int duration = 0;
            TIME_INC(pTemp->total, pTemp->start, current, duration);
            pTemp->count ++;

            if( duration < 500000)
            {
                 //rt_sleep_ns(1000000-duration);
                 usleep((500000-duration)/1000);
            }
        }
        clock_gettime(CLOCK_MONOTONIC, &current);
        pTemp->start.tv_sec = current.tv_sec;
        pTemp->start.tv_nsec = current.tv_nsec;
    }
}
void TIME_DUMP()
{
    //return;

    for( int threadid = 0; threadid < PERF_MAX_THREAD_NUM; threadid++)
    {
        if( gthread[threadid] == 0 )
        {
            continue;
        }
        print_info("==================THREAD[%d] %d=========================\n", 
            threadid, gthread[threadid]);
        for( int i = 0; i < PERF_MAX_ITEM_NUM; i++)
        {
            if( gtotal[threadid][i].count == 0)
            {
                continue;
            }
            print_info("func[%02d]:%-30s total:  %5llu s %3llu.%03llu ms  count: %10d average %3llu.%03llu ms \n",
                        i, 
                        gname[i],
                        gtotal[threadid][i].total.tv_sec,
                        gtotal[threadid][i].total.tv_nsec/1000,
                        gtotal[threadid][i].total.tv_nsec%1000,
                        gtotal[threadid][i].count,
                        ((gtotal[threadid][i].total.tv_sec*1000000+gtotal[threadid][i].total.tv_nsec)/gtotal[threadid][i].count)/1000,
                        ((gtotal[threadid][i].total.tv_sec*1000000+gtotal[threadid][i].total.tv_nsec)/gtotal[threadid][i].count)%1000
                        );
            gtotal[threadid][i].total.tv_sec = 0;
            gtotal[threadid][i].total.tv_nsec = 0;
            gtotal[threadid][i].count = 0;
        }
    }
}


int initial_sync_old(PHY_VARS_UE *phy_vars_ue, runmode_t mode)
{

  int32_t sync_pos,sync_pos2,sync_pos_slot;
  int32_t metric_fdd_ncp=0,metric_fdd_ecp=0,metric_tdd_ncp=0,metric_tdd_ecp=0;
  uint8_t phase_fdd_ncp,phase_fdd_ecp,phase_tdd_ncp,phase_tdd_ecp;
  uint8_t flip_fdd_ncp=0,flip_fdd_ecp,flip_tdd_ncp,flip_tdd_ecp;
  //  uint16_t Nid_cell_fdd_ncp=0,Nid_cell_fdd_ecp=0,Nid_cell_tdd_ncp=0,Nid_cell_tdd_ecp=0;
  LTE_DL_FRAME_PARMS *frame_parms = &phy_vars_ue->lte_frame_parms;
  int i;
  int ret=-1;
  int aarx,rx_power=0;
  #ifdef OAI_USRP
  __m128i *rxdata128;
  #endif
  //  LOG_I(PHY,"**************************************************************\n");
  // First try FDD normal prefix
  frame_parms->Ncp=NORMAL;
  frame_parms->frame_type=FDD;
  init_frame_parms(frame_parms,1);

  //  write_output("rxdata0.m","rxd0",phy_vars_ue->lte_ue_common_vars.rxdata[0],10*frame_parms->samples_per_tti,1,1);

#if defined(__x86_64__) 
    LOG_I(PHY,"[UE%d] Initial sync =============================== x86 64 ============== \n",
        phy_vars_ue->Mod_id);
#else

#if defined(__i386__)
    LOG_I(PHY,"[UE%d] Initial sync ============================ i386 ============= \n",
        phy_vars_ue->Mod_id);
#else
    LOG_I(PHY,"[UE%d] Initial sync ============================ ??? ============= \n",
        phy_vars_ue->Mod_id);

#endif
#endif

#ifdef OAI_USRP1
  LOG_I(PHY,"[UE%d] Initial sync : OAI_USRP call _mm_srai_epi16\n",
     phy_vars_ue->Mod_id);
  //print_frame_syn(phy_vars_ue->lte_ue_common_vars.rxdata[0], "AAA");

  //向右移4位
  for (aarx = 0; aarx<frame_parms->nb_antennas_rx;aarx++) 
  {
    rxdata128 = (__m128i*)phy_vars_ue->lte_ue_common_vars.rxdata[aarx];
    for (i=0; i<(frame_parms->samples_per_tti*10)>>2; i++) {
      rxdata128[i] = _mm_srai_epi16(rxdata128[i],4);
    } 
  }
  //print_frame_syn(phy_vars_ue->lte_ue_common_vars.rxdata[0], "BBB");
 #endif
 //xhd_oai_ue only for test
 //frame_parms->Nid_cell = 0;

 
  sync_pos = lte_sync_time(phy_vars_ue->lte_ue_common_vars.rxdata,
                           frame_parms,
                           (int *)&phy_vars_ue->lte_ue_common_vars.eNb_id);

  //  write_output("rxdata1.m","rxd1",phy_vars_ue->lte_ue_common_vars.rxdata[0],10*frame_parms->samples_per_tti,1,1);
  if (sync_pos >= frame_parms->nb_prefix_samples)
    sync_pos2 = sync_pos - frame_parms->nb_prefix_samples;
  else
    sync_pos2 = sync_pos + FRAME_LENGTH_COMPLEX_SAMPLES - frame_parms->nb_prefix_samples;

#ifdef DEBUG_INITIAL_SYNCH
  LOG_I(PHY,"[UE%d] Initial sync : Estimated PSS position %d, Nid2=%d sync_pos2=%d nb_prefix_samples=%d samples_per_tti=%d ofdm_symbol_size=%d\n",
      phy_vars_ue->Mod_id,sync_pos,
      phy_vars_ue->lte_ue_common_vars.eNb_id,
      sync_pos2, frame_parms->nb_prefix_samples,
      frame_parms->samples_per_tti,
      frame_parms->ofdm_symbol_size);
#endif


  for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++)
    rx_power += signal_energy(&phy_vars_ue->lte_ue_common_vars.rxdata[aarx][sync_pos2],
                              frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples);

  phy_vars_ue->PHY_measurements.rx_power_avg_dB[0] = dB_fixed(rx_power/frame_parms->nb_antennas_rx);

#ifdef DEBUG_INITIAL_SYNCH
  LOG_I(PHY,"[UE%d] Initial sync : Estimated power: %d dB\n",phy_vars_ue->Mod_id,phy_vars_ue->PHY_measurements.rx_power_avg_dB[0] );
#endif

#ifdef EXMIMO

  if ((openair_daq_vars.rx_gain_mode == DAQ_AGC_ON) &&
      (mode != rx_calib_ue) && (mode != rx_calib_ue_med) && (mode != rx_calib_ue_byp) )
    //phy_adjust_gain(phy_vars_ue,0);
    gain_control_all(phy_vars_ue->PHY_measurements.rx_power_avg_dB[0],0);

#else
#ifndef OAI_USRP
  phy_adjust_gain(phy_vars_ue,0);
#endif
#endif

  // SSS detection

  // PSS is hypothesized in last symbol of first slot in Frame
  sync_pos_slot = (frame_parms->samples_per_tti>>1) - frame_parms->ofdm_symbol_size - frame_parms->nb_prefix_samples;

  if (sync_pos2 >= sync_pos_slot)
    phy_vars_ue->rx_offset = sync_pos2 - sync_pos_slot;
  else
    phy_vars_ue->rx_offset = FRAME_LENGTH_COMPLEX_SAMPLES + sync_pos2 - sync_pos_slot;

  if (((sync_pos2 - sync_pos_slot) >=0 ) &&
      ((sync_pos2 - sync_pos_slot) < ((FRAME_LENGTH_COMPLEX_SAMPLES-frame_parms->samples_per_tti/2)))) {
#ifdef DEBUG_INITIAL_SYNCH
    LOG_I(PHY,"Calling sss detection (FDD normal CP) sync_pos2=%d sync_pos_slot=%d sync_pos=%d rx_offset=%d flip_fdd_ncp=%d\n", 
        sync_pos2,sync_pos_slot,sync_pos, phy_vars_ue->rx_offset, flip_fdd_ncp);
#endif
    rx_sss(phy_vars_ue,&metric_fdd_ncp,&flip_fdd_ncp,&phase_fdd_ncp);
    frame_parms->nushift  = frame_parms->Nid_cell%6;

    printf("flip_fdd_ncp=%d\n", flip_fdd_ncp);
    if (flip_fdd_ncp==1)
    {
        phy_vars_ue->rx_offset += (FRAME_LENGTH_COMPLEX_SAMPLES>>1);
    }
    

    init_frame_parms(&phy_vars_ue->lte_frame_parms,1);
    lte_gold(frame_parms,phy_vars_ue->lte_gold_table[0],frame_parms->Nid_cell);
    ret = pbch_detection(phy_vars_ue,mode);
    //   write_output("rxdata2.m","rxd2",phy_vars_ue->lte_ue_common_vars.rxdata[0],10*frame_parms->samples_per_tti,1,1);

#ifdef DEBUG_INITIAL_SYNCH
    LOG_I(PHY,"FDD Normal prefix: CellId %d metric %d, phase %d, flip %d, pbch %d\n",
          frame_parms->Nid_cell,metric_fdd_ncp,phase_fdd_ncp,flip_fdd_ncp,ret);
#endif
  } else {
#ifdef DEBUG_INITIAL_SYNCH
    LOG_I(PHY,"FDD Normal prefix: SSS error condition: sync_pos %d, sync_pos_slot %d\n", sync_pos, sync_pos_slot);
#endif
  }


  if (ret==-1) {

    // Now FDD extended prefix
    frame_parms->Ncp=EXTENDED;
    frame_parms->frame_type=FDD;
    init_frame_parms(frame_parms,1);

    if (sync_pos < frame_parms->nb_prefix_samples)
      sync_pos2 = sync_pos + FRAME_LENGTH_COMPLEX_SAMPLES - frame_parms->nb_prefix_samples;
    else
      sync_pos2 = sync_pos - frame_parms->nb_prefix_samples;

    // PSS is hypothesized in last symbol of first slot in Frame
    sync_pos_slot = (frame_parms->samples_per_tti>>1) - frame_parms->ofdm_symbol_size - (frame_parms->nb_prefix_samples);

    if (sync_pos2 >= sync_pos_slot)
      phy_vars_ue->rx_offset = sync_pos2 - sync_pos_slot;
    else
      phy_vars_ue->rx_offset = FRAME_LENGTH_COMPLEX_SAMPLES + sync_pos2 - sync_pos_slot;

    //msg("nb_prefix_samples %d, rx_offset %d\n",frame_parms->nb_prefix_samples,phy_vars_ue->rx_offset);

    if (((sync_pos2 - sync_pos_slot) >=0 ) &&
        ((sync_pos2 - sync_pos_slot) < ((FRAME_LENGTH_COMPLEX_SAMPLES-frame_parms->samples_per_tti/2)))) {

      rx_sss(phy_vars_ue,&metric_fdd_ecp,&flip_fdd_ecp,&phase_fdd_ecp);
      frame_parms->nushift  = frame_parms->Nid_cell%6;

      if (flip_fdd_ecp==1)
        phy_vars_ue->rx_offset += (FRAME_LENGTH_COMPLEX_SAMPLES>>1);

      init_frame_parms(&phy_vars_ue->lte_frame_parms,1);
      lte_gold(frame_parms,phy_vars_ue->lte_gold_table[0],frame_parms->Nid_cell);
      ret = pbch_detection(phy_vars_ue,mode);
      //     write_output("rxdata3.m","rxd3",phy_vars_ue->lte_ue_common_vars.rxdata[0],10*frame_parms->samples_per_tti,1,1);
#ifdef DEBUG_INITIAL_SYNCH
      LOG_I(PHY,"FDD Extended prefix: CellId %d metric %d, phase %d, flip %d, pbch %d\n",
            frame_parms->Nid_cell,metric_fdd_ecp,phase_fdd_ecp,flip_fdd_ecp,ret);
#endif
    } else {
#ifdef DEBUG_INITIAL_SYNCH
      LOG_I(PHY,"FDD Extended prefix: SSS error condition: sync_pos %d, sync_pos_slot %d\n", sync_pos, sync_pos_slot);
#endif
    }

    if (ret==-1) {
      // Now TDD normal prefix
      frame_parms->Ncp=NORMAL;
      frame_parms->frame_type=TDD;
      init_frame_parms(frame_parms,1);

      if (sync_pos >= frame_parms->nb_prefix_samples)
        sync_pos2 = sync_pos - frame_parms->nb_prefix_samples;
      else
        sync_pos2 = sync_pos + FRAME_LENGTH_COMPLEX_SAMPLES - frame_parms->nb_prefix_samples;

      // PSS is hypothesized in 2nd symbol of third slot in Frame (S-subframe)
      sync_pos_slot = frame_parms->samples_per_tti +
                      (frame_parms->ofdm_symbol_size<<1) +
                      frame_parms->nb_prefix_samples0 +
                      (frame_parms->nb_prefix_samples);

      if (sync_pos2 >= sync_pos_slot)
        phy_vars_ue->rx_offset = sync_pos2 - sync_pos_slot;
      else
        phy_vars_ue->rx_offset = (FRAME_LENGTH_COMPLEX_SAMPLES>>1) + sync_pos2 - sync_pos_slot;

      /*if (((sync_pos2 - sync_pos_slot) >=0 ) &&
      ((sync_pos2 - sync_pos_slot) < ((FRAME_LENGTH_COMPLEX_SAMPLES-frame_parms->samples_per_tti/2)))) {*/


      rx_sss(phy_vars_ue,&metric_tdd_ncp,&flip_tdd_ncp,&phase_tdd_ncp);

      if (flip_tdd_ncp==1)
        phy_vars_ue->rx_offset += (FRAME_LENGTH_COMPLEX_SAMPLES>>1);

      frame_parms->nushift  = frame_parms->Nid_cell%6;
      init_frame_parms(&phy_vars_ue->lte_frame_parms,1);

      lte_gold(frame_parms,phy_vars_ue->lte_gold_table[0],frame_parms->Nid_cell);
      ret = pbch_detection(phy_vars_ue,mode);
      //      write_output("rxdata4.m","rxd4",phy_vars_ue->lte_ue_common_vars.rxdata[0],10*frame_parms->samples_per_tti,1,1);

#ifdef DEBUG_INITIAL_SYNCH
      LOG_I(PHY,"TDD Normal prefix: CellId %d metric %d, phase %d, flip %d, pbch %d\n",
            frame_parms->Nid_cell,metric_tdd_ncp,phase_tdd_ncp,flip_tdd_ncp,ret);
#endif
      /*}
          else {
      #ifdef DEBUG_INITIAL_SYNCH
              LOG_I(PHY,"TDD Normal prefix: SSS error condition: sync_pos %d, sync_pos_slot %d\n", sync_pos, sync_pos_slot);
      #endif
      }*/


      if (ret==-1) {
        // Now TDD extended prefix
        frame_parms->Ncp=EXTENDED;
        frame_parms->frame_type=TDD;
        init_frame_parms(frame_parms,1);
        sync_pos2 = sync_pos - frame_parms->nb_prefix_samples;

        if (sync_pos >= frame_parms->nb_prefix_samples)
          sync_pos2 = sync_pos - frame_parms->nb_prefix_samples;
        else
          sync_pos2 = sync_pos + FRAME_LENGTH_COMPLEX_SAMPLES - frame_parms->nb_prefix_samples;

        // PSS is hypothesized in 2nd symbol of third slot in Frame (S-subframe)
        sync_pos_slot = frame_parms->samples_per_tti + (frame_parms->ofdm_symbol_size<<1) + (frame_parms->nb_prefix_samples<<1);

        if (sync_pos2 >= sync_pos_slot)
          phy_vars_ue->rx_offset = sync_pos2 - sync_pos_slot;
        else
          phy_vars_ue->rx_offset = (FRAME_LENGTH_COMPLEX_SAMPLES>>1) + sync_pos2 - sync_pos_slot;

        /*if (((sync_pos2 - sync_pos_slot) >=0 ) &&
          ((sync_pos2 - sync_pos_slot) < ((FRAME_LENGTH_COMPLEX_SAMPLES-frame_parms->samples_per_tti/2)))) {*/

        rx_sss(phy_vars_ue,&metric_tdd_ecp,&flip_tdd_ecp,&phase_tdd_ecp);
        frame_parms->nushift  = frame_parms->Nid_cell%6;

        if (flip_tdd_ecp==1)
          phy_vars_ue->rx_offset += (FRAME_LENGTH_COMPLEX_SAMPLES>>1);

        init_frame_parms(&phy_vars_ue->lte_frame_parms,1);
        lte_gold(frame_parms,phy_vars_ue->lte_gold_table[0],frame_parms->Nid_cell);
        ret = pbch_detection(phy_vars_ue,mode);

	//	write_output("rxdata5.m","rxd5",phy_vars_ue->lte_ue_common_vars.rxdata[0],10*frame_parms->samples_per_tti,1,1);
#ifdef DEBUG_INITIAL_SYNCH
        LOG_I(PHY,"TDD Extended prefix: CellId %d metric %d, phase %d, flip %d, pbch %d\n",
              frame_parms->Nid_cell,metric_tdd_ecp,phase_tdd_ecp,flip_tdd_ecp,ret);
#endif
        /*}
        else {
        #ifdef DEBUG_INITIAL_SYNCH
          LOG_I(PHY,"TDD Extended prefix: SSS error condition: sync_pos %d, sync_pos_slot %d\n", sync_pos, sync_pos_slot);
        #endif
        }*/

      }
    }
  }

  if (ret==0) {  // PBCH found so indicate sync to higher layers and configure frame parameters

#ifdef DEBUG_INITIAL_SYNCH
    LOG_I(PHY,"[UE%d] In synch, rx_offset %d samples\n",phy_vars_ue->Mod_id, phy_vars_ue->rx_offset);
#endif

    if (phy_vars_ue->UE_scan_carrier == 0) {
#ifdef OPENAIR2
      LOG_I(PHY,"[UE%d] Sending synch status to higher layers\n",phy_vars_ue->Mod_id);
      //mac_resynch();
      mac_xface->dl_phy_sync_success(phy_vars_ue->Mod_id,phy_vars_ue->frame_rx,0,1);//phy_vars_ue->lte_ue_common_vars.eNb_id);
#endif //OPENAIR2
      
      generate_pcfich_reg_mapping(frame_parms);
      generate_phich_reg_mapping(frame_parms);
      //    init_prach625(frame_parms);
#ifndef OPENAIR2
      phy_vars_ue->UE_mode[0] = PUSCH;
#else
      phy_vars_ue->UE_mode[0] = PRACH;
#endif
      //phy_vars_ue->lte_ue_pbch_vars[0]->pdu_errors=0;
      phy_vars_ue->lte_ue_pbch_vars[0]->pdu_errors_conseq=0;
    //phy_vars_ue->lte_ue_pbch_vars[0]->pdu_errors_last=0;
    }

    LOG_I(PHY,"[UE %d] Frame %d RRC Measurements => rssi %3.1f dBm (dig %3.1f dB, gain %d), N0 %d dBm,  rsrp %3.1f dBm/RE, rsrq %3.1f dB\n",phy_vars_ue->Mod_id,
	  phy_vars_ue->frame_rx,
	  10*log10(phy_vars_ue->PHY_measurements.rssi)-phy_vars_ue->rx_total_gain_dB,
	  10*log10(phy_vars_ue->PHY_measurements.rssi),
	  phy_vars_ue->rx_total_gain_dB,
	  phy_vars_ue->PHY_measurements.n0_power_tot_dBm,
	  10*log10(phy_vars_ue->PHY_measurements.rsrp[0])-phy_vars_ue->rx_total_gain_dB,
	  (10*log10(phy_vars_ue->PHY_measurements.rsrq[0])));
    
    
    LOG_I(PHY,"[UE %d] Frame %d MIB Information => %s, %s, NidCell %d, N_RB_DL %d, PHICH DURATION %d, PHICH RESOURCE %s, TX_ANT %d\n",
	  phy_vars_ue->Mod_id,
	  phy_vars_ue->frame_rx,
	  duplex_string[phy_vars_ue->lte_frame_parms.frame_type],
	  prefix_string[phy_vars_ue->lte_frame_parms.Ncp],
	  phy_vars_ue->lte_frame_parms.Nid_cell,
	  phy_vars_ue->lte_frame_parms.N_RB_DL,
	  phy_vars_ue->lte_frame_parms.phich_config_common.phich_duration,
	  phich_string[phy_vars_ue->lte_frame_parms.phich_config_common.phich_resource],
	  phy_vars_ue->lte_frame_parms.nb_antennas_tx_eNB);
#if defined(OAI_USRP) || defined(EXMIMO)
    LOG_I(PHY,"[UE %d] Frame %d Measured Carrier Frequency %.0f Hz (offset %d Hz)\n",
	  phy_vars_ue->Mod_id,
	  phy_vars_ue->frame_rx,
	  openair0_cfg[0].rx_freq[0]-phy_vars_ue->lte_ue_common_vars.freq_offset,
	  phy_vars_ue->lte_ue_common_vars.freq_offset);
#endif
  } else {
#ifdef DEBUG_INITIAL_SYNC
    LOG_I(PHY,"[UE%d] Initial sync : PBCH not ok\n",phy_vars_ue->Mod_id);
    LOG_I(PHY,"[UE%d] Initial sync : Estimated PSS position %d, Nid2 %d\n",phy_vars_ue->Mod_id,sync_pos,phy_vars_ue->lte_ue_common_vars.eNb_id);
    /*      LOG_I(PHY,"[UE%d] Initial sync: (metric fdd_ncp %d (%d), metric fdd_ecp %d (%d), metric_tdd_ncp %d (%d), metric_tdd_ecp %d (%d))\n",
          phy_vars_ue->Mod_id,
          metric_fdd_ncp,Nid_cell_fdd_ncp,
          metric_fdd_ecp,Nid_cell_fdd_ecp,
          metric_tdd_ncp,Nid_cell_tdd_ncp,
          metric_tdd_ecp,Nid_cell_tdd_ecp);*/
    LOG_I(PHY,"[UE%d] Initial sync : Estimated Nid_cell %d, Frame_type %d\n",phy_vars_ue->Mod_id,
          frame_parms->Nid_cell,frame_parms->frame_type);
#endif

    phy_vars_ue->UE_mode[0] = NOT_SYNCHED;
    phy_vars_ue->lte_ue_pbch_vars[0]->pdu_errors_last=phy_vars_ue->lte_ue_pbch_vars[0]->pdu_errors;
    phy_vars_ue->lte_ue_pbch_vars[0]->pdu_errors++;
    phy_vars_ue->lte_ue_pbch_vars[0]->pdu_errors_conseq++;

  }

  //  exit_fun("debug exit");
  return ret;
}

int initial_sync(PHY_VARS_UE *phy_vars_ue, runmode_t mode)
{

  int32_t sync_pos,sync_pos2,sync_pos_slot;
  int32_t metric_fdd_ncp=0,metric_fdd_ecp=0,metric_tdd_ncp=0,metric_tdd_ecp=0;
  uint8_t phase_fdd_ncp,phase_fdd_ecp,phase_tdd_ncp,phase_tdd_ecp;
  uint8_t flip_fdd_ncp=0,flip_fdd_ecp,flip_tdd_ncp,flip_tdd_ecp;
  //  uint16_t Nid_cell_fdd_ncp=0,Nid_cell_fdd_ecp=0,Nid_cell_tdd_ncp=0,Nid_cell_tdd_ecp=0;
  LTE_DL_FRAME_PARMS *frame_parms = &phy_vars_ue->lte_frame_parms;
  int i;
  int ret=-1;
  int aarx,rx_power=0;
  #ifdef OAI_USRP
  __m128i *rxdata128;
  #endif
  //  LOG_I(PHY,"**************************************************************\n");
  // First try FDD normal prefix
  frame_parms->Ncp=NORMAL;
  frame_parms->frame_type=FDD;
  init_frame_parms(frame_parms,1);

  //  write_output("rxdata0.m","rxd0",phy_vars_ue->lte_ue_common_vars.rxdata[0],10*frame_parms->samples_per_tti,1,1);

#if defined(__x86_64__) 
    LOG_I(PHY,"[UE%d] Initial sync =============================== x86 64 ============== \n",
        phy_vars_ue->Mod_id);
#else

#if defined(__i386__)
    LOG_I(PHY,"[UE%d] Initial sync ============================ i386 ============= \n",
        phy_vars_ue->Mod_id);
#else
    LOG_I(PHY,"[UE%d] Initial sync ============================ ??? ============= \n",
        phy_vars_ue->Mod_id);

#endif
#endif

#ifdef OAI_USRP1
  LOG_I(PHY,"[UE%d] Initial sync : OAI_USRP call _mm_srai_epi16\n",
     phy_vars_ue->Mod_id);
  //print_frame_syn(phy_vars_ue->lte_ue_common_vars.rxdata[0], "AAA");

  //向右移4位
  for (aarx = 0; aarx<frame_parms->nb_antennas_rx;aarx++) 
  {
    rxdata128 = (__m128i*)phy_vars_ue->lte_ue_common_vars.rxdata[aarx];
    for (i=0; i<(frame_parms->samples_per_tti*10)>>2; i++) {
      rxdata128[i] = _mm_srai_epi16(rxdata128[i],4);
    } 
  }
  //print_frame_syn(phy_vars_ue->lte_ue_common_vars.rxdata[0], "BBB");
 #endif
 //xhd_oai_ue only for test
 //frame_parms->Nid_cell = 0;

 
  sync_pos = lte_sync_time(phy_vars_ue->lte_ue_common_vars.rxdata,
                           frame_parms,
                           (int *)&phy_vars_ue->lte_ue_common_vars.eNb_id);

  //  write_output("rxdata1.m","rxd1",phy_vars_ue->lte_ue_common_vars.rxdata[0],10*frame_parms->samples_per_tti,1,1);
  if (sync_pos >= frame_parms->nb_prefix_samples)
    sync_pos2 = sync_pos - frame_parms->nb_prefix_samples;
  else
    sync_pos2 = sync_pos + FRAME_LENGTH_COMPLEX_SAMPLES - frame_parms->nb_prefix_samples;

  LOG_I(PHY,"[UE%d] Initial sync : Estimated PSS position %d, Nid2=%d sync_pos2=%d nb_prefix_samples=%d samples_per_tti=%d ofdm_symbol_size=%d\n",
      phy_vars_ue->Mod_id,sync_pos,
      phy_vars_ue->lte_ue_common_vars.eNb_id,
      sync_pos2, frame_parms->nb_prefix_samples,
      frame_parms->samples_per_tti,
      frame_parms->ofdm_symbol_size);


  for (aarx=0; aarx<frame_parms->nb_antennas_rx; aarx++)
    rx_power += signal_energy(&phy_vars_ue->lte_ue_common_vars.rxdata[aarx][sync_pos2],
                              frame_parms->ofdm_symbol_size+frame_parms->nb_prefix_samples);

  phy_vars_ue->PHY_measurements.rx_power_avg_dB[0] = dB_fixed(rx_power/frame_parms->nb_antennas_rx);

  LOG_I(PHY,"[UE%d] Initial sync : Estimated power: %d dB\n",phy_vars_ue->Mod_id,phy_vars_ue->PHY_measurements.rx_power_avg_dB[0] );

#ifndef OAI_USRP
  phy_adjust_gain(phy_vars_ue,0);
#endif

  // SSS detection

  // PSS is hypothesized in last symbol of first slot in Frame
  sync_pos_slot = (frame_parms->samples_per_tti>>1) - frame_parms->ofdm_symbol_size - frame_parms->nb_prefix_samples;

  if (sync_pos2 >= sync_pos_slot)
    phy_vars_ue->rx_offset = sync_pos2 - sync_pos_slot;
  else
    phy_vars_ue->rx_offset = FRAME_LENGTH_COMPLEX_SAMPLES + sync_pos2 - sync_pos_slot;

  if (((sync_pos2 - sync_pos_slot) >=0 ) &&
      ((sync_pos2 - sync_pos_slot) < ((FRAME_LENGTH_COMPLEX_SAMPLES-frame_parms->samples_per_tti/2)))) {
    LOG_I(PHY,"Calling sss detection (FDD normal CP) sync_pos2=%d sync_pos_slot=%d sync_pos=%d rx_offset=%d flip_fdd_ncp=%d\n", 
        sync_pos2,sync_pos_slot,sync_pos, phy_vars_ue->rx_offset, flip_fdd_ncp);
    rx_sss(phy_vars_ue,&metric_fdd_ncp,&flip_fdd_ncp,&phase_fdd_ncp);
    frame_parms->nushift  = frame_parms->Nid_cell%6;

    printf("flip_fdd_ncp=%d\n", flip_fdd_ncp);
    if (flip_fdd_ncp==1)
    {
        phy_vars_ue->rx_offset += (FRAME_LENGTH_COMPLEX_SAMPLES>>1);
    }
    

    init_frame_parms(&phy_vars_ue->lte_frame_parms,1);
    lte_gold(frame_parms,phy_vars_ue->lte_gold_table[0],frame_parms->Nid_cell);
    ret = pbch_detection(phy_vars_ue,mode);
    //   write_output("rxdata2.m","rxd2",phy_vars_ue->lte_ue_common_vars.rxdata[0],10*frame_parms->samples_per_tti,1,1);

    LOG_I(PHY,"FDD Normal prefix: CellId %d metric %d, phase %d, flip %d, pbch %d\n",
          frame_parms->Nid_cell,metric_fdd_ncp,phase_fdd_ncp,flip_fdd_ncp,ret);
  }
  else 
  {
    LOG_I(PHY,"FDD Normal prefix: SSS error condition: sync_pos %d, sync_pos_slot %d\n", sync_pos, sync_pos_slot);
  }


  if (ret==-1) 
  {

    // Now FDD extended prefix
    frame_parms->Ncp=EXTENDED;
    frame_parms->frame_type=FDD;
    init_frame_parms(frame_parms,1);

    if (sync_pos < frame_parms->nb_prefix_samples)
      sync_pos2 = sync_pos + FRAME_LENGTH_COMPLEX_SAMPLES - frame_parms->nb_prefix_samples;
    else
      sync_pos2 = sync_pos - frame_parms->nb_prefix_samples;

    // PSS is hypothesized in last symbol of first slot in Frame
    sync_pos_slot = (frame_parms->samples_per_tti>>1) - frame_parms->ofdm_symbol_size - (frame_parms->nb_prefix_samples);

    if (sync_pos2 >= sync_pos_slot)
      phy_vars_ue->rx_offset = sync_pos2 - sync_pos_slot;
    else
      phy_vars_ue->rx_offset = FRAME_LENGTH_COMPLEX_SAMPLES + sync_pos2 - sync_pos_slot;

    //msg("nb_prefix_samples %d, rx_offset %d\n",frame_parms->nb_prefix_samples,phy_vars_ue->rx_offset);

    if (((sync_pos2 - sync_pos_slot) >=0 ) &&
        ((sync_pos2 - sync_pos_slot) < ((FRAME_LENGTH_COMPLEX_SAMPLES-frame_parms->samples_per_tti/2)))) {

      rx_sss(phy_vars_ue,&metric_fdd_ecp,&flip_fdd_ecp,&phase_fdd_ecp);
      frame_parms->nushift  = frame_parms->Nid_cell%6;

      if (flip_fdd_ecp==1)
        phy_vars_ue->rx_offset += (FRAME_LENGTH_COMPLEX_SAMPLES>>1);

      init_frame_parms(&phy_vars_ue->lte_frame_parms,1);
      lte_gold(frame_parms,phy_vars_ue->lte_gold_table[0],frame_parms->Nid_cell);
      ret = pbch_detection(phy_vars_ue,mode);
      //     write_output("rxdata3.m","rxd3",phy_vars_ue->lte_ue_common_vars.rxdata[0],10*frame_parms->samples_per_tti,1,1);
      LOG_I(PHY,"FDD Extended prefix: CellId %d metric %d, phase %d, flip %d, pbch %d\n",
            frame_parms->Nid_cell,metric_fdd_ecp,phase_fdd_ecp,flip_fdd_ecp,ret);
    } 
    else 
    {
      LOG_I(PHY,"FDD Extended prefix: SSS error condition: sync_pos %d, sync_pos_slot %d\n", sync_pos, sync_pos_slot);
    }
  }
  
  if (ret==-1) 
  {
      // Now TDD normal prefix
      frame_parms->Ncp=NORMAL;
      frame_parms->frame_type=TDD;
      init_frame_parms(frame_parms,1);

      if (sync_pos >= frame_parms->nb_prefix_samples)
        sync_pos2 = sync_pos - frame_parms->nb_prefix_samples;
      else
        sync_pos2 = sync_pos + FRAME_LENGTH_COMPLEX_SAMPLES - frame_parms->nb_prefix_samples;

      // PSS is hypothesized in 2nd symbol of third slot in Frame (S-subframe)
      sync_pos_slot = frame_parms->samples_per_tti +
                      (frame_parms->ofdm_symbol_size<<1) +
                      frame_parms->nb_prefix_samples0 +
                      (frame_parms->nb_prefix_samples);

      if (sync_pos2 >= sync_pos_slot)
        phy_vars_ue->rx_offset = sync_pos2 - sync_pos_slot;
      else
        phy_vars_ue->rx_offset = (FRAME_LENGTH_COMPLEX_SAMPLES>>1) + sync_pos2 - sync_pos_slot;

      /*if (((sync_pos2 - sync_pos_slot) >=0 ) &&
      ((sync_pos2 - sync_pos_slot) < ((FRAME_LENGTH_COMPLEX_SAMPLES-frame_parms->samples_per_tti/2)))) {*/


      rx_sss(phy_vars_ue,&metric_tdd_ncp,&flip_tdd_ncp,&phase_tdd_ncp);

      if (flip_tdd_ncp==1)
        phy_vars_ue->rx_offset += (FRAME_LENGTH_COMPLEX_SAMPLES>>1);

      frame_parms->nushift  = frame_parms->Nid_cell%6;
      init_frame_parms(&phy_vars_ue->lte_frame_parms,1);

      lte_gold(frame_parms,phy_vars_ue->lte_gold_table[0],frame_parms->Nid_cell);
      ret = pbch_detection(phy_vars_ue,mode);
      //      write_output("rxdata4.m","rxd4",phy_vars_ue->lte_ue_common_vars.rxdata[0],10*frame_parms->samples_per_tti,1,1);

      LOG_I(PHY,"TDD Normal prefix: CellId %d metric %d, phase %d, flip %d, pbch %d\n",
            frame_parms->Nid_cell,metric_tdd_ncp,phase_tdd_ncp,flip_tdd_ncp,ret);
    }

    if (ret==-1) 
    {
        // Now TDD extended prefix
        frame_parms->Ncp=EXTENDED;
        frame_parms->frame_type=TDD;
        init_frame_parms(frame_parms,1);
        sync_pos2 = sync_pos - frame_parms->nb_prefix_samples;

        if (sync_pos >= frame_parms->nb_prefix_samples)
          sync_pos2 = sync_pos - frame_parms->nb_prefix_samples;
        else
          sync_pos2 = sync_pos + FRAME_LENGTH_COMPLEX_SAMPLES - frame_parms->nb_prefix_samples;

        // PSS is hypothesized in 2nd symbol of third slot in Frame (S-subframe)
        sync_pos_slot = frame_parms->samples_per_tti + (frame_parms->ofdm_symbol_size<<1) + (frame_parms->nb_prefix_samples<<1);

        if (sync_pos2 >= sync_pos_slot)
          phy_vars_ue->rx_offset = sync_pos2 - sync_pos_slot;
        else
          phy_vars_ue->rx_offset = (FRAME_LENGTH_COMPLEX_SAMPLES>>1) + sync_pos2 - sync_pos_slot;

        /*if (((sync_pos2 - sync_pos_slot) >=0 ) &&
          ((sync_pos2 - sync_pos_slot) < ((FRAME_LENGTH_COMPLEX_SAMPLES-frame_parms->samples_per_tti/2)))) {*/

        rx_sss(phy_vars_ue,&metric_tdd_ecp,&flip_tdd_ecp,&phase_tdd_ecp);
        frame_parms->nushift  = frame_parms->Nid_cell%6;

        if (flip_tdd_ecp==1)
          phy_vars_ue->rx_offset += (FRAME_LENGTH_COMPLEX_SAMPLES>>1);

        init_frame_parms(&phy_vars_ue->lte_frame_parms,1);
        lte_gold(frame_parms,phy_vars_ue->lte_gold_table[0],frame_parms->Nid_cell);
        ret = pbch_detection(phy_vars_ue,mode);

	//	write_output("rxdata5.m","rxd5",phy_vars_ue->lte_ue_common_vars.rxdata[0],10*frame_parms->samples_per_tti,1,1);
        LOG_I(PHY,"TDD Extended prefix: CellId %d metric %d, phase %d, flip %d, pbch %d\n",
              frame_parms->Nid_cell,metric_tdd_ecp,phase_tdd_ecp,flip_tdd_ecp,ret);

  }

  if (ret==0) 
  {  
    // PBCH found so indicate sync to higher layers and configure frame parameters

    LOG_I(PHY,"[UE%d] In synch, rx_offset %d samples\n",phy_vars_ue->Mod_id, phy_vars_ue->rx_offset);

    if (phy_vars_ue->UE_scan_carrier == 0) {
#ifdef OPENAIR2
      LOG_I(PHY,"[UE%d] Sending synch status to higher layers\n",phy_vars_ue->Mod_id);
      //mac_resynch();
      mac_xface->dl_phy_sync_success(phy_vars_ue->Mod_id,phy_vars_ue->frame_rx,0,1);//phy_vars_ue->lte_ue_common_vars.eNb_id);
#endif //OPENAIR2
      
      generate_pcfich_reg_mapping(frame_parms);
      generate_phich_reg_mapping(frame_parms);
      //    init_prach625(frame_parms);
#ifndef OPENAIR2
      phy_vars_ue->UE_mode[0] = PUSCH;
#else
      phy_vars_ue->UE_mode[0] = PRACH;
#endif
      //phy_vars_ue->lte_ue_pbch_vars[0]->pdu_errors=0;
      phy_vars_ue->lte_ue_pbch_vars[0]->pdu_errors_conseq=0;
    //phy_vars_ue->lte_ue_pbch_vars[0]->pdu_errors_last=0;
    }

    LOG_I(PHY,"[UE %d] Frame %d RRC Measurements => rssi %3.1f dBm (dig %3.1f dB, gain %d), N0 %d dBm,  rsrp %3.1f dBm/RE, rsrq %3.1f dB\n",phy_vars_ue->Mod_id,
	  phy_vars_ue->frame_rx,
	  10*log10(phy_vars_ue->PHY_measurements.rssi)-phy_vars_ue->rx_total_gain_dB,
	  10*log10(phy_vars_ue->PHY_measurements.rssi),
	  phy_vars_ue->rx_total_gain_dB,
	  phy_vars_ue->PHY_measurements.n0_power_tot_dBm,
	  10*log10(phy_vars_ue->PHY_measurements.rsrp[0])-phy_vars_ue->rx_total_gain_dB,
	  (10*log10(phy_vars_ue->PHY_measurements.rsrq[0])));
    
    
    LOG_I(PHY,"[UE %d] Frame %d MIB Information => %s, %s, NidCell %d, N_RB_DL %d, PHICH DURATION %d, PHICH RESOURCE %s, TX_ANT %d\n",
	  phy_vars_ue->Mod_id,
	  phy_vars_ue->frame_rx,
	  duplex_string[phy_vars_ue->lte_frame_parms.frame_type],
	  prefix_string[phy_vars_ue->lte_frame_parms.Ncp],
	  phy_vars_ue->lte_frame_parms.Nid_cell,
	  phy_vars_ue->lte_frame_parms.N_RB_DL,
	  phy_vars_ue->lte_frame_parms.phich_config_common.phich_duration,
	  phich_string[phy_vars_ue->lte_frame_parms.phich_config_common.phich_resource],
	  phy_vars_ue->lte_frame_parms.nb_antennas_tx_eNB);
    LOG_I(PHY,"[UE %d] Frame %d Measured Carrier Frequency %.0f Hz (offset %d Hz)\n",
	  phy_vars_ue->Mod_id,
	  phy_vars_ue->frame_rx,
	  openair0_cfg[0].rx_freq[0]-phy_vars_ue->lte_ue_common_vars.freq_offset,
	  phy_vars_ue->lte_ue_common_vars.freq_offset);
  }
  else 
  {
    LOG_I(PHY,"[UE%d] Initial sync : PBCH not ok\n",phy_vars_ue->Mod_id);
    LOG_I(PHY,"[UE%d] Initial sync : Estimated PSS position %d, Nid2 %d\n",phy_vars_ue->Mod_id,sync_pos,phy_vars_ue->lte_ue_common_vars.eNb_id);
    /*      LOG_I(PHY,"[UE%d] Initial sync: (metric fdd_ncp %d (%d), metric fdd_ecp %d (%d), metric_tdd_ncp %d (%d), metric_tdd_ecp %d (%d))\n",
          phy_vars_ue->Mod_id,
          metric_fdd_ncp,Nid_cell_fdd_ncp,
          metric_fdd_ecp,Nid_cell_fdd_ecp,
          metric_tdd_ncp,Nid_cell_tdd_ncp,
          metric_tdd_ecp,Nid_cell_tdd_ecp);*/
    LOG_I(PHY,"[UE%d] Initial sync : Estimated Nid_cell %d, Frame_type %d\n",phy_vars_ue->Mod_id,
          frame_parms->Nid_cell,frame_parms->frame_type);

    phy_vars_ue->UE_mode[0] = NOT_SYNCHED;
    phy_vars_ue->lte_ue_pbch_vars[0]->pdu_errors_last=phy_vars_ue->lte_ue_pbch_vars[0]->pdu_errors;
    phy_vars_ue->lte_ue_pbch_vars[0]->pdu_errors++;
    phy_vars_ue->lte_ue_pbch_vars[0]->pdu_errors_conseq++;

  }

  //  exit_fun("debug exit");
  return ret;
}

