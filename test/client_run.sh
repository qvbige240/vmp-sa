#!/bin/sh
# qing

for i in `seq 1 50`  
do  
    echo $i  
    sim_num=$((13800000 + $i))
    echo $sim_num 
    ./tvmpss_client -f raw_data.video -s 172.20.25.228 -p 9999 -d 8 -b $sim_num -n 1 -l 1 &
done 
