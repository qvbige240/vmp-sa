#!/bin/bash


function current_dir {
  echo "$(cd "$(dirname $0)"; pwd)"
}

./tvmpss_client -f raw_data.media -s 192.168.1.113 -p 9999 -n 1

