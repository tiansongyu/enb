echo "######### Kill Old Process  ###########" 
sudo killall -9 oai_enb
sudo rmmod intel_powerclamp
echo "######### Start IP  ###########" 
sudo ifconfig lo:2 172.16.1.98 up

echo "######### Set CpuFreq to performance  ###########" 

sudo cpufreq-set -g performance -c 0
sudo cpufreq-set -g performance -c 1
sudo cpufreq-set -g performance -c 2 
sudo cpufreq-set -g performance -c 3

echo "######### Start enb  ###########" 
cd bin
ulimit -c unlimited
if [ $# -eq 1 ]; then
./oai.enb   -O ../conf/enb."$1"band3.conf -N ../conf/enb.46011.conf
else
./oai.enb   -O ../conf/enb.f25band3.conf -N ../conf/enb.46011.conf 
fi

echo "######### FINISH  ###########" 
#sudo ./lte-softmodem.Rel10 -U -C 2680000000 >eth_ue.log& 

