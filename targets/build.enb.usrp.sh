#! /bin/bash
echo "######### Kill Old Process  ###########" 
sudo killall -9 oai.enb.usrp
sudo killall -9 oai.ue.usrp
echo "######### Start build eth  ###########" 
cd ../cmake_targets
./build_oai -c --eNB -w USRP 
cd ../targets/bin
sudo ln -s oai.enb lte-softmodem.Rel10
