#! /bin/bash
echo "######### Kill Old Process  ###########" 
sudo killall -9 oai.enb.eth
sudo rmmod intel_powerclamp
echo "######### Start IP  ###########" 
sudo ifconfig lo:2 172.16.1.98 up

echo "######### Set CpuFreq to performance  ###########" 
sudo cpufreq-set -g performance -c 0
sudo cpufreq-set -g performance -c 1
sudo cpufreq-set -g performance -c 2 
sudo cpufreq-set -g performance -c 3
sudo cpufreq-set -g performance -c 4
sudo cpufreq-set -g performance -c 5
sudo cpufreq-set -g performance -c 6
sudo cpufreq-set -g performance -c 7

ulimit -c unlimited

echo "######### Start enb  ###########" 

cd bin
./oai.enb.eth -O ../conf/enb-f25b3.conf -N ../conf/enb.base.band3.conf  

echo "######### FINISH  ###########" 
