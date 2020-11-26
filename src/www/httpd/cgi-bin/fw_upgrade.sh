#!/bin/sh

YI_HACK_PREFIX="/home/yi-hack"

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/lib:/home/yi-hack/lib:/tmp/sd/yi-hack/lib
export PATH=$PATH:/home/base/tools:/home/yi-hack/bin:/home/yi-hack/sbin:/tmp/sd/yi-hack/bin:/tmp/sd/yi-hack/sbin

NAME="$(echo $QUERY_STRING | cut -d'=' -f1)"
VAL="$(echo $QUERY_STRING | cut -d'=' -f2)"

if [ "$NAME" != "get" ] ; then
    exit
fi

if [ "$VAL" == "info" ] ; then
    printf "Content-type: application/json\r\n\r\n"

    FW_VERSION=`cat /home/yi-hack/version`
    LATEST_FW=`wget -O -  https://api.github.com/repos/meriororen/yi-hack-MStar/releases/latest 2>&1 | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/'`

    printf "{\n"
    printf "\"%s\":\"%s\",\n" "fw_version"      "$FW_VERSION"
    printf "\"%s\":\"%s\"\n" "latest_fw"       "$LATEST_FW"
    printf "}"

elif [ "$VAL" == "upgrade" ] ; then

    FREE_SD=$(df /tmp/sd/ | grep mmc | awk '{print $4}')
    if [ -z "$FREE_SD" ]; then
        printf "Content-type: text/html\r\n\r\n"
        printf "No SD detected."
        exit
    fi

    if [ $FREE_SD -lt 100000 ]; then
        printf "Content-type: text/html\r\n\r\n"
        printf "No space left on SD."
        exit
    fi

    rm -rf /tmp/sd/.fw_upgrade
    mkdir -p /tmp/sd/.fw_upgrade
    cd /tmp/sd/.fw_upgrade

    CAMERA_NAME=`cat $YI_HACK_PREFIX/camera_name`
    FW_VERSION=`cat /home/yi-hack/version`
    LATEST_FW=`wget -O -  https://api.github.com/repos/meriororen/yi-hack-MStar/releases/latest 2>&1 | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/'`
    if [ "$FW_VERSION" == "$LATEST_FW" ]; then
        printf "Content-type: text/html\r\n\r\n"
        printf "No new firmware available."
        exit
    fi

    FW_FILES=`wget -O -  https://api.github.com/repos/meriororen/yi-hack-MStar/releases/latest 2>&1 | grep '"browser_download_url":' | sed -E 's/.*"([^"]+)".*/\1/'`
                                                                                                                                                         
    file_exist="false"
    if [ "$FW_FILES" != "" ]; then                                                                                                                           
        for file in $FW_FILES; do                                                                                                                            
           if [ "https://github.com/meriororen/yi-hack-MStar/releases/download/$LATEST_FW/${CAMERA_NAME}_${LATEST_FW}.tgz" == "$file" ]; then              
              file_exist="true"
           fi                                                                                                                                               
        done                                                                                                                                                 
                                                                                                                                                         
        if [ "$file_exist" == "false" ]; then
           printf "Content-type: text/html\r\n\r\n"                                                                                                         
           printf "No update for this camera in newest release"                                                                                  
           exit                                                                                                                                             
        fi                                                                                                                                                   
    fi

    wget https://github.com/meriororen/yi-hack-MStar/releases/download/$LATEST_FW/${CAMERA_NAME}_${LATEST_FW}.tgz
    if [ ! -f ${CAMERA_NAME}_${LATEST_FW}.tgz ]; then
        printf "Content-type: text/html\r\n\r\n"
        printf "Unable to download firmware file."
        exit
    fi

    tar zxvf ${CAMERA_NAME}_${LATEST_FW}.tgz
    rm ${CAMERA_NAME}_${LATEST_FW}.tgz
    mv -f * ..
    cp -f $YI_HACK_PREFIX/etc/*.conf .
    if [ -f /etc/hostname ]; then
        cp -f /etc/hostname .
    fi
    if [ -f /etc/TZ ]; then
        cp /etc/TZ .
    fi

    # Report the status to the caller
    printf "Content-type: text/html\r\n\r\n"
    printf "Download completed, rebooting and upgrading."

    sync
    sync
    sync
    # Kill httpd otherwise reboot command truncates the TCP session
    killall httpd
    sleep 1
    reboot -f
fi
