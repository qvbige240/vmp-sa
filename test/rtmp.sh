#/bin/bash

#s='2019-04-16 17:32:53 INFO   (tima_rtmp_publisher.c:57) - connected[0]: rtmp://127.0.0.1:1935/live/130001000000_1'
#echo $s | awk '{print $NF}' |sed 's/127.0.0.1/172.17.25.131/g'
url=`grep connected log/tima.log | awk '{print $NF}' |sed 's/127.0.0.1/172.17.25.131/g'`
for i in $url
do
	#echo "====url is ${i}"
	echo ${i}
done
echo $url | wc

