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
/*! \file pre_processor.c
 * \brief eNB scheduler preprocessing fuction prior to scheduling
 * \author Navid Nikaein and Ankit Bhamri
 * \date 2013 - 2014
 * \email navid.nikaein@eurecom.fr
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
#include "UTIL/OPT/opt.h"
#include "OCG.h"
#include "OCG_extern.h"
#include "RRC/LITE/extern.h"
#include "RRC/L2_INTERFACE/openair_rrc_L2_interface.h"
#include "rlc.h"



#define DEBUG_eNB_SCHEDULER 1
#define DEBUG_HEADER_PARSING 1
//#define DEBUG_PACKET_TRACE 1

//#define ICIC 0

/*
  #ifndef USER_MODE
  #define msg debug_msg
  #endif
*/

extern void phy_add_count_value(uint16_t rnti, uint16_t usCountId, int value);
extern void phy_add_count(uint16_t rnti, uint16_t usCountId);

// This function stores the downlink buffer for all the logical channels
void store_dlsch_buffer (module_id_t Mod_id,
                         frame_t     frameP,
                         sub_frame_t subframeP)
{

  int                   UE_id,i;
  rnti_t                rnti;
  mac_rlc_status_resp_t rlc_status;
  UE_info_t             *UE_info;
  UE_list_t             *UE_list = &eNB_mac_inst[Mod_id].UE_list[subframeP];
  UE_TEMPLATE           *UE_template;

  for (UE_id=UE_list->head; UE_id>=0; UE_id=UE_list->next[UE_id]) 
  {

    UE_info = &eNB_mac_inst[Mod_id].UE_info[UE_id];
    UE_template = &UE_info->UE_template[UE_PCCID(Mod_id,UE_id)];

    // clear logical channel interface variables
    UE_template->dl_buffer_total[subframeP] = 0;
    UE_template->dl_pdus_total[subframeP] = 0;

    for(i=0; i< MAX_NUM_LCID; i++) {
      UE_template->dl_buffer_info[i]=0;
      UE_template->dl_pdus_in_buffer[i]=0;
      UE_template->dl_buffer_head_sdu_creation_time[i]=0;
      UE_template->dl_buffer_head_sdu_remaining_size_to_send[i]=0;
    }

    rnti = UE_RNTI(Mod_id,UE_id);

    for(i=0; i< MAX_NUM_LCID; i++) 
    { 

      // loop over all the logical channels

      rlc_status = mac_rlc_status_ind(Mod_id,rnti, Mod_id,frameP,ENB_FLAG_YES,MBMS_FLAG_NO,i,0 );
      UE_template->dl_buffer_info[i] = rlc_status.bytes_in_buffer; //storing the dlsch buffer for each logical channel
      UE_template->dl_pdus_in_buffer[i] = rlc_status.pdus_in_buffer;
      UE_template->dl_buffer_head_sdu_creation_time[i] = rlc_status.head_sdu_creation_time ;
      UE_template->dl_buffer_head_sdu_creation_time_max = cmax(UE_template->dl_buffer_head_sdu_creation_time_max,
          rlc_status.head_sdu_creation_time );
      UE_template->dl_buffer_head_sdu_remaining_size_to_send[i] = rlc_status.head_sdu_remaining_size_to_send;
      UE_template->dl_buffer_head_sdu_is_segmented[i] = rlc_status.head_sdu_is_segmented;
      UE_template->dl_buffer_total[subframeP] += UE_template->dl_buffer_info[i];//storing the total dlsch buffer
      UE_template->dl_pdus_total[subframeP]   += UE_template->dl_pdus_in_buffer[i];

#ifdef DEBUG_eNB_SCHEDULER

      /* note for dl_buffer_head_sdu_remaining_size_to_send[i] :
       * 0 if head SDU has not been segmented (yet), else remaining size not already segmented and sent
       */
      if (UE_template->dl_buffer_info[i]>0)
        LOG_D(MAC,
              "[eNB %d] Frame %d Subframe %d : RLC status for UE %d in LCID%d: total of %d pdus and size %d, head sdu queuing time %d, remaining size %d, is segmeneted %d \n",
              Mod_id, frameP, subframeP, UE_id,
              i, UE_template->dl_pdus_in_buffer[i],UE_template->dl_buffer_info[i],
              UE_template->dl_buffer_head_sdu_creation_time[i],
              UE_template->dl_buffer_head_sdu_remaining_size_to_send[i],
              UE_template->dl_buffer_head_sdu_is_segmented[i]
             );

#endif

    }
    //xhd_oai_multuser SDU send success
    //phy_add_count_value(rnti, 15, UE_template->dl_buffer_total>>10);
    //phy_add_count_value(rnti, 16, UE_template->dl_pdus_total);
    //phy_add_count(rnti, 17);
    

    //#ifdef DEBUG_eNB_SCHEDULER
    if ( UE_template->dl_buffer_total[subframeP]>0)
      LOG_D(MAC,"[eNB %d] Frame %d Subframe %d : RLC status for UE %d : total DL buffer size %d and total number of pdu %d \n",
            Mod_id, frameP, subframeP, UE_id,
            UE_template->dl_buffer_total[subframeP],
            UE_template->dl_pdus_total[subframeP]
           );

    //#endif
  }
}

extern int gtpu_total_size;
extern int gtpu_total_count;

extern int cqi_num[NUMBER_OF_UE_MAX][10];
extern int cqi_sum[NUMBER_OF_UE_MAX][10];

// This function returns the estimated number of RBs required by each UE for downlink scheduling
void assign_rbs_required (module_id_t Mod_id,
                          frame_t     frameP,
                          sub_frame_t subframe,
                          uint16_t    nb_rbs_required[MAX_NUM_CCs][NUMBER_OF_UE_MAX],
                          int         min_rb_unit[MAX_NUM_CCs])
{


  rnti_t           rnti;
  uint16_t         TBS = 0;
  LTE_eNB_UE_stats *eNB_UE_stats[MAX_NUM_CCs];
  int              UE_id,n,i,j,CC_id,pCCid,tmp;
  UE_info_t        *UE_info = &eNB_mac_inst[Mod_id].UE_info;
  UE_list_t        *UE_list = &eNB_mac_inst[Mod_id].UE_list[subframe];
  //  UE_TEMPLATE           *UE_template;
  LTE_DL_FRAME_PARMS   *frame_parms[MAX_NUM_CCs];

  // clear rb allocations across all CC_ids
  for (UE_id=UE_list->head; UE_id>=0; UE_id=UE_list->next[UE_id]) 
  {
    UE_info = &eNB_mac_inst[Mod_id].UE_info[UE_id];
    pCCid = UE_PCCID(Mod_id,UE_id);
    rnti = UE_info->UE_template[pCCid].rnti;

    //update CQI information across component carriers
    for (n=0; n<UE_info->numactiveCCs; n++) {

      CC_id = UE_info->ordered_CCids[n];
      frame_parms[CC_id] = mac_xface->get_lte_frame_parms(Mod_id,CC_id);
      eNB_UE_stats[CC_id] = mac_xface->get_eNB_UE_stats(Mod_id,CC_id,rnti);
      if( eNB_UE_stats[CC_id] == NULL )
      {
        break;
      }
      /*
      DevCheck(((eNB_UE_stats[CC_id]->DL_cqi[0] < MIN_CQI_VALUE) || (eNB_UE_stats[CC_id]->DL_cqi[0] > MAX_CQI_VALUE)),
      eNB_UE_stats[CC_id]->DL_cqi[0], MIN_CQI_VALUE, MAX_CQI_VALUE);
      */
      //xhd_oai_rb for core dump
      if( eNB_UE_stats[CC_id]->DL_cqi[0] > MAX_CQI_VALUE )
      {
        printf("assign_rbs_required: FATAL error! DL_cqi %d overflow!\n",eNB_UE_stats[CC_id]->DL_cqi[0]);
        eNB_UE_stats[CC_id]->DL_cqi[0] = MAX_CQI_VALUE;
      }
      eNB_UE_stats[CC_id]->dlsch_mcs1= cqi_to_mcs[eNB_UE_stats[CC_id]->DL_cqi[0]];
      eNB_UE_stats[CC_id]->dlsch_mcs1 = cmin(eNB_UE_stats[CC_id]->dlsch_mcs1,openair_daq_vars.target_ue_dl_mcs);

    }
    if( eNB_UE_stats[0] == NULL )
    {
      continue;
    }

    // provide the list of CCs sorted according to MCS
    for (i=0; i<UE_info->numactiveCCs; i++) {
      for (j=i+1; j<UE_info->numactiveCCs; j++) {
        DevAssert( j < MAX_NUM_CCs );

        if (eNB_UE_stats[UE_info->ordered_CCids[i]]->dlsch_mcs1 >
            eNB_UE_stats[UE_info->ordered_CCids[j]]->dlsch_mcs1) {
          tmp = UE_info->ordered_CCids[i];
          UE_info->ordered_CCids[i] = UE_info->ordered_CCids[j];
          UE_info->ordered_CCids[j] = tmp;
        }
      }
    }

    /*
    if ((mac_get_rrc_status(Mod_id,1,UE_id) < RRC_RECONFIGURED)){  // If we still don't have a default radio bearer
      nb_rbs_required[pCCid][UE_id] = PHY_vars_eNB_g[Mod_id][pCCid]->lte_frame_parms.N_RB_DL;
      continue;
    }
    */
    /* NN --> RK
     * check the index of UE_template"
     */
    //    if (UE_list->UE_template[UE_id]->dl_buffer_total> 0) {

    
    static int count = 0;
    static int fail_count = 0;
    static int dl_buffer_sum = 0;
    static int tbs_sum = 0;
    static int dl_rb_sum = 0;

    //xhd_oai_test
    if( 0 == subframe  )
    {
        UE_info->eNB_UE_stats[CC_id].sdu_bytes_per_frame = (130000); //控制在1Mbps
    }

    if (UE_info->UE_template[pCCid].dl_buffer_total[subframe]> 0
        && UE_info->eNB_UE_stats[CC_id].sdu_bytes_per_frame > 0) {
      LOG_D(MAC,"[preprocessor] assign RB for UE %d\n",UE_id);

      for (i=0; i<UE_info->numactiveCCs; i++) {
        CC_id = UE_info->ordered_CCids[i];
        frame_parms[CC_id] = mac_xface->get_lte_frame_parms(Mod_id,CC_id);
        eNB_UE_stats[CC_id] = mac_xface->get_eNB_UE_stats(Mod_id,CC_id,rnti);

        if (eNB_UE_stats[CC_id]->dlsch_mcs1==0) {

          nb_rbs_required[CC_id][UE_id] = 4;  // don't let the TBS get too small
        } else {
          nb_rbs_required[CC_id][UE_id] = min_rb_unit[CC_id];
        }

        TBS = mac_xface->get_TBS_DL(eNB_UE_stats[CC_id]->dlsch_mcs1,nb_rbs_required[CC_id][UE_id]);

        //xhd_oai_20users
        TBS = 0;
        nb_rbs_required[CC_id][UE_id] = 0;

        LOG_D(MAC,"[preprocessor] start RB assignement for UE %d CC_id %d dl buffer %d (RB unit %d, MCS %d, TBS %d) \n",
              UE_id, CC_id, UE_info->UE_template[pCCid].dl_buffer_total[subframe],
              nb_rbs_required[CC_id][UE_id],eNB_UE_stats[CC_id]->dlsch_mcs1,TBS);

        
        /* calculating required number of RBs for each UE */
        while (TBS < UE_info->UE_template[pCCid].dl_buffer_total[subframe]
            &&  TBS < UE_info->eNB_UE_stats[CC_id].sdu_bytes_per_frame )  {
          nb_rbs_required[CC_id][UE_id] += min_rb_unit[CC_id];

          if (nb_rbs_required[CC_id][UE_id] > frame_parms[CC_id]->N_RB_DL) {
            TBS = mac_xface->get_TBS_DL(eNB_UE_stats[CC_id]->dlsch_mcs1,frame_parms[CC_id]->N_RB_DL);
            nb_rbs_required[CC_id][UE_id] = frame_parms[CC_id]->N_RB_DL;
            break;
          }

          TBS = mac_xface->get_TBS_DL(eNB_UE_stats[CC_id]->dlsch_mcs1,nb_rbs_required[CC_id][UE_id]);
        } // end of while

        //only for test
        //TBS = mac_xface->get_TBS_DL(eNB_UE_stats[CC_id]->dlsch_mcs1,frame_parms[CC_id]->N_RB_DL);
        //nb_rbs_required[CC_id][UE_id] = frame_parms[CC_id]->N_RB_DL;


        tbs_sum += TBS;
        dl_buffer_sum += UE_info->UE_template[pCCid].dl_buffer_total[subframe];
        dl_rb_sum += nb_rbs_required[CC_id][UE_id];
        
      }
    }
    else
    {
        fail_count++;
    }
    if( 0 /*++count == 1000*/)
    {
        LOG_I(MAC,"[eNB %d] Frame %d: UE %d on CC %d: RB unit %d,  nb_required RB %d N_RB_DL:%d(TBS %d total %d, mcs %d)\n",
              Mod_id, frameP,UE_id, CC_id,  min_rb_unit[CC_id], 
              nb_rbs_required[CC_id][UE_id], frame_parms[CC_id]->N_RB_DL,
              TBS, UE_info->UE_template[pCCid].dl_buffer_total[subframe], eNB_UE_stats[CC_id]->dlsch_mcs1);
    
        printf("Average TBS=%d dl_buffer=%d dl_rb=%d GTPU: total:%d count:%d fail_count=%d\n",
            tbs_sum/1000, dl_buffer_sum/1000, dl_rb_sum/1000,
            gtpu_total_size, gtpu_total_count, fail_count);                  
        count = 0;
        dl_buffer_sum = 0;
        tbs_sum = 0;
        dl_rb_sum = 0;
        gtpu_total_size = 0;
        gtpu_total_count = 0;
        fail_count = 0;
    }
    
  }
}


// This function scans all CC_ids for a particular UE to find the maximum round index of its HARQ processes
#if 0
int maxround(module_id_t Mod_id,uint16_t rnti,int frame,sub_frame_t subframe,uint8_t ul_flag )
{

  uint8_t round,round_max=0,UE_id;
  int CC_id;
  UE_info_t *UE_info = &eNB_mac_inst[Mod_id].UE_info;

  for (CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) {

    UE_id = find_UE_id(Mod_id,rnti);
    round    = UE_info->UE_sched_ctrl[UE_id].round[CC_id][subframe];
    if (round > round_max) {
      round_max = round;
    }
  }

  return round_max;
}
#endif

// This function scans all CC_ids for a particular UE to find the maximum DL CQI

int maxcqi(module_id_t Mod_id,int32_t UE_id)
{

  LTE_eNB_UE_stats *eNB_UE_stats = NULL;
  UE_info_t *UE_info = &eNB_mac_inst[Mod_id].UE_info[UE_id];
  int CC_id,n;
  int CQI = 0;

  for (n=0; n<UE_info->numactiveCCs; n++) {
    CC_id = UE_info->ordered_CCids[n];
    eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,CC_id,UE_RNTI(Mod_id,UE_id));

    if (eNB_UE_stats==NULL) {
      //mac_xface->macphy_exit("maxcqi: could not get eNB_UE_stats\n");
      printf("maxcqi: could not get eNB_UE_stats UE_id=%d CC_id=%d\n",UE_id,CC_id);
      return 0; // not reached
    }

    if (eNB_UE_stats->DL_cqi[0] > CQI) {
      CQI = eNB_UE_stats->DL_cqi[0];
    }
  }

  return(CQI);
}

int rrcstatus(module_id_t Mod_id,int32_t UE_id)
{

  LTE_eNB_UE_stats *eNB_UE_stats = NULL;
  UE_info_t *UE_info = &eNB_mac_inst[Mod_id].UE_info[UE_id];
  int CC_id,n;
  int CQI = 0;

  for (n=0; n<UE_info->numactiveCCs; n++) {
    CC_id = UE_info->ordered_CCids[n];
    eNB_UE_stats = mac_xface->get_eNB_UE_stats(Mod_id,CC_id,UE_RNTI(Mod_id,UE_id));

    if (eNB_UE_stats==NULL) {
      //mac_xface->macphy_exit("rrcstatus: could not get eNB_UE_stats\n");
      printf("rrcstatus: could not get eNB_UE_stats  UE_id=%d CC_id=%d\n", UE_id,CC_id);
      return 0; // not reached
    }

    if (eNB_UE_stats->DL_cqi[0] > CQI) {
      CQI = eNB_UE_stats->DL_cqi[0];
    }
  }

  return(CQI);
}

#define OLTE_MAX_USER NUMBER_OF_UE_MAX
extern int tx_count[OLTE_MAX_USER];
extern int rx_count[OLTE_MAX_USER];
extern int phy_delete_flag[OLTE_MAX_USER];

void setup_ue_list(module_id_t module_idP,int frameP, sub_frame_t subframeP)
{

    UE_info_t *UE_info;
    UE_list_t *UE_list = &eNB_mac_inst[module_idP].UE_list[subframeP];
    int CC_id,i;
    rnti_t rnti;
    
    UE_list->head = -1;
    UE_list->head_ul = -1;
    int last_ue = -1;

    for( i = 0; i < NUMBER_OF_UE_MAX; i++)
    {

        UE_info = &eNB_mac_inst[module_idP].UE_info[i];
        if(UE_info->active==FALSE)
        {
            continue;
        }
        rnti = UE_RNTI(module_idP, i);
        CC_id = UE_PCCID(module_idP, i);
        LOG_D(MAC,"UE %d: rnti %x (%p)\n", i, rnti,
              mac_xface->get_eNB_UE_stats(module_idP, CC_id, rnti));
        
        if (mac_xface->get_eNB_UE_stats(module_idP, CC_id, rnti)==NULL || phy_delete_flag[i] != 0) 
        {
            printf("setup_ue_list: remove inactive ue%d rnti=%x module_idP=%d CC_id=%d phy_delete_flag=%d\n",
                i,rnti,module_idP, CC_id, phy_delete_flag[i]);
            mac_remove_ue(module_idP, i, frameP, subframeP);
            continue;
        }

        if(last_ue<0)
        {
            UE_list->head = i;
            UE_list->head_ul = i;
        }
        else
        {
            UE_list->next[last_ue] = i;
            UE_list->next_ul[last_ue] = i;
        }
        last_ue  = i;
        UE_list->next[i] = -1;
        UE_list->next_ul[i] = -1;
    }
}
void sort_ue_ul (module_id_t module_idP,int frameP, sub_frame_t subframeP)
{

  int               UE_id1,UE_id2;
  int               pCCid1,pCCid2;
  int               round1,round2;
  int               i=0,ii=0,j=0;
  rnti_t            rnti1,rnti2;

  uint8_t  ucUeCount = 0;
  
  UE_info_t *UE_info2;
  UE_info_t *UE_infom;
  UE_list_t *UE_list = &eNB_mac_inst[module_idP].UE_list[subframeP];

  
  for( i = UE_list->head_ul; i < NUMBER_OF_UE_MAX  && i >= 0; i=UE_list->next_ul[i])
  {

      UE_sched_ctrl *ue_sched_ctl = &UE_list->UE_sched_ctrl[i];
      rnti_t rnti = UE_RNTI(module_idP,i);
      
      // initialize harq_pid and round
      
      ue_sched_ctl->max_round_ul= 0;
      for (uint8_t CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) 
      {
          mac_xface->get_ue_active_harq_pid(module_idP,CC_id,rnti,
                        frameP,subframeP,
                        &ue_sched_ctl->harq_pid_ul[CC_id],
                        &ue_sched_ctl->round_ul[CC_id],
                        1);
          if(ue_sched_ctl->round_ul[CC_id] > ue_sched_ctl->max_round_ul)
          {
            ue_sched_ctl->max_round_ul= ue_sched_ctl->round_ul[CC_id];
            if(CC_id != 0 )
            {
                printf("sort_ue_ul:strange UE%d CC_id=%d round=%d\n", i, CC_id, ue_sched_ctl->round_ul[CC_id]);
            }
          }
      }

  }
  
  
  int8_t UE_id_max = UE_list->head_ul;
  while(UE_id_max >=0 && UE_list->next_ul[UE_id_max]>=0)
  {
    
    //LOG_I(MAC,"sort ue ul i %d\n",i);
    int8_t UE_id_max_old = UE_id_max;
    for (ii=UE_list->next_ul[UE_id_max]; ii>=0; ii=UE_list->next_ul[ii]) {
      //LOG_I(MAC,"sort ul ue 2 ii %d\n",ii);
 
      //UE_id1  = i;
      //rnti1 = UE_RNTI(module_idP,UE_id1);
      
      //if(rnti1 == NOT_A_RNTI)
      //continue;
      UE_info2 = &eNB_mac_inst[module_idP].UE_info[UE_id2];
      UE_infom = &eNB_mac_inst[module_idP].UE_info[UE_id_max];

      pCCid1 = UE_PCCID(module_idP,UE_id_max);
      round1  = UE_list->UE_sched_ctrl[UE_id_max].max_round_ul; //maxround(module_idP,rnti1,frameP,subframeP,1);
      uint8_t ucRrcStatus1 = rrc_isRrcConnected(module_idP,UE_RNTI(module_idP,UE_id_max),NULL,NULL);
      
      UE_id2  = ii;
      //rnti2 = UE_RNTI(module_idP,UE_id2);
      
      //if(rnti2 == NOT_A_RNTI)
      //  continue;

      pCCid2 = UE_PCCID(module_idP,UE_id2);
      round2  = UE_list->UE_sched_ctrl[UE_id2].max_round_ul; //maxround(module_idP,rnti2,frameP,subframeP,1);
      uint8_t ucRrcStatus2 = rrc_isRrcConnected(module_idP,UE_RNTI(module_idP,UE_id2),NULL,NULL);

      if(round2 > round1) {
           UE_id_max = UE_id2;
      } else if (round2 == round1) {
        if(rx_count[UE_id2] * 5 < rx_count[UE_id1])
        {   
            //上行成功率严重偏少的UE优先
            UE_id_max = UE_id2;
        }
        else if( ucRrcStatus1 > ucRrcStatus2 )
        {
            //连接未建立的UE优先
            UE_id_max = UE_id2;
        }
        else if (UE_infom->UE_template[pCCid1].ul_buffer_info[LCGID0] 
            < UE_info2->UE_template[pCCid2].ul_buffer_info[LCGID0]) 
        {
            UE_id_max = UE_id2;
        } 
        else if (UE_infom->UE_template[pCCid1].ul_total_buffer[subframeP] 
            <  UE_info2->UE_template[pCCid2].ul_total_buffer[subframeP]) 
        {
            UE_id_max = UE_id2;
        } 
        else if (UE_infom->UE_template[pCCid1].pre_assigned_mcs_ul[subframeP] 
            <  UE_info2->UE_template[pCCid2].pre_assigned_mcs_ul[subframeP]) 
        {
          if (UE_info2->UE_template[pCCid2].ul_total_buffer[subframeP] > 0 ) {
              UE_id_max = UE_id2;
          }
        }
      }
    }
    if( UE_id_max != UE_id_max_old)
    {
        swap_UEs(UE_list,UE_id_max_old,UE_id_max,1);
    }
    UE_id_max = UE_list->next_ul[UE_id_max];
  }

  uint8_t last;
  uint8_t curr;
  uint8_t err = 0;
  
  for (i=UE_list->head_ul; i>=0; i=UE_list->next_ul[i]) 
  {
      UE_sched_ctrl *ue_sched_ctl = &UE_list->UE_sched_ctrl[i];
      curr = ue_sched_ctl->max_round_ul;
      if( i != UE_list->head_ul && curr > last)
      {
        err = 1;
        break;
      }
      last = curr;
  }
  if(err)
  {
      printf("sort_ue_ul:ERROR [UE round]");
      for (i=UE_list->head_ul; i>=0; i=UE_list->next_ul[i]) 
      {
         printf("[%d %d]", i, UE_list->UE_sched_ctrl[i].max_round_ul);
      }
      printf("\n");
  }
}
void sort_UEs (module_id_t module_idP,int frameP, sub_frame_t subframeP)
{

  int               UE_id1,UE_id2,last_ue;
  int               pCCid1,pCCid2;
  int               round1,round2;
  int               i=0,ii=0,j=0;
  rnti_t            rnti1,rnti2;
  int               cqi1,cqi2;

  
  UE_info_t *UE_info2 ;
  UE_info_t *UE_infom ;
  UE_list_t *UE_list = &eNB_mac_inst[module_idP].UE_list[subframeP];


  for( i = UE_list->head; i < NUMBER_OF_UE_MAX && i >= 0; i=UE_list->next[i])
  {

      UE_sched_ctrl *ue_sched_ctl = &UE_list->UE_sched_ctrl[i];
      rnti_t rnti = UE_RNTI(module_idP,i);
      
      // initialize harq_pid and round

      ue_sched_ctl->max_round = 0;
      for (uint8_t CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) 
      {
          mac_xface->get_ue_active_harq_pid(module_idP,CC_id,rnti,
                        frameP,subframeP,
                        &ue_sched_ctl->harq_pid[CC_id],
                        &ue_sched_ctl->round[CC_id],
                        0);
          if(ue_sched_ctl->round[CC_id] > ue_sched_ctl->max_round)
          {
            ue_sched_ctl->max_round = ue_sched_ctl->round[CC_id];
            if(CC_id != 0 )
            {
                printf("sort_ue_ul:strange UE%d CC_id=%d round=%d\n", i, CC_id, ue_sched_ctl->round[CC_id]);
            }
          }
      }
  }
  
  uint8_t UE_id_max = UE_list->head;
  while(UE_list->next[UE_id_max]>=0)
  {
    
    //LOG_I(MAC,"sort ue ul i %d\n",i);
    uint8_t UE_id_max_old = UE_id_max;
    for (ii=UE_list->next[UE_id_max]; ii>=0; ii=UE_list->next[ii]) 
    {
      //LOG_I(MAC,"sort ul ue 2 ii %d\n",ii);
 
      //UE_id1  = i;
      //rnti1 = UE_RNTI(module_idP,UE_id1);
      
      //if(rnti1 == NOT_A_RNTI)
      //continue;
      UE_info2 = &eNB_mac_inst[module_idP].UE_info[UE_id2];
      UE_infom = &eNB_mac_inst[module_idP].UE_info[UE_id_max];

      pCCid1 = UE_PCCID(module_idP,UE_id_max);
      round1  = UE_list->UE_sched_ctrl[UE_id_max].max_round; //maxround(module_idP,rnti1,frameP,subframeP,1);
      cqi1    = maxcqi(module_idP,UE_id_max); //
      uint8_t ucRrcStatus1 = rrc_isRrcConnected(module_idP,UE_RNTI(module_idP,UE_id_max),NULL,NULL);
      
      UE_id2  = ii;
      //rnti2 = UE_RNTI(module_idP,UE_id2);
      
      //if(rnti2 == NOT_A_RNTI)
      //  continue;

      pCCid2 = UE_PCCID(module_idP,UE_id2);
      round2  = UE_list->UE_sched_ctrl[UE_id2].max_round; //maxround(module_idP,rnti2,frameP,subframeP,1);
      cqi2    = maxcqi(module_idP,UE_id2);
      uint8_t ucRrcStatus2 = rrc_isRrcConnected(module_idP,UE_RNTI(module_idP,UE_id2),NULL,NULL);

      if(round2 > round1) {
           UE_id_max = UE_id2;
      } else if (round2 == round1) {
        if(tx_count[UE_id2] * 20 < tx_count[UE_id1])
        {   
            //下行成功率严重偏少的UE优先
            UE_id_max = UE_id2;
        }
        else if( ucRrcStatus1 > ucRrcStatus2 )
        {
            //连接未建立的UE优先
            UE_id_max = UE_id2;
        }
        else if ( (UE_infom->UE_template[pCCid1].dl_buffer_info[1] + UE_infom->UE_template[pCCid1].dl_buffer_info[2]) <
             (UE_info2->UE_template[pCCid2].dl_buffer_info[1] + UE_info2->UE_template[pCCid2].dl_buffer_info[2])   ) 
        {
            UE_id_max = UE_id2;
        } else if (UE_infom->UE_template[pCCid1].dl_buffer_head_sdu_creation_time_max <
                   UE_info2->UE_template[pCCid2].dl_buffer_head_sdu_creation_time_max   ) 
        {
            UE_id_max = UE_id2;
        } 
        else if (cqi1 < cqi2) 
        {
            UE_id_max = UE_id2;
        }
      }
    }
    if( UE_id_max != UE_id_max_old)
    {
        swap_UEs(UE_list,UE_id_max_old,UE_id_max,0);
    }
    UE_id_max = UE_list->next[UE_id_max];
  }

  uint8_t last;
  uint8_t curr;
  uint8_t err = 0;
  
  for (i=UE_list->head; i>=0; i=UE_list->next[i]) 
  {
      UE_sched_ctrl *ue_sched_ctl = &UE_list->UE_sched_ctrl[i];
      curr = ue_sched_ctl->max_round;
      if( i != UE_list->head && curr > last)
      {
        err = 1;
        break;
      }
      last = curr;
  }
  if(err)
  {
      printf("sort_ue:ERROR [UE round]");
      for (i=UE_list->head; i>=0; i=UE_list->next[i]) 
      {
         printf("[%d %d]", i, UE_list->UE_sched_ctrl[i].max_round);
      }
      printf("\n");
  }
}

// This fuction sorts the UE in order their dlsch buffer and CQI
#if 0
void sort_UEs_old (module_id_t Mod_idP,
               int         frameP,
               sub_frame_t subframeP)
{


  int               UE_id1,UE_id2;
  int               pCC_id1,pCC_id2;
  int               cqi1,cqi2,round1,round2;
  int               i=0,ii=0;//,j=0;
  rnti_t            rnti1,rnti2;

  UE_list_t *UE_list = &eNB_mac_inst[Mod_idP].UE_list;

  for (i=UE_list->head; i>=0; i=UE_list->next[i]) {

    for(ii=UE_list->next[i]; ii>=0; ii=UE_list->next[ii]) {

      UE_id1  = i;
      rnti1 = UE_RNTI(Mod_idP,UE_id1);
      if(rnti1 == NOT_A_RNTI)
        	continue;
      pCC_id1 = UE_PCCID(Mod_idP,UE_id1);
      cqi1    = maxcqi(Mod_idP,UE_id1); //
      round1  = maxround(Mod_idP,rnti1,frameP,subframeP,0);

      UE_id2 = ii;
      rnti2 = UE_RNTI(Mod_idP,UE_id2);
      if(rnti2 == NOT_A_RNTI)
            continue;
      cqi2    = maxcqi(Mod_idP,UE_id2);
      round2  = maxround(Mod_idP,rnti2,frameP,subframeP,0);  //mac_xface->get_ue_active_harq_pid(Mod_id,rnti2,subframe,&harq_pid2,&round2,0);
      pCC_id2 = UE_PCCID(Mod_idP,UE_id2);

      if(round2 > round1) { // Check first if one of the UEs has an active HARQ process which needs service and swap order
        swap_UEs(UE_list,UE_id1,UE_id2,0);
      } else if (round2 == round1) {
        // RK->NN : I guess this is for fairness in the scheduling. This doesn't make sense unless all UEs have the same configuration of logical channels.  This should be done on the sum of all information that has to be sent.  And still it wouldn't ensure fairness.  It should be based on throughput seen by each UE or maybe using the head_sdu_creation_time, i.e. swap UEs if one is waiting longer for service.
        //  for(j=0;j<MAX_NUM_LCID;j++){
        //    if (eNB_mac_inst[Mod_id][pCC_id1].UE_template[UE_id1].dl_buffer_info[j] <
        //      eNB_mac_inst[Mod_id][pCC_id2].UE_template[UE_id2].dl_buffer_info[j]){

        // first check the buffer status for SRB1 and SRB2

        if ( (UE_list->UE_template[pCC_id1][UE_id1].dl_buffer_info[1] + UE_list->UE_template[pCC_id1][UE_id1].dl_buffer_info[2]) <
             (UE_list->UE_template[pCC_id2][UE_id2].dl_buffer_info[1] + UE_list->UE_template[pCC_id2][UE_id2].dl_buffer_info[2])   ) {
          swap_UEs(UE_list,UE_id1,UE_id2,0);
        } else if (UE_list->UE_template[pCC_id1][UE_id1].dl_buffer_head_sdu_creation_time_max <
                   UE_list->UE_template[pCC_id2][UE_id2].dl_buffer_head_sdu_creation_time_max   ) {
          swap_UEs(UE_list,UE_id1,UE_id2,0);
        } else if (UE_list->UE_template[pCC_id1][UE_id1].dl_buffer_total <
                   UE_list->UE_template[pCC_id2][UE_id2].dl_buffer_total   ) {
          swap_UEs(UE_list,UE_id1,UE_id2,0);
        } else if (cqi1 < cqi2) {
          swap_UEs(UE_list,UE_id1,UE_id2,0);
        }
      }
    }
  }
}
#endif




#if 0
// This function assigns pre-available RBS to each UE in specified sub-bands before scheduling is done
void dlsch_scheduler_pre_processor_ver(module_id_t   Mod_id,
                                    frame_t       frameP,
                                    sub_frame_t   subframeP,
                                    int           N_RBG[MAX_NUM_CCs],
                                    int           *mbsfn_flag)
{

  unsigned char rballoc_sub[MAX_NUM_CCs][N_RBG_MAX],harq_pid=0,round=0,total_ue_count;
  unsigned char MIMO_mode_indicator[MAX_NUM_CCs][N_RBG_MAX];
  int                     UE_id, i; 
  uint16_t                ii,j;
  uint16_t                nb_rbs_required[MAX_NUM_CCs][NUMBER_OF_UE_MAX];
  uint16_t                nb_rbs_required_remaining[MAX_NUM_CCs][NUMBER_OF_UE_MAX];
  uint16_t                nb_rbs_required_remaining_1[MAX_NUM_CCs][NUMBER_OF_UE_MAX];
  uint16_t                average_rbs_per_user[MAX_NUM_CCs] = {0};
  rnti_t             rnti;
  int                min_rb_unit[MAX_NUM_CCs];
  uint16_t r1=0;
  uint8_t CC_id;
  UE_list_t *UE_list = &eNB_mac_inst[Mod_id].UE_list;
  LTE_DL_FRAME_PARMS   *frame_parms[MAX_NUM_CCs] = {0};

  int transmission_mode = 0;
  UE_sched_ctrl *ue_sched_ctl;
  //  int rrc_status           = RRC_IDLE;

#ifdef TM5
  int harq_pid1=0,harq_pid2=0;
  int round1=0,round2=0;
  int UE_id2;
  uint16_t                i1,i2,i3;
  rnti_t             rnti1,rnti2;
  LTE_eNB_UE_stats  *eNB_UE_stats1 = NULL;
  LTE_eNB_UE_stats  *eNB_UE_stats2 = NULL;
  UE_sched_ctrl *ue_sched_ctl1,*ue_sched_ctl2;
#endif

  for (CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) {

    if (mbsfn_flag[CC_id]>0)  // If this CC is allocated for MBSFN skip it here
      continue;

    frame_parms[CC_id] = mac_xface->get_lte_frame_parms(Mod_id,CC_id);


    min_rb_unit[CC_id]=get_min_rb_unit(Mod_id,CC_id);

    for (i=UE_list->head; i>=0; i=UE_list->next[i]) {
      UE_id = i;
      // Initialize scheduling information for all active UEs
      


      dlsch_scheduler_pre_processor_reset(Mod_id,
        UE_id,
        CC_id,
        frameP,
        subframeP,
        N_RBG[CC_id],
        nb_rbs_required,
        nb_rbs_required_remaining,
        rballoc_sub,
        MIMO_mode_indicator);

    }
  }


  // Store the DLSCH buffer for each logical channel
  store_dlsch_buffer (Mod_id,frameP,subframeP);



  // Calculate the number of RBs required by each UE on the basis of logical channel's buffer
  assign_rbs_required (Mod_id,frameP,subframeP,nb_rbs_required,min_rb_unit);



  // Sorts the user on the basis of dlsch logical channel buffer and CQI
  sort_UEs (Mod_id,frameP,subframeP);



  total_ue_count =0;

  // loop over all active UEs
  for (i=UE_list->head; i>=0; i=UE_list->next[i]) {
    rnti = UE_RNTI(Mod_id,i);

    if(rnti == NOT_A_RNTI)
      continue;

    UE_id = i;

    // if there is no available harq_process, skip the UE
    if (UE_list->UE_sched_ctrl[UE_id].harq_pid[CC_id][subframeP]<0)
      continue;

    for (ii=0; ii<UE_num_active_CC(UE_list,UE_id); ii++) {
      CC_id = UE_list->ordered_CCids[ii][UE_id];
      ue_sched_ctl = &UE_list->UE_sched_ctrl[UE_id];
      harq_pid = ue_sched_ctl->harq_pid[CC_id][subframeP];
      round    = ue_sched_ctl->round[CC_id][subframeP];

      average_rbs_per_user[CC_id]=0;

      frame_parms[CC_id] = mac_xface->get_lte_frame_parms(Mod_id,CC_id);

      //      mac_xface->get_ue_active_harq_pid(Mod_id,CC_id,rnti,frameP,subframeP,&harq_pid,&round,0);

      if(round>0) {
        nb_rbs_required[CC_id][UE_id] = UE_list->UE_template[CC_id][UE_id].nb_rb[harq_pid];
      }

      //nb_rbs_required_remaining[UE_id] = nb_rbs_required[UE_id];
      if (nb_rbs_required[CC_id][UE_id] > 0) {
        total_ue_count = total_ue_count + 1;
      }


      // hypotetical assignement
      /*
       * If schedule is enabled and if the priority of the UEs is modified
       * The average rbs per logical channel per user will depend on the level of
       * priority. Concerning the hypothetical assignement, we should assign more
       * rbs to prioritized users. Maybe, we can do a mapping between the
       * average rbs per user and the level of priority or multiply the average rbs
       * per user by a coefficient which represents the degree of priority.
       */

      if (total_ue_count == 0) {
        average_rbs_per_user[CC_id] = 0;
      } else if( (min_rb_unit[CC_id] * total_ue_count) <= (frame_parms[CC_id]->N_RB_DL) ) {
        average_rbs_per_user[CC_id] = (uint16_t) floor(frame_parms[CC_id]->N_RB_DL/total_ue_count);
      } else {
        average_rbs_per_user[CC_id] = min_rb_unit[CC_id]; // consider the total number of use that can be scheduled UE
      }
    }
  }

  // note: nb_rbs_required is assigned according to total_buffer_dl
  // extend nb_rbs_required to capture per LCID RB required
  for(i=UE_list->head; i>=0; i=UE_list->next[i]) {
    rnti = UE_RNTI(Mod_id,i);

    for (ii=0; ii<UE_num_active_CC(UE_list,i); ii++) {
      CC_id = UE_list->ordered_CCids[ii][i];

      // control channel
      if (mac_eNB_get_rrc_status(Mod_id,rnti) < RRC_RECONFIGURED) {
        nb_rbs_required_remaining_1[CC_id][i] = nb_rbs_required[CC_id][i];
      } else {
        nb_rbs_required_remaining_1[CC_id][i] = cmin(average_rbs_per_user[CC_id],nb_rbs_required[CC_id][i]);

      }
    }
  }

  //Allocation to UEs is done in 2 rounds,
  // 1st stage: average number of RBs allocated to each UE
  // 2nd stage: remaining RBs are allocated to high priority UEs
  for(r1=0; r1<2; r1++) {

    for(i=UE_list->head; i>=0; i=UE_list->next[i]) {
      for (ii=0; ii<UE_num_active_CC(UE_list,i); ii++) {
        CC_id = UE_list->ordered_CCids[ii][i];

        if(r1 == 0) {
          nb_rbs_required_remaining[CC_id][i] = nb_rbs_required_remaining_1[CC_id][i];
        } else { // rb required based only on the buffer - rb allloctaed in the 1st round + extra reaming rb form the 1st round
          nb_rbs_required_remaining[CC_id][i] = nb_rbs_required[CC_id][i]-nb_rbs_required_remaining_1[CC_id][i]+nb_rbs_required_remaining[CC_id][i];
        }

        if (nb_rbs_required[CC_id][i]> 0 )
          LOG_D(MAC,"round %d : nb_rbs_required_remaining[%d][%d]= %d (remaining_1 %d, required %d,  pre_nb_available_rbs %d, N_RBG %d, rb_unit %d)\n",
                r1, CC_id, i,
                nb_rbs_required_remaining[CC_id][i],
                nb_rbs_required_remaining_1[CC_id][i],
                nb_rbs_required[CC_id][i],
                UE_list->UE_sched_ctrl[i].pre_nb_available_rbs[CC_id][subframeP],
                N_RBG[CC_id],
                min_rb_unit[CC_id]);

      }
    }

    if (total_ue_count > 0 ) {
      for(i=UE_list->head; i>=0; i=UE_list->next[i]) {
        UE_id = i;

        for (ii=0; ii<UE_num_active_CC(UE_list,UE_id); ii++) {
          CC_id = UE_list->ordered_CCids[ii][UE_id];
	  ue_sched_ctl = &UE_list->UE_sched_ctrl[UE_id];
	  harq_pid = ue_sched_ctl->harq_pid[CC_id][subframeP];
	  round    = ue_sched_ctl->round[CC_id][subframeP];

          rnti = UE_RNTI(Mod_id,UE_id);

          // LOG_D(MAC,"UE %d rnti 0x\n", UE_id, rnti );
          if(rnti == NOT_A_RNTI)
            continue;

          transmission_mode = mac_xface->get_transmission_mode(Mod_id,CC_id,rnti);
	  //          mac_xface->get_ue_active_harq_pid(Mod_id,CC_id,rnti,frameP,subframeP,&harq_pid,&round,0);
          //rrc_status = mac_eNB_get_rrc_status(Mod_id,rnti);
          /* 1st allocate for the retx */

          // retransmission in data channels
          // control channel in the 1st transmission
          // data channel for all TM
          LOG_T(MAC,"calling dlsch_scheduler_pre_processor_allocate .. \n ");
          dlsch_scheduler_pre_processor_allocate (Mod_id,
                                                  UE_id,
                                                  CC_id,
                                                  N_RBG[CC_id],
                                                  transmission_mode,
                                                  min_rb_unit[CC_id],
                                                  frame_parms[CC_id]->N_RB_DL,
                                                  nb_rbs_required,
                                                  nb_rbs_required_remaining,
                                                  rballoc_sub,
                                                  MIMO_mode_indicator,
                                                  subframeP);

#ifdef TM5

          // data chanel TM5: to be revisted
          if ((round == 0 )  &&
              (transmission_mode == 5)  &&
              (ue_sched_ctl->dl_pow_off[CC_id][subframeP] != 1)) {

            for(j=0; j<N_RBG[CC_id]; j+=2) {

              if( (((j == (N_RBG[CC_id]-1))&& (rballoc_sub[CC_id][j] == 0) && (ue_sched_ctl->rballoc_sub_UE[CC_id][j][subframeP] == 0))  ||
                   ((j < (N_RBG[CC_id]-1)) && (rballoc_sub[CC_id][j+1] == 0) && (ue_sched_ctl->rballoc_sub_UE[CC_id][j+1][subframeP] == 0)) ) &&
                  (nb_rbs_required_remaining[CC_id][UE_id]>0)) {

                for (ii = UE_list->next[i+1]; ii >=0; ii=UE_list->next[ii]) {

                  UE_id2 = ii;
                  rnti2 = UE_RNTI(Mod_id,UE_id2);
        		  ue_sched_ctl2 = &UE_list->UE_sched_ctrl[UE_id2];
        		  harq_pid2 = ue_sched_ctl2->harq_pid[CC_id][subframeP];
        		  round2    = ue_sched_ctl2->round[CC_id][subframeP];
                  if(rnti2 == NOT_A_RNTI)
                    continue;

                  eNB_UE_stats2 = mac_xface->get_eNB_UE_stats(Mod_id,CC_id,rnti2);
                  //mac_xface->get_ue_active_harq_pid(Mod_id,CC_id,rnti2,frameP,subframeP,&harq_pid2,&round2,0);

                  if ((mac_eNB_get_rrc_status(Mod_id,rnti2) >= RRC_RECONFIGURED) &&
                      (round2==0) &&
                      (mac_xface->get_transmission_mode(Mod_id,CC_id,rnti2)==5) &&
                      (ue_sched_ctl->dl_pow_off[CC_id][subframeP] != 1)) {

                    if( (((j == (N_RBG[CC_id]-1)) && (ue_sched_ctl->rballoc_sub_UE[CC_id][j][subframeP] == 0)) ||
                         ((j < (N_RBG[CC_id]-1)) && (ue_sched_ctl->rballoc_sub_UE[CC_id][j+1][subframeP] == 0))  ) &&
                        (nb_rbs_required_remaining[CC_id][UE_id2]>0)) {

                      if((((eNB_UE_stats2->DL_pmi_single^eNB_UE_stats1->DL_pmi_single)<<(14-j))&0xc000)== 0x4000) { //MU-MIMO only for 25 RBs configuration

                        rballoc_sub[CC_id][j] = 1;
                        ue_sched_ctl->rballoc_sub_UE[CC_id][j][subframeP] = 1;
                        ue_sched_ctl2->rballoc_sub_UE[CC_id][j][subframeP] = 1;
                        MIMO_mode_indicator[CC_id][j] = 0;

                        if (j< N_RBG[CC_id]-1) {
                          rballoc_sub[CC_id][j+1] = 1;
                          ue_sched_ctl->rballoc_sub_UE[CC_id][j+1][subframeP] = 1;
                          ue_sched_ctl2->rballoc_sub_UE[CC_id][j+1][subframeP] = 1;
                          MIMO_mode_indicator[CC_id][j+1] = 0;
                        }

                        ue_sched_ctl->dl_pow_off[CC_id][subframeP] = 0;
                        ue_sched_ctl2->dl_pow_off[CC_id][subframeP] = 0;


                        if ((j == N_RBG[CC_id]-1) &&
                            ((PHY_vars_eNB_g[Mod_id][CC_id]->lte_frame_parms.N_RB_DL == 25) ||
                             (PHY_vars_eNB_g[Mod_id][CC_id]->lte_frame_parms.N_RB_DL == 50))) {
			  
                          nb_rbs_required_remaining[CC_id][UE_id] = nb_rbs_required_remaining[CC_id][UE_id] - min_rb_unit[CC_id]+1;
                          ue_sched_ctl->pre_nb_available_rbs[CC_id][subframeP] 
                            = ue_sched_ctl->pre_nb_available_rbs[CC_id][subframeP] + min_rb_unit[CC_id]-1;
                          nb_rbs_required_remaining[CC_id][UE_id2] = nb_rbs_required_remaining[CC_id][UE_id2] - min_rb_unit[CC_id]+1;
                          ue_sched_ctl2->pre_nb_available_rbs[CC_id][subframeP] 
                            = ue_sched_ctl2->pre_nb_available_rbs[CC_id][subframeP] + min_rb_unit[CC_id]-1;
                        } else {
                          
			              nb_rbs_required_remaining[CC_id][UE_id] = nb_rbs_required_remaining[CC_id][UE_id] - 4;
                          ue_sched_ctl->pre_nb_available_rbs[CC_id][subframeP] 
                            = ue_sched_ctl->pre_nb_available_rbs[CC_id][subframeP] + 4;
                          nb_rbs_required_remaining[CC_id][UE_id2] = nb_rbs_required_remaining[CC_id][UE_id2] - 4;
                          ue_sched_ctl2->pre_nb_available_rbs[CC_id][subframeP] 
                            = ue_sched_ctl2->pre_nb_available_rbs[CC_id][subframeP] + 4;
                        }

                        break;
                      }
                    }
                  }
                }
              }
            }
          }

#endif
        }
      }
    } // total_ue_count
  } // end of for for r1 and r2

#ifdef TM5

  // This has to be revisited!!!!
  for (CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) {
    i1=0;
    i2=0;
    i3=0;

    for (j=0; j<N_RBG[CC_id]; j++) {
      if(MIMO_mode_indicator[CC_id][j] == 2) {
        i1 = i1+1;
      } else if(MIMO_mode_indicator[CC_id][j] == 1) {
        i2 = i2+1;
      } else if(MIMO_mode_indicator[CC_id][j] == 0) {
        i3 = i3+1;
      }
    }

    if((i1 < N_RBG[CC_id]) && (i2>0) && (i3==0)) {
      PHY_vars_eNB_g[Mod_id][CC_id]->check_for_SUMIMO_transmissions = PHY_vars_eNB_g[Mod_id][CC_id]->check_for_SUMIMO_transmissions + 1;
    }

    if(i3 == N_RBG[CC_id] && i1==0 && i2==0) {
      PHY_vars_eNB_g[Mod_id][CC_id]->FULL_MUMIMO_transmissions = PHY_vars_eNB_g[Mod_id][CC_id]->FULL_MUMIMO_transmissions + 1;
    }

    if((i1 < N_RBG[CC_id]) && (i3 > 0)) {
      PHY_vars_eNB_g[Mod_id][CC_id]->check_for_MUMIMO_transmissions = PHY_vars_eNB_g[Mod_id][CC_id]->check_for_MUMIMO_transmissions + 1;
    }

    PHY_vars_eNB_g[Mod_id][CC_id]->check_for_total_transmissions = PHY_vars_eNB_g[Mod_id][CC_id]->check_for_total_transmissions + 1;

  }

#endif

  for(i=UE_list->head; i>=0; i=UE_list->next[i]) {
    UE_id = i;
    ue_sched_ctl = &UE_list->UE_sched_ctrl[UE_id];

    for (ii=0; ii<UE_num_active_CC(UE_list,UE_id); ii++) {
      CC_id = UE_list->ordered_CCids[ii][UE_id];
      //PHY_vars_eNB_g[Mod_id]->mu_mimo_mode[UE_id].dl_pow_off = dl_pow_off[UE_id];

      if (ue_sched_ctl->pre_nb_available_rbs[CC_id][subframeP] > 0 ) {
        LOG_D(MAC,"******************DL Scheduling Information for UE%d ************************\n",UE_id);
        LOG_D(MAC,"dl power offset UE%d = %d \n",UE_id,ue_sched_ctl->dl_pow_off[CC_id][subframeP]);
        LOG_D(MAC,"***********RB Alloc for every subband for UE%d ***********\n",UE_id);

        for(j=0; j<N_RBG[CC_id]; j++) {
          //PHY_vars_eNB_g[Mod_id]->mu_mimo_mode[UE_id].rballoc_sub[i] = rballoc_sub_UE[CC_id][UE_id][i];
          LOG_D(MAC,"RB Alloc for UE%d and Subband%d = %d\n",UE_id,j,ue_sched_ctl->rballoc_sub_UE[CC_id][j][subframeP]);
        }

        //PHY_vars_eNB_g[Mod_id]->mu_mimo_mode[UE_id].pre_nb_available_rbs = pre_nb_available_rbs[CC_id][UE_id];
        LOG_D(MAC,"Total RBs allocated for UE%d = %d\n",UE_id,ue_sched_ctl->pre_nb_available_rbs[CC_id][subframeP]);
      }
    }
  }
}
#endif

// This function assigns pre-available RBS to each UE in specified sub-bands before scheduling is done
void dlsch_scheduler_pre_processor (module_id_t   Mod_id,
                                    frame_t       frameP,
                                    sub_frame_t   subframeP,
                                    int           N_RBG[MAX_NUM_CCs],
                                    int           *mbsfn_flag)
{

  unsigned char rballoc_sub[MAX_NUM_CCs][N_RBG_MAX],harq_pid=0,round=0,total_ue_count;
  unsigned char MIMO_mode_indicator[MAX_NUM_CCs][N_RBG_MAX];
  int                     UE_id, i; 
  uint16_t                ii,j;
  uint16_t                nb_rbs_required[MAX_NUM_CCs][NUMBER_OF_UE_MAX];
  uint16_t                nb_rbs_required_remaining[MAX_NUM_CCs][NUMBER_OF_UE_MAX];
  uint16_t                nb_rbs_required_remaining_1[MAX_NUM_CCs][NUMBER_OF_UE_MAX];
  uint16_t                average_rbs_per_user[MAX_NUM_CCs] = {0};
  rnti_t             rnti;
  int                min_rb_unit[MAX_NUM_CCs];
  uint16_t r1=0;
  uint8_t CC_id;

  UE_info_t *UE_info ;
  UE_list_t *UE_list = &eNB_mac_inst[Mod_id].UE_list[subframeP];
  LTE_DL_FRAME_PARMS   *frame_parms[MAX_NUM_CCs] = {0};

  int transmission_mode = 0;
  UE_sched_ctrl *ue_sched_ctl;
  //  int rrc_status           = RRC_IDLE;

  //xhd_oai_50
  static int fail_count = 0;

  static int round_great_count = 0;
  static int round_less_count = 0;

  int old_rb[NUMBER_OF_UE_MAX]={0,0,0,0};
  


  for (CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) 
  {

    if (mbsfn_flag[CC_id]>0)  // If this CC is allocated for MBSFN skip it here
      continue;

    frame_parms[CC_id] = mac_xface->get_lte_frame_parms(Mod_id,CC_id);


    min_rb_unit[CC_id]=get_min_rb_unit(Mod_id,CC_id);

    for (i=UE_list->head; i>=0; i=UE_list->next[i]) 
    {
      UE_id = i;
      // Initialize scheduling information for all active UEs
      

      dlsch_scheduler_pre_processor_reset(Mod_id,
        UE_id,
        CC_id,
        frameP,
        subframeP,
        N_RBG[CC_id],
        nb_rbs_required,
        nb_rbs_required_remaining,
        rballoc_sub,
        MIMO_mode_indicator);

    }
  }


  // Store the DLSCH buffer for each logical channel
  store_dlsch_buffer (Mod_id,frameP,subframeP);



  // Calculate the number of RBs required by each UE on the basis of logical channel's buffer
  assign_rbs_required (Mod_id,frameP,subframeP,nb_rbs_required,min_rb_unit);

  old_rb[0]=nb_rbs_required[0][0];
  old_rb[1]=nb_rbs_required[0][1];
  old_rb[2]=nb_rbs_required[0][2];
  old_rb[3]=nb_rbs_required[0][3];

  // Sorts the user on the basis of dlsch logical channel buffer and CQI
  sort_UEs (Mod_id,frameP,subframeP);



  total_ue_count =0;

  // loop over all active UEs
  for (i=UE_list->head; i>=0; i=UE_list->next[i]) {
    rnti = UE_RNTI(Mod_id,i);

    if(rnti == NOT_A_RNTI)
      continue;

    UE_id = i;
    UE_info = &eNB_mac_inst[Mod_id].UE_info[UE_id];
    // if there is no available harq_process, skip the UE

    ue_sched_ctl = &UE_list->UE_sched_ctrl[UE_id];
    

    for (ii=0; ii<UE_num_active_CC(UE_info); ii++) 
    {
      CC_id = UE_info->ordered_CCids[ii];

      if (ue_sched_ctl->harq_pid[CC_id]<0)
      {
        continue;
      }

      harq_pid = ue_sched_ctl->harq_pid[CC_id];
      round    = ue_sched_ctl->round[CC_id];

      average_rbs_per_user[CC_id]=0;

      frame_parms[CC_id] = mac_xface->get_lte_frame_parms(Mod_id,CC_id);

      //      mac_xface->get_ue_active_harq_pid(Mod_id,CC_id,rnti,frameP,subframeP,&harq_pid,&round,0);

      
      if(round>0) 
      {
        nb_rbs_required[CC_id][UE_id] = UE_info->UE_template[CC_id].nb_rb[harq_pid];
        round_great_count++;
      }
      else
      {
        round_less_count++;
      }

      //nb_rbs_required_remaining[UE_id] = nb_rbs_required[UE_id];
      if (nb_rbs_required[CC_id][UE_id] > 0) {
        total_ue_count = total_ue_count + 1;
      }


      // hypotetical assignement
      /*
       * If schedule is enabled and if the priority of the UEs is modified
       * The average rbs per logical channel per user will depend on the level of
       * priority. Concerning the hypothetical assignement, we should assign more
       * rbs to prioritized users. Maybe, we can do a mapping between the
       * average rbs per user and the level of priority or multiply the average rbs
       * per user by a coefficient which represents the degree of priority.
       */

      if (total_ue_count == 0) {
        average_rbs_per_user[CC_id] = 0;
      } else if( (min_rb_unit[CC_id] * total_ue_count) <= (frame_parms[CC_id]->N_RB_DL) ) {
        average_rbs_per_user[CC_id] = (uint16_t) floor(frame_parms[CC_id]->N_RB_DL/total_ue_count);
        //xhd_oai_multuser
        //if( average_rbs_per_user[CC_id] > 10 )
        {
            //average_rbs_per_user[CC_id] -= 5;
        }
      } else {
        average_rbs_per_user[CC_id] = min_rb_unit[CC_id]; // consider the total number of use that can be scheduled UE
      }
    }
  }

  int nb_rbs_required_remaining_1_ttt = 0;
  int remaining_ttt[2];
  
  // note: nb_rbs_required is assigned according to total_buffer_dl
  // extend nb_rbs_required to capture per LCID RB required
  for(i=UE_list->head; i>=0; i=UE_list->next[i]) {
    rnti = UE_RNTI(Mod_id,i);
    
    UE_info = &eNB_mac_inst[Mod_id].UE_info[i];

    for (ii=0; ii<UE_num_active_CC(UE_info); ii++) {
      CC_id = UE_info->ordered_CCids[ii];

      //xhd_oai_multuser: this is a bug!
      //ue_sched_ctl = &UE_list->UE_sched_ctrl[UE_id];
      ue_sched_ctl = &UE_list->UE_sched_ctrl[i];
      harq_pid = ue_sched_ctl->harq_pid[CC_id];
      round    = ue_sched_ctl->round[CC_id];

      // control channel
      if (mac_eNB_get_rrc_status(Mod_id,rnti) < RRC_RECONFIGURED
        || round > 0) 
      {
        //优先保证业务建立过程中的RB需要
        nb_rbs_required_remaining_1[CC_id][i] = nb_rbs_required[CC_id][i];
        
      } 
      else 
      {
        nb_rbs_required_remaining_1[CC_id][i] = cmin(average_rbs_per_user[CC_id],nb_rbs_required[CC_id][i]);

      }
      //xhd_oai_multuser only for test
      ue_sched_ctl->remain_1[CC_id] = nb_rbs_required_remaining_1[CC_id][i];
      ue_sched_ctl->nb_rbs_required[CC_id] = nb_rbs_required[CC_id][i];
      
      nb_rbs_required_remaining_1_ttt = nb_rbs_required_remaining_1[CC_id][i];
    }
  }

  //Allocation to UEs is done in 2 rounds,
  // 1st stage: average number of RBs allocated to each UE
  // 2nd stage: remaining RBs are allocated to high priority UEs
  for(r1=0; r1<2; r1++) 
  {

    for(i=UE_list->head; i>=0; i=UE_list->next[i]) 
    {


      UE_info = &eNB_mac_inst[Mod_id].UE_info[i];
      for (ii=0; ii<UE_num_active_CC(UE_info); ii++) {
        CC_id = UE_info->ordered_CCids[ii];

        if(r1 == 0) 
        {
            //第一轮，不大于平均值
            nb_rbs_required_remaining[CC_id][i] = nb_rbs_required_remaining_1[CC_id][i];
        } 
        else 
        { 
            //第二轮，加上剩余值
            // rb required based only on the buffer - rb allloctaed in the 1st round + extra reaming rb form the 1st round
            nb_rbs_required_remaining[CC_id][i] 
               = nb_rbs_required[CC_id][i]
                 -nb_rbs_required_remaining_1[CC_id][i]
                 +nb_rbs_required_remaining[CC_id][i]; //可能上一次仍然有剩余
        }
        remaining_ttt[r1]=nb_rbs_required_remaining[CC_id][i];
        
        if (nb_rbs_required[CC_id][i]> 0 )
          LOG_D(MAC,"round %d : nb_rbs_required_remaining[%d][%d]= %d (remaining_1 %d, required %d,  pre_nb_available_rbs %d, N_RBG %d, rb_unit %d)\n",
                r1, CC_id, i,
                nb_rbs_required_remaining[CC_id][i],
                nb_rbs_required_remaining_1[CC_id][i],
                nb_rbs_required[CC_id][i],
                UE_list->UE_sched_ctrl[i].pre_nb_available_rbs[CC_id],
                N_RBG[CC_id],
                min_rb_unit[CC_id]);

      }
    }

    if (total_ue_count > 0 ) 
    {
      for(i=UE_list->head; i>=0; i=UE_list->next[i]) 
      {
        UE_id = i;
        UE_info = &eNB_mac_inst[Mod_id].UE_info[i];

        for (ii=0; ii<UE_num_active_CC(UE_info); ii++) 
        {
            CC_id = UE_info->ordered_CCids[ii];
            ue_sched_ctl = &UE_list->UE_sched_ctrl[UE_id];
            harq_pid = ue_sched_ctl->harq_pid[CC_id];
            round    = ue_sched_ctl->round[CC_id];

            rnti = UE_RNTI(Mod_id,UE_id);

            // LOG_D(MAC,"UE %d rnti 0x\n", UE_id, rnti );
            if(rnti == NOT_A_RNTI)
            continue;

            transmission_mode = mac_xface->get_transmission_mode(Mod_id,CC_id,rnti);
            //          mac_xface->get_ue_active_harq_pid(Mod_id,CC_id,rnti,frameP,subframeP,&harq_pid,&round,0);
            //rrc_status = mac_eNB_get_rrc_status(Mod_id,rnti);
            /* 1st allocate for the retx */

            // retransmission in data channels
            // control channel in the 1st transmission
            // data channel for all TM
            LOG_T(MAC,"calling dlsch_scheduler_pre_processor_allocate .. \n ");
            ue_sched_ctl->remain_1b[CC_id] = nb_rbs_required_remaining[CC_id][i];
            ue_sched_ctl->nb_rbs_requiredb[CC_id] = nb_rbs_required[CC_id][i];

            dlsch_scheduler_pre_processor_allocate (Mod_id,
                                                  UE_id,
                                                  CC_id,
                                                  N_RBG[CC_id],
                                                  transmission_mode,
                                                  min_rb_unit[CC_id],
                                                  frame_parms[CC_id]->N_RB_DL,
                                                  nb_rbs_required,
                                                  nb_rbs_required_remaining,
                                                  rballoc_sub,
                                                  MIMO_mode_indicator,
                                                  subframeP);

        }
      }
    } // total_ue_count
  } // end of for for r1 and r2


  static int schedule_count = 0;
  static int succ_count = 0;
  static int rb_sum = 0;

  int rb_total = 0;
  
  for(i=UE_list->head; i>=0; i=UE_list->next[i]) {
    UE_id = i;

    UE_info = &eNB_mac_inst[Mod_id].UE_info[i];
    ue_sched_ctl = &UE_list->UE_sched_ctrl[UE_id];

    for (ii=0; ii<UE_num_active_CC(UE_info); ii++) {
      CC_id = UE_info->ordered_CCids[ii];
      //PHY_vars_eNB_g[Mod_id]->mu_mimo_mode[UE_id].dl_pow_off = dl_pow_off[UE_id];

      if (ue_sched_ctl->pre_nb_available_rbs[CC_id] > 0 ) {
        LOG_D(MAC,"******************DL Scheduling Information for UE%d ************************\n",UE_id);
        LOG_D(MAC,"dl power offset UE%d = %d \n",UE_id,ue_sched_ctl->dl_pow_off[CC_id]);
        LOG_D(MAC,"***********RB Alloc for every subband for UE%d ***********\n",UE_id);

        for(j=0; j<N_RBG[CC_id]; j++) {
          //PHY_vars_eNB_g[Mod_id]->mu_mimo_mode[UE_id].rballoc_sub[i] = rballoc_sub_UE[CC_id][UE_id][i];
          LOG_D(MAC,"RB Alloc for UE%d and Subband%d = %d\n",UE_id,j,
              ue_sched_ctl->rballoc_sub_UE[CC_id][j]);
        }
        rb_sum += ue_sched_ctl->pre_nb_available_rbs[CC_id];\
        succ_count ++;
        
      }
      else
      {
         if( fail_count == 100 )
         {
             printf("nb_rbs_required=%d remaining=%d %d round=%d old_rb=%d %d %d %d\n", 
                 nb_rbs_required[CC_id][i], remaining_ttt[0], remaining_ttt[1], 
                 round,
                 old_rb[0],
                 old_rb[1],
                 old_rb[2],
                 old_rb[3]
                 );
         }
         fail_count ++;
      }
    }
    rb_total+=ue_sched_ctl->pre_nb_available_rbs[0];
    if( 0/*++schedule_count== 1000*/)
    {
        int avarage_rb_sum = 0;
        if( succ_count > 0)
        {
           avarage_rb_sum = rb_sum/succ_count;
        }
        
        //PHY_vars_eNB_g[Mod_id]->mu_mimo_mode[UE_id].pre_nb_available_rbs = pre_nb_available_rbs[CC_id][UE_id];
        LOG_I(MAC,"Total RBs allocated for UE%d = %d  Average:%d N_RBG=%d fail_count=%d round %d %d\n",
            UE_id,ue_sched_ctl->pre_nb_available_rbs[CC_id], avarage_rb_sum, N_RBG[CC_id], fail_count,
            round_great_count, round_less_count);
            
        schedule_count = 0;
        rb_sum = 0;
        succ_count = 0;
        fail_count = 0;
        round_great_count = 0;
        round_less_count = 0;
    }

  }
  if(rb_total > frame_parms[CC_id]->N_RB_DL)
  {
        printf("dlsch_scheduler_pre_processor:RB OVER FLOW:%d\n", rb_total);
  }


}

// This function assigns pre-available RBS to each UE in specified sub-bands before scheduling is done
void dlsch_scheduler_pre_processor_si_ri (module_id_t   Mod_id,
                                    frame_t       frameP,
                                    sub_frame_t   subframeP,
                                    int           N_RBG[MAX_NUM_CCs],
                                    int           *mbsfn_flag,
                                    unsigned char rballoc_sub[MAX_NUM_CCs][N_RBG_MAX])
{

  unsigned char harq_pid=0,round=0,total_ue_count;
  unsigned char MIMO_mode_indicator[MAX_NUM_CCs][N_RBG_MAX];
  int                     UE_id, i; 
  uint16_t                ii,j;
  uint16_t                nb_rbs_required[MAX_NUM_CCs][NUMBER_OF_UE_MAX];
  uint16_t                nb_rbs_required_remaining[MAX_NUM_CCs][NUMBER_OF_UE_MAX];
  uint16_t                nb_rbs_required_remaining_1[MAX_NUM_CCs][NUMBER_OF_UE_MAX];
  uint16_t                average_rbs_per_user[MAX_NUM_CCs] = {0};
  rnti_t             rnti;
  int                min_rb_unit[MAX_NUM_CCs];
  uint16_t r1=0;
  uint8_t CC_id;

  UE_info_t *UE_info ;
  UE_list_t *UE_list = &eNB_mac_inst[Mod_id].UE_list[subframeP];
  LTE_DL_FRAME_PARMS   *frame_parms[MAX_NUM_CCs] = {0};

  int transmission_mode = 0;
  UE_sched_ctrl *ue_sched_ctl;
  //  int rrc_status           = RRC_IDLE;

  //xhd_oai_50
  static int fail_count = 0;

  static int round_great_count = 0;
  static int round_less_count = 0;

  int old_rb[NUMBER_OF_UE_MAX]={0,0,0,0};
  


  for (CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) 
  {

    if (mbsfn_flag[CC_id]>0)  // If this CC is allocated for MBSFN skip it here
      continue;

    frame_parms[CC_id] = mac_xface->get_lte_frame_parms(Mod_id,CC_id);


    min_rb_unit[CC_id]=get_min_rb_unit(Mod_id,CC_id);

    for (i=UE_list->head; i>=0; i=UE_list->next[i]) 
    {
      UE_id = i;
      // Initialize scheduling information for all active UEs
      

      dlsch_scheduler_pre_processor_reset(Mod_id,
        UE_id,
        CC_id,
        frameP,
        subframeP,
        N_RBG[CC_id],
        nb_rbs_required,
        nb_rbs_required_remaining,
        NULL, //rballoc_sub,
        MIMO_mode_indicator);

    }
  }


  // Store the DLSCH buffer for each logical channel
  store_dlsch_buffer (Mod_id,frameP,subframeP);



  // Calculate the number of RBs required by each UE on the basis of logical channel's buffer
  assign_rbs_required (Mod_id,frameP,subframeP,nb_rbs_required,min_rb_unit);

  old_rb[0]=nb_rbs_required[0][0];
  old_rb[1]=nb_rbs_required[0][1];
  old_rb[2]=nb_rbs_required[0][2];
  old_rb[3]=nb_rbs_required[0][3];

  // Sorts the user on the basis of dlsch logical channel buffer and CQI
  sort_UEs (Mod_id,frameP,subframeP);



  total_ue_count =0;

  // loop over all active UEs
  for (i=UE_list->head; i>=0; i=UE_list->next[i]) {
    rnti = UE_RNTI(Mod_id,i);

    if(rnti == NOT_A_RNTI)
      continue;

    UE_id = i;
    UE_info = &eNB_mac_inst[Mod_id].UE_info[UE_id];
    // if there is no available harq_process, skip the UE

    ue_sched_ctl = &UE_list->UE_sched_ctrl[UE_id];
    

    for (ii=0; ii<UE_num_active_CC(UE_info); ii++) 
    {
      CC_id = UE_info->ordered_CCids[ii];

      if (ue_sched_ctl->harq_pid[CC_id]<0)
      {
        continue;
      }

      harq_pid = ue_sched_ctl->harq_pid[CC_id];
      round    = ue_sched_ctl->round[CC_id];

      average_rbs_per_user[CC_id]=0;

      frame_parms[CC_id] = mac_xface->get_lte_frame_parms(Mod_id,CC_id);

      //      mac_xface->get_ue_active_harq_pid(Mod_id,CC_id,rnti,frameP,subframeP,&harq_pid,&round,0);

      
      if(round>0) 
      {
        nb_rbs_required[CC_id][UE_id] = UE_info->UE_template[CC_id].nb_rb[harq_pid];
        round_great_count++;
      }
      else
      {
        round_less_count++;
      }

      //nb_rbs_required_remaining[UE_id] = nb_rbs_required[UE_id];
      if (nb_rbs_required[CC_id][UE_id] > 0) {
        total_ue_count = total_ue_count + 1;
      }


      // hypotetical assignement
      /*
       * If schedule is enabled and if the priority of the UEs is modified
       * The average rbs per logical channel per user will depend on the level of
       * priority. Concerning the hypothetical assignement, we should assign more
       * rbs to prioritized users. Maybe, we can do a mapping between the
       * average rbs per user and the level of priority or multiply the average rbs
       * per user by a coefficient which represents the degree of priority.
       */

      if (total_ue_count == 0) {
        average_rbs_per_user[CC_id] = 0;
      } else if( (min_rb_unit[CC_id] * total_ue_count) <= (frame_parms[CC_id]->N_RB_DL) ) {
        average_rbs_per_user[CC_id] = (uint16_t) floor(frame_parms[CC_id]->N_RB_DL/total_ue_count);
        //xhd_oai_multuser
        //if( average_rbs_per_user[CC_id] > 10 )
        {
            //average_rbs_per_user[CC_id] -= 5;
        }
      } else {
        average_rbs_per_user[CC_id] = min_rb_unit[CC_id]; // consider the total number of use that can be scheduled UE
      }
    }
  }

  int nb_rbs_required_remaining_1_ttt = 0;
  int remaining_ttt[2];
  
  // note: nb_rbs_required is assigned according to total_buffer_dl
  // extend nb_rbs_required to capture per LCID RB required
  for(i=UE_list->head; i>=0; i=UE_list->next[i]) {
    rnti = UE_RNTI(Mod_id,i);
    
    UE_info = &eNB_mac_inst[Mod_id].UE_info[i];

    for (ii=0; ii<UE_num_active_CC(UE_info); ii++) {
      CC_id = UE_info->ordered_CCids[ii];

      //xhd_oai_multuser: this is a bug!
      //ue_sched_ctl = &UE_list->UE_sched_ctrl[UE_id];
      ue_sched_ctl = &UE_list->UE_sched_ctrl[i];
      harq_pid = ue_sched_ctl->harq_pid[CC_id];
      round    = ue_sched_ctl->round[CC_id];

      // control channel
      if (mac_eNB_get_rrc_status(Mod_id,rnti) < RRC_RECONFIGURED
        || round > 0) 
      {
        //优先保证业务建立过程中的RB需要
        nb_rbs_required_remaining_1[CC_id][i] = nb_rbs_required[CC_id][i];
        
      } 
      else 
      {
        nb_rbs_required_remaining_1[CC_id][i] = cmin(average_rbs_per_user[CC_id],nb_rbs_required[CC_id][i]);

      }
      //xhd_oai_multuser only for test
      ue_sched_ctl->remain_1[CC_id] = nb_rbs_required_remaining_1[CC_id][i];
      ue_sched_ctl->nb_rbs_required[CC_id] = nb_rbs_required[CC_id][i];
      
      nb_rbs_required_remaining_1_ttt = nb_rbs_required_remaining_1[CC_id][i];
    }
  }

  //Allocation to UEs is done in 2 rounds,
  // 1st stage: average number of RBs allocated to each UE
  // 2nd stage: remaining RBs are allocated to high priority UEs
  for(r1=0; r1<2; r1++) 
  {

    for(i=UE_list->head; i>=0; i=UE_list->next[i]) 
    {


      UE_info = &eNB_mac_inst[Mod_id].UE_info[i];
      for (ii=0; ii<UE_num_active_CC(UE_info); ii++) {
        CC_id = UE_info->ordered_CCids[ii];

        if(r1 == 0) 
        {
            //第一轮，不大于平均值
            nb_rbs_required_remaining[CC_id][i] = nb_rbs_required_remaining_1[CC_id][i];
        } 
        else 
        { 
            //第二轮，加上剩余值
            // rb required based only on the buffer - rb allloctaed in the 1st round + extra reaming rb form the 1st round
            nb_rbs_required_remaining[CC_id][i] 
               = nb_rbs_required[CC_id][i]
                 -nb_rbs_required_remaining_1[CC_id][i]
                 +nb_rbs_required_remaining[CC_id][i]; //可能上一次仍然有剩余
        }
        remaining_ttt[r1]=nb_rbs_required_remaining[CC_id][i];
        
        if (nb_rbs_required[CC_id][i]> 0 )
          LOG_D(MAC,"round %d : nb_rbs_required_remaining[%d][%d]= %d (remaining_1 %d, required %d,  pre_nb_available_rbs %d, N_RBG %d, rb_unit %d)\n",
                r1, CC_id, i,
                nb_rbs_required_remaining[CC_id][i],
                nb_rbs_required_remaining_1[CC_id][i],
                nb_rbs_required[CC_id][i],
                UE_list->UE_sched_ctrl[i].pre_nb_available_rbs[CC_id],
                N_RBG[CC_id],
                min_rb_unit[CC_id]);

      }
    }

    if (total_ue_count > 0 ) 
    {
      for(i=UE_list->head; i>=0; i=UE_list->next[i]) 
      {
        UE_id = i;
        UE_info = &eNB_mac_inst[Mod_id].UE_info[i];

        for (ii=0; ii<UE_num_active_CC(UE_info); ii++) 
        {
            CC_id = UE_info->ordered_CCids[ii];
            ue_sched_ctl = &UE_list->UE_sched_ctrl[UE_id];
            harq_pid = ue_sched_ctl->harq_pid[CC_id];
            round    = ue_sched_ctl->round[CC_id];

            rnti = UE_RNTI(Mod_id,UE_id);

            // LOG_D(MAC,"UE %d rnti 0x\n", UE_id, rnti );
            if(rnti == NOT_A_RNTI)
            continue;

            transmission_mode = mac_xface->get_transmission_mode(Mod_id,CC_id,rnti);
            //          mac_xface->get_ue_active_harq_pid(Mod_id,CC_id,rnti,frameP,subframeP,&harq_pid,&round,0);
            //rrc_status = mac_eNB_get_rrc_status(Mod_id,rnti);
            /* 1st allocate for the retx */

            // retransmission in data channels
            // control channel in the 1st transmission
            // data channel for all TM
            LOG_T(MAC,"calling dlsch_scheduler_pre_processor_allocate .. \n ");
            ue_sched_ctl->remain_1b[CC_id] = nb_rbs_required_remaining[CC_id][i];
            ue_sched_ctl->nb_rbs_requiredb[CC_id] = nb_rbs_required[CC_id][i];

            dlsch_scheduler_pre_processor_allocate (Mod_id,
                                                  UE_id,
                                                  CC_id,
                                                  N_RBG[CC_id],
                                                  transmission_mode,
                                                  min_rb_unit[CC_id],
                                                  frame_parms[CC_id]->N_RB_DL,
                                                  nb_rbs_required,
                                                  nb_rbs_required_remaining,
                                                  rballoc_sub,
                                                  MIMO_mode_indicator,
                                                  subframeP);

        }
      }
    } // total_ue_count
  } // end of for for r1 and r2


  static int schedule_count = 0;
  static int succ_count = 0;
  static int rb_sum = 0;

  int rb_total = 0;
  
  for(i=UE_list->head; i>=0; i=UE_list->next[i]) {
    UE_id = i;

    UE_info = &eNB_mac_inst[Mod_id].UE_info[i];
    ue_sched_ctl = &UE_list->UE_sched_ctrl[UE_id];

    for (ii=0; ii<UE_num_active_CC(UE_info); ii++) {
      CC_id = UE_info->ordered_CCids[ii];
      //PHY_vars_eNB_g[Mod_id]->mu_mimo_mode[UE_id].dl_pow_off = dl_pow_off[UE_id];

      if (ue_sched_ctl->pre_nb_available_rbs[CC_id] > 0 ) {
        LOG_D(MAC,"******************DL Scheduling Information for UE%d ************************\n",UE_id);
        LOG_D(MAC,"dl power offset UE%d = %d \n",UE_id,ue_sched_ctl->dl_pow_off[CC_id]);
        LOG_D(MAC,"***********RB Alloc for every subband for UE%d ***********\n",UE_id);

        for(j=0; j<N_RBG[CC_id]; j++) {
          //PHY_vars_eNB_g[Mod_id]->mu_mimo_mode[UE_id].rballoc_sub[i] = rballoc_sub_UE[CC_id][UE_id][i];
          LOG_D(MAC,"RB Alloc for UE%d and Subband%d = %d\n",UE_id,j,
              ue_sched_ctl->rballoc_sub_UE[CC_id][j]);
        }
        rb_sum += ue_sched_ctl->pre_nb_available_rbs[CC_id];\
        succ_count ++;
        
      }
      else
      {
         if( fail_count == 100 )
         {
             printf("nb_rbs_required=%d remaining=%d %d round=%d old_rb=%d %d %d %d\n", 
                 nb_rbs_required[CC_id][i], remaining_ttt[0], remaining_ttt[1], 
                 round,
                 old_rb[0],
                 old_rb[1],
                 old_rb[2],
                 old_rb[3]
                 );
         }
         fail_count ++;
      }
    }
    rb_total+=ue_sched_ctl->pre_nb_available_rbs[0];
    if( 0/*++schedule_count== 1000*/)
    {
        int avarage_rb_sum = 0;
        if( succ_count > 0)
        {
           avarage_rb_sum = rb_sum/succ_count;
        }
        
        //PHY_vars_eNB_g[Mod_id]->mu_mimo_mode[UE_id].pre_nb_available_rbs = pre_nb_available_rbs[CC_id][UE_id];
        LOG_I(MAC,"Total RBs allocated for UE%d = %d  Average:%d N_RBG=%d fail_count=%d round %d %d\n",
            UE_id,ue_sched_ctl->pre_nb_available_rbs[CC_id], avarage_rb_sum, N_RBG[CC_id], fail_count,
            round_great_count, round_less_count);
            
        schedule_count = 0;
        rb_sum = 0;
        succ_count = 0;
        fail_count = 0;
        round_great_count = 0;
        round_less_count = 0;
    }

  }
  if(rb_total > frame_parms[CC_id]->N_RB_DL)
  {
        printf("dlsch_scheduler_pre_processor:RB OVER FLOW:%d\n", rb_total);
  }


}

void dlsch_scheduler_pre_processor_reset (int module_idP,
    int UE_id,
    uint8_t  CC_id,
    int frameP,
    int subframeP,					  
    int N_RBG,
    uint16_t nb_rbs_required[MAX_NUM_CCs][NUMBER_OF_UE_MAX],
    uint16_t nb_rbs_required_remaining[MAX_NUM_CCs][NUMBER_OF_UE_MAX],
    unsigned char rballoc_sub[MAX_NUM_CCs][N_RBG_MAX],
    unsigned char MIMO_mode_indicator[MAX_NUM_CCs][N_RBG_MAX])
{
  int i;
  UE_list_t          *UE_list = &eNB_mac_inst[module_idP].UE_list[subframeP];
  UE_sched_ctrl *ue_sched_ctl = &UE_list->UE_sched_ctrl[UE_id];
  rnti_t rnti = UE_RNTI(module_idP,UE_id);

  // initialize harq_pid and round
  mac_xface->get_ue_active_harq_pid(module_idP,CC_id,rnti,
				    frameP,subframeP,
				    &ue_sched_ctl->harq_pid[CC_id],
				    &ue_sched_ctl->round[CC_id],
				    0);

  nb_rbs_required[CC_id][UE_id]=0;
  ue_sched_ctl->pre_nb_available_rbs[CC_id] = 0;
  ue_sched_ctl->dl_pow_off[CC_id] = 2;
  nb_rbs_required_remaining[CC_id][UE_id] = 0;

  for (i=0; i<N_RBG; i++) {
    ue_sched_ctl->rballoc_sub_UE[CC_id][i] = 0;
    if( NULL != rballoc_sub )
    {
        rballoc_sub[CC_id][i] = 0;
    }
    MIMO_mode_indicator[CC_id][i] = 2;
  }
}


void dlsch_scheduler_pre_processor_allocate (module_id_t   Mod_id,
    int           UE_id,
    uint8_t       CC_id,
    int           N_RBG,
    int           transmission_mode,
    int           min_rb_unit,
    uint8_t       N_RB_DL,
    uint16_t      nb_rbs_required[MAX_NUM_CCs][NUMBER_OF_UE_MAX],
    uint16_t      nb_rbs_required_remaining[MAX_NUM_CCs][NUMBER_OF_UE_MAX],
    unsigned char rballoc_sub[MAX_NUM_CCs][N_RBG_MAX],
    unsigned char MIMO_mode_indicator[MAX_NUM_CCs][N_RBG_MAX],
    sub_frame_t   subframeP)
{

  int i;
  UE_list_t          *UE_list = &eNB_mac_inst[Mod_id].UE_list[subframeP];
  UE_sched_ctrl *ue_sched_ctl = &UE_list->UE_sched_ctrl[UE_id];

  for(i=0; i<N_RBG; i++) 
  {

    if((rballoc_sub[CC_id][i] == 0)           &&
        (ue_sched_ctl->rballoc_sub_UE[CC_id][i] == 0) &&
        (nb_rbs_required_remaining[CC_id][UE_id]>0)   &&
        (ue_sched_ctl->pre_nb_available_rbs[CC_id] 
           < nb_rbs_required[CC_id][UE_id])) 
    {

      // if this UE is not scheduled for TM5
      if (ue_sched_ctl->dl_pow_off[CC_id] != 0 )  
      {

        	if ((i == N_RBG-1) && ((N_RB_DL == 25) || (N_RB_DL == 50))) 
        	{
                if (transmission_mode == 5 ) {
                  ue_sched_ctl->dl_pow_off[CC_id] = 1;
                }   

                  //xhd_oai_rb
                  if(nb_rbs_required_remaining[CC_id][UE_id] < (min_rb_unit - 1))
                  {
                      nb_rbs_required_remaining[CC_id][UE_id]=0;
                  }
                  else
                  {
                      nb_rbs_required_remaining[CC_id][UE_id] = nb_rbs_required_remaining[CC_id][UE_id] - min_rb_unit+1;
                  }
                  
            	  rballoc_sub[CC_id][i] = 1;
            	  ue_sched_ctl->rballoc_sub_UE[CC_id][i] = 1;
            	  MIMO_mode_indicator[CC_id][i] = 1;
                  ue_sched_ctl->pre_nb_available_rbs[CC_id] 
                    = ue_sched_ctl->pre_nb_available_rbs[CC_id] + min_rb_unit - 1;
            } 
            else if (nb_rbs_required_remaining[CC_id][UE_id] >=  min_rb_unit)
    	    {
        	    rballoc_sub[CC_id][i] = 1;
        	    ue_sched_ctl->rballoc_sub_UE[CC_id][i] = 1;
        	    MIMO_mode_indicator[CC_id][i] = 1;
        	    if (transmission_mode == 5 ) {
        	      ue_sched_ctl->dl_pow_off[CC_id] = 1;
        	    }
        	    nb_rbs_required_remaining[CC_id][UE_id] = nb_rbs_required_remaining[CC_id][UE_id] - min_rb_unit;
        	    ue_sched_ctl->pre_nb_available_rbs[CC_id] 
                    = ue_sched_ctl->pre_nb_available_rbs[CC_id] + min_rb_unit;
    	    }
      } // dl_pow_off[CC_id][UE_id] ! = 0
    }
  }

}


/// ULSCH PRE_PROCESSOR


void ulsch_scheduler_pre_processor(module_id_t module_idP,
                                   int frameP,
                                   sub_frame_t subframeP,unsigned char sched_subframe,
                                   uint16_t *first_rb,
                                   uint8_t aggregation,
                                   uint32_t *nCCE)
{

  int16_t            i;
  uint16_t           UE_id,n,r;
  uint8_t            CC_id, round, harq_pid;
  uint16_t           nb_allocated_rbs[MAX_NUM_CCs][NUMBER_OF_UE_MAX],total_allocated_rbs[MAX_NUM_CCs],average_rbs_per_user[MAX_NUM_CCs];
  int16_t            total_remaining_rbs[MAX_NUM_CCs];
  uint16_t           max_num_ue_to_be_scheduled=0,total_ue_count=0;
  rnti_t             rnti= -1;
  uint32_t            nCCE_to_be_used[MAX_NUM_CCs];
  UE_info_t          *UE_info ;
  UE_list_t          *UE_list = &eNB_mac_inst[module_idP].UE_list[subframeP];
  UE_TEMPLATE        *UE_template = 0;
  LTE_DL_FRAME_PARMS   *frame_parms = 0;

  if( UE_list->head_ul < 0 )
  {
    return;
  }

  // LOG_I(MAC,"store ulsch buffers\n");
  // convert BSR to bytes for comparison with tbs
  store_ulsch_buffer(module_idP,frameP, subframeP, sched_subframe);

  //LOG_I(MAC,"assign max mcs min rb\n");
  // maximize MCS and then allocate required RB according to the buffer occupancy with the limit of max available UL RB
  assign_max_mcs_min_rb(module_idP,frameP, subframeP, first_rb);

  //LOG_I(MAC,"sort ue \n");
  // sort ues
  
  sort_ue_ul (module_idP,frameP, subframeP);


  // we need to distribute RBs among UEs
  // step1:  reset the vars
  for (CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) {
    nCCE_to_be_used[CC_id]= nCCE[CC_id];
    total_allocated_rbs[CC_id]=0;
    total_remaining_rbs[CC_id]=0;
    average_rbs_per_user[CC_id]=0;

    for (i=UE_list->head_ul; i>=0; i=UE_list->next_ul[i]) {
      nb_allocated_rbs[CC_id][i]=0;
    }
  }

  //LOG_I(MAC,"step2 \n");
  // step 2: calculate the average rb per UE
  total_ue_count =0;
  max_num_ue_to_be_scheduled=0;

  for (i=UE_list->head_ul; i>=0; i=UE_list->next_ul[i]) 
  {

    rnti = UE_RNTI(module_idP,i);

    if (rnti==NOT_A_RNTI)
      continue;

    UE_id = i;
    UE_info = &eNB_mac_inst[module_idP].UE_info[UE_id];

    for (n=0; n<UE_info->numactiveULCCs; n++) 
    {
      // This is the actual CC_id in the list
      CC_id = UE_info->ordered_ULCCids[n];
      UE_template = &UE_info->UE_template[CC_id];
      average_rbs_per_user[CC_id]=0;
      frame_parms = mac_xface->get_lte_frame_parms(module_idP,CC_id);

      if (UE_template->pre_allocated_nb_rb_ul[subframeP] > 0) {
        total_ue_count+=1;
      }

      if((mac_xface->get_nCCE_max(module_idP,CC_id) - nCCE_to_be_used[CC_id])  > (1<<aggregation)) {
        nCCE_to_be_used[CC_id] = nCCE_to_be_used[CC_id] + (1<<aggregation);
        max_num_ue_to_be_scheduled+=1;
      }

      if (total_ue_count == 0) 
      {
        average_rbs_per_user[CC_id] = 0;
      } 
      else if (total_ue_count == 1 ) 
      { 
        // increase the available RBs, special case,
        average_rbs_per_user[CC_id] = frame_parms->N_RB_UL-first_rb[CC_id]+1;
      } 
      else if( (total_ue_count <= (frame_parms->N_RB_DL-first_rb[CC_id])) &&
                 (total_ue_count <= max_num_ue_to_be_scheduled)) 
      {
        average_rbs_per_user[CC_id] = (uint16_t) floor((frame_parms->N_RB_UL-first_rb[CC_id])/total_ue_count);
      } 
      else if (max_num_ue_to_be_scheduled > 0 ) 
      {
        average_rbs_per_user[CC_id] = (uint16_t) floor((frame_parms->N_RB_UL-first_rb[CC_id])/max_num_ue_to_be_scheduled);
      } 
      else 
      {
        average_rbs_per_user[CC_id]=1;
        LOG_W(MAC,"[eNB %d] frame %d subframe %d: UE %d CC %d: can't get average rb per user (should not be here)\n",
              module_idP,frameP,subframeP,UE_id,CC_id);
      }
    }
  }
  if (total_ue_count > 0)
    LOG_D(MAC,"[eNB %d] Frame %d subframe %d: total ue to be scheduled %d/%d\n",
	  module_idP, frameP, subframeP,total_ue_count, max_num_ue_to_be_scheduled);

  //LOG_D(MAC,"step3\n");

  // step 3: assigne RBS
  for (i=UE_list->head_ul; i>=0; i=UE_list->next_ul[i]) 
  {
    rnti = UE_RNTI(module_idP,i);

    if (rnti==NOT_A_RNTI)
      continue;

    UE_id = i;
    UE_info = &eNB_mac_inst[module_idP].UE_info[UE_id];

    for (n=0; n<UE_info->numactiveULCCs; n++) {
      // This is the actual CC_id in the list
      CC_id = UE_info->ordered_ULCCids[n];

      mac_xface->get_ue_active_harq_pid(module_idP,CC_id,rnti,frameP,subframeP,&harq_pid,&round,1);

      if(round>0) {
        nb_allocated_rbs[CC_id][UE_id] = UE_info->UE_template[CC_id].nb_rb_ul[harq_pid];
      } else {
        nb_allocated_rbs[CC_id][UE_id] = cmin(UE_info->UE_template[CC_id].pre_allocated_nb_rb_ul[subframeP], average_rbs_per_user[CC_id]);

        //xhd_oai_rb
        if( nb_allocated_rbs[CC_id][UE_id] < 3 && nb_allocated_rbs[CC_id][UE_id] > 0)//xhd_oai_debug
        //if( nb_allocated_rbs[CC_id][UE_id] < 3 )
        {
            //nb_allocated_rbs[CC_id][UE_id] = 3; 
        }
        //xhd_oai_60m
        if( nb_allocated_rbs[CC_id][UE_id] < 10 )
        {
            //nb_allocated_rbs[CC_id][UE_id] = 10; //xhd_oai_debug
        }
      }

      total_allocated_rbs[CC_id]+= nb_allocated_rbs[CC_id][UE_id];

    }
  }

  // step 4: assigne the remaining RBs and set the pre_allocated rbs accordingly
  for(r=0; r<2; r++) {

    for (i=UE_list->head_ul; i>=0; i=UE_list->next_ul[i]) {
      rnti = UE_RNTI(module_idP,i);

      if (rnti==NOT_A_RNTI)
        continue;

      UE_id = i;
      UE_info = &eNB_mac_inst[module_idP].UE_info[UE_id];

      for (n=0; n<UE_info->numactiveULCCs; n++) {
        // This is the actual CC_id in the list
        CC_id = UE_info->ordered_ULCCids[n];
        UE_template = &UE_info->UE_template[CC_id];
        frame_parms = mac_xface->get_lte_frame_parms(module_idP,CC_id);
        total_remaining_rbs[CC_id]=frame_parms->N_RB_UL - first_rb[CC_id] - total_allocated_rbs[CC_id];

        if (total_ue_count == 1 ) {
          total_remaining_rbs[CC_id]+=1;
        }

        if ( r == 0 ) {
          while ( (UE_template->pre_allocated_nb_rb_ul[subframeP] > 0 ) &&
                  (nb_allocated_rbs[CC_id][UE_id] < UE_template->pre_allocated_nb_rb_ul[subframeP]) &&
                  (total_remaining_rbs[CC_id] > 0)) {
            nb_allocated_rbs[CC_id][UE_id] = cmin(nb_allocated_rbs[CC_id][UE_id]+1,UE_template->pre_allocated_nb_rb_ul[subframeP]);
            total_remaining_rbs[CC_id]--;
            total_allocated_rbs[CC_id]++;
          }
        } else {
          UE_template->pre_allocated_nb_rb_ul[subframeP] = nb_allocated_rbs[CC_id][UE_id];
          LOG_D(MAC,"******************UL Scheduling Information for UE%d CC_id %d ************************\n",UE_id, CC_id);
          LOG_D(MAC,"[eNB %d] total RB allocated for UE%d CC_id %d  = %d\n", 
            module_idP, UE_id, CC_id, UE_template->pre_allocated_nb_rb_ul[subframeP]);
        }
      }
    }
  }

  for (CC_id=0; CC_id<MAX_NUM_CCs; CC_id++) {
    frame_parms= mac_xface->get_lte_frame_parms(module_idP,CC_id);

    if (total_allocated_rbs[CC_id]>0) {
      LOG_D(MAC,"[eNB %d] total RB allocated for all UEs = %d/%d\n", module_idP, total_allocated_rbs[CC_id], frame_parms->N_RB_UL - first_rb[CC_id]);
    }
  }
}

#define OLTE_MAX_USER NUMBER_OF_UE_MAX
extern int sr_sched[OLTE_MAX_USER][10];
extern int bsr_sched[OLTE_MAX_USER][10];

void store_ulsch_buffer(module_id_t module_idP, int frameP, sub_frame_t subframeP, unsigned char sched_subframe)
{

  int                 UE_id,pCC_id,lcgid;
  UE_info_t           *UE_info;
  UE_list_t           *UE_list = &eNB_mac_inst[module_idP].UE_list[subframeP];
  UE_TEMPLATE         *UE_template;

  for (UE_id=UE_list->head_ul; UE_id>=0; UE_id=UE_list->next_ul[UE_id]) 
  {

    UE_info = &eNB_mac_inst[module_idP].UE_info[UE_id];
    UE_template = &UE_info->UE_template[UE_PCCID(module_idP,UE_id)];
    //LOG_I(MAC,"[UE %d next %d] SR is %d\n",UE_id, UE_list->next_ul[UE_id], UE_template->ul_SR);

    UE_template->ul_total_buffer[subframeP]=0;
    UE_template->ul_SR_idle_exe[sched_subframe] = 0;

    for (lcgid=0; lcgid<MAX_NUM_LCGID; lcgid++) 
    {
      //xhd_oai_multuser
      if( 0 == UE_template->bsr_info[lcgid] )
      {
        //xhd_oai_ue
        //UE_template->bsr_info[lcgid] = 1; //xhd_oai_debug
      }
      UE_template->ul_buffer_info[lcgid]=BSR_TABLE[UE_template->bsr_info[lcgid]];
      UE_template->ul_total_buffer[subframeP]+= UE_template->ul_buffer_info[lcgid]; // apply traffic aggregtaion if packets are small
      //   UE_template->ul_buffer_creation_time_max=cmax(UE_template->ul_buffer_creation_time_max, frame_cycle*1024 + frameP-UE_template->ul_buffer_creation_time[lcgid]));
    }

    if ( UE_template->ul_total_buffer[subframeP] >0)
    {
      LOG_D(MAC,"[eNB %d] Frame %d subframe %d UE %d CC id %d: LCGID0 %d, LCGID1 %d, LCGID2 %d LCGID3 %d, BO %d\n",
            module_idP, frameP,subframeP, UE_id, UE_PCCID(module_idP,UE_id),
            UE_template->ul_buffer_info[LCGID0],
            UE_template->ul_buffer_info[LCGID1],
            UE_template->ul_buffer_info[LCGID2],
            UE_template->ul_buffer_info[LCGID3],
            UE_template->ul_total_buffer[subframeP]);
      phy_add_count(UE_template->rnti,23);
      phy_add_count_subframe(UE_template->rnti,255,3,subframeP);
      phy_add_count_subframe1(UE_template->rnti,subframeP);
      
      bsr_sched[UE_id][subframeP]++;

    }
    //else if (UE_is_to_be_scheduled_SR(module_idP,UE_PCCID(module_idP,UE_id),UE_id,subframeP) > 0 ) 
    else if(UE_template->ul_SR>0)
    {
        lcgid = 0;
        UE_template->bsr_info[lcgid] = 11;
        UE_template->ul_buffer_info[lcgid]=BSR_TABLE[UE_template->bsr_info[lcgid]];
        UE_template->ul_total_buffer[subframeP] = UE_template->ul_buffer_info[lcgid];

        phy_add_count(UE_template->rnti,24);
        phy_add_count_subframe(UE_template->rnti,255,4,subframeP);
        phy_add_count_subframe2(UE_template->rnti,subframeP);
        sr_sched[UE_id][subframeP]++;

        static int count=0;
        if(count ++ == 1000)
        {
          LOG_D(MAC,"[eNB %d] Frame %d subframe %d UE %d CC id %d: SR active, set BO to %d ul_SR=%d\n",
                module_idP, frameP,subframeP, UE_id, UE_PCCID(module_idP,UE_id),
                UE_template->ul_total_buffer[subframeP], UE_template->ul_SR);
          count = 0;
        }
    }
    //else if (UE_is_to_be_scheduled_SR_Idle(module_idP,UE_PCCID(module_idP,UE_id),UE_id,subframeP, sched_subframe) > 0 ) 
    else if(UE_template->ul_SR_idle[subframeP]>0 )
    {
       UE_template->ul_total_buffer[subframeP] = BSR_TABLE[11];
       UE_template->ul_SR_idle_exe[sched_subframe]=1;
       UE_template->ul_SR_idle[subframeP] = 0;

      phy_add_count(UE_template->rnti,25);
      phy_add_count_subframe(UE_template->rnti,255,0,subframeP);
      phy_add_count_subframe3(UE_template->rnti,subframeP);
      LOG_D(MAC,"[eNB %d] Frame %d subframe %d UE %d CC id %d: SR active, set BO to %d \n",
            module_idP, frameP,subframeP, UE_id, UE_PCCID(module_idP,UE_id),
            UE_template->ul_total_buffer[subframeP]);
    }
    else
    {
        phy_add_count(UE_template->rnti,26);
        phy_add_count_subframe(UE_template->rnti,255,7,subframeP);
        phy_add_count_subframe4(UE_template->rnti,subframeP);
    }
  }
}



void assign_max_mcs_min_rb(module_id_t module_idP,int frameP, sub_frame_t subframeP, uint16_t *first_rb)
{

  int                i;
  uint16_t           n,UE_id;
  uint8_t            CC_id;
  rnti_t             rnti           = -1;
  int                mcs=cmin(16,openair_daq_vars.target_ue_ul_mcs);
  int                rb_table_index=0,tbs,tx_power;
  eNB_MAC_INST       *eNB = &eNB_mac_inst[module_idP];

  UE_info_t          *UE_info;
  UE_list_t          *UE_list = &eNB->UE_list[subframeP];

  UE_TEMPLATE       *UE_template;
  LTE_DL_FRAME_PARMS   *frame_parms;

  for (i=UE_list->head_ul; i>=0; i=UE_list->next_ul[i]) {

    rnti = UE_RNTI(module_idP,i);

    if (rnti==NOT_A_RNTI)
      continue;

    UE_id = i;
    UE_info = &eNB_mac_inst[module_idP].UE_info[UE_id];
    
    for (n=0; n<UE_info->numactiveULCCs; n++) {
      // This is the actual CC_id in the list
      CC_id = UE_info->ordered_ULCCids[n];

      if (CC_id >= MAX_NUM_CCs) {
        LOG_E( MAC, "CC_id %u should be < %u, loop n=%u < numactiveULCCs[%u]=%u",
               CC_id,
               MAX_NUM_CCs,
               n,
               UE_id,
               UE_info->numactiveULCCs);
      }

      AssertFatal( CC_id < MAX_NUM_CCs, "CC_id %u should be < %u, loop n=%u < numactiveULCCs[%u]=%u",
                   CC_id,
                   MAX_NUM_CCs,
                   n,
                   UE_id,
                   UE_info->numactiveULCCs);
      frame_parms=mac_xface->get_lte_frame_parms(module_idP,CC_id);
      UE_template = &UE_info->UE_template[CC_id];

      // if this UE has UL traffic
      if (UE_template->ul_total_buffer[subframeP] > 0 ) {

        tbs = mac_xface->get_TBS_UL(mcs,3);  // 1 or 2 PRB with cqi enabled does not work well!
        // fixme: set use_srs flag
        tx_power= mac_xface->estimate_ue_tx_power(tbs,rb_table[rb_table_index],0,frame_parms->Ncp,0);

        while ((((UE_template->phr_info - tx_power) < 0 ) || (tbs > UE_template->ul_total_buffer[subframeP]))&&
               (mcs > 3)) {
          // LOG_I(MAC,"UE_template->phr_info %d tx_power %d mcs %d\n", UE_template->phr_info,tx_power, mcs);
          mcs--;
          tbs = mac_xface->get_TBS_UL(mcs,rb_table[rb_table_index]);
          tx_power = mac_xface->estimate_ue_tx_power(tbs,rb_table[rb_table_index],0,frame_parms->Ncp,0); // fixme: set use_srs
        }

        while ((tbs < UE_template->ul_total_buffer[subframeP]) &&
               (rb_table[rb_table_index]<(frame_parms->N_RB_UL-first_rb[CC_id])) &&
               ((UE_template->phr_info - tx_power) > 0) &&
               (rb_table_index < 32 )) {
          //  LOG_I(MAC,"tbs %d ul buffer %d rb table %d max ul rb %d\n", tbs, UE_template->ul_total_buffer, rb_table[rb_table_index], frame_parms->N_RB_UL-first_rb[CC_id]);
          rb_table_index++;
          tbs = mac_xface->get_TBS_UL(mcs,rb_table[rb_table_index]);
          tx_power = mac_xface->estimate_ue_tx_power(tbs,rb_table[rb_table_index],0,frame_parms->Ncp,0);
        }

        UE_template->ue_tx_power = tx_power;

        if (rb_table[rb_table_index]>(frame_parms->N_RB_UL-first_rb[CC_id]-1)) {
          rb_table_index--;
        }

        // 1 or 2 PRB with cqi enabled does not work well!
	    if (rb_table[rb_table_index]<3) 
        {
          rb_table_index=2; //3PRB
        }
        //xhd_oai_60m
	    if (rb_table[rb_table_index]<10) 
        {
          //rb_table_index=10; //3PRB
        }
        //xhd_oai_20users
	    if (rb_table[rb_table_index]<6) 
        {
          //rb_table_index=5; //3PRB
        }

        UE_template->pre_assigned_mcs_ul[subframeP]=mcs;
        UE_template->pre_allocated_rb_table_index_ul[subframeP]=rb_table_index;
        UE_template->pre_allocated_nb_rb_ul[subframeP]= rb_table[rb_table_index];
        LOG_D(MAC,"[eNB %d] frame %d subframe %d: for UE %d CC %d: pre-assigned mcs %d, pre-allocated rb_table[%d]=%d RBs (phr %d, tx power %d)\n",
              module_idP, frameP, subframeP, UE_id, CC_id,
              UE_template->pre_assigned_mcs_ul,
              UE_template->pre_allocated_rb_table_index_ul[subframeP],
              UE_template->pre_allocated_nb_rb_ul[subframeP],
              UE_template->phr_info,tx_power);
      } 
      else 
      {
        UE_template->pre_allocated_rb_table_index_ul[subframeP] =-1;
        UE_template->pre_allocated_nb_rb_ul[subframeP] =0;
      }
    }
  }
}
