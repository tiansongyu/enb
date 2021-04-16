#! /bin/bash
LTEIF=oip0

load_module() {
  mod_name=${1##*/}
  mod_name=${mod_name%.*}
  if awk "/$mod_name/ {found=1 ;exit} END {if (found!=1) exit 1}" /proc/modules
    then
      echo "module $mod_name already loaded: I remove it first"
      sudo rmmod $mod_name
  fi
  echo loading $mod_name
  sudo insmod $1
}
echo "######### kill old process ######"
sudo killall -9 oai.ue.usrp
echo "######### Start oip0  ###########" 

cd bin
load_module ./ue_ip.ko

echo "bring up oip0 interface for UE"
#ifconfig oip0 up
ifconfig oip0 172.18.1.88 netmask 255.255.0.0 broadcast 172.18.255.255 up

echo "ip route flush cache"
ip route flush cache
sleep 1

echo "sysctl 1"
sysctl -w net.ipv4.conf.all.log_martians=1

echo "Disabling reverse path filtering"

sysctl -w net.ipv4.conf.oip0.rp_filter=0
sysctl -w net.ipv4.conf.all.rp_filter=0
sysctl -w net.ipv4.conf.default.rp_filter=0


echo "ip route flush cache again"
ip route flush cache

# Check table 200 lte in /etc/iproute2/rt_tables

echo "fgrep lte"
fgrep lte /etc/iproute2/rt_tables  > /dev/null 
if [ $? -ne 0 ]; then
    echo "200 lte " >> /etc/iproute2/rt_tables
fi

echo "ip rule add"
ip rule add fwmark 1 table lte

echo "ip route add"
ip route add default dev $LTEIF table lte
echo "######### Set CpuFreq to performance  ###########" 

echo "######### Start UE  ###########" 
ulimit -c unlimited
sudo rm -r core
./oai.ue.usrp -U -C 2360000000 -r 25 --ue-txgain 101 --ue-rxgain  110 --ue-scan-carrier --ue-amp 200
