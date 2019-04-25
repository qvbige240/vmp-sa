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

    if [ ! -f ../conf/tvmpssd.conf ]; then
        echo -e "ERROR: config file \'tvmpssd.conf\' not exist."
        return -1
    fi
    
    ./tvmpssd -c ../conf/tvmpssd.conf &
}

stream_server_run

