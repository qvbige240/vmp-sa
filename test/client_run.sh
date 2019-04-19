#!/bin/bash
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

function rand()
{  
    min=$1  
    max=$(($2-$min+1))  
    num=$(cat /proc/sys/kernel/random/uuid | cksum | awk -F ' ' '{print $1}')  
	echo $(($num%$max+$min))  
}

function rand_sleep()
{
    rnd=$(rand 2 100)
    #echo $rnd 
    decimals=$(echo "scale=3; $rnd * 0.001" | bc)
    echo $decimals
}

cycle=0
function client_run()
{
	let cycle+=$1
	for i in `seq 1 $total` 
	do  
		echo $i  
		sim_num=$((13100000 + $i + $cycle))
		echo $sim_num 
		#./tvmpss_client -f raw_data.video -s 172.20.25.228 -p 9999 -d 8 -b $sim_num -n 1 -l 1 &
		#./tvmpss_client -f raw_data.video -s 127.0.0.1 -p 9999 -d 8 -b $sim_num -n 1 -l 1 &
		#./tvmpss_client -f raw_data.video -s 172.17.25.131 -p 9999 -d 8 -b $sim_num -n 1 -l 1 &
		./tvmpss_client -f cif_raw_data.video -s 172.17.25.131 -p 9999 -d 12 -b $sim_num -n 1 -l 1 &
		#./tvmpss_client -f one_nalu_raw_data.video -s 127.0.0.1 -p 9999 -d 8 -b $sim_num -n 1 -l 0 &
		#./tvmpss_client -f raw_data.video -s 192.168.1.118 -p 9999 -d 8 -b $sim_num -n 1 -l 1 &
		#./tvmpss_client -f cif_raw_data.video -s 192.168.1.118 -p 9999 -d 12 -b $sim_num -n 1 -l 1 &
		#sleep 0.02

		if [ $i -ge 100 ] ;then
		    echo "$total:  $i ge 100"
		    echo " "
		    total=$(($total-$i))
		    sleep 5
		    echo "total $total"
		    client_run $i
		    return 0
		fi

		a=$(rand_sleep)
		#echo "a: $a"
		sleep $a
	done
}

client_run 0


#./tvmpss_client -f one_nalu_raw_data.video -s 172.20.25.228 -p 9999 -d 8 -b 13900000 -n 1 -l 0
#./tvmpss_client -f cif_raw_data.video -s 127.0.0.1 -p 9999 -d 12 -b 13000000 -n 1 -l 1
#./tvmpss_client -f raw_data.video -s 127.0.0.1 -p 9999 -d 8 -b 016180560371 -n 1 -l 1
