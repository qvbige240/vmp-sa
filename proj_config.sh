#!/bin/bash

WORKDIR="$( cd "$( dirname "${BASH_SOURCE[0]}"  )" && pwd  )"
echo "WORKDIR: $WORKDIR "
#cur_time=`date +%Y-%m-%d`
#APP_DIR=${WORKDIR}/app
#PKG_DIR=`pwd`/bin

if false ; then

INC_DIR=${WORKDIR}/include
PROJECT_CODE="K90"
APP_NAME="timapp"
VERSION_CODE="v1.0.1"
cur_time=`date +%y%m%d`
TOOLS_NUM=`svn info | grep "Last Changed Rev: " | sed -e "s/Last Changed Rev: //g"`
#VERSION_DETAILS="TIMA_VERSIONS\",build:\"BUILD_TOOLNUM\":\"BUILD_DATE"
VERSION_DETAILS="TIMA_VERSIONS\".\"BUILD_DATE\",build:\"BUILD_TOOLNUM"

echo "================= set version =================="
echo "${TOOLS_NUM}"

function version_sed()
{
    head_src=""
    head_dst=""
    if [ -f ${INC_DIR}/$1 ]; then
        head_src=${INC_DIR}/$1
    else
        echo "file not exists, \'${head_src}\'"
        return 1
    fi

    if [ "$2"x == x ]; then
        return 1
    fi

    head_dst=${INC_DIR}/$2

    #cp ${head_src} ${head_dst}

    #`sed -i "/^#define ENVEE_FEATURES.*/s/_[a-z].*/_$features_suffix/" $features_file`
    #`sed -i "/^#define TIMA_PROJECT.*/s/_[A-Z].*/_PROJECT\     \"${PROJECT_CODE}\"/" $head_dst`

    `sed -e "/^#define TIMA_PROJECT.*/s/_[A-Z].*/_PROJECT\     \"${PROJECT_CODE}\"/" $head_src | \
    sed "/^#define TIMA_NAME.*/s/_[A-Z].*/_NAME\     \"${APP_NAME}\"/" | \
    sed "/^#define TIMA_VERSIONS.*/s/_[A-Z].*/_VERSIONS\     \"${VERSION_CODE}\"/" | \
    sed "/^#define BUILD_DATE.*/s/_[A-Z].*/_DATE\     \"${cur_time}\"/" | \
    sed "/^#define BUILD_TOOLNUM.*/s/_[A-Z].*/_TOOLNUM\     \"${TOOLS_NUM}\"/" | \
    sed "/^#define SW_VERSION.*/s/_[A-Z].*/_VERSION\     ${VERSION_DETAILS}/" > vertmp`

    mv vertmp ${head_dst}
}

version_sed version.h tima_version.h
if [ $? -ne 0  ]; then
    echo "return error"
fi

fi


function proj_path_get()
{
    if [ -d "${WORKDIR}/../$1" ]; then
        echo "${WORKDIR}/../$1"
    elif [ -d "${WORKDIR}/../../$1/workspace" ]; then
        echo "${WORKDIR}/../../$1/workspace"
    else
        echo -e "$1 not exist"
        exit -1
    fi
}

function find_osource_library()
{
    dir_premake=premake
    if [ ! -d ${dir_premake} ]; then
       
        local osource_library=$(proj_path_get premake)
        echo "======= osource_path: $osource_library"
        if [ -d $osource_library ]; then
            ln -s ${osource_library} ${WORKDIR}/premake
        else
            exist -1
        fi

    fi
}

find_osource_library

