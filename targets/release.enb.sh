#! /bin/bash
#cd /home/oai/trunk/targets   
cd ../..
tar zcvf release_enb20`date +'%02y%02m%02d'`.tar.z  \
 enb/targets/
