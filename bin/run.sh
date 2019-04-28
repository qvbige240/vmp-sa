#!/bin/bash
# Qing.

WORKDIR="$( cd "$( dirname "${BASH_SOURCE[0]}"   )" && pwd   )"

echo "WORKDIR: $WORKDIR "

function stream_server_run()
{
    if [ ! -f tvmpssd ]; then
        echo -e "ERROR: tvmpssd not exist."
        return -1
    fi

    if [ ! -f ../conf/vmp-sa.conf ]; then
        echo -e "ERROR: config file \'vmp-sa.conf\' not exist."
        return -1
    fi
    
    ./tvmpssd -c ../conf/vmp-sa.conf &
}

stream_server_run

