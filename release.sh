#!/bin/bash
# Qing.

WORKDIR="$( cd "$( dirname "${BASH_SOURCE[0]}"  )" && pwd  )"
echo "WORKDIR: $WORKDIR "
cur_time=`date +%y%m%d`
#CODE_VERSION=`svn info | grep "Last Changed Rev: " | sed -e "s/Last Changed Rev: //g"`
APP_DIR=${WORKDIR}/bin

cd ${WORKDIR}

source env.conf
#source env.profile
platform="ubuntu"
dst_platform=""
dst_release="debug"
TOOLCHAIN=0
DEBUG_MODE=1
TIMA_PRODUCT=1
STRESS_TEST=0
DEPEND_PREMAKE=1
#P2P_SOLUTION=1

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
       
        local osource_library=$(proj_path_get premake-vmp)
        if [ -d $osource_library ]; then
            ln -s ${osource_library} ${WORKDIR}/premake
        else
            exist -1
        fi

    fi
}

function environment_source()
{
    if [ -d $1 ]; then
        local files=`ls $1/env*.sh`
        for f in ${files[@]}; do
            . ${f}
        done
    fi
}

function environment_init()
{
    local env_path=${WORKDIR}/premake/${platform}
    environment_source $env_path
}


if [ $# -gt 0 ]; then
    platform=$(echo $1 | tr '[A-Z]' '[a-z]')
    echo "platform: $1"
    if [ "nt966x" == $platform ]; then
        TOOLCHAIN=1
    elif [ "nt966x_d048" == $platform ]; then
        TOOLCHAIN=2
    else
        TOOLCHAIN=0
    fi

    if [ "release" == $(echo $2 | tr '[A-Z]' '[a-z]') ]; then
        DEBUG_MODE=0
    else
        DEBUG_MODE=1
    fi

    if [ "produce" == $(echo $3 | tr '[A-Z]' '[a-z]') ]; then
        TIMA_PRODUCT=1
    else
        TIMA_PRODUCT=0
    fi

    if [ "stress" == $(echo $4 | tr '[A-Z]' '[a-z]') ]; then
        STRESS_TEST=1
    else
        STRESS_TEST=0
    fi
    
    if false ; then
        if [ "tima" == $(echo $5 | tr '[A-Z]' '[a-z]') ]; then
            P2P_SOLUTION=1
        elif [ "tutk" == $(echo $5 | tr '[A-Z]' '[a-z]') ]; then
            P2P_SOLUTION=2
        elif [ "none" == $(echo $5 | tr '[A-Z]' '[a-z]') ]; then
            P2P_SOLUTION=0
        else
            P2P_SOLUTION=0
        fi
    fi

else
    TOOLCHAIN=0
fi

if [ "$platform" == "ubuntu" ]; then
    dst_platform="ubuntu"
else
    dst_platform="centos"
fi

if [ "$DEBUG_MODE" == "1" ]; then
    dst_release="debug"
else
    dst_release="release"
fi

echo "dst_platform: ${dst_platform}"

echo "TOOLCHAIN:${TOOLCHAIN}, DEBUG_MODE:${DEBUG_MODE}, TIMA_PRODUCT:${TIMA_PRODUCT}, STRESS_TEST:${STRESS_TEST} PLATFORMS:${platform} DEPEND_PREMAKE=1"

function compile()
{
    make clean
    echo "make -e TOOLCHAIN=${TOOLCHAIN} DEBUG_MODE=${DEBUG_MODE} TIMA_PRODUCT=${TIMA_PRODUCT} STRESS_TEST=${STRESS_TEST} PLATFORMS=${platform} DEPEND_PREMAKE=1"
    make -e TOOLCHAIN=${TOOLCHAIN} DEBUG_MODE=${DEBUG_MODE} TIMA_PRODUCT=${TIMA_PRODUCT} STRESS_TEST=${STRESS_TEST} PLATFORMS=${platform} DEPEND_PREMAKE=1
    if [ $? -ne 0  ]; then
        return 1
    fi
}

function distribution()
{
    source etc.profile
    mkdir -p ${PKG_DIR}/bin
    for file in "${app_res[@]}"
    do
        config_file="${APP_DIR}/${file}"
        if [ ! -f $config_file ]; then
            echo "error: $config_file not exists."
            #continue
            return 1
        fi
        echo "cp -f $config_file $PKG_DIR/bin"
        cp -f $config_file $PKG_DIR/bin
    done
    
    if [ -f ${WORKDIR}/conf/tvmpssd.conf ]; then
        cp -R ${WORKDIR}/conf $PKG_DIR
    fi
    
    if [ -f ${WORKDIR}/start.sh ]; then
        cp -f ${WORKDIR}/start.sh $PKG_DIR
    fi

if false; then    
#    if [ "$dst_platform" == "nt9x" ]; then
#        cp ${APP_DIR}/zlog.conf_mips $PKG_DIR/zlog.conf
#        cp ${APP_DIR}/zlog.conf_mips_d048 $PKG_DIR/zlog.conf
#    fi

    if [ "$dst_platform" == "x86" ]; then
        cp ${APP_DIR}/dev_info_pc.json $PKG_DIR
        cp ${APP_DIR}/network_config.json $PKG_DIR
    fi

#    if [ "$dst_platform" == "x86" ]; then
        #cp ${APP_DIR}/sqlite3_x86 $PKG_DIR/sqlite3
        cp ${APP_DIR}/makefile_demo $PKG_DIR/makefile
        cp ${APP_DIR}/*.c $PKG_DIR
        cp ${APP_DIR}/*.h $PKG_DIR
        cp -R ${WORKDIR}/sdk/include ${PKG_DIR}
        mkdir ${PKG_DIR}/lib
        cp ${WORKDIR}/sdk/lib/${platform}/* ${PKG_DIR}/lib
        rm ${PKG_DIR}/lib/libtmsdk.a
        #if [ "$DEBUG_MODE" == "0" ]; then
            if [ -d ${WORKDIR}/demo/resources ]; then
                cp -R ${WORKDIR}/demo/resources $PKG_DIR
            fi
        #fi
#    fi
fi
}

function release()
{
    environment_init
    compile
    if [ $? -ne 0  ]; then
        return 1
    fi
 
    #PKG_DIR=tima-sdk-${dst_platform}.${cur_time}.${CODE_VERSION}
    PKG_DIR=vmp-server-${dst_platform}.${cur_time}
    if [ -d $PKG_DIR ]; then
        rm -rf ${PKG_DIR}/*
    else
        mkdir ${PKG_DIR}
    fi

    distribution 
    if [ $? -ne 0  ]; then
        return 1
    fi
   
    #if [ -f ${PKG_DIR}.tgz ]; then
    #    rm ${PKG_DIR}.tgz
    #fi
    if ls tima-*.tgz >/dev/null 2>&1; then
        rm tima-*.tgz
    fi
    echo "tar -zcvf ${PKG_DIR}.tgz ${PKG_DIR}"
    tar -zcvf ${PKG_DIR}.tgz ${PKG_DIR} 
    if [ $? -ne 0  ]; then
        return 1
    fi

    if [ -d $PKG_DIR ]; then
        rm -rf ${PKG_DIR}
    fi
}

find_osource_library

release

cd -

