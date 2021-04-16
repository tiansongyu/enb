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

/*! \file eNB_scheduler_ulsch.c
 * \brief eNB procedures for the ULSCH transport channel
 * \author Navid Nikaein and Raymond Knopp
 * \date 2010 - 2014
 * \email: navid.nikaein@eurecom.fr
 * \version 1.0
 * @ingroup _mac

 */

#include "assertions.h"
#include "PHY/defs.h"
#include "PHY/extern.h"

#include "SCHED/defs.h"
#include "SCHED/extern.h"

#include "LAYER2/MAC/defs.h"
#include "LAYER2/MAC/proto.h"
#include "LAYER2/MAC/extern.h"
#include "UTIL/LOG/log.h"
#include "UTIL/LOG/vcd_signal_dumper.h"
#include "UTIL/OPT/opt.h"
#include "OCG.h"
#include "OCG_extern.h"

#include "RRC/LITE/extern.h"
#include "RRC/L2_INTERFACE/openair_rrc_L2_interface.h"

//#include "LAYER2/MAC/pre_processor.c"
#include "pdcp.h"

#if defined(ENABLE_ITTI)
# include "intertask_interface.h"
#endif

#define ENABLE_MAC_PAYLOAD_DEBUG
#define DEBUG_eNB_SCHEDULER 1

extern void phy_add_count(uint16_t rnti, uint16_t usCountId);
extern void phy_print_user(uint16_t usUser,int print_count, uint8_t clear);

#define OLTE_MAX_USER NUMBER_OF_UE_MAX
extern int bsr_report[OLTE_MAX_USER][10];
extern int bsr_report_zero[OLTE_MAX_USER][10];

// This table holds the allowable PRB sizes for ULSCH transmissions
uint8_t rb_table[33] = {1,2,3,4,5,6,8,9,10,12,15,16,18,20,24,25,27,30,32,36,40,45,48,50,54,60,72,75,80,81,90,96,100};

void MAC_CompareRb(const rnti_t rntiP, uint8_t harq_pid, uint16_t first_rb, uint16_t nb_rb)
{
    int    UE_id = find_UE_id(0,rntiP);
    if( UE_id == -1)
    {
        printf("MAC_CompareRb: rnti%x not found!\n", rntiP);
        return;
    }
    eNB_MAC_INST *eNB = &eNB_mac_inst[0];
    UE_info_t *UE_info= &eNB->UE_info[UE_id];
    UE_TEMPLATE       *UE_template = &UE_info->UE_template[0];

    if( UE_template->first_rb_ul[harq_pid] != first_rb
        || UE_template->nb_rb_ul[harq_pid] != nb_rb)
    {
        printf("MAC_CompareRb: FATAL ERROR, RB MISMATCH: rnti%x first_rb%d-%d nb_rb%d-%d!\n", 
            rntiP,
            UE_template->first_rb_ul[harq_pid],first_rb,
            UE_template->nb_rb_ul[harq_pid],nb_rb);
    }

}

void rx_sdu(
  const module_id_t enb_mod_idP,
  const int         CC_idP,
  const frame_t     frameP,
  const sub_frame_t subframeP,
  const rnti_t      rntiP,
  uint8_t          *sduP,
  const uint16_t    sdu_lenP,
  const int         harq_pidP,
  uint8_t          *msg3_flagP,
  rnti_t           *crnti,
  uint8_t           ucRound,
  uint8_t           ucFirstRb,
  uint8_t           ucRbNum
  )
{

  unsigned char  rx_ces[MAX_NUM_CE],num_ce,num_sdu,i,*payload_ptr;
  unsigned char  rx_lcids[NB_RB_MAX];
  unsigned short rx_lengths[NB_RB_MAX];
  int    UE_id = find_UE_id(enb_mod_idP,rntiP);
  int ii,j;
  eNB_MAC_INST *eNB = &eNB_mac_inst[enb_mod_idP];
  UE_info_t *UE_info= &eNB->UE_info[UE_id];

  //xhd_oai_multuser
  rnti_t      rntiMac = rntiP;

  start_meas(&eNB->rx_ulsch_sdu);

  if ((UE_id >  NUMBER_OF_UE_MAX) || (UE_id == -1)  )
    for(ii=0; ii<NB_RB_MAX; ii++) {
      rx_lengths[ii] = 0;
    }

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_RX_SDU,1);
  if (opt_enabled == 1) {
    trace_pdu(0, sduP,sdu_lenP, 0, 3, rntiP,subframeP, 0,0);
    LOG_D(OPT,"[eNB %d][ULSCH] Frame %d  rnti %x  with size %d\n",
    		  enb_mod_idP, frameP, rntiP, sdu_lenP);
  }

  
#if 0
  if(sduP[5]+sduP[6]+sduP[7] > 0)
  {
      LOG_D(MAC,"[eNB %d] CC_id %d Frame %d, subframe %d (harq_pidP=%d round=%d rb %d-%d bytes %d): Received ULSCH sdu from PHY (rnti %x, UE_id %d)\n",
        enb_mod_idP,CC_idP,frameP, subframeP, harq_pidP, ucRound, ucFirstRb, ucRbNum,
        sdu_lenP, rntiP,UE_id);
      for (int i=0; i<sdu_lenP; i++)
        printf("%x.",sduP[i]);
      
      printf("\n");

  }
#endif
  
  if( subframeP == 40)
  {
      print_buff(sduP,sdu_lenP,0,"[eNB %d] CC_id %d Frame %d, subframe %d (harq_pidP=%d round=%d rb %d-%d bytes %d): Received ULSCH sdu from PHY (rnti %x, UE_id %d)\n",
        enb_mod_idP,CC_idP,frameP, subframeP, harq_pidP, ucRound, ucFirstRb, ucRbNum,
        sdu_lenP, rntiP,UE_id);
  }
  payload_ptr = parse_ulsch_header(sduP,&num_ce,&num_sdu,rx_ces,rx_lcids,rx_lengths,sdu_lenP);

  //printf("rx_sdu: num_ce=%d num_sdu=%d UE_id=%d\n",num_ce,num_sdu,UE_id);
  
  eNB->eNB_stats[CC_idP].ulsch_bytes_rx=sdu_lenP;
  eNB->eNB_stats[CC_idP].total_ulsch_bytes_rx+=sdu_lenP;
  eNB->eNB_stats[CC_idP].total_ulsch_pdus_rx+=1;
  // control element
  for (i=0; i<num_ce; i++) {

    switch (rx_ces[i]) { // implement and process BSR + CRNTI +
    case POWER_HEADROOM:
      if (UE_id != -1) {
        UE_info->UE_template[CC_idP].phr_info =  (payload_ptr[0] & 0x3f) - PHR_MAPPING_OFFSET;
        //LOG_I(MAC, "[eNB %d] CC_id %d MAC CE_LCID %d : Received PHR PH = %d (db)\n",
        //      enb_mod_idP, CC_idP, rx_ces[i], UE_list->UE_template[CC_idP][UE_id].phr_info);
        UE_info->UE_template[CC_idP].phr_info_configured=1;
      }

      payload_ptr+=sizeof(POWER_HEADROOM_CMD);
      break;

    case CRNTI:
      LOG_D(MAC, "[eNB %d] CC_id %d MAC CE_LCID %d (ce %d/%d): Received CRNTI %2.2x%2.2x\n",
            enb_mod_idP, CC_idP, rx_ces[i], i,num_ce, payload_ptr[0], payload_ptr[1]);

      rnti_t oldRnti = (((uint16_t)payload_ptr[0])<<8) + payload_ptr[1];

      int UE_id_cu = UE_id;
      
      UE_id = find_UE_id(enb_mod_idP,oldRnti);
      
      LOG_E(MAC, "[eNB %d] CC_id %d frame=%d subframe=%d MAC CE_LCID %d (ce %d/%d): rnti: %x(UE_id %d) CRNTI %x (UE_id %d) in Msg3\n",
             enb_mod_idP, CC_idP, frameP, subframeP, rx_ces[i], i,num_ce, rntiP, UE_id_cu,oldRnti, UE_id);


      if( UE_id != -1)
      {
          phy_print_user(UE_id,0,1);
      }

      payload_ptr+=2;
      /* we don't process this CE yet */
      //xhd_oai_enb AAA
      //xhd_oai_enb
      //mac_xface->phy_remain_ue(enb_mod_idP, CC_idP, rntiP);
      //cancel_ra_proc(0, 0, 0, oldRnti);
      //xhd_oai_20users handle CRNTI MAC CE
      if( UE_id == -1)
      {

          if (msg3_flagP != NULL ) 
          {
    	       *msg3_flagP = 10;
          }
      }
      else 
      {
          if (msg3_flagP != NULL ) 
          {
    	       *msg3_flagP = 11;
          }
            rntiMac = oldRnti;
            if(crnti != NULL )
            {
                *crnti = oldRnti;
            }
            //xhd_oai_multuser 更新MAC模块UE表的rnti
            //UE_list->UE_template[UE_PCCID(enb_mod_idP,UE_id)][UE_id].rnti=rntiP;

           mac_rrc_crnti_ind(
              enb_mod_idP,
              CC_idP,
              frameP,subframeP,
              rntiMac,
              ENB_FLAG_YES,
              enb_mod_idP);

           SR_indication(enb_mod_idP, CC_idP, frameP,  rntiP, subframeP);
           UE_info= &eNB->UE_info[UE_id];
      }

      for (j=0; j<num_sdu; j++) 
      {
        LOG_W(MAC,"SDU Number %d MAC Subheader SDU_LCID %d, length %d\n",j,rx_lcids[j],rx_lengths[j]);
        //if(j == 0 && 12 == rx_lengths[j])
        {
            printf("rx_sdu:Payload: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                payload_ptr[0],
                payload_ptr[1],
                payload_ptr[2],
                payload_ptr[3],
                payload_ptr[4],
                payload_ptr[5],
                payload_ptr[7],
                payload_ptr[8],
                payload_ptr[9],
                payload_ptr[10],
                payload_ptr[11]);
            //payload_ptr++;
        }
      }
      
      break;

    case TRUNCATED_BSR:
    case SHORT_BSR: {
      uint8_t lcgid = (payload_ptr[0] >> 6);
      uint8_t bsr = payload_ptr[0] & 0x3f;

      LOG_D(MAC, "[eNB %d] CC_id %d MAC CE_LCID %d : Received short BSR LCGID = %u bsr = %d\n",
	    enb_mod_idP, CC_idP, rx_ces[i], lcgid, bsr);

      if( rntiMac != rntiP )
      {
          LOG_E(MAC, "[eNB %d] CC_id %d MAC CE_LCID %d : Received short BSR LCGID = %u bsr = %d  rnti=%x crnti=%x\n",
    	    enb_mod_idP, CC_idP, rx_ces[i], lcgid, bsr, 
    	    rntiP, rntiMac);
          //UE_id = find_UE_id(enb_mod_idP,rntiMac);
      }

      if (UE_id  != -1) 
      {

            UE_info->UE_template[CC_idP].bsr_info[lcgid] = bsr;
        

            if (UE_info->UE_template[CC_idP].ul_buffer_creation_time[lcgid] == 0 ) {
              UE_info->UE_template[CC_idP].ul_buffer_creation_time[lcgid]=frameP;
            }
            phy_add_count(rntiP, 16);
            bsr_report[UE_id][subframeP]++;
            if(UE_info->UE_template[CC_idP].bsr_info[lcgid] == 0)
            {
                if( (UE_info->UE_template[CC_idP].bsr_zero_flag%8) != 9)
                {
                    //UE_info->UE_template[CC_idP].bsr_info[lcgid]=1;
                }
                UE_info->UE_template[CC_idP].bsr_zero_flag++;
                #if 0
                LOG_E(MAC, "[eNB %d] CC_id %d frameP=%d subframe=%d MAC CE_LCID %d(%d) : Received ZERO short BSR LCGID = %u bsr = %d payload=%x  \n",
                  enb_mod_idP, CC_idP, frameP, subframeP,rx_ces[i], i, lcgid, payload_ptr[0] & 0x3f, payload_ptr[0]);
                
                parse_ulsch_header_print(sduP,sdu_lenP);
                #endif
            }
            else
            {
                bsr_report_zero[UE_id][subframeP]++;
                UE_info->UE_template[CC_idP].bsr_zero_flag = 0;
                #if 0
                LOG_W(MAC, "[eNB %d] CC_id %d frameP=%d subframe=%d MAC CE_LCID %d(%d) : Received  short BSR LCGID = %u bsr = %d payload=%x  \n",
                  enb_mod_idP, CC_idP, frameP, subframeP, rx_ces[i], i, lcgid, payload_ptr[0] & 0x3f, payload_ptr[0]);
                
                parse_ulsch_header_print(sduP,sdu_lenP);
                #endif
            }
      }
      else {

      }
      payload_ptr += 1;//sizeof(SHORT_BSR); // fixme
    }
    break;

    case LONG_BSR:{
        uint8_t bsr[4];

        bsr[0]=((payload_ptr[0] & 0xFC) >> 2);
        bsr[1]=((payload_ptr[0] & 0x03) << 4) | ((payload_ptr[1] & 0xF0) >> 4);
        bsr[2]=((payload_ptr[1] & 0x0F) << 2) | ((payload_ptr[2] & 0xC0) >> 6);
        bsr[3]= (payload_ptr[2] & 0x3F);
        
        if( rntiMac != rntiP )
        {
            LOG_E(MAC, "[eNB %d] CC_id %d MAC CE_LCID %d: Received long BSR LCGID0 = %u LCGID1 = "
                  "%u LCGID2 = %u LCGID3 = %u rnti=%x crnti=%x\n",
                  enb_mod_idP, CC_idP,
                  rx_ces[i],
                  bsr[0],
                  bsr[1],
                  bsr[2],
                  bsr[3],
                  rntiP, rntiMac);
            //UE_id = find_UE_id(enb_mod_idP,rntiMac);
            //UE_info= &eNB->UE_info[UE_id];
        }

      if (UE_id  != -1) {
        phy_add_count(rntiP, 17);
        bsr_report[UE_id][subframeP]++;
        
        UE_info->UE_template[CC_idP].bsr_info[LCGID0] = bsr[0];
        UE_info->UE_template[CC_idP].bsr_info[LCGID1] = bsr[1];
        UE_info->UE_template[CC_idP].bsr_info[LCGID2] = bsr[2];
        UE_info->UE_template[CC_idP].bsr_info[LCGID3] = bsr[3];


        if((UE_info->UE_template[CC_idP].bsr_info[LCGID0] 
            + UE_info->UE_template[CC_idP].bsr_info[LCGID1] 
            + UE_info->UE_template[CC_idP].bsr_info[LCGID2] 
            + UE_info->UE_template[CC_idP].bsr_info[LCGID3]) == 0)
        {
            if( (UE_info->UE_template[CC_idP].bsr_zero_flag%8) != 9)
            {
                //UE_info->UE_template[CC_idP].bsr_info[LCGID3] = 1;
            }
            UE_info->UE_template[CC_idP].bsr_zero_flag++;
            
            #if 0
            LOG_E(MAC, "[eNB %d] CC_id %d frameP=%d subframe=%d MAC CE_LCID %d : Received ZERO long BSR  bsr = %d %d %d %d payload=%x %x %x \n",
              enb_mod_idP, CC_idP, frameP, subframeP, rx_ces[i], 
              UE_info->UE_template[CC_idP].bsr_info[LCGID0],
              UE_info->UE_template[CC_idP].bsr_info[LCGID1],
              UE_info->UE_template[CC_idP].bsr_info[LCGID2],
              UE_info->UE_template[CC_idP].bsr_info[LCGID3],
              payload_ptr[0],payload_ptr[1],payload_ptr[2]);
            parse_ulsch_header_print(sduP,sdu_lenP);
            #endif
        }
        else
        {
            bsr_report_zero[UE_id][subframeP]++;
            UE_info->UE_template[CC_idP].bsr_zero_flag = 0;
            #if 0
            LOG_W(MAC, "[eNB %d] CC_id %d frameP=%d subframe=%d MAC CE_LCID %d : Received long BSR  bsr = %d %d %d %d payload=%x %x %x \n",
              enb_mod_idP, CC_idP, frameP, subframeP, rx_ces[i], 
              UE_info->UE_template[CC_idP].bsr_info[LCGID0],
              UE_info->UE_template[CC_idP].bsr_info[LCGID1],
              UE_info->UE_template[CC_idP].bsr_info[LCGID2],
              UE_info->UE_template[CC_idP].bsr_info[LCGID3],
              payload_ptr[0],payload_ptr[1],payload_ptr[2]);
            parse_ulsch_header_print(sduP,sdu_lenP);
            #endif
        }

        if (UE_info->UE_template[CC_idP].bsr_info[LCGID0] == 0 ) {
          UE_info->UE_template[CC_idP].ul_buffer_creation_time[LCGID0]=0;
        } else if (UE_info->UE_template[CC_idP].ul_buffer_creation_time[LCGID0] == 0) {
          UE_info->UE_template[CC_idP].ul_buffer_creation_time[LCGID0]=frameP;
        }

        if (UE_info->UE_template[CC_idP].bsr_info[LCGID1] == 0 ) {
          UE_info->UE_template[CC_idP].ul_buffer_creation_time[LCGID1]=0;
        } else if (UE_info->UE_template[CC_idP].ul_buffer_creation_time[LCGID1] == 0) {
          UE_info->UE_template[CC_idP].ul_buffer_creation_time[LCGID1]=frameP;
        }

        if (UE_info->UE_template[CC_idP].bsr_info[LCGID2] == 0 ) {
          UE_info->UE_template[CC_idP].ul_buffer_creation_time[LCGID2]=0;
        } else if (UE_info->UE_template[CC_idP].ul_buffer_creation_time[LCGID2] == 0) {
          UE_info->UE_template[CC_idP].ul_buffer_creation_time[LCGID2]=frameP;
        }

        if (UE_info->UE_template[CC_idP].bsr_info[LCGID3] == 0 ) {
          UE_info->UE_template[CC_idP].ul_buffer_creation_time[LCGID3]= 0;
        } else if (UE_info->UE_template[CC_idP].ul_buffer_creation_time[LCGID3] == 0) {
          UE_info->UE_template[CC_idP].ul_buffer_creation_time[LCGID3]=frameP;

        }
      }

      payload_ptr += 3;////sizeof(LONG_BSR);
      }
      break;

    default:
      LOG_E(MAC, "[eNB %d] CC_id %d Received unknown MAC header (0x%02x)\n", enb_mod_idP, CC_idP, rx_ces[i]);
      break;
    }
  }

  for (i=0; i<num_sdu; i++) 
  {
    LOG_D(MAC,"SDU Number %d MAC Subheader SDU_LCID %d, length %d\n",i,rx_lcids[i],rx_lengths[i]);

    switch (rx_lcids[i]) {
    case CCCH :
      LOG_I(MAC,"[eNB %d][RAPROC] CC_id %d Frame %d, Received CCCH:  %x.%x.%x.%x.%x.%x, Terminating RA procedure for UE rnti %x\n",
            enb_mod_idP,CC_idP,frameP,
            payload_ptr[0],payload_ptr[1],payload_ptr[2],payload_ptr[3],payload_ptr[4], payload_ptr[5], rntiP);

      for (ii=0; ii<NB_RA_PROC_MAX; ii++) {
        LOG_D(MAC,"[eNB %d][RAPROC] CC_id %p Checking proc %d : rnti (%x, %x), active %d\n",
              enb_mod_idP, CC_idP, ii,
              eNB->common_channels[CC_idP].RA_template[ii].rnti, rntiP,
              eNB->common_channels[CC_idP].RA_template[ii].RA_active);

        if ((eNB->common_channels[CC_idP].RA_template[ii].rnti==rntiP) &&
            (eNB->common_channels[CC_idP].RA_template[ii].RA_active==TRUE)) {

          //payload_ptr = parse_ulsch_header(msg3,&num_ce,&num_sdu,rx_ces,rx_lcids,rx_lengths,msg3_len);

          if (UE_id < 0) {
            memcpy(&eNB->common_channels[CC_idP].RA_template[ii].cont_res_id[0],payload_ptr,6);

            if( rx_lengths[i] < 1000 && rx_lengths[i] != 0 )
            {
                LOG_D(MAC,"[eNB %d][RAPROC] CC_id %d Frame %d CCCH: Received Msg3: length %d, offset %d\n",
                      enb_mod_idP,CC_idP,frameP,rx_lengths[i],payload_ptr-sduP);
                if ((UE_id=add_new_ue(enb_mod_idP,CC_idP,eNB->common_channels[CC_idP].RA_template[ii].rnti,harq_pidP,frameP,subframeP)) == -1 ) 
                {
                  mac_xface->macphy_exit("[MAC][eNB] Max user count reached\n");
        	      // kill RA procedure
                } 
                else
                {
                  LOG_I(MAC,"[eNB %d][RAPROC] CC_id %d Frame %d CCCH: Received Msg3: Added user with rnti %x => UE %d\n",
                        enb_mod_idP,CC_idP,frameP,eNB->common_channels[CC_idP].RA_template[ii].rnti,UE_id);
                }
            }
            else
            {
                LOG_E(MAC,"[eNB %d][RAPROC] CC_id %d Frame %d CCCH: Received Msg3: length %d, offset %d\n",
                      enb_mod_idP,CC_idP,frameP,rx_lengths[i],payload_ptr-sduP);
                LOG_E(MAC,"[eNB] num_sdu %d num_ces:%d sdu:%02x %02x %02x %02x %02x %02x %02x %02x - %02x %02x %02x %02x %02x %02x %02x %02x\n",
                      num_sdu,num_ce,
                      sduP[0],sduP[1],sduP[2],sduP[3],sduP[4],sduP[5],sduP[6],sduP[7],
                      sduP[8],sduP[9],sduP[10],sduP[11],sduP[12],sduP[13],sduP[14],sduP[15]);
                for(int j = 0; j < num_sdu; j++)
                {
                  printf("SDU %d, lcid=%d len=%d\n", j,rx_lcids[j],rx_lengths[j]);
                }
                for(int j = 0; j < num_ce; j++)
                {
                  printf("CE %d, lcid=%d \n", j,rx_ces[j]);
                }
            }
          } 
          else 
          {
            LOG_I(MAC,"[eNB %d][RAPROC] CC_id %d Frame %d CCCH: Received Msg3 from already registered UE %d: length %d, offset %d\n",
                  enb_mod_idP,CC_idP,frameP,UE_id,rx_lengths[ii],payload_ptr-sduP);
	        // kill RA procedure
          }

          if (Is_rrc_registered == 1)
            mac_rrc_data_ind(
              enb_mod_idP,
              CC_idP,
              frameP,subframeP,
              rntiMac, //rntiP,
              CCCH,
              (uint8_t*)payload_ptr,
              rx_lengths[i],
              ENB_FLAG_YES,
              enb_mod_idP,
              0);


          if (num_ce >0) {  // handle msg3 which is not RRCConnectionRequest
            //  process_ra_message(msg3,num_ce,rx_lcids,rx_ces);
          }

          eNB->common_channels[CC_idP].RA_template[ii].generate_Msg4 = 1;
          eNB->common_channels[CC_idP].RA_template[ii].wait_ack_Msg4 = 0;

        } // if process is active
      } // loop on RA processes

      break;

    case  DCCH :
    case DCCH1 :
      //      if(eNB_mac_inst[module_idP][CC_idP].Dcch_lchan[UE_id].Active==1){

#if defined(ENABLE_MAC_PAYLOAD_DEBUG)
      LOG_T(MAC,"offset: %d\n",(unsigned char)((unsigned char*)payload_ptr-sduP));

      for (j=0; j<32; j++) {
        LOG_T(MAC,"%x ",payload_ptr[j]);
      }

      LOG_T(MAC,"\n");
#endif

      if (UE_id != -1) {
        //phy_print_user(UE_id);
        //  This check is just to make sure we didn't get a bogus SDU length, to be removed ...
        if (rx_lengths[i]<CCCH_PAYLOAD_SIZE_MAX) {
          LOG_D(MAC,"[eNB %d] CC_id %d Frame %d : ULSCH -> UL-DCCH, received %d bytes form UE %d on LCID %d rntiP=%x rntiMac=%x\n",
                enb_mod_idP,CC_idP,frameP, rx_lengths[i], UE_id, rx_lcids[i],rntiP,rntiMac);
          

          mac_rlc_data_ind(
            enb_mod_idP,
            rntiMac, //rntiP,  xhd_oai_multuser
	        enb_mod_idP,
            frameP,
            ENB_FLAG_YES,
            MBMS_FLAG_NO,
            rx_lcids[i],
            (char *)payload_ptr,
            rx_lengths[i],
            1,
            NULL);//(unsigned int*)crc_status);
          UE_info->eNB_UE_stats[CC_idP].num_pdu_rx[rx_lcids[i]]+=1;
          UE_info->eNB_UE_stats[CC_idP].num_bytes_rx[rx_lcids[i]]+=rx_lengths[i];
        }
      } /* UE_id != -1 */

      //      }
      break;

    case DTCH: // default DRB
      //      if(eNB_mac_inst[module_idP][CC_idP].Dcch_lchan[UE_id].Active==1){

#if defined(ENABLE_MAC_PAYLOAD_DEBUG)
      LOG_T(MAC,"offset: %d\n",(unsigned char)((unsigned char*)payload_ptr-sduP));

      for (j=0; j<32; j++) {
        LOG_T(MAC,"%x ",payload_ptr[j]);
      }

      LOG_T(MAC,"\n");
#endif

      LOG_D(MAC,"[eNB %d] CC_id %d Frame %d : ULSCH -> UL-DTCH, received %d bytes from UE %d for lcid %d rntiP=%x rntiMac=%x\n",
            enb_mod_idP,CC_idP,frameP, rx_lengths[i], UE_id,rx_lcids[i],rntiP,rntiMac);


      //xhd_oai_multuser
      if(rntiP != rntiMac)
      {
          LOG_I(MAC,"[eNB %d] CC_id %d Frame %d : ULSCH -> UL-DTCH, received %d bytes from UE %d for lcid %d rntiP=%x rntiMac=%x\n",
                enb_mod_idP,CC_idP,frameP, rx_lengths[i], UE_id,rx_lcids[i],rntiP,rntiMac);
      }

      if (UE_id != -1) {
        if ((rx_lengths[i] <SCH_PAYLOAD_SIZE_MAX) &&  (rx_lengths[i] > 0) ) {   // MAX SIZE OF transport block
          mac_rlc_data_ind(
            enb_mod_idP,
            rntiMac, //rntiP, xhd_oai_multuser
            enb_mod_idP,
            frameP,
            ENB_FLAG_YES,
            MBMS_FLAG_NO,
            DTCH,
            (char *)payload_ptr,
            rx_lengths[i],
            1,
            NULL);//(unsigned int*)crc_status);
          UE_info->eNB_UE_stats[CC_idP].num_pdu_rx[rx_lcids[i]]+=1;
          UE_info->eNB_UE_stats[CC_idP].num_bytes_rx[rx_lcids[i]]+=rx_lengths[i];
        }
      } /* UE_id != -1 */

      //      }
      break;

    default :  //if (rx_lcids[i] >= DTCH) {
      if (UE_id != -1)
        UE_info->eNB_UE_stats[CC_idP].num_errors_rx+=1;
      LOG_E(MAC,"[eNB %d] CC_id %d Frame %d : received unsupported or unknown LCID %d from UE %d ",
            enb_mod_idP, CC_idP, frameP, rx_lcids[i], UE_id);
      break;
    }

    payload_ptr+=rx_lengths[i];
  }

  /* NN--> FK: we could either check the payload, or use a phy helper to detect a false msg3 */
  if ((num_sdu == 0) && (num_ce==0)) {
    if (UE_id != -1)
      UE_info->eNB_UE_stats[CC_idP].total_num_errors_rx+=1;

    if (msg3_flagP != NULL) {
      if( *msg3_flagP == 1 ) {
        LOG_N(MAC,"[eNB %d] CC_id %d frame %d : false msg3 detection: signal phy to canceling RA and remove the UE\n", enb_mod_idP, CC_idP, frameP);
        *msg3_flagP=0;
      }
    }
  } else {
    if (UE_id != -1) {
      UE_info->eNB_UE_stats[CC_idP].pdu_bytes_rx=sdu_lenP;
      UE_info->eNB_UE_stats[CC_idP].total_pdu_bytes_rx+=sdu_lenP;
      UE_info->eNB_UE_stats[CC_idP].total_num_pdus_rx+=1;
    }
  }

  VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(VCD_SIGNAL_DUMPER_FUNCTIONS_RX_SDU,0);
  stop_meas(&eNB->rx_ulsch_sdu);
}


uint32_t bytes_to_bsr_index(int32_t nbytes)
{

  uint32_t i=0;

  if (nbytes<0) {
    return(0);
  }

  while ((i<BSR_TABLE_SIZE)&&
         (BSR_TABLE[i]<=nbytes)) {
    i++;
  }

  return(i-1);
}

void adjust_bsr_info(int buffer_occupancy,
                     uint16_t TBS,
                     UE_TEMPLATE *UE_template)
{

  uint32_t         tmp_bsr;

  // could not serve all the uplink traffic
  if (buffer_occupancy > 0 ) 
  {
    if (BSR_TABLE[UE_template->bsr_info[LCGID0]] <=  TBS ) 
    {
      tmp_bsr = BSR_TABLE[UE_template->bsr_info[LCGID0]]; // serving this amout of  bytes
      UE_template->bsr_info[LCGID0] = 0;

      if (BSR_TABLE[UE_template->bsr_info[LCGID1]] <= (TBS-tmp_bsr)) {
        tmp_bsr += BSR_TABLE[UE_template->bsr_info[LCGID1]];
        UE_template->bsr_info[LCGID1] = 0;

        if (BSR_TABLE[UE_template->bsr_info[LCGID2]] <= (TBS-tmp_bsr)) {
          tmp_bsr += BSR_TABLE[UE_template->bsr_info[LCGID2]];
          UE_template->bsr_info[LCGID2] = 0;

          if (BSR_TABLE[UE_template->bsr_info[LCGID3]] <= (TBS-tmp_bsr)) {
            tmp_bsr += BSR_TABLE[UE_template->bsr_info[LCGID3]];
            UE_template->bsr_info[LCGID3] = 0;
          } else {
            UE_template->bsr_info[LCGID3] = bytes_to_bsr_index((int32_t)BSR_TABLE[UE_template->bsr_info[LCGID3]] - ((int32_t) TBS - (int32_t)tmp_bsr));
          }
        } else {
          UE_template->bsr_info[LCGID2] = bytes_to_bsr_index((int32_t)BSR_TABLE[UE_template->bsr_info[LCGID2]] - ((int32_t)TBS - (int32_t)tmp_bsr));
        }
      } else {
        UE_template->bsr_info[LCGID1] = bytes_to_bsr_index((int32_t)BSR_TABLE[UE_template->bsr_info[LCGID1]] - ((int32_t)TBS - (int32_t)tmp_bsr));
      }
    } 
    else 
    {
      UE_template->bsr_info[LCGID0] = bytes_to_bsr_index((int32_t)BSR_TABLE[UE_template->bsr_info[LCGID0]] - (int32_t)TBS);
    }
  } 
  else 
  { 
    // we have flushed all buffers so clear bsr
    UE_template->bsr_info[LCGID0] = 0;
    UE_template->bsr_info[LCGID1] = 0;
    UE_template->bsr_info[LCGID2] = 0;
    UE_template->bsr_info[LCGID3] = 0;
  }


}
void add_ue_ulsch_info(module_id_t module_idP, int CC_id, int UE_id, sub_frame_t subframeP, UE_ULSCH_STATUS status)
{

  eNB_ulsch_info[module_idP][CC_id][UE_id].rnti             = UE_RNTI(module_idP,UE_id);
  eNB_ulsch_info[module_idP][CC_id][UE_id].subframe         = subframeP;
  eNB_ulsch_info[module_idP][CC_id][UE_id].status           = status;

  eNB_ulsch_info[module_idP][CC_id][UE_id].serving_num++;

}

// This seems not to be used anymore
/*
int schedule_next_ulue(module_id_t module_idP, int UE_id, sub_frame_t subframeP){

  int next_ue;

  // first phase: scheduling for ACK
  switch (subframeP) {
    // scheduling for subframeP 2: for scheduled user during subframeP 5 and 6
  case 8:
    if  ((eNB_dlsch_info[module_idP][UE_id].status == S_DL_SCHEDULED) &&
   (eNB_dlsch_info[module_idP][UE_id].subframe == 5 || eNB_dlsch_info[module_idP][UE_id].subframe == 6)){
      // set the downlink status
      eNB_dlsch_info[module_idP][UE_id].status = S_DL_BUFFERED;
      return UE_id;
    }
    break;
    // scheduling for subframeP 3: for scheduled user during subframeP 7 and 8
  case 9:
    if  ((eNB_dlsch_info[module_idP][UE_id].status == S_DL_SCHEDULED) &&
   (eNB_dlsch_info[module_idP][UE_id].subframe == 7 || eNB_dlsch_info[module_idP][UE_id].subframe == 8)){
      eNB_dlsch_info[module_idP][UE_id].status = S_DL_BUFFERED;
      return UE_id;
    }
    break;
    // scheduling UL subframeP 4: for scheduled user during subframeP 9 and 0
  case 0 :
    if  ((eNB_dlsch_info[module_idP][UE_id].status == S_DL_SCHEDULED) &&
   (eNB_dlsch_info[module_idP][UE_id].subframe == 9 || eNB_dlsch_info[module_idP][UE_id].subframe == 0)){
      eNB_dlsch_info[module_idP][UE_id].status = S_DL_BUFFERED;
      return UE_id;
    }
    break;
  default:
    break;
  }

  // second phase
  for (next_ue=0; next_ue <NUMBER_OF_UE_MAX; next_ue++ ){

    if  (eNB_ulsch_info[module_idP][next_ue].status == S_UL_WAITING )
      return next_ue;
    else if (eNB_ulsch_info[module_idP][next_ue].status == S_UL_SCHEDULED){
      eNB_ulsch_info[module_idP][next_ue].status = S_UL_BUFFERED;
    }
  }
  for (next_ue=0; next_ue <NUMBER_OF_UE_MAX; next_ue++ ){
    if (eNB_ulsch_info[module_idP][next_ue].status != S_UL_NONE )// do this just for active UEs
      eNB_ulsch_info[module_idP][next_ue].status = S_UL_WAITING;
  }
  next_ue = 0;
  return next_ue;

}
 */





unsigned char *parse_ulsch_header(unsigned char *mac_header,
                                  unsigned char *num_ce,
                                  unsigned char *num_sdu,
                                  unsigned char *rx_ces,
                                  unsigned char *rx_lcids,
                                  unsigned short *rx_lengths,
                                  unsigned short tb_length)
{

  unsigned char not_done=1,num_ces=0,num_sdus=0,lcid,num_sdu_cnt;
  unsigned char *mac_header_ptr = mac_header;
  unsigned short length, ce_len=0;

  unsigned char *mac_header_ptr0;

  while (not_done==1) {

    if (((SCH_SUBHEADER_FIXED*)mac_header_ptr)->E == 0) {
      not_done = 0;
    }

    lcid = ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->LCID;
    mac_header_ptr0 = mac_header_ptr;
    if (lcid < EXTENDED_POWER_HEADROOM) 
    {
      if (not_done==0) 
      { 
        // last MAC SDU, length is implicit
        mac_header_ptr++;
        length = tb_length-(mac_header_ptr-mac_header)-ce_len;

        for (num_sdu_cnt=0; num_sdu_cnt < num_sdus ; num_sdu_cnt++) {
          length -= rx_lengths[num_sdu_cnt];
        }
      } 
      else 
      {
        if (((SCH_SUBHEADER_SHORT *)mac_header_ptr)->F == 0) {
          length = ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->L;
          mac_header_ptr += 2;//sizeof(SCH_SUBHEADER_SHORT);
        } 
        else 
        { 
          // F = 1
          length = ((((SCH_SUBHEADER_LONG *)mac_header_ptr)->L_MSB & 0x7f ) << 8 ) 
                   | (((SCH_SUBHEADER_LONG *)mac_header_ptr)->L_LSB & 0xff);
          mac_header_ptr += 3;//sizeof(SCH_SUBHEADER_LONG);

        }
      }

      LOG_D(MAC,"[eNB] sdu %d lcid %d tb_length %d length %d (offset now %d)\n",
            num_sdus,lcid,tb_length, length,mac_header_ptr-mac_header);
      rx_lcids[num_sdus] = lcid;
      rx_lengths[num_sdus] = length;
      num_sdus++;

      if(lcid==CCCH && length > 1000)
      {
          LOG_E(MAC,"[eNB] sdu %d lcid %d tb_length %d length %d (offset now %d) num_sdus%d num_ces:%d sdu:%02x %02x %02x %02x %02x %02x %02x %02x - %02x %02x %02x %02x %02x %02x %02x %02x\n",
                num_sdus,lcid,tb_length, length,mac_header_ptr-mac_header,num_sdus,num_ces,
                mac_header[0],mac_header[1],mac_header[2],mac_header[3],mac_header[4],mac_header[5],mac_header[6],mac_header[7],
                mac_header[8],mac_header[9],mac_header[10],mac_header[11],mac_header[12],mac_header[13],mac_header[14],mac_header[15]);
          for(int j = 0; j < num_sdus; j++)
          {
            printf("SDU %d, lcid=%d len=%d\n", j,rx_lcids[j],rx_lengths[j]);
          }
          for(int j = 0; j < num_ces; j++)
          {
            printf("CE %d, lcid=%d \n", j,rx_ces[j]);
          }
          if (not_done==0) 
          { 
              if (((SCH_SUBHEADER_SHORT *)mac_header_ptr0)->F == 0) {
                printf("AAA F=%d L=%d\n",
                    ((SCH_SUBHEADER_SHORT *)mac_header_ptr0)->F,
                    ((SCH_SUBHEADER_SHORT *)mac_header_ptr0)->L);
              } 
              else 
              { 
                  printf("BBB F=%d MSB=%d LSB=%d\n",
                      ((SCH_SUBHEADER_LONG *)mac_header_ptr0)->F,
                      ((SCH_SUBHEADER_LONG *)mac_header_ptr0)->L_MSB,
                      ((SCH_SUBHEADER_LONG *)mac_header_ptr0)->L_LSB
                      );
              
              }
          }
          else
          {
              printf("CCC \n");
          }
      }
    }
    else 
    { 
      // This is a control element subheader POWER_HEADROOM, BSR and CRNTI
      if (lcid == SHORT_PADDING) {
        mac_header_ptr++;
      } 
      else 
      {
        rx_ces[num_ces] = lcid;
        num_ces++;
        mac_header_ptr++;

        if (lcid==LONG_BSR) {
          ce_len+=3;
        } else if (lcid==CRNTI) {
          ce_len+=2;
        } else if ((lcid==POWER_HEADROOM) || (lcid==TRUNCATED_BSR)|| (lcid== SHORT_BSR)) {
          ce_len++;
        } else {
          LOG_E(MAC,"unknown CE %d \n", lcid);
          ce_len++;
          //mac_xface->macphy_exit("unknown CE");
        }
      }
    }
  }

  *num_ce = num_ces;
  *num_sdu = num_sdus;

  return(mac_header_ptr);
}

void parse_ulsch_header_print(unsigned char *mac_header,
                                  unsigned short tb_length)
{

  unsigned char not_done=1,num_ces=0,num_sdus=0,lcid,num_sdu_cnt;
  unsigned char *mac_header_ptr = mac_header;
  unsigned short length, ce_len=0;
  unsigned short rx_lengths[20];

  unsigned char *mac_header_ptr0;
  
  printf("mac len=%d [",tb_length);
  for(int i = 0; i<tb_length && i < 30; i++)
  {
      printf("%02x ", mac_header[i]);
  }
  printf("][");
  for(int i = tb_length-30; i<tb_length && i>0; i++)
  {
      printf("%02x ", mac_header[i]);
  }
  printf("]\n");

  while (not_done==1) {

    if (((SCH_SUBHEADER_FIXED*)mac_header_ptr)->E == 0) {
      not_done = 0;
    }

    lcid = ((SCH_SUBHEADER_FIXED *)mac_header_ptr)->LCID;
    mac_header_ptr0 = mac_header_ptr;
    if (lcid < EXTENDED_POWER_HEADROOM) 
    {
      if (not_done==0) 
      { 
        printf("SDU[%d lcid=%d len=%d p=%x]", 
            num_sdus, lcid, tb_length-(mac_header_ptr+1-mac_header)-ce_len, mac_header_ptr[0]);
        // last MAC SDU, length is implicit
        mac_header_ptr++;
        length = tb_length-(mac_header_ptr-mac_header)-ce_len;

        for (num_sdu_cnt=0; num_sdu_cnt < num_sdus ; num_sdu_cnt++) {
          length -= rx_lengths[num_sdu_cnt];
        }
      } 
      else 
      {
        if (((SCH_SUBHEADER_SHORT *)mac_header_ptr)->F == 0) 
        {
          length = ((SCH_SUBHEADER_SHORT *)mac_header_ptr)->L;
          printf("SDU[%d lcid=%d len=%d p=%x %x]", num_sdus, lcid, length, mac_header_ptr[0], mac_header_ptr[1]);
          mac_header_ptr += 2;//sizeof(SCH_SUBHEADER_SHORT);
        } 
        else 
        { 
          // F = 1
          length = ((((SCH_SUBHEADER_LONG *)mac_header_ptr)->L_MSB & 0x7f ) << 8 ) 
                   | (((SCH_SUBHEADER_LONG *)mac_header_ptr)->L_LSB & 0xff);
          printf("SDU[%d lcid=%d len=%d p=%x %x %x]", num_sdus, lcid, length, mac_header_ptr[0], mac_header_ptr[1], mac_header_ptr[2]);
          mac_header_ptr += 3;//sizeof(SCH_SUBHEADER_LONG);

        }
      }

      LOG_D(MAC,"[eNB] sdu %d lcid %d tb_length %d length %d (offset now %d)\n",
            num_sdus,lcid,tb_length, length,mac_header_ptr-mac_header);

      num_sdus++;

    }
    else 
    { 
      // This is a control element subheader POWER_HEADROOM, BSR and CRNTI
      if (lcid == SHORT_PADDING) {
          printf("CE[SHORT_PADDING p=%x]", mac_header_ptr[0]);
        mac_header_ptr++;
      } 
      else 
      {

        if (lcid==LONG_BSR) {
          ce_len+=3;
        } else if (lcid==CRNTI) {
          ce_len+=2;
        } else if ((lcid==POWER_HEADROOM) || (lcid==TRUNCATED_BSR)|| (lcid== SHORT_BSR)) {
          ce_len++;
        } else {
          LOG_E(MAC,"unknown CE %d \n", lcid);
          ce_len++;
          //mac_xface->macphy_exit("unknown CE");
        }
        printf("CE[%d lcid=%d p=%x]", num_ces, lcid, mac_header_ptr[0]);

        num_ces++;
        mac_header_ptr++;

      }
    }
  }
  printf("\n");

  return;
}

void schedule_ulsch(module_id_t module_idP, frame_t frameP,unsigned char cooperation_flag,sub_frame_t subframeP, unsigned char sched_subframe,
                    unsigned int *nCCE)  //,int calibration_flag) {
{


  unsigned int nCCE_available[MAX_NUM_CCs];
  uint16_t first_rb[MAX_NUM_CCs],i;
  int CC_id;
  eNB_MAC_INST *eNB=&eNB_mac_inst[module_idP];

  start_meas(&eNB->schedule_ulsch);


  for (CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) {

    first_rb[CC_id] = 1;
    nCCE_available[CC_id] = mac_xface->get_nCCE_max(module_idP,CC_id) - nCCE[CC_id];

    // UE data info;
    // check which UE has data to transmit
    // function to decide the scheduling
    // e.g. scheduling_rslt = Greedy(granted_UEs, nb_RB)

    // default function for default scheduling
    //

    // output of scheduling, the UE numbers in RBs, where it is in the code???
    // check if RA (Msg3) is active in this subframeP, if so skip the PRBs used for Msg3
    // Msg3 is using 1 PRB so we need to increase first_rb accordingly
    // not sure about the break (can there be more than 1 active RA procedure?)

    for (i=0; i<NB_RA_PROC_MAX; i++) {
      if ((eNB->common_channels[CC_id].RA_template[i].RA_active == TRUE) &&
          (eNB->common_channels[CC_id].RA_template[i].generate_rar == 0) &&
          (eNB->common_channels[CC_id].RA_template[i].Msg3_subframe == sched_subframe)) {
	//leave out first RB for PUCCH
        first_rb[CC_id]++;
        break;
      }
    }

    /*
    if (mac_xface->is_prach_subframe(&(mac_xface->lte_frame_parms),frameP,subframeP)) {
      first_rb[CC_id] = (mac_xface->get_prach_prb_offset(&(mac_xface->lte_frame_parms),
    */

  }

  schedule_ulsch_rnti(module_idP, cooperation_flag, frameP, subframeP, sched_subframe, nCCE, nCCE_available, first_rb);

#ifdef CBA
  schedule_ulsch_cba_rnti(module_idP, cooperation_flag, frameP, subframeP, sched_subframe, nCCE, nCCE_available, first_rb);
#endif


  stop_meas(&eNB->schedule_ulsch);

}

extern uint8_t get_phy_ueid_by_rnti(uint16_t rnti);

void phy_add_rb(uint16_t rnti, uint16_t harq_pid, uint16_t first_rb, uint16_t nb_rb,
                    frame_t       frameP, sub_frame_t   subframeP, uint8_t  round, uint8_t *printf_flg, uint32_t rballoc);

uint16_t DAI_sum[10]={0};

void add_dai(uint8_t sched_subframe, uint8_t ucValue)
{
    DAI_sum[sched_subframe]+= ucValue;
}

void print_dai()
{
    printf("DAI subframe 2 3 4 is %d %d %d\n", DAI_sum[2], DAI_sum[3], DAI_sum[4]);
    DAI_sum[2] = 0;
    DAI_sum[3] = 0;
    DAI_sum[4] = 0;
}
extern int bsr_clear[OLTE_MAX_USER][10];
extern int bsr_clear_zero[OLTE_MAX_USER][10];


void dci_set_ulsch(DCI_PDU *DCI_pdu, void *ULSCH_dci_t,  uint8_t frame_type, 
  uint8_t N_RB_UL, uint8_t mcs,  uint8_t ndi, 
  uint8_t tpc,uint8_t dai, uint16_t rballoc, uint8_t cshift, uint8_t cqi_req, 
  uint8_t aggregation, rnti_t  rnti)
{
    DCI_PDU *ULSCH_dci = (DCI_PDU *)ULSCH_dci_t;
    
    if (frame_type == TDD) 
    {
        //add_dai(sched_subframe, (UE_template->DAI_ul[sched_subframe]+1)&3);
        switch (N_RB_UL) {
        case 6:

          ((DCI0_1_5MHz_TDD_1_6_t *)ULSCH_dci)->type     = 0;
          ((DCI0_1_5MHz_TDD_1_6_t *)ULSCH_dci)->hopping  = 0;
          ((DCI0_1_5MHz_TDD_1_6_t *)ULSCH_dci)->rballoc  = rballoc;
          ((DCI0_1_5MHz_TDD_1_6_t *)ULSCH_dci)->mcs      = mcs;
          ((DCI0_1_5MHz_TDD_1_6_t *)ULSCH_dci)->ndi      = ndi;
          ((DCI0_1_5MHz_TDD_1_6_t *)ULSCH_dci)->TPC      = tpc;
          ((DCI0_1_5MHz_TDD_1_6_t *)ULSCH_dci)->cshift   = cshift;
          ((DCI0_1_5MHz_TDD_1_6_t *)ULSCH_dci)->padding  = 0;
          ((DCI0_1_5MHz_TDD_1_6_t *)ULSCH_dci)->dai      = dai;
          ((DCI0_1_5MHz_TDD_1_6_t *)ULSCH_dci)->cqi_req  = cqi_req;

          add_ue_spec_dci(DCI_pdu,
                          ULSCH_dci,
                          rnti,
                          sizeof(DCI0_1_5MHz_TDD_1_6_t),
                          aggregation,
                          sizeof_DCI0_1_5MHz_TDD_1_6_t,
                          format0,
                          0);
          break;

        default:
        case 25:

          ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->type     = 0;
          ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->hopping  = 0;
          ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->rballoc  = rballoc;
          ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->mcs      = mcs;
          ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->ndi      = ndi;
          ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->TPC      = tpc;
          ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->cshift   = cshift;
          ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->padding  = 0;
          ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->dai      = dai;
          ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->cqi_req  = cqi_req;

          add_ue_spec_dci(DCI_pdu,
                          ULSCH_dci,
                          rnti,
                          sizeof(DCI0_5MHz_TDD_1_6_t),
                          aggregation,
                          sizeof_DCI0_5MHz_TDD_1_6_t,
                          format0,
                          0);
          break;

        case 50:

          ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->type     = 0;
          ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->hopping  = 0;
          ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->rballoc  = rballoc;
          ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->mcs      = mcs;
          ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->ndi      = ndi;
          ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->TPC      = tpc;
          ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->cshift   = cshift;
          ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->padding  = 0;
          ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->dai      = dai;
          ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->cqi_req  = cqi_req;

          add_ue_spec_dci(DCI_pdu,
                          ULSCH_dci,
                          rnti,
                          sizeof(DCI0_10MHz_TDD_1_6_t),
                          aggregation,
                          sizeof_DCI0_10MHz_TDD_1_6_t,
                          format0,
                          0);
          break;

        case 100:

          ((DCI0_20MHz_TDD_1_6_t *)ULSCH_dci)->type     = 0;
          ((DCI0_20MHz_TDD_1_6_t *)ULSCH_dci)->hopping  = 0;
          ((DCI0_20MHz_TDD_1_6_t *)ULSCH_dci)->rballoc  = rballoc;
          ((DCI0_20MHz_TDD_1_6_t *)ULSCH_dci)->mcs      = mcs;
          ((DCI0_20MHz_TDD_1_6_t *)ULSCH_dci)->ndi      = ndi;
          ((DCI0_20MHz_TDD_1_6_t *)ULSCH_dci)->TPC      = tpc;
          ((DCI0_20MHz_TDD_1_6_t *)ULSCH_dci)->cshift   = cshift;
          ((DCI0_20MHz_TDD_1_6_t *)ULSCH_dci)->padding  = 0;
          ((DCI0_20MHz_TDD_1_6_t *)ULSCH_dci)->dai      = dai;
          ((DCI0_20MHz_TDD_1_6_t *)ULSCH_dci)->cqi_req  = cqi_req;

          add_ue_spec_dci(DCI_pdu,
                          ULSCH_dci,
                          rnti,
                          sizeof(DCI0_20MHz_TDD_1_6_t),
                          aggregation,
                          sizeof_DCI0_20MHz_TDD_1_6_t,
                          format0,
                          0);
          break;
        }
      } // TDD
      else 
      { 
        //FDD
        switch (N_RB_UL) {
        case 25:
        default:


          ((DCI0_5MHz_FDD_t *)ULSCH_dci)->type     = 0;
          ((DCI0_5MHz_FDD_t *)ULSCH_dci)->hopping  = 0;
          ((DCI0_5MHz_FDD_t *)ULSCH_dci)->rballoc  = rballoc;
          ((DCI0_5MHz_FDD_t *)ULSCH_dci)->mcs      = mcs;
          ((DCI0_5MHz_FDD_t *)ULSCH_dci)->ndi      = ndi;
          ((DCI0_5MHz_FDD_t *)ULSCH_dci)->TPC      = tpc;
          ((DCI0_5MHz_FDD_t *)ULSCH_dci)->cshift   = cshift;
          ((DCI0_5MHz_FDD_t *)ULSCH_dci)->padding  = 0;
          ((DCI0_5MHz_FDD_t *)ULSCH_dci)->cqi_req  = cqi_req;

          add_ue_spec_dci(DCI_pdu,
                          ULSCH_dci,
                          rnti,
                          sizeof(DCI0_5MHz_FDD_t),
                          aggregation,
                          sizeof_DCI0_5MHz_FDD_t,
                          format0,
                          0);
          break;

        case 6:

          ((DCI0_1_5MHz_FDD_t *)ULSCH_dci)->type     = 0;
          ((DCI0_1_5MHz_FDD_t *)ULSCH_dci)->hopping  = 0;
          ((DCI0_1_5MHz_FDD_t *)ULSCH_dci)->rballoc  = rballoc;
          ((DCI0_1_5MHz_FDD_t *)ULSCH_dci)->mcs      = mcs;
          ((DCI0_1_5MHz_FDD_t *)ULSCH_dci)->ndi      = ndi;
          ((DCI0_1_5MHz_FDD_t *)ULSCH_dci)->TPC      = tpc;
          ((DCI0_1_5MHz_FDD_t *)ULSCH_dci)->cshift   = cshift;
          ((DCI0_1_5MHz_FDD_t *)ULSCH_dci)->padding  = 0;
          ((DCI0_1_5MHz_FDD_t *)ULSCH_dci)->cqi_req  = cqi_req;

          add_ue_spec_dci(DCI_pdu,
                          ULSCH_dci,
                          rnti,
                          sizeof(DCI0_1_5MHz_FDD_t),
                          aggregation,
                          sizeof_DCI0_1_5MHz_FDD_t,
                          format0,
                          0);
          break;

        case 50:

          ((DCI0_10MHz_FDD_t *)ULSCH_dci)->type     = 0;
          ((DCI0_10MHz_FDD_t *)ULSCH_dci)->hopping  = 0;
          ((DCI0_10MHz_FDD_t *)ULSCH_dci)->rballoc  = rballoc;
          ((DCI0_10MHz_FDD_t *)ULSCH_dci)->mcs      = mcs;
          ((DCI0_10MHz_FDD_t *)ULSCH_dci)->ndi      = ndi;
          ((DCI0_10MHz_FDD_t *)ULSCH_dci)->TPC      = tpc;
          ((DCI0_10MHz_FDD_t *)ULSCH_dci)->padding  = 0;
          ((DCI0_10MHz_FDD_t *)ULSCH_dci)->cshift   = cshift;
          ((DCI0_10MHz_FDD_t *)ULSCH_dci)->cqi_req  = cqi_req;

          add_ue_spec_dci(DCI_pdu,
                          ULSCH_dci,
                          rnti,
                          sizeof(DCI0_10MHz_FDD_t),
                          aggregation,
                          sizeof_DCI0_10MHz_FDD_t,
                          format0,
                          0);
          break;

        case 100:

          ((DCI0_20MHz_FDD_t *)ULSCH_dci)->type     = 0;
          ((DCI0_20MHz_FDD_t *)ULSCH_dci)->hopping  = 0;
          ((DCI0_20MHz_FDD_t *)ULSCH_dci)->rballoc  = rballoc;
          ((DCI0_20MHz_FDD_t *)ULSCH_dci)->mcs      = mcs;
          ((DCI0_20MHz_FDD_t *)ULSCH_dci)->ndi      = ndi;
          ((DCI0_20MHz_FDD_t *)ULSCH_dci)->TPC      = tpc;
          ((DCI0_20MHz_FDD_t *)ULSCH_dci)->padding  = 0;
          ((DCI0_20MHz_FDD_t *)ULSCH_dci)->cshift   = cshift;
          ((DCI0_20MHz_FDD_t *)ULSCH_dci)->cqi_req  = cqi_req;

          add_ue_spec_dci(DCI_pdu,
                          ULSCH_dci,
                          rnti,
                          sizeof(DCI0_20MHz_FDD_t),
                          aggregation,
                          sizeof_DCI0_20MHz_FDD_t,
                          format0,
                          0);
          break;

        }
    }
}

int MacSrFlagGet(int  UE_id, uint32_t harq_pid)
{
    eNB_MAC_INST      *eNB=&eNB_mac_inst[0];
    UE_info_t         *UE_info=&eNB->UE_info[UE_id];
    UE_TEMPLATE       *UE_template= &UE_info->UE_template[0];
    return UE_template->ul_SR_flag[harq_pid];
}

void MacSrFlagClear(int  UE_id, uint32_t harq_pid)
{
    eNB_MAC_INST      *eNB=&eNB_mac_inst[0];
    UE_info_t         *UE_info=&eNB->UE_info[UE_id];
    UE_TEMPLATE       *UE_template= &UE_info->UE_template[0];
    UE_template->ul_SR_flag[harq_pid] = 0;
}
void MacSrFlagSet(int  UE_id, uint32_t harq_pid)
{
    eNB_MAC_INST      *eNB=&eNB_mac_inst[0];
    UE_info_t         *UE_info=&eNB->UE_info[UE_id];
    UE_TEMPLATE       *UE_template= &UE_info->UE_template[0];
    UE_template->ul_SR_flag[harq_pid] = 1;
}

void schedule_ulsch_rnti(module_id_t   module_idP,
                         unsigned char cooperation_flag,
                         frame_t       frameP,
                         sub_frame_t   subframeP,
                         unsigned char sched_subframe,
                         unsigned int *nCCE,
                         unsigned int *nCCE_available,
                         uint16_t     *first_rb)
{

  int                UE_id;
  //xhd_oai_ue
  uint8_t            aggregation    = 0;//0; //1; //2;  xhd_oai_multuser
  rnti_t             rnti           = -1;
  uint8_t            round          = 0;
  uint8_t            harq_pid       = 0;
  void              *ULSCH_dci      = NULL;
  LTE_eNB_UE_stats  *eNB_UE_stats   = NULL;
  DCI_PDU           *DCI_pdu;
  uint8_t                 status         = 0;
  uint8_t                 rb_table_index = -1;
  uint16_t                TBS = 0;
  int32_t                buffer_occupancy=0;
  uint32_t                cqi_req,cshift,ndi,mcs,rballoc,tpc;
  int32_t                 normalized_rx_power, target_rx_power=-90;
  static int32_t          tpc_accumulated=0;
  static int32_t          tpc_count=0;

  int n,CC_id = 0;
  eNB_MAC_INST      *eNB=&eNB_mac_inst[module_idP];
  UE_info_t         *UE_info;
  UE_list_t         *UE_list=&eNB->UE_list[subframeP];
  UE_TEMPLATE       *UE_template;
  int                rvidx_tab[4] = {0,2,3,1};
  LTE_DL_FRAME_PARMS   *frame_parms;
  uint8_t printf_flg = 0;


  uint8_t m_first_rb[16]={0};
  uint8_t m_nb_rb[16]={0};
  
  //  LOG_I(MAC,"entering ulsch preprocesor\n");

  ulsch_scheduler_pre_processor(module_idP,
                                frameP,
                                subframeP,sched_subframe,
                                first_rb,
                                aggregation,
                                nCCE);

  //  LOG_I(MAC,"exiting ulsch preprocesor\n");

  // loop over all active UEs
  for (UE_id=UE_list->head_ul; UE_id>=0; UE_id=UE_list->next_ul[UE_id]) 
  {
    UE_info=&eNB->UE_info[UE_id];

    UE_info->UE_template[UE_PCCID(module_idP,UE_id)].sche_ul=0;
    
    // don't schedule if Msg4 is not received yet
    if (UE_info->UE_template[UE_PCCID(module_idP,UE_id)].configured==FALSE) 
    {
        phy_add_count(rnti,14);
        continue;
    }

    rnti = UE_RNTI(module_idP,UE_id);

   uint16_t usUeId_phy = UE_id; //get_phy_ueid_by_rnti(rnti);

   #if 0
   if( usUeId_phy <= 2)
   {
       if( PHY_vars_eNB_g[0][0]->ulsch_eNB[usUeId_phy]->print_rb_flag < 100 )
       {
           printf("schedule_ulsch_rnti:frameP=%d subframeP=%d module_idP=%d UE_id=%d usUeId_phy=%d rnti=0x%x\n",
            frameP, subframeP, module_idP,UE_id, usUeId_phy, rnti );
       }
   }
   #endif

    if (rnti==NOT_A_RNTI) {
      LOG_W(MAC,"[eNB %d] frame %d subfarme %d, UE %d: no RNTI \n", module_idP,frameP,subframeP,UE_id);
      continue;
    }

    // loop over all active UL CC_ids for this UE
    for (n=0; n<UE_info->numactiveULCCs; n++) 
    {
      // This is the actual CC_id in the list
      CC_id = UE_info->ordered_ULCCids[n];
      UE_template = &UE_info->UE_template[CC_id];

      frame_parms = mac_xface->get_lte_frame_parms(module_idP,CC_id);
      eNB_UE_stats = mac_xface->get_eNB_UE_stats(module_idP,CC_id,rnti);

      if (eNB_UE_stats==NULL) {
        LOG_W(MAC,"[eNB %d] frame %d subframe %d, UE %d CC %d: no PHY context\n", module_idP,frameP,subframeP,UE_id,CC_id);
        continue; // mac_xface->macphy_exit("[MAC][eNB] Cannot find eNB_UE_stats\n");
      }

      if (nCCE_available[CC_id] < (1<<aggregation)) {
        LOG_W(MAC,"[eNB %d] frame %d subframe %d, UE %d CC %d: not enough nCCE (%d)\n", module_idP,frameP,subframeP,UE_id,CC_id,nCCE_available[CC_id]);
        continue; // break;
      }


      if (eNB_UE_stats->mode == PUSCH) 
      { 
        // ue has a ulsch channel
        //xhd_oai_multuser
        phy_add_count(rnti,13);
        
        DCI_pdu = &eNB->common_channels[CC_id].DCI_pdu[subframeP];

        if (mac_xface->get_ue_active_harq_pid(module_idP,CC_id,rnti,frameP,subframeP,&harq_pid,&round,1) == -1 ) 
        {
          LOG_W(MAC,"[eNB %d] Scheduler Frame %d, subframeP %d: candidate harq_pid from PHY for UE %d CC %d RNTI %x\n",
                module_idP,frameP,subframeP, UE_id, CC_id, rnti);
          //  NN --> RK: Don't schedule UE if we cannot get harq pid
          //should we continue or set harq_pid to 0?
          //xhd_oai_multuser
          //phy_add_count(rnti,14);

          continue;
        } 
        else
        {
          LOG_T(MAC,"[eNB %d] Frame %d, subframeP %d, UE %d CC %d : got harq pid %d  round %d (nCCE %d, rnti %x,mode %s)\n",
                module_idP,frameP,subframeP,UE_id,CC_id, harq_pid, round,nCCE[CC_id],rnti,mode_string[eNB_UE_stats->mode]);
        }

#if 0
#ifndef EXMIMO_IOT

        if (((UE_is_to_be_scheduled(module_idP,CC_id,UE_id)>0)) || (round>0) || ((frameP%10)==0))
          // if there is information on bsr of DCCH, DTCH or if there is UL_SR, or if there is a packet to retransmit, or we want to schedule a periodic feedback every 10 frames
#else
        if (round==0)
#endif
#endif
            // this is the normalized RX power and this should be constant (regardless of mcs
            normalized_rx_power = eNB_UE_stats->UL_rssi[0];
            target_rx_power = mac_xface->get_target_pusch_rx_power(module_idP,CC_id);
        
            // this assumes accumulated tpc
        // make sure that we are only sending a tpc update once a frame, otherwise the control loop will freak out
        tpc = 1;
        if( round == 0)
        {
            uint16_t usDelay = 10;
            int32_t framex10psubframe = UE_template->pusch_tpc_tx_frame*10+UE_template->pusch_tpc_tx_subframe;

            if (((framex10psubframe+usDelay)<=(frameP*10+subframeP)) || //normal case
                ((framex10psubframe>(frameP*10+subframeP)) && (((10240-framex10psubframe+frameP*10+subframeP)>=usDelay)))) //frame wrap-around
            {
                  UE_template->pusch_tpc_tx_frame=frameP;
                  UE_template->pusch_tpc_tx_subframe=subframeP;
                  if (normalized_rx_power>(target_rx_power+2)) {
                    tpc = 0; //-1
                    tpc_accumulated--;
                    tpc_count++;
                  } else if (normalized_rx_power<(target_rx_power-2)) {
                    tpc = 2; //+1
                    tpc_accumulated++;
                    tpc_count++;
                  }
            } 
            
            if (tpc!=1 && (frameP&0x1ff) == 0) 
            {
              LOG_D(MAC,"[eNB %d] UE%d ULSCH scheduler: fomat0(for pusch) frame %d, subframe %d, harq_pid %d, tpc %d, count %d accumulated %d, normalized/target rx power %d/%d\n",
                module_idP,UE_id, frameP,subframeP,harq_pid,tpc,
                tpc_count,tpc_accumulated,normalized_rx_power,target_rx_power);
              print_info("[eNB %d] UE%d ULSCH scheduler: fomat0(for pusch) frame %d, subframe %d, harq_pid %d, tpc %d, count %d accumulated %d, normalized/target rx power %d/%d\n",
                module_idP,UE_id, frameP,subframeP,harq_pid,tpc,
                tpc_count,tpc_accumulated,normalized_rx_power,target_rx_power);
            }
        }
        //if (((UE_is_to_be_scheduled(module_idP,CC_id,UE_id)>0)) || (round>0) || ((frameP%10)==0))
        status = mac_eNB_get_rrc_status(module_idP,rnti);
        cqi_req = (status < RRC_CONNECTED)? 0:1;

        if (round==0)
        {
          // reset the scheduling request
          if( UE_template->ul_SR != 0)
          {
              UE_template->ul_SR = 0;
              UE_template->ul_SR_flag[harq_pid]=1;
              phy_add_count_subframe(UE_template->rnti,255, 6,subframeP);
          }

          aggregation = process_ue_cqi(module_idP,UE_id); // =2 by default!!

          //power control
          //compute the expected ULSCH RX power (for the stats)


          // new transmission
          //if (round==0) 
          //{

            ndi = 1-UE_template->oldNDI_UL[harq_pid];
            UE_template->oldNDI_UL[harq_pid]=ndi;
    	    UE_info->eNB_UE_stats[CC_id].normalized_rx_power=normalized_rx_power;
    	    UE_info->eNB_UE_stats[CC_id].target_rx_power=target_rx_power;
    	    UE_info->eNB_UE_stats[CC_id].ulsch_mcs1 = UE_template->pre_assigned_mcs_ul[subframeP];
            mcs = cmin (UE_template->pre_assigned_mcs_ul[subframeP], openair_daq_vars.target_ue_ul_mcs); // adjust, based on user-defined MCS

            if (UE_template->pre_allocated_rb_table_index_ul[subframeP] >0) 
            {
                  mcs=16; //16; //cmin (16, openair_daq_vars.target_ue_ul_mcs);//test
                  rb_table_index=UE_template->pre_allocated_rb_table_index_ul[subframeP];
            } 
            else 
            {
                  //if( ((UE_id % 10) != ((subframeP+2) % 10))
                  //    && ((UE_id % 10) != ((subframeP+9) % 10)))//xhd_oai_20users
                  {
                      continue; 
                  }
    	          mcs=cmin (10, openair_daq_vars.target_ue_ul_mcs);
                  rb_table_index=2; // for PHR

                  //xhd_oai_20users statics for idle schedule in ulsch
                  phy_add_count(rnti,22);

    	    }

            UE_info->eNB_UE_stats[CC_id].ulsch_mcs2=mcs;
            buffer_occupancy = UE_template->ul_total_buffer[subframeP];

            //#if 0
            if( (rb_table[rb_table_index]> (33)) /*&& (UE_list->next_ul[UE_id] >= 0)*/)
            {
                while (((rb_table[rb_table_index]>(33)) 
                       ||(rb_table[rb_table_index]>(frame_parms->N_RB_UL-first_rb[CC_id])))&&
                       (rb_table_index>0)) {
                  rb_table_index--;
                }
            }
            else
            //#endif
            {
                while ((rb_table[rb_table_index]>(frame_parms->N_RB_UL-1-first_rb[CC_id])) &&
                       (rb_table_index>0)) {
                  rb_table_index--;
                }
            }

            //xhd_oai_rb only for test
            //while( rb_table[rb_table_index] > frame_parms->N_RB_UL / 2 )
            {
                //rb_table_index--;
            }
            
            if( first_rb[CC_id]+ rb_table[rb_table_index] > frame_parms->N_RB_UL )
            {
                //printf("schedule_ulsch_rnti:FATAL ERROR AAA:UE%d first_rb=%d nb_rb_ul=%d N_RB_UL=%d\n",
                //    UE_id,first_rb[CC_id],rb_table[rb_table_index],frame_parms->N_RB_UL );
                printf_flg = 1;
                break;
            }
            if( rb_table[rb_table_index] <= 2 )
            {
                //printf("schedule_ulsch_rnti:FATAL ERROR BBB:UE%d first_rb=%d nb_rb_ul=%d N_RB_UL=%d\n",
                //    UE_id,first_rb[CC_id],rb_table[rb_table_index],frame_parms->N_RB_UL );
                printf_flg = 1;
                break;
            }

            TBS = mac_xface->get_TBS_UL(mcs,rb_table[rb_table_index]);
    	    UE_info->eNB_UE_stats[CC_id].total_rbs_used_rx+=rb_table[rb_table_index];
    	    UE_info->eNB_UE_stats[CC_id].ulsch_TBS=TBS;
            buffer_occupancy -= (TBS/*8*/);
            rballoc = mac_xface->computeRIV(frame_parms->N_RB_UL,
                                            first_rb[CC_id],
                                            rb_table[rb_table_index]);
            //xhd_oai_multuser UL Schedule
            phy_add_rb(rnti, harq_pid, first_rb[CC_id], rb_table_index, frameP, subframeP, round, &printf_flg, rballoc);
            UE_template->first_rb_ul[harq_pid] = first_rb[CC_id];
            UE_template->round_ul[harq_pid] = round;
            UE_template->sche_ul = 1;
            // increment for next UE allocation
            first_rb[CC_id]+=rb_table[rb_table_index];
            //store for possible retransmission
            UE_template->nb_rb_ul[harq_pid] = rb_table[rb_table_index];
            UE_template->mcs[harq_pid] = mcs;

            if( harq_pid == 4)
            {

                print_info("schedule_ulsch_rnti: new data[eNB %d][PUSCH %d/%x] CC_id %d Frame %d subframeP %d Scheduled UE %d (mcs %d, first rb %d, nb_rb %d, rb_table_index %d, TBS %d, harq_pid %d)\n",
                      module_idP,harq_pid,rnti,CC_id,frameP,subframeP,UE_id,mcs,
                      first_rb[CC_id],rb_table[rb_table_index],
                      rb_table_index,TBS,harq_pid);
            }
            /*
               print_info("[eNB %d][PUSCH %d/%x] CC_id %d Frame %d subframeP %d Scheduled UE %d (mcs %d, first rb %d, nb_rb %d, rb_table_index %d, TBS %d, harq_pid %d)\n",
                  module_idP,harq_pid,rnti,CC_id,frameP,subframeP,UE_id,mcs,
                  first_rb[CC_id],rb_table[rb_table_index],
                  rb_table_index,TBS,harq_pid);*/
            
            // Adjust BSR entries for LCGIDs
            adjust_bsr_info(buffer_occupancy,
                            TBS/*8*/,
                            UE_template);
            if((UE_template->bsr_info[LCGID0]
                + UE_template->bsr_info[LCGID1]
                +UE_template->bsr_info[LCGID2]
                +UE_template->bsr_info[LCGID3])==0)
            {
                bsr_clear_zero[UE_id][subframeP]++;
            }
            else
            {
                bsr_clear[UE_id][subframeP]++;
            }

          } 
          else if (round > 0) 
          { 
            //we schedule a retransmission
            #if 0
              if (round == 1) //frame wrap-around
              {
                  tpc = 2;
                } else {
                  tpc = 1; //0
                }
            #endif
            //tpc = 0;
            
            ndi = UE_template->oldNDI_UL[harq_pid];

            if ((round&3)==0) 
            {
                mcs = UE_template->mcs[harq_pid];
                //mcs = openair_daq_vars.target_ue_ul_mcs;
            } 
            else 
            {
                
              mcs = rvidx_tab[round&3] + 28; //not correct for round==4!

            }
            mcs = UE_template->mcs[harq_pid];
            #if 0
            if(mcs != 10)
            {
                printf("schedule_ulsch_rnti:@@@@@@@@@@@@@@@retransmission [eNB %d][PUSCH %d/%x] CC_id %d Frame %d subframeP %d Scheduled UE  (mcs %d, first rb %d, nb_rb %d, harq_pid %d, round %d)\n",
                      module_idP,UE_id,rnti,CC_id,frameP,subframeP,mcs,
                      first_rb[CC_id],UE_template->nb_rb_ul[harq_pid],
            		  harq_pid, round);
                mcs = 10;
            }
            #endif

            if( harq_pid == 4)
            {
                print_info("schedule_ulsch_rnti:retransmission [eNB %d][PUSCH %d/%x] CC_id %d Frame %d subframeP %d Scheduled UE  (mcs %d, first rb %d, nb_rb %d, harq_pid %d, round %d)\n",
                      module_idP,UE_id,rnti,CC_id,frameP,subframeP,mcs,
                      first_rb[CC_id],UE_template->nb_rb_ul[harq_pid],
            		  harq_pid, round);
            }
            if( first_rb[CC_id]+ UE_template->nb_rb_ul[harq_pid] >frame_parms->N_RB_UL )
            {
                printf("schedule_ulsch_rnti:FATAL ERROR BBB:UE%d round=%d first_rb=%d nb_rb_ul=%d N_RB_UL=%d\n",
                    UE_id, round, first_rb[CC_id],UE_template->nb_rb_ul[harq_pid],frame_parms->N_RB_UL );
                printf_flg = 1;
                break;
            }
            rballoc = mac_xface->computeRIV(frame_parms->N_RB_UL,
                                            first_rb[CC_id],
                                            UE_template->nb_rb_ul[harq_pid]);

            //xhd_oai_multuser UL Schedule
            UE_template->first_rb_ul[harq_pid] = first_rb[CC_id];
            UE_template->round_ul[harq_pid] = round;
            UE_template->sche_ul = 1;
            
            phy_add_rb(rnti, harq_pid, first_rb[CC_id], UE_template->nb_rb_ul[harq_pid], frameP, subframeP, round, &printf_flg, rballoc);
            
            first_rb[CC_id]+=UE_template->nb_rb_ul[harq_pid];  // increment for next UE allocation
         
    	    UE_info->eNB_UE_stats[CC_id].num_retransmission_rx+=1;
    	    UE_info->eNB_UE_stats[CC_id].rbs_used_retx_rx=UE_template->nb_rb_ul[harq_pid];
    	    UE_info->eNB_UE_stats[CC_id].total_rbs_used_rx+=UE_template->nb_rb_ul[harq_pid];
    	    UE_info->eNB_UE_stats[CC_id].ulsch_mcs1=mcs;
    	    UE_info->eNB_UE_stats[CC_id].ulsch_mcs2=mcs;
    	  }

          // Cyclic shift for DM RS
          if(cooperation_flag == 2) {
            if(UE_id == 1) { // For Distriibuted Alamouti, cyclic shift applied to 2nd UE
              cshift = 1;
            } else {
              cshift = 0;
            }
          } else {
            cshift = 0;// values from 0 to 7 can be used for mapping the cyclic shift (36.211 , Table 5.5.2.1.1-1)
          }

          //xhd_oai_20users
          dci_set_ulsch(DCI_pdu, (void *)(UE_template->ULSCH_DCI[harq_pid]),  
            frame_parms->frame_type, frame_parms->N_RB_UL, mcs,  ndi, 
            tpc, UE_template->DAI_ul[sched_subframe], 
            rballoc, cshift, cqi_req, aggregation, rnti);
          

          add_ue_ulsch_info(module_idP,
                            CC_id,
                            UE_id,
                            subframeP,
                            S_UL_SCHEDULED);

          //xhd_oai_multuser
          phy_add_count(rnti,15);

          nCCE[CC_id] = nCCE[CC_id] + (1<<aggregation);
          nCCE_available[CC_id] = mac_xface->get_nCCE_max(module_idP,CC_id) - nCCE[CC_id];

          LOG_D(MAC,"[eNB %d] CC_id %d Frame %d, subframeP %d: Generated ULSCH DCI for next UE_id %d, format 0\n", module_idP,CC_id,frameP,subframeP,UE_id);
#ifdef DEBUG
          dump_dci(frame_parms, &DCI_pdu->dci_alloc[DCI_pdu->Num_common_dci+DCI_pdu->Num_ue_spec_dci-1]);
#endif

        //} // UE_is_to_be_scheduled
      } // UE is in PUSCH
    } // loop over UE_id
  } // loop of CC_id

  //if( printf_flg == 0 )
  {
    return;
  }
  printf("schedule_ulsch_rnti:harq_pid=%d result:[UE_id round RB first-count]",harq_pid);
  for (UE_id=UE_list->head_ul; UE_id>=0; UE_id=UE_list->next_ul[UE_id]) 
  {
    UE_info=&eNB->UE_info[UE_id];
    UE_template = &UE_info->UE_template[0];
    UE_sched_ctrl *ue_sched_ctl = &UE_list->UE_sched_ctrl[UE_id];

    if( UE_template->sche_ul == 0)
    {
       continue;
    }
    printf("[%d %d-%d %d-%d]", 
        UE_id, UE_template->round_ul[harq_pid],ue_sched_ctl->max_round_ul,
        UE_template->first_rb_ul[harq_pid], UE_template->nb_rb_ul[harq_pid]);
  }
  printf("\n");
  
}

#if 0
#ifdef CBA
void schedule_ulsch_cba_rnti(module_id_t module_idP, unsigned char cooperation_flag, frame_t frameP, sub_frame_t subframeP, unsigned char sched_subframe, unsigned int *nCCE,
                             unsigned int *nCCE_available, uint16_t *first_rb)
{

  eNB_MAC_INST *eNB = &eNB_mac_inst[module_idP];
  UE_list_t         *UE_list=&eNB->UE_list;
  //UE_TEMPLATE       *UE_template;
  void              *ULSCH_dci      = NULL;
  DCI_PDU *DCI_pdu;
  uint8_t CC_id=0;
  uint8_t rb_table_index=0, aggregation=2;
  uint32_t rballoc;
  uint8_t cba_group, cba_resources;
  uint8_t required_rbs[NUM_MAX_CBA_GROUP];
  int8_t num_cba_resources[NUM_MAX_CBA_GROUP];// , weight[NUM_MAX_CBA_GROUP]
  // the following vars should become a vector [MAX_NUM_CCs]
  LTE_DL_FRAME_PARMS   *frame_parms;
  int8_t available_rbs=0;
  uint8_t remaining_rbs=0;
  uint8_t allocated_rbs=0;
  uint8_t total_UEs=UE_list->num_UEs;
  uint8_t active_UEs[NUM_MAX_CBA_GROUP];
  uint8_t total_groups=eNB_mac_inst[module_idP].common_channels[CC_id].num_active_cba_groups;
  int     min_rb_unit=2;
  uint8_t cba_policy=CBA_ES;
  int     UE_id;
  uint8_t mcs[NUM_MAX_CBA_GROUP];
  uint32_t total_cba_resources=0;
  uint32_t total_rbs=0;
  // We compute the weight of each group and initialize some variables

  // loop over all active UEs
  //  for (UE_id=UE_list->head_ul;UE_id>=0;UE_id=UE_list->next_ul[UE_id]) {

  for (cba_group=0; cba_group<total_groups; cba_group++) {
    // UEs in PUSCH with traffic
    //    weight[cba_group] = 0;
    required_rbs[cba_group] = 0;
    num_cba_resources[cba_group]=0;
    active_UEs[cba_group]=0;
    mcs[cba_group]=openair_daq_vars.target_ue_ul_mcs;
  }

  //LOG_D(MAC, "[eNB ] CBA granted ues are %d\n",granted_UEs );

  for (CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) {

    frame_parms=mac_xface->get_lte_frame_parms(module_idP,CC_id);
    available_rbs=frame_parms->N_RB_DL-1-first_rb[CC_id];
    remaining_rbs=available_rbs;
    total_groups=eNB_mac_inst[module_idP].common_channels[CC_id].num_active_cba_groups;
    min_rb_unit=get_min_rb_unit(module_idP,CC_id);

    if (available_rbs  < min_rb_unit )
      continue;

    // remove this condition later
    // cba group template uses the exisitng UE template, and thus if a UE
    // is scheduled, the correspodning group can't be used for CBA
    // this can be fixed later
    if ((total_groups > 0) && (nCCE[CC_id] == 0)) {
      DCI_pdu = &eNB_mac_inst[module_idP].common_channels[CC_id].DCI_pdu[subframeP];

      for (cba_group=0;
           (cba_group<total_groups)  && (nCCE_available[CC_id]* (total_cba_resources+1) > (1<<aggregation));
           cba_group++) {
        // equal weight
        //weight[cba_group] = floor(total_UEs/active_groups);//find_num_active_UEs_in_cbagroup(module_idP, cba_group);

        for (UE_id=UE_list->head_ul; UE_id>=0; UE_id=UE_list->next_ul[UE_id]) {
          if (UE_RNTI(module_idP,UE_id)==NOT_A_RNTI)
            continue;

          // simple UE identity based grouping
          if ((UE_id % total_groups) == cba_group) { // this could be simplifed to  active_UEs[UE_id % total_groups]++;
            if ((mac_get_rrc_status(module_idP,1,UE_id) > RRC_CONNECTED) &&
                (UE_is_to_be_scheduled(module_idP,CC_id,UE_id) == 0)) {
              active_UEs[cba_group]++;
            }
          }

          if (UE_list->UE_template[CC_id][UE_id].pre_assigned_mcs_ul[subframeP] <= 2) {
            mcs[cba_group]= 8; // apply fixed scheduling
          } else if ((UE_id % total_groups) == cba_group) {
            mcs[cba_group]= cmin(mcs[cba_group],UE_list->UE_template[CC_id][UE_id].pre_assigned_mcs_ul[subframeP]);
          }
        }

        mcs[cba_group]= cmin(mcs[cba_group],openair_daq_vars.target_ue_ul_mcs);

        if (available_rbs < min_rb_unit )
          break;

        // If the group needs some resource
        // determine the total number of allocations (one or multiple DCIs): to be refined
        if ((active_UEs[cba_group] > 0) && (eNB_mac_inst[module_idP].common_channels[CC_id].cba_rnti[cba_group] != 0)) {
          // to be refined in case of : total_UEs >> weight[cba_group]*available_rbs

          switch(cba_policy) {
          case CBA_ES:
            required_rbs[cba_group] = (uint8_t)floor(available_rbs/total_groups); // allocate equally among groups
            num_cba_resources[cba_group]=1;
            break;

            // can't have more than one allocation for the same group/UE
            /*  case CBA_ES_S:
            required_rbs[cba_group] = (uint8_t)floor(available_rbs/total_groups); // allocate equally among groups
            if (required_rbs[cba_group] > min_rb_unit)
            num_cba_resources[cba_group]=(uint8_t)(required_rbs[cba_group]/ min_rb_unit);
            break;*/
          case CBA_PF:
            required_rbs[cba_group] = (uint8_t)floor((active_UEs[cba_group]*available_rbs)/total_UEs);
            num_cba_resources[cba_group]=1;
            break;
            /* case CBA_PF_S:
            required_rbs[cba_group] = (uint8_t)ceil((active_UEs[cba_group]*available_rbs)/total_UEs);
            if (required_rbs[cba_group] > min_rb_unit)
            num_cba_resources[cba_group]=(uint8_t) floor(required_rbs[cba_group] / min_rb_unit);
            break;*/

          default:
            LOG_W(MAC,"unknown CBA policy\n");
            break;
          }

          total_cba_resources+=num_cba_resources[cba_group];
          total_rbs+=required_rbs[cba_group];

          /*  while ((remaining_rbs < required_rbs[cba_group]) &&
          (required_rbs[cba_group] > 0) &&
          (required_rbs[cba_group] < min_rb_unit ))
          required_rbs[cba_group]--;
           */

          /*
          while (rb_table[rb_table_index] < required_rbs[cba_group])
          rb_table_index++;

          while (rb_table[rb_table_index] > remaining_rbs )
          rb_table_index--;

          remaining_rbs-=rb_table[rb_table_index];
          required_rbs[cba_group]=rb_table[rb_table_index];
           */
          //    num_cba_resources[cba_group]=1;

        }
      }

      // phase 2 reduce the number of cba allocations among the groups
      cba_group=0;

      while  (nCCE[CC_id] + (1<<aggregation) * total_cba_resources >= nCCE_available[CC_id]) {
        num_cba_resources[cba_group%total_groups]--;
        total_cba_resources--;
        //  LOG_N(MAC,"reducing num cba resources to %d for group %d \n", num_cba_resources[cba_group%total_groups], cba_group%total_groups );
        cba_group++;
      }

      if (total_cba_resources <= 0) {
        return;
      }

      // increase rb if any left: to be done
      cba_group=0;

      while (total_rbs  < available_rbs - 1 ) {
        required_rbs[cba_group%total_groups]++;
        total_rbs++;
        cba_group++;
      }

      // phase 3:
      for (cba_group=0; cba_group<total_groups; cba_group++) {

        LOG_N(MAC,
              "[eNB %d] CC_id %d Frame %d, subframe %d: cba group %d active_ues %d total groups %d mcs %d, available/required rb (%d/%d), num resources %d, ncce (%d/%d required %d \n",
              module_idP, CC_id, frameP, subframeP, cba_group,active_UEs[cba_group],total_groups,
              mcs[cba_group], available_rbs,required_rbs[cba_group],
              num_cba_resources[cba_group],
              nCCE[CC_id],nCCE_available[CC_id],(1<<aggregation) * num_cba_resources[cba_group]);

        for (cba_resources=0; cba_resources < num_cba_resources[cba_group]; cba_resources++) {
          rb_table_index =0;

          // check if there was an allocation for this group in the 1st phase
          if (required_rbs[cba_group] == 0 )
            continue;

          while (rb_table[rb_table_index] < (uint8_t) ceil(required_rbs[cba_group] / num_cba_resources[cba_group]) ) {
            rb_table_index++;
          }

          while (rb_table[rb_table_index] > remaining_rbs ) {
            rb_table_index--;
          }

          remaining_rbs-=rb_table[rb_table_index];
          allocated_rbs=rb_table[rb_table_index];

          rballoc = mac_xface->computeRIV(mac_xface->lte_frame_parms->N_RB_UL,
                                          first_rb[CC_id],
                                          rb_table[rb_table_index]);

          first_rb[CC_id]+=rb_table[rb_table_index];
          LOG_N(MAC,
                "[eNB %d] CC_id %d Frame %d, subframeP %d: schedule CBA access %d rnti %x, total/required/allocated/remaining rbs (%d/%d/%d/%d), mcs %d, rballoc %d, nCCE (%d/%d)\n",
                module_idP, CC_id, frameP, subframeP, cba_group,eNB_mac_inst[module_idP].common_channels[CC_id].cba_rnti[cba_group],
                available_rbs, required_rbs[cba_group], allocated_rbs, remaining_rbs,
                mcs[cba_group],rballoc,nCCE_available[CC_id],nCCE[CC_id]);

          switch (frame_parms->N_RB_UL) {
          case 6:
            mac_xface->macphy_exit("[MAC][eNB] CBA RB=6 not supported \n");
            break;

          case 25:
            if (frame_parms->frame_type == TDD) {
              ULSCH_dci = UE_list->UE_template[CC_id][cba_group].ULSCH_DCI[0];

              ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->type     = 0;
              ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->hopping  = 0;
              ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->rballoc  = rballoc;
              ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->mcs      = mcs[cba_group];
              ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->ndi      = 1;
              ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->TPC      = 1;//tpc;
              ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->cshift   = cba_group;
              ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->dai      = UE_list->UE_template[CC_id][cba_group].DAI_ul[sched_subframe];
              ((DCI0_5MHz_TDD_1_6_t *)ULSCH_dci)->cqi_req  = 1;

              //add_ue_spec_dci
              add_common_dci(DCI_pdu,
                             ULSCH_dci,
                             eNB_mac_inst[module_idP].common_channels[CC_id].cba_rnti[cba_group],
                             sizeof(DCI0_5MHz_TDD_1_6_t),
                             aggregation,
                             sizeof_DCI0_5MHz_TDD_1_6_t,
                             format0,
                             0);
            } else {
              ULSCH_dci = UE_list->UE_template[CC_id][cba_group].ULSCH_DCI[0];

              ((DCI0_5MHz_FDD_t *)ULSCH_dci)->type     = 0;
              ((DCI0_5MHz_FDD_t *)ULSCH_dci)->hopping  = 0;
              ((DCI0_5MHz_FDD_t *)ULSCH_dci)->rballoc  = rballoc;
              ((DCI0_5MHz_FDD_t *)ULSCH_dci)->mcs      = mcs[cba_group];
              ((DCI0_5MHz_FDD_t *)ULSCH_dci)->ndi      = 1;
              ((DCI0_5MHz_FDD_t *)ULSCH_dci)->TPC      = 1;//tpc;
              ((DCI0_5MHz_FDD_t *)ULSCH_dci)->cshift   = cba_group;
              ((DCI0_5MHz_FDD_t *)ULSCH_dci)->cqi_req  = 1;

              //add_ue_spec_dci
              add_common_dci(DCI_pdu,
                             ULSCH_dci,
                             eNB_mac_inst[module_idP].common_channels[CC_id].cba_rnti[cba_group],
                             sizeof(DCI0_5MHz_FDD_t),
                             aggregation,
                             sizeof_DCI0_5MHz_FDD_t,
                             format0,
                             0);
            }

            LOG_D(MAC,"[eNB %d] CC_id %d Frame %d, subframeP %d: Generated ULSCH DCI for CBA group %d, RB 25 format 0\n", module_idP,CC_id,frameP,subframeP,cba_group);
            break;

          case 50:
            if (frame_parms->frame_type == TDD) {
              ULSCH_dci = UE_list->UE_template[CC_id][cba_group].ULSCH_DCI[0];

              ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->type     = 0;
              ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->hopping  = 0;
              ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->rballoc  = rballoc;
              ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->mcs      = mcs[cba_group];
              ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->ndi      = 1;
              ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->TPC      = 1;//tpc;
              ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->cshift   = cba_group;
              ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->dai      = UE_list->UE_template[CC_id][cba_group].DAI_ul[sched_subframe];
              ((DCI0_10MHz_TDD_1_6_t *)ULSCH_dci)->cqi_req  = 1;

              //add_ue_spec_dci
              add_common_dci(DCI_pdu,
                             ULSCH_dci,
                             eNB_mac_inst[module_idP].common_channels[CC_id].cba_rnti[cba_group],
                             sizeof(DCI0_10MHz_TDD_1_6_t),
                             aggregation,
                             sizeof_DCI0_10MHz_TDD_1_6_t,
                             format0,
                             0);
            } else {
              ULSCH_dci = UE_list->UE_template[CC_id][cba_group].ULSCH_DCI[0];

              ((DCI0_10MHz_FDD_t *)ULSCH_dci)->type     = 0;
              ((DCI0_10MHz_FDD_t *)ULSCH_dci)->hopping  = 0;
              ((DCI0_10MHz_FDD_t *)ULSCH_dci)->rballoc  = rballoc;
              ((DCI0_10MHz_FDD_t *)ULSCH_dci)->mcs      = mcs[cba_group];
              ((DCI0_10MHz_FDD_t *)ULSCH_dci)->ndi      = 1;
              ((DCI0_10MHz_FDD_t *)ULSCH_dci)->TPC      = 1;//tpc;
              ((DCI0_10MHz_FDD_t *)ULSCH_dci)->cshift   = cba_group;
              ((DCI0_10MHz_FDD_t *)ULSCH_dci)->cqi_req  = 1;

              //add_ue_spec_dci
              add_common_dci(DCI_pdu,
                             ULSCH_dci,
                             eNB_mac_inst[module_idP].common_channels[CC_id].cba_rnti[cba_group],
                             sizeof(DCI0_10MHz_FDD_t),
                             aggregation,
                             sizeof_DCI0_10MHz_FDD_t,
                             format0,
                             0);
            }

            LOG_D(MAC,"[eNB %d] CC_id %d Frame %d, subframeP %d: Generated ULSCH DCI for CBA group %d, RB 50 format 0\n", module_idP,CC_id,frameP,subframeP,cba_group);
            break;

          case 100:
            mac_xface->macphy_exit("[MAC][eNB] CBA RB=100 not supported \n");
            break;

          default:
            break;
          }

          nCCE[CC_id] = nCCE[CC_id] + (1<<aggregation) ;
          nCCE_available[CC_id] = mac_xface->get_nCCE_max(module_idP,CC_id) - nCCE[CC_id];
          //      break;// for the moment only schedule one
        }
      }
    }
  }
}
#endif
#endif
