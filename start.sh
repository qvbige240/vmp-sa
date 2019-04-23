#!/bin/bash
# Qing.

WORKDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

function stream_server()
{
    if [ -f "${WORKDIR}/bin/run.sh"  ]; then
        cd ${WORKDIR}/bin
        ${WORKDIR}/bin/run.sh
        cd -
    else
        echo -e "bin/run.sh not exist"
        return -1
    fi
}

stream_server

