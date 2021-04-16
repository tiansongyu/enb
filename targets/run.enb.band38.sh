echo "######### Kill Old Process  ###########" 
sudo killall -9 oai.enb.usrp
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
./oai.enb.usrp   -O ../conf/enb-"$1"b38.conf -N ../conf/enb.base.band38.conf #> enb.log.20171107.txt & 
else
./oai.enb.usrp   -O ../conf/enb-f25b38.conf -N ../conf/enb.base.band38.conf #> enb.log.20171107.txt & 
fi

#if [ $# -eq 4 ]; then
#./oai_enb   -O ../conf/enb-"$1".conf -N ../conf/enb_base.conf --usrp-sample "$2" --usrp-delay "$3" --usrp-forward "$4"
#fi
echo "######### FINISH  ###########" 
#sudo ./lte-softmodem.Rel10 -U -C 2680000000 >eth_ue.log& 

