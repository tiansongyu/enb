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

/*! \file vcd_signal_dumper.h
 * \brief Output functions call to VCD file which is readable by gtkwave.
 * \author ROUX Sebastien
 * \author S. Roux
 * \maintainer: navid nikaein
 * \date 2012 - 2104
 * \version 0.1
 * \company Eurecom
 * \email: navid.nikaein@eurecom.fr
 * \note
 * \warning
 */

#ifndef VCD_SIGNAL_DUMPER_H_
#define VCD_SIGNAL_DUMPER_H_

//#define ENABLE_USE_CPU_EXECUTION_TIME

/* WARNING: if you edit the enums below, update also string definitions in vcd_signal_dumper.c */
typedef enum {
  VCD_SIGNAL_DUMPER_VARIABLES_FRAME_NUMBER_TX_ENB = 0,
  VCD_SIGNAL_DUMPER_VARIABLES_FRAME_NUMBER_RX_ENB,
  VCD_SIGNAL_DUMPER_VARIABLES_RUNTIME_TX_ENB,
  VCD_SIGNAL_DUMPER_VARIABLES_RUNTIME_RX_ENB,
  VCD_SIGNAL_DUMPER_VARIABLES_FRAME_NUMBER_TX_UE,
  VCD_SIGNAL_DUMPER_VARIABLES_FRAME_NUMBER_RX_UE,
  VCD_SIGNAL_DUMPER_VARIABLES_SLOT_NUMBER_TX_UE,
  VCD_SIGNAL_DUMPER_VARIABLES_SLOT_NUMBER_RX_UE,
  VCD_SIGNAL_DUMPER_VARIABLES_SUBFRAME_NUMBER_TX_UE,
  VCD_SIGNAL_DUMPER_VARIABLES_SUBFRAME_NUMBER_RX_UE,
  VCD_SIGNAL_DUMPER_VARIABLES_MISSED_SLOTS_ENB,
  VCD_SIGNAL_DUMPER_VARIABLES_DAQ_MBOX,
  VCD_SIGNAL_DUMPER_VARIABLES_UE_OFFSET_MBOX,
  VCD_SIGNAL_DUMPER_VARIABLES_UE_RX_OFFSET,
  VCD_SIGNAL_DUMPER_VARIABLES_DIFF,
  VCD_SIGNAL_DUMPER_VARIABLES_HW_SUBFRAME,
  VCD_SIGNAL_DUMPER_VARIABLES_HW_FRAME,
  VCD_SIGNAL_DUMPER_VARIABLES_HW_SUBFRAME_RX,
  VCD_SIGNAL_DUMPER_VARIABLES_HW_FRAME_RX,
  VCD_SIGNAL_DUMPER_VARIABLES_TXCNT,
  VCD_SIGNAL_DUMPER_VARIABLES_RXCNT,
  VCD_SIGNAL_DUMPER_VARIABLES_TRX_TS,
  VCD_SIGNAL_DUMPER_VARIABLES_TRX_TST,
  VCD_SIGNAL_DUMPER_VARIABLES_TX_TS,
  VCD_SIGNAL_DUMPER_VARIABLES_RX_TS,
  VCD_SIGNAL_DUMPER_VARIABLES_RX_HWCNT,
  VCD_SIGNAL_DUMPER_VARIABLES_RX_LHWCNT,
  VCD_SIGNAL_DUMPER_VARIABLES_TX_HWCNT,
  VCD_SIGNAL_DUMPER_VARIABLES_TX_LHWCNT,
  VCD_SIGNAL_DUMPER_VARIABLES_RX_PCK,
  VCD_SIGNAL_DUMPER_VARIABLES_TX_PCK,
  VCD_SIGNAL_DUMPER_VARIABLES_CNT,
  VCD_SIGNAL_DUMPER_VARIABLES_DUMMY_DUMP,
  VCD_SIGNAL_DUMPER_VARIABLE_ITTI_SEND_MSG,
  VCD_SIGNAL_DUMPER_VARIABLE_ITTI_POLL_MSG,
  VCD_SIGNAL_DUMPER_VARIABLE_ITTI_RECV_MSG,
  VCD_SIGNAL_DUMPER_VARIABLE_ITTI_ALLOC_MSG,
  VCD_SIGNAL_DUMPER_VARIABLE_MP_ALLOC,
  VCD_SIGNAL_DUMPER_VARIABLE_MP_FREE,
  VCD_SIGNAL_DUMPER_VARIABLES_UE_INST_CNT_RX,
  VCD_SIGNAL_DUMPER_VARIABLES_UE_INST_CNT_TX,
  VCD_SIGNAL_DUMPER_VARIABLES_LAST,
  VCD_SIGNAL_DUMPER_VARIABLES_END = VCD_SIGNAL_DUMPER_VARIABLES_LAST,
} vcd_signal_dump_variables;

typedef enum {
  /* softmodem signals  */
  VCD_SIGNAL_DUMPER_FUNCTIONS_RT_SLEEP=0,
  VCD_SIGNAL_DUMPER_FUNCTIONS_TRX_READ,
  VCD_SIGNAL_DUMPER_FUNCTIONS_TRX_WRITE,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_TX0,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_RX0,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_TX1,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_RX1,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_TX2,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_RX2,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_TX3,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_RX3,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_TX4,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_RX4,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_TX5,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_RX5,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_TX6,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_RX6,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_TX7,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_RX7,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_TX8,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_RX8,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_TX9,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_RX9,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_THREAD_TX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_THREAD_RX,

  /* RRH signals  */ 
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_TX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_RX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_TRX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_TM,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_RX_SLEEP,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_TX_SLEEP,
  VCD_SIGNAL_DUMPER_FUNCTIONS_eNB_PROC_SLEEP,
  VCD_SIGNAL_DUMPER_FUNCTIONS_TRX_READ_RF,
  VCD_SIGNAL_DUMPER_FUNCTIONS_TRX_WRITE_RF,

  /* PHY signals  */
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_SYNCH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_SLOT_FEP,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_RRC_MEASUREMENTS,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GAIN_CONTROL,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ADJUST_SYNCH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_MEASUREMENT_PROCEDURES,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PDCCH_PROCEDURES,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PBCH_PROCEDURES,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_ENB_TX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_ENB_RX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_TX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_RX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_ENB_LTE,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_PROCEDURES_UE_LTE,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PDSCH_THREAD,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_THREAD0,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_THREAD1,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_THREAD2,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_THREAD3,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_THREAD4,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_THREAD5,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_THREAD6,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_THREAD7,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_DECODING0,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_DECODING1,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_DECODING2,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_DECODING3,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_DECODING4,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_DECODING5,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_DECODING6,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_DECODING7,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RX_PDCCH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DCI_DECODING,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RX_PHICH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_UE_CONFIG_SIB2,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_CONFIG_SIB1_ENB,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_CONFIG_SIB2_ENB,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_CONFIG_DEDICATED_ENB,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_UE_COMPUTE_PRACH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_ULSCH_DECODING,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_SFGEN,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_PRACH_RX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_PDCCH_TX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PHY_ENB_RS_TX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GENERATE_PRACH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ULSCH_MODULATION,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ULSCH_ENCODING,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_ULSCH_SCRAMBLING,
  VCD_SIGNAL_DUMPER_FUNCTIONS_ENB_DLSCH_MODULATION,
  VCD_SIGNAL_DUMPER_FUNCTIONS_ENB_DLSCH_ENCODING,
  VCD_SIGNAL_DUMPER_FUNCTIONS_ENB_DLSCH_SCRAMBLING,

  /* MAC signals  */
  VCD_SIGNAL_DUMPER_FUNCTIONS_MACPHY_INIT,
  VCD_SIGNAL_DUMPER_FUNCTIONS_MACPHY_EXIT,
  VCD_SIGNAL_DUMPER_FUNCTIONS_ENB_DLSCH_ULSCH_SCHEDULER,
  VCD_SIGNAL_DUMPER_FUNCTIONS_FILL_RAR,
  VCD_SIGNAL_DUMPER_FUNCTIONS_TERMINATE_RA_PROC,
  VCD_SIGNAL_DUMPER_FUNCTIONS_INITIATE_RA_PROC,
  VCD_SIGNAL_DUMPER_FUNCTIONS_CANCEL_RA_PROC,
  VCD_SIGNAL_DUMPER_FUNCTIONS_GET_DCI_SDU,
  VCD_SIGNAL_DUMPER_FUNCTIONS_GET_DLSCH_SDU,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RX_SDU,
  VCD_SIGNAL_DUMPER_FUNCTIONS_MRBCH_PHY_SYNC_FAILURE,
  VCD_SIGNAL_DUMPER_FUNCTIONS_SR_INDICATION,
  VCD_SIGNAL_DUMPER_FUNCTIONS_DLSCH_PREPROCESSOR,
  VCD_SIGNAL_DUMPER_FUNCTIONS_SCHEDULE_DLSCH, // schedule_ue_spec
  VCD_SIGNAL_DUMPER_FUNCTIONS_FILL_DLSCH_DCI,

  VCD_SIGNAL_DUMPER_FUNCTIONS_OUT_OF_SYNC_IND,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_DECODE_SI,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_DECODE_CCCH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_DECODE_BCCH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_SEND_SDU,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GET_SDU,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GET_RACH,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_PROCESS_RAR,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_SCHEDULER,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_GET_SR,
  VCD_SIGNAL_DUMPER_FUNCTIONS_UE_SEND_MCH_SDU,

  /* RLC signals  */
  VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_DATA_REQ,
  // VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_DATA_IND,
  VCD_SIGNAL_DUMPER_FUNCTIONS_MAC_RLC_STATUS_IND,
  VCD_SIGNAL_DUMPER_FUNCTIONS_MAC_RLC_DATA_REQ,
  VCD_SIGNAL_DUMPER_FUNCTIONS_MAC_RLC_DATA_IND,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_UM_TRY_REASSEMBLY,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_UM_CHECK_TIMER_DAR_TIME_OUT,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RLC_UM_RECEIVE_PROCESS_DAR,

  /* PDCP signals  */
  VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_RUN,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_DATA_REQ,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_DATA_IND,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_APPLY_SECURITY,
  VCD_SIGNAL_DUMPER_FUNCTIONS_PDCP_VALIDATE_SECURITY,

  /* RRC signals  */
  VCD_SIGNAL_DUMPER_FUNCTIONS_RRC_RX_TX,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RRC_MAC_CONFIG,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RRC_UE_DECODE_SIB1,
  VCD_SIGNAL_DUMPER_FUNCTIONS_RRC_UE_DECODE_SI,

  /* GTPV1U signals */
  VCD_SIGNAL_DUMPER_FUNCTIONS_GTPV1U_ENB_TASK,
  VCD_SIGNAL_DUMPER_FUNCTIONS_GTPV1U_PROCESS_UDP_REQ,
  VCD_SIGNAL_DUMPER_FUNCTIONS_GTPV1U_PROCESS_TUNNEL_DATA_REQ,

  /* UDP signals */
  VCD_SIGNAL_DUMPER_FUNCTIONS_UDP_ENB_TASK,

  /* MISC signals  */
  VCD_SIGNAL_DUMPER_FUNCTIONS_EMU_TRANSPORT,
  VCD_SIGNAL_DUMPER_FUNCTIONS_LOG_RECORD,
  VCD_SIGNAL_DUMPER_FUNCTIONS_ITTI_ENQUEUE_MESSAGE,
  VCD_SIGNAL_DUMPER_FUNCTIONS_ITTI_DUMP_ENQUEUE_MESSAGE,
  VCD_SIGNAL_DUMPER_FUNCTIONS_ITTI_DUMP_ENQUEUE_MESSAGE_MALLOC,
  VCD_SIGNAL_DUMPER_FUNCTIONS_ITTI_RELAY_THREAD,
  VCD_SIGNAL_DUMPER_FUNCTIONS_TEST,
  VCD_SIGNAL_DUMPER_FUNCTIONS_LAST,
  VCD_SIGNAL_DUMPER_FUNCTIONS_END = VCD_SIGNAL_DUMPER_FUNCTIONS_LAST,
} vcd_signal_dump_functions;

typedef enum {
  VCD_SIGNAL_DUMPER_MODULE_FREE = 0,

  VCD_SIGNAL_DUMPER_MODULE_VARIABLES,
  VCD_SIGNAL_DUMPER_MODULE_FUNCTIONS,
  //     VCD_SIGNAL_DUMPER_MODULE_UE_PROCEDURES_FUNCTIONS,
  VCD_SIGNAL_DUMPER_MODULE_LAST,
  VCD_SIGNAL_DUMPER_MODULE_END = VCD_SIGNAL_DUMPER_MODULE_LAST,
} vcd_signal_dumper_modules;

typedef enum {
  VCD_FUNCTION_OUT,
  VCD_FUNCTION_IN,
  VCD_FUNCTION_LAST,
} vcd_signal_dump_in_out;

typedef enum {
  VCD_REAL, // REAL = variable
  VCD_WIRE, // WIRE = Function
} vcd_signal_type;

/*!
 * \brief Init function for the vcd dumper.
 * @param None
 */
void vcd_signal_dumper_init(char* filename);
/*!
 * \brief Close file descriptor.
 * @param None
 */
void vcd_signal_dumper_close(void);
/*!
 * \brief Create header for VCD file.
 * @param None
 */
void vcd_signal_dumper_create_header(void);
/*!
 * \brief Dump state of a variable
 * @param Name of the variable to dump (see the corresponding enum)
 * @param Value of the variable to dump (type: unsigned long)
 */
void vcd_signal_dumper_dump_variable_by_name(vcd_signal_dump_variables variable_name,
    unsigned long             value);
/*!
 * \brief Dump function usage
 * @param Name Function name to dump (see the corresponding enum)
 * @param State: either VCD_FUNCTION_START or VCD_FUNCTION_END
 */
void vcd_signal_dumper_dump_function_by_name(vcd_signal_dump_functions  function_name,
    vcd_signal_dump_in_out     function_in_out);

extern int ouput_vcd;


#if defined(ENABLE_VCD)
#define VCD_SIGNAL_DUMPER_INIT(aRgUmEnT)
#define VCD_SIGNAL_DUMPER_CLOSE()
#define VCD_SIGNAL_DUMPER_CREATE_HEADER()
#define VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME(vAr1,vAr2)
#define VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(vAr1,vAr2)
#if 0
   #define VCD_SIGNAL_DUMPER_INIT(aRgUmEnT)                   vcd_signal_dumper_init(aRgUmEnT)
   #define VCD_SIGNAL_DUMPER_CLOSE()                          vcd_signal_dumper_close()
   #define VCD_SIGNAL_DUMPER_CREATE_HEADER()                  vcd_signal_dumper_create_header()
   #define VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME(vAr1,vAr2) vcd_signal_dumper_dump_variable_by_name(vAr1,vAr2)
   #define VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(vAr1,vAr2) vcd_signal_dumper_dump_function_by_name(vAr1,vAr2)
#endif
#else
   #define VCD_SIGNAL_DUMPER_INIT(aRgUmEnT)
   #define VCD_SIGNAL_DUMPER_CLOSE()
   #define VCD_SIGNAL_DUMPER_CREATE_HEADER()
   #define VCD_SIGNAL_DUMPER_DUMP_VARIABLE_BY_NAME(vAr1,vAr2)
   #define VCD_SIGNAL_DUMPER_DUMP_FUNCTION_BY_NAME(vAr1,vAr2)
#endif

#endif /* !defined (VCD_SIGNAL_DUMPER_H_) */

