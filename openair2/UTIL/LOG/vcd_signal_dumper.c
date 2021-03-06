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

/*! \file vcd_signal_dumper.c
 * \brief Dump functions calls and variables to VCD file. Use GTKWave to display this file.
 * \author S. Roux
 * \maintainer: navid nikaein
 * \date 2012 - 2104
 * \version 0.1
 * \company Eurecom
 * \email: navid.nikaein@eurecom.fr
 * \note
 * \warning
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <error.h>
#include <time.h>
#include <unistd.h>

#include "assertions.h"
#include "signals.h"

#include "vcd_signal_dumper.h"

#if defined(ENABLE_RTAI_CLOCK)
#include "rtai_lxrt.h"
#endif

#define VCDSIGNALDUMPER_VERSION_MAJOR 0
#define VCDSIGNALDUMPER_VERSION_MINOR 1

// Global variable. If the VCD option is set at execution time, output VCD trace. Otherwise this module has no effect.
int ouput_vcd = 0;

struct vcd_module_s {
  const char     *name;
  int             number_of_signals;
  const char    **signals_names;
  vcd_signal_type signal_type;
  int             signal_size;
} vcd_module_s;

const char* eurecomVariablesNames[] = {
  "frame_number_TX_eNB",
  "frame_number_RX_eNB",
  "runtime_TX_eNB",
  "runtime_RX_eNB",
  "frame_number_TX_UE",
  "frame_number_RX_UE",
  "slot_number_TX_UE",
  "slot_number_RX_UE",
  "subframe_number_TX_UE",
  "subframe_number_RX_UE",
  "missed_slot_enb",
  "daq_mbox",
  "rx_offset_mbox",
  "ue_rx_offset",
  "diff2",
  "hw_subframe",
  "hw_frame",
  "hw_subframe_rx",
  "hw_frame_rx",
  "txcnt",
  "rxcnt",
  "trx_ts",
  "trx_tst",
  "tx_ts",
  "rx_ts",
  "hw_cnt_rx",
  "lhw_cnt_rx",
  "hw_cnt_tx",
  "lhw_cnt_tx",
  "pck_rx",
  "pck_tx",
  "cnt",
  "dummy_dump",
  "itti_send_msg",
  "itti_poll_msg",
  "itti_recv_msg",
  "itti_alloc_msg",
  "mp_alloc",
  "mp_free",
  "ue_inst_cnt_rx",
  "ue_inst_cnt_tx"
};

const char* eurecomFunctionsNames[] = {
  /*  softmodem signals   */
  "rt_sleep",
  "trx_read",
  "trx_write",
  "eNB_thread_tx0",
  "eNB_thread_rx0",
  "eNB_thread_tx1",
  "eNB_thread_rx1",
  "eNB_thread_tx2",
  "eNB_thread_rx2",
  "eNB_thread_tx3",
  "eNB_thread_rx3",
  "eNB_thread_tx4",
  "eNB_thread_rx4",
  "eNB_thread_tx5",
  "eNB_thread_rx5",
  "eNB_thread_tx6",
  "eNB_thread_rx6",
  "eNB_thread_tx7",
  "eNB_thread_rx7",
  "eNB_thread_tx8",
  "eNB_thread_rx8",
  "eNB_thread_tx9",
  "eNB_thread_rx9",
  "ue_thread_tx",
  "ue_thread_rx",

 /* RRH signals  */ 
  "eNB_tx",
  "eNB_rx",
  "eNB_trx",
  "eNB_tm",
  "eNB_rx_sleep",
  "eNB_tx_sleep",
  "eNB_proc_sleep",
  "trx_read_rf",
  "trx_write_rf",

  /* PHY signals  */
  "ue_synch",
  "ue_slot_fep",
  "ue_rrc_measurements",
  "ue_gain_control",
  "ue_adjust_synch",
  "lte_ue_measurement_procedures",
  "lte_ue_pdcch_procedures",
  "lte_ue_pbch_procedures",
  "phy_procedures_eNb_tx",
  "phy_procedures_eNb_rx",
  "phy_procedures_ue_tx",
  "phy_procedures_ue_rx",
  "phy_procedures_eNB_lte",
  "phy_procedures_UE_lte",
  "pdsch_thread",
  "dlsch_thread0",
  "dlsch_thread1",
  "dlsch_thread2",
  "dlsch_thread3",
  "dlsch_thread4",
  "dlsch_thread5",
  "dlsch_thread6",
  "dlsch_thread7",
  "dlsch_decoding0",
  "dlsch_decoding1",
  "dlsch_decoding2",
  "dlsch_decoding3",
  "dlsch_decoding4",
  "dlsch_decoding5",
  "dlsch_decoding6",
  "dlsch_decoding7",
  "rx_pdcch",
  "dci_decoding",
  "rx_phich",
  "phy_ue_config_sib2",
  "macxface_phy_config_sib1_eNB",
  "macxface_phy_config_sib2_eNB",
  "macxface_phy_config_dedicated_eNB",
  "phy_ue_compute_prach",
  "phy_enb_ulsch_decoding",
  "phy_enb_sfgen",
  "phy_enb_prach_rx",
  "phy_enb_pdcch_tx",
  "phy_enb_rs_tx",
  "phy_ue_generate_prach",
  "phy_ue_ulsch_modulation",
  "phy_ue_ulsch_encoding",
  "phy_ue_ulsch_scrambling",
  "phy_eNB_dlsch_modulation",
  "phy_eNB_dlsch_encoding",
  "phy_eNB_dlsch_scrambling",

  /* MAC  signals  */
  "macxface_macphy_init",
  "macxface_macphy_exit",
  "macxface_eNB_dlsch_ulsch_scheduler",
  "macxface_fill_rar",
  "macxface_terminate_ra_proc",
  "macxface_initiate_ra_proc",
  "macxface_cancel_ra_proc",
  "macxface_get_dci_sdu",
  "macxface_get_dlsch_sdu",
  "macxface_rx_sdu",
  "macxface_mrbch_phy_sync_failure",
  "macxface_SR_indication",
  "mac_dlsch_preprocessor",
  "mac_schedule_dlsch",
  "mac_fill_dlsch_dci",

  "macxface_out_of_sync_ind",
  "macxface_ue_decode_si",
  "macxface_ue_decode_ccch",
  "macxface_ue_decode_bcch",
  "macxface_ue_send_sdu",
  "macxface_ue_get_sdu",
  "macxface_ue_get_rach",
  "macxface_ue_process_rar",
  "macxface_ue_scheduler",
  "macxface_ue_get_sr",

  "ue_send_mch_sdu",

  /*RLC signals   */
  "rlc_data_req",
  // "rlc_data_ind", // this calls "pdcp_data_ind",
  "mac_rlc_status_ind",
  "mac_rlc_data_req",
  "mac_rlc_data_ind",
  "rlc_um_try_reassembly",
  "rlc_um_check_timer_dar_time_out",
  "rlc_um_receive_process_dar",

  /* PDCP signals   */
  "pdcp_run",
  "pdcp_data_req",
  "pdcp_data_ind",
  "pdcp_apply_security",
  "pdcp_validate_security",
  /* RRC signals  */
  "rrc_rx_tx",
  "rrc_mac_config_req",
  "rrc_ue_decode_sib1",
  "rrc_ue_decode_si",
  /* GTPV1U signals */
  "gtpv1u_enb_task",
  "gtpv1u_process_udp_req",
  "gtpv1u_process_tunnel_data_req",
  /* UDP signals */
  "udp_enb_task",
  /* MISC signals  */
  "emu_transport",
  "log_record",
  "itti_enqueue_message",
  "itti_dump_enqueue_message",
  "itti_dump_enqueue_message_malloc",
  "itti_relay_thread",
  "test"
};

struct vcd_module_s vcd_modules[VCD_SIGNAL_DUMPER_MODULE_END] = {
  { "variables", VCD_SIGNAL_DUMPER_VARIABLES_END, eurecomVariablesNames, VCD_WIRE, 64 },
  { "functions", VCD_SIGNAL_DUMPER_FUNCTIONS_END, eurecomFunctionsNames, VCD_WIRE, 1 },
  //    { "ue_procedures_functions", VCD_SIGNAL_DUMPER_UE_PROCEDURES_FUNCTIONS_END, eurecomUEFunctionsNames, VCD_WIRE, 1 },
};

FILE *vcd_fd = NULL;
static inline unsigned long long int vcd_get_time(void);

#if defined(ENABLE_USE_CPU_EXECUTION_TIME)
struct timespec     g_time_start;
#elif defined(ENABLE_RTAI_CLOCK)
RTIME start;
#endif


#if defined(ENABLE_VCD_FIFO)

# define VCD_POLL_DELAY         (500)           // Poll delay in micro-seconds
# define VCD_MAX_WAIT_DELAY     (200 * 1000)    // Maximum data ready wait delay in micro-seconds
# define VCD_FIFO_NB_ELEMENTS   (1 << 24)       // Must be a power of 2
# define VCD_FIFO_MASK          (VCD_FIFO_NB_ELEMENTS - 1)

typedef struct vcd_queue_user_data_s {
  uint32_t log_id;
  vcd_signal_dumper_modules module;
  union data_u {
    struct function_s {
      vcd_signal_dump_functions function_name;
      vcd_signal_dump_in_out    in_out;
    } function;
    struct variable_s {
      vcd_signal_dump_variables variable_name;
      unsigned long value;
    } variable;
  } data;

  long long unsigned int time;
} vcd_queue_user_data_t;

typedef struct vcd_fifo_s {
  vcd_queue_user_data_t user_data[VCD_FIFO_NB_ELEMENTS];

  volatile uint32_t write_index;
  volatile uint32_t read_index;
} vcd_fifo_t;

vcd_fifo_t vcd_fifo;

pthread_t vcd_dumper_thread;
#endif

#define BYTE_SIZE   8
#define NIBBLE_SIZE 4
static void uint64_to_binary(uint64_t value, char *binary)
{
  static const char * const nibbles_start[] = {
    "",    "1",   "10",   "11",
    "100",  "101",  "110",  "111",
    "1000", "1001", "1010", "1011",
    "1100", "1101", "1110", "1111",
  };
  static const char * const nibbles[] = {
    "0000", "0001", "0010", "0011",
    "0100", "0101", "0110", "0111",
    "1000", "1001", "1010", "1011",
    "1100", "1101", "1110", "1111",
  };
  int nibble;
  int nibble_value;
  int nibble_size;
  int zero = 1;

  for (nibble = 0; nibble < (sizeof (uint64_t) * (BYTE_SIZE / NIBBLE_SIZE)); nibble++) {
    nibble_value = value >> ((sizeof (uint64_t) * BYTE_SIZE) - NIBBLE_SIZE);

    if (zero) {
      if (nibble_value > 0) {
        zero = 0;
        nibble_size = strlen(nibbles_start[nibble_value]);
        memcpy (binary, nibbles_start[nibble_value], nibble_size);
        binary += nibble_size;
      }
    } else {
      memcpy (binary, nibbles[nibble_value], NIBBLE_SIZE);
      binary += NIBBLE_SIZE;
    }

    value <<= NIBBLE_SIZE;
  }

  /* Add a '0' if the value was null */
  if (zero) {
    binary[0] = '0';
    binary ++;
  }

  /* Add a null value at the end of the string */
  binary[0] = '\0';
}

#if defined(ENABLE_VCD_FIFO)
inline static uint32_t vcd_get_write_index(void)
{
  uint32_t write_index;
  uint32_t read_index;

  /* Get current write index and increment it (atomic operation) */
  write_index = __sync_fetch_and_add(&vcd_fifo.write_index, 1);
  /* Wrap index */
  write_index &= VCD_FIFO_MASK;

  /* Check FIFO overflow (increase VCD_FIFO_NB_ELEMENTS if this assert is triggered) */
  DevCheck((read_index = vcd_fifo.read_index, ((write_index + 1) & VCD_FIFO_MASK) != read_index), write_index, read_index, VCD_FIFO_NB_ELEMENTS);

  return write_index;
}

void *vcd_dumper_thread_rt(void *args)
{
  vcd_queue_user_data_t *data;
  char binary_string[(sizeof (uint64_t) * BYTE_SIZE) + 1];
  struct sched_param sched_param;
  uint32_t data_ready_wait;

# if defined(ENABLE_ITTI)
  signal_mask();
# endif

  sched_param.sched_priority = sched_get_priority_min(SCHED_FIFO) + 1;
  sched_setscheduler(0, SCHED_FIFO, &sched_param);

  while(1) {
    if (vcd_fifo.read_index == (vcd_fifo.write_index & VCD_FIFO_MASK)) {
      /* No element -> sleep a while */
      usleep(VCD_POLL_DELAY);
    } else {
      data = &vcd_fifo.user_data[vcd_fifo.read_index];
      data_ready_wait = 0;

      while (data->module == VCD_SIGNAL_DUMPER_MODULE_FREE) {
        /* Check wait delay (increase VCD_MAX_WAIT_DELAY if this assert is triggered and that no thread is locked) */
        DevCheck(data_ready_wait < VCD_MAX_WAIT_DELAY, data_ready_wait, VCD_MAX_WAIT_DELAY, 0);

        /* data is not yet ready, wait for it to be completed */
        data_ready_wait += VCD_POLL_DELAY;
        usleep(VCD_POLL_DELAY);
      }

      switch (data->module) {
      case VCD_SIGNAL_DUMPER_MODULE_VARIABLES:
        if (vcd_fd != NULL) {
          int variable_name;
          variable_name = (int)data->data.variable.variable_name;
          fprintf(vcd_fd, "#%llu\n", data->time);
          /* Set variable to value */
          uint64_to_binary(data->data.variable.value, binary_string);
          fprintf(vcd_fd, "b%s %s_w\n", binary_string,
                  eurecomVariablesNames[variable_name]);
        }

        break;

      case VCD_SIGNAL_DUMPER_MODULE_FUNCTIONS:
        if (vcd_fd != NULL) {
          int function_name;

          function_name = (int)data->data.function.function_name;
          fprintf(vcd_fd, "#%llu\n", data->time);

          /* Check if we are entering or leaving the function ( 0 = leaving, 1 = entering) */
          if (data->data.function.in_out == VCD_FUNCTION_IN)
            /* Set event to 1 */
            fprintf(vcd_fd, "1%s_w\n", eurecomFunctionsNames[function_name]);
          else
            fprintf(vcd_fd, "0%s_w\n", eurecomFunctionsNames[function_name]);

          fflush(vcd_fd);
        }

        break;

      default:
        DevParam(data->module, 0, 0);
        break;
      }

      data->module = VCD_SIGNAL_DUMPER_MODULE_FREE;
      vcd_fifo.read_index = (vcd_fifo.read_index + 1) & VCD_FIFO_MASK;
    }
  }

  return NULL;
}
#endif

void vcd_signal_dumper_init(char *filename)
{
  if (ouput_vcd) {
    //        char filename[] = "/tmp/openair_vcd_dump.vcd";

    if ((vcd_fd = fopen(filename, "w+")) == NULL) {
      perror("vcd_signal_dumper_init: cannot open file");
      return;
    }

#if defined(ENABLE_USE_CPU_EXECUTION_TIME)
    clock_gettime(CLOCK_MONOTONIC, &g_time_start);
#elif defined(ENABLE_RTAI_CLOCK)
    start=rt_get_time_ns();
#endif

    vcd_signal_dumper_create_header();

#if defined(ENABLE_VCD_FIFO)
    vcd_fifo.write_index = 0;
    vcd_fifo.read_index = 0;

    fprintf(stderr, "[VCD] Creating dumper thread\n");

    if (pthread_create(&vcd_dumper_thread, NULL, vcd_dumper_thread_rt, NULL) < 0) {
      fprintf(stderr, "vcd_signal_dumper_init: Failed to create thread: %s\n",
              strerror(errno));
      ouput_vcd = 0;
      return;
    }

#endif
  }
}

void vcd_signal_dumper_close(void)
{
  if (ouput_vcd) {
#if defined(ENABLE_VCD_FIFO)

#else

    if (vcd_fd != NULL) {
      fclose(vcd_fd);
      vcd_fd = NULL;
    }

#endif
  }
}

static inline void vcd_signal_dumper_print_time_since_start(void)
{
  if (vcd_fd != NULL) {
#if defined(ENABLE_USE_CPU_EXECUTION_TIME)
    struct timespec time;
    long long unsigned int nanosecondsSinceStart;
    long long unsigned int secondsSinceStart;

    clock_gettime(CLOCK_MONOTONIC, &time);

    /* Get current execution time in nanoseconds */
    nanosecondsSinceStart = (long long unsigned int)((time.tv_nsec - g_time_start.tv_nsec));
    secondsSinceStart     = (long long unsigned int)time.tv_sec - (long long unsigned int)g_time_start.tv_sec;
    /* Write time in nanoseconds */
    fprintf(vcd_fd, "#%llu\n", nanosecondsSinceStart + (secondsSinceStart * 1000000000UL));
#elif defined(ENABLE_RTAI_CLOCK)
    /* Write time in nanoseconds */
    fprintf(vcd_fd, "#%llu\n",rt_get_time_ns()-start);
#endif
  }
}

static inline unsigned long long int vcd_get_time(void)
{
#if defined(ENABLE_USE_CPU_EXECUTION_TIME)
  struct timespec time;

  clock_gettime(CLOCK_MONOTONIC, &time);

  return (long long unsigned int)((time.tv_nsec - g_time_start.tv_nsec)) +
         ((long long unsigned int)time.tv_sec - (long long unsigned int)g_time_start.tv_sec) * 1000000000UL;
#elif defined(ENABLE_RTAI_CLOCK)
  return rt_get_time_ns() - start;
#endif
}

void vcd_signal_dumper_create_header(void)
{
  if (ouput_vcd) {
    struct tm *pDate;
    time_t intps;

    intps = time(NULL);
    pDate = localtime(&intps);

    if (vcd_fd != NULL) {
      int i, j;
      fprintf(vcd_fd, "$date\n\t%s$end\n", asctime(pDate));
      // Display version
      fprintf(vcd_fd, "$version\n\tVCD plugin ver%d.%d\n$end\n", VCDSIGNALDUMPER_VERSION_MAJOR, VCDSIGNALDUMPER_VERSION_MINOR);
      // Init timescale, here = 1ns
      fprintf(vcd_fd, "$timescale 1 ns $end\n");

      /* Initialize each module definition */
      for(i = 0; i < VCD_SIGNAL_DUMPER_MODULE_END; i++) {
        struct vcd_module_s *module;
        module = &vcd_modules[i];
        fprintf(vcd_fd, "$scope module %s $end\n", module->name);

        /* Declare each signal as defined in array */
        for (j = 0; j < module->number_of_signals; j++) {
          const char *signal_name;
          signal_name = module->signals_names[j];

          if (VCD_WIRE == module->signal_type) {
            fprintf(vcd_fd, "$var wire %d %s_w %s $end\n", module->signal_size, signal_name, signal_name);
          } else  if (VCD_REAL == module->signal_type) {
            fprintf(vcd_fd, "$var real %d %s_r %s $end\n", module->signal_size, signal_name, signal_name);
          } else {
            // Handle error here
          }
        }

        fprintf(vcd_fd, "$upscope $end\n");
      }

      /* Init variables and functions to 0 */
      fprintf(vcd_fd, "$dumpvars\n");

      for(i = 0; i < VCD_SIGNAL_DUMPER_MODULE_END; i++) {
        struct vcd_module_s *module;
        module = &vcd_modules[i];

        /* Declare each signal as defined in array */
        for (j = 0; j < module->number_of_signals; j++) {
          const char *signal_name;
          signal_name = module->signals_names[j];

          if (VCD_WIRE == module->signal_type) {
            if (module->signal_size > 1) {
              fprintf(vcd_fd, "b0 %s_w $end\n", signal_name);
            } else {
              fprintf(vcd_fd, "0%s_w $end\n", signal_name);
            }
          } else  if (VCD_REAL == module->signal_type) {
            fprintf(vcd_fd, "r0 %s_r $end\n", signal_name);
          } else {
            // Handle error here
          }
        }
      }

      fprintf(vcd_fd, "$end\n");
      fprintf(vcd_fd, "$enddefinitions $end\n\n");
      //fflush(vcd_fd);
    }
  }
}

void vcd_signal_dumper_dump_variable_by_name(vcd_signal_dump_variables variable_name,
    unsigned long             value)
{
  DevCheck((0 <= variable_name) && (variable_name < VCD_SIGNAL_DUMPER_VARIABLES_END),
           variable_name, VCD_SIGNAL_DUMPER_VARIABLES_END, 0);

  if (ouput_vcd) {
#if defined(ENABLE_VCD_FIFO)
    uint32_t write_index = vcd_get_write_index();

    vcd_fifo.user_data[write_index].time = vcd_get_time();
    vcd_fifo.user_data[write_index].data.variable.variable_name = variable_name;
    vcd_fifo.user_data[write_index].data.variable.value = value;
    vcd_fifo.user_data[write_index].module = VCD_SIGNAL_DUMPER_MODULE_VARIABLES; // Set when all other fields are set to validate the user_data
#else
    char binary_string[(sizeof (uint64_t) * BYTE_SIZE) + 1];

    if (vcd_fd != NULL) {
      vcd_signal_dumper_print_time_since_start();

      /* Set variable to value */
      uint64_to_binary(value, binary_string);
      fprintf(vcd_fd, "b%s %s_w\n", binary_string, eurecomVariablesNames[variable_name]);
      //fflush(vcd_fd);
    }

#endif
  }
}

void vcd_signal_dumper_dump_function_by_name(vcd_signal_dump_functions  function_name,
    vcd_signal_dump_in_out     in_out)
{
  DevCheck((0 <= function_name) && (function_name < VCD_SIGNAL_DUMPER_FUNCTIONS_END),
           function_name, VCD_SIGNAL_DUMPER_FUNCTIONS_END, 0);

  if (ouput_vcd) {
#if defined(ENABLE_VCD_FIFO)
    uint32_t write_index = vcd_get_write_index();

    vcd_fifo.user_data[write_index].time = vcd_get_time();
    vcd_fifo.user_data[write_index].data.function.function_name = function_name;
    vcd_fifo.user_data[write_index].data.function.in_out = in_out;
    vcd_fifo.user_data[write_index].module = VCD_SIGNAL_DUMPER_MODULE_FUNCTIONS; // Set when all other fields are set to validate the user_data
#else

    if (vcd_fd != NULL) {
      vcd_signal_dumper_print_time_since_start();

      /* Check if we are entering or leaving the function ( 0 = leaving, 1 = entering) */
      if (in_out == VCD_FUNCTION_IN)
        /* Set event to 1 */
        fprintf(vcd_fd, "1%s_w\n", eurecomFunctionsNames[function_name]);
      else
        fprintf(vcd_fd, "0%s_w\n", eurecomFunctionsNames[function_name]);

      //fflush(vcd_fd);
    }

#endif
  }
}

