#!/bin/bash


function current_dir {
  echo "$(cd "$(dirname $0)"; pwd)"
}

#./tvmpss_client -f raw_data.media -s 192.168.1.163 -p 9999 -n 1
#./tvmpss_client -f raw_data.media -s 192.168.1.113 -p 9999 -d 40 -n 1
#./../bin/tvmpss_client -f raw_data.video -s 172.20.25.228 -p 9999 -d 8 -b 01380000 -n 3
./../bin/tvmpss_client -f raw_data.video -s 172.20.25.228 -p 9999 -l 1 -d 8 -b 01300000 -n 10
