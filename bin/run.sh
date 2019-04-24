#!/bin/bash
# Qing.

WORKDIR="$( cd "$( dirname "${BASH_SOURCE[0]}"   )" && pwd   )"

echo "WORKDIR: $WORKDIR "

if [ ! -f tvmpssd ]; then
    echo -e "tvmpssd not exist."
fi

if [ ! -f ../conf/tvmpssd.conf ]; then
    echo -e "config file \'tvmpssd.conf\' not exist."
fi

./tvmpssd -c ../conf/tvmpssd.conf

