#!/bin/sh
# Qing

for i in `seq 1 70`  
do  
    echo $i  
    sim_num=$((13900000 + $i))
    echo $sim_num 
    #./tvmpss_client -f raw_data.video -s 172.20.25.228 -p 9999 -d 8 -b $sim_num -n 1 -l 1 &
    #./tvmpss_client -f raw_data.video -s 127.0.0.1 -p 9999 -d 8 -b $sim_num -n 1 -l 1 &
    #./tvmpss_client -f one_nalu_raw_data.video -s 127.0.0.1 -p 9999 -d 8 -b $sim_num -n 1 -l 0 &
    #./tvmpss_client -f raw_data.video -s 192.168.1.118 -p 9999 -d 8 -b $sim_num -n 1 -l 1 &
    ./tvmpss_client -f cif_raw_data.video -s 192.168.1.118 -p 9999 -d 12 -b $sim_num -n 1 -l 1 &
done 

#./tvmpss_client -f one_nalu_raw_data.video -s 172.20.25.228 -p 9999 -d 8 -b 13900000 -n 1 -l 0
#./tvmpss_client -f cif_raw_data.video -s 127.0.0.1 -p 9999 -d 12 -b 13000000 -n 1 -l 1
