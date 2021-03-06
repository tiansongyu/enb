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

/*! \file rt_wrapper.h
* \brief provides a wrapper for the timing function, runtime calculations for real-time opeartions depending on weather RTAI or LOWLATENCY kernels are used or not
* \author F. Kaltenberger and Navid Nikaein
* \date 2013
* \version 0.1
* \company Eurecom
* \email: florian.kaltenberger@eurecom.fr, navid.nikaein@eurecom.fr
* \note
* \warning
*/

#include "rt_wrapper.h"
static int latency_target_fd = -1;
static int32_t latency_target_value = 0;
/* Latency trick - taken from cyclictest.c
 * if the file /dev/cpu_dma_latency exists,
 * open it and write a zero into it. This will tell
 * the power management system not to transition to
 * a high cstate (in fact, the system acts like idle=poll)
 * When the fd to /dev/cpu_dma_latency is closed, the behavior
 * goes back to the system default.
 *
 * Documentation/power/pm_qos_interface.txt
 */
void set_latency_target(void) {
  struct stat s;
  int ret;

  if (stat("/dev/cpu_dma_latency", &s) == 0) {
    latency_target_fd = open("/dev/cpu_dma_latency", O_RDWR);

    if (latency_target_fd == -1)
      return;

    ret = write(latency_target_fd, &latency_target_value, 4);

    if (ret == 0) {
      printf("# error setting cpu_dma_latency to %d!: %s\n", latency_target_value, strerror(errno));
      close(latency_target_fd);
      return;
    }

    printf("# /dev/cpu_dma_latency set to %dus\n", latency_target_value);
  }
}


#ifndef RTAI

struct timespec interval, next, now, res;
clockid_t clock_id = CLOCK_MONOTONIC; //other options are CLOCK_MONOTONIC, CLOCK_REALTIME, CLOCK_PROCESS_CPUTIME_ID, CLOCK_THREAD_CPUTIME_ID
RTIME rt_get_time_ns (void)
{
  clock_gettime(clock_id, &now);
  return(now.tv_sec*1e9+now.tv_nsec);
}

int rt_sleep_ns (RTIME x)
{
  int ret;
  clock_gettime(clock_id, &now);
  interval.tv_sec = x/((RTIME)1000000000);
  interval.tv_nsec = x%((RTIME)1000000000);
  //rt_printk("sleeping for %d sec and %d ns\n",interval.tv_sec,interval.tv_nsec);
  next = now;
  next.tv_sec += interval.tv_sec;
  next.tv_nsec += interval.tv_nsec;

  if (next.tv_nsec>=1000000000) {
    next.tv_nsec -= 1000000000;
    next.tv_sec++;
  }

  ret = clock_nanosleep(clock_id, TIMER_ABSTIME, &next, NULL);

  /*
  if (ret==EFAULT)
    rt_printk("rt_sleep_ns returned EFAULT (%d), reqested %d sec and %d ns\n",ret,next.tv_sec,next.tv_nsec);
  if (ret==EINVAL)
    rt_printk("rt_sleep_ns returned EINVAL (%d), reqested %d sec and %d ns\n",ret,next.tv_sec,next.tv_nsec);
  if (ret==EINTR)
    rt_printk("rt_sleep_ns returned EINTR (%d), reqested %d sec and %d ns\n",ret,next.tv_sec,next.tv_nsec);
  */

  return(ret);
}

void check_clock(void)
{
  if (clock_getres(clock_id, &res)) {
    printf("clock_getres failed");
  } else {
    printf("reported resolution = %llu ns\n", (long long int) ((int) 1e9 * res.tv_sec) + (long long int) res.tv_nsec);
  }
}

uint16_t cell_processing_dl[6]={10,15,24,42,80,112};
uint16_t platform_processing_dl=20; // upperbound for GPP, LXC, DOCKER and KVM
uint16_t user_processing_dl_a[6]={2,4,5,7,10,12};
uint16_t user_processing_dl_b[6]={10, 15, 25, 70, 110, 150};
uint16_t user_processing_dl_err[6]={20, 40, 60, 90, 120, 160};
uint16_t protocol_processing_dl[6]={150, 250, 350, 450, 650, 800}; // assumption: max MCS 27 --> gives an upper bound for the transport block size : to be measured 

uint16_t cell_processing_ul[6]={10,15,24,42,80,112};
uint16_t platform_processing_ul=30; // upperbound for GPP, LXC, DOCKER and KVM
uint16_t user_processing_ul_a[6]={5, 9, 12, 24, 33, 42};
uint16_t user_processing_ul_b[6]={20, 30, 40, 76, 140, 200};
uint16_t user_processing_ul_err[6]={15, 25, 32, 60, 80, 95};
uint16_t protocol_processing_ul[6]={100, 200, 300, 400, 550, 700}; // assumption: max MCS 16 --> gives an upper bound for the transport block size 

int fill_modeled_runtime_table(uint16_t runtime_phy_rx[29][6],
			       uint16_t runtime_phy_tx[29][6]){
  //double cpu_freq;
  //cpu_freq = get_cpu_freq_GHz();
  // target_dl_mcs
  // target_ul_mcs
  // frame_parms[0]->N_RB_DL
  int i,j;
  memset(runtime_phy_rx,0,sizeof(uint16_t)*29*6);
  memset(runtime_phy_tx,0,sizeof(uint16_t)*29*6);
  /* only the BBU/PHY procesing time */ 
  for (i=0;i<29;i++){
    for (j=0;j<6;j++){
      runtime_phy_rx[i][j] = cell_processing_ul[j] + platform_processing_ul + user_processing_ul_err[j] + user_processing_ul_a[j]*i+  user_processing_ul_b[j];
      runtime_phy_tx[i][j] = cell_processing_dl[j] + platform_processing_dl + user_processing_dl_err[j] + user_processing_dl_a[j]*i+  user_processing_dl_b[j];
    }
  }
}
 
// int runtime_upper_layers[6]; // values for different RBs
// int runtime_phy_rx[29][6]; // SISO [MCS 0-28][RBs 0-5 : 6, 15, 25, 50, 75, 100]
// int runtime_phy_tx[29][6]; // SISO [MCS 0-28][RBs 0-5 : 6, 15, 25, 50, 75, 100]

// target_dl_mcs
  // target_ul_mcs
  // frame_parms[0]->N_RB_DL
  //runtime_upper_layers[6]; // values for different RBs
  // int runtime_phy_rx[29][6]; // SISO [MCS 0-28][RBs 0-5 : 6, 15, 25, 50, 75, 100] 
  // int runtime_phy_tx[29][6]

double get_runtime_tx(int tx_subframe, uint16_t runtime_phy_tx[29][6], uint32_t mcs, int N_RB_DL,double cpuf,int nb_tx_antenna){
   int i;	
   double runtime;
   //printf("cpuf =%lf  \n",cpuf);
   switch(N_RB_DL){
   case 6:
     i = 0;
     break;
   case 15:
     i = 1;
     break;
   case 25:
     i = 2;
     break;
   case 50:
     i = 3;
     break;
   case 75:
     i = 4;
     break;
   case 100:
     i = 5;
     break;
   default:
     i = 3;
     break;
   }			
   
   runtime = ( (3.2/cpuf)*(double)runtime_phy_tx[mcs][i] + (3.2/cpuf)*(double)protocol_processing_dl[i])/1000 ;
   //printf("Setting tx %d runtime value (ms) = %lf\n",tx_subframe,runtime);
   
   return runtime;	
 }
 
double get_runtime_rx(int rx_subframe, uint16_t runtime_phy_rx[29][6], uint32_t mcs, int N_RB_DL,double cpuf,int nb_rx_antenna){
   int i;	
   double runtime;
   
   //printf("N_RB_DL=%d  cpuf =%lf  \n",N_RB_DL, cpuf);
   switch(N_RB_DL){
   case 6:
     i = 0;
     break;
   case 15:
     i = 1;
     break;
   case 25:
     i = 2;
     break;
   case 50:
     i = 3;
     break;
   case 75:
     i = 4;
     break;
   case 100:
     i = 5;
     break;
   default:
     i = 3;
     break;
   }			
   
   runtime = ((3.2/cpuf)*(double)runtime_phy_rx[mcs][i] + (3.2/cpuf)*(double)protocol_processing_ul[i])/1000 ;
   //printf("Setting rx %d runtime value (ms) = %lf \n",rx_subframe, runtime);
   
   return runtime;	
}

//xhd_oai_60m
////#ifdef LOWLATENCY
int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags)
{

  return syscall(__NR_sched_setattr, pid, attr, flags);
}


int sched_getattr(pid_t pid,struct sched_attr *attr,unsigned int size, unsigned int flags)
{

  return syscall(__NR_sched_getattr, pid, attr, size, flags);
}

////#endif

#else

int rt_sleep_ns(RTIME x)
{
  rt_sleep(nano2count(x));
  return(0);
}

#endif
