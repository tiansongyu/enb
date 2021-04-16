#! /bin/bash
echo "######### Kill Old Process  ###########" 
sudo killall -9 oai.enb.eth
sudo killall -9 oai.ue.eth
echo "######### Start build eth  ###########" 
cd ../cmake_targets
./build_oai -c --eNB --UE -w ETHERNET 
cd ../targets/bin
sudo ln -s lte-softmodem.Rel10 oai.enb.eth
sudo ln -s lte-softmodem.Rel10 oai.ue.eth
