#!/bin/sh
# Qing

total=

if [ ! -n "$1" ] ;then
    echo "Usge: $0 client number ?"
    return
else
    total=$1
fi
if [ $# -gt 0 ] ;then
    echo $1
    total=$1
else
    total=70
fi
echo "total: $total"

for i in `seq 1 $total` 
do  
    echo $i  
    sim_num=$((13900000 + $i))
    echo $sim_num 
    #./tvmpss_client -f raw_data.video -s 172.20.25.228 -p 9999 -d 8 -b $sim_num -n 1 -l 1 &
    ./tvmpss_client -f raw_data.video -s 127.0.0.1 -p 9999 -d 8 -b $sim_num -n 1 -l 1 &
    #./tvmpss_client -f one_nalu_raw_data.video -s 127.0.0.1 -p 9999 -d 8 -b $sim_num -n 1 -l 0 &
    #./tvmpss_client -f raw_data.video -s 192.168.1.118 -p 9999 -d 8 -b $sim_num -n 1 -l 1 &
    #./tvmpss_client -f cif_raw_data.video -s 192.168.1.118 -p 9999 -d 12 -b $sim_num -n 1 -l 1 &
    sleep 0.02
done

#./tvmpss_client -f one_nalu_raw_data.video -s 172.20.25.228 -p 9999 -d 8 -b 13900000 -n 1 -l 0
#./tvmpss_client -f cif_raw_data.video -s 127.0.0.1 -p 9999 -d 12 -b 13000000 -n 1 -l 1
