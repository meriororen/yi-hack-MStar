#!/bin/sh

DIR="none"
TIME="0.3"
POINT="none"
XPOINT="-1"
YPOINT="-1"

for I in 1 2 3
do
    CONF="$(echo $QUERY_STRING | cut -d'&' -f$I | cut -d'=' -f1)"
    VAL="$(echo $QUERY_STRING | cut -d'&' -f$I | cut -d'=' -f2)"

    if [ "$CONF" == "dir" ] ; then
        DIR="-m $VAL"
    elif [ "$CONF" == "time" ] ; then
        TIME="$VAL"
    elif [ "$CONF" == "point" ] ; then
	POINT="$VAL"
	XPOINT="$(echo $VAL | cut -d',' -f1)"
	YPOINT="$(echo $VAL | cut -d',' -f2)"
    fi
done

if [ "$DIR" != "none" ] ; then
    ipc_cmd $DIR
    sleep $TIME
    ipc_cmd -m stop
fi

if [ "$POINT" != "none" ] ; then
    ipc_cmd -j $VAL || ipc_cmd -j $XPOINT,$YPOINT
fi

printf "Content-type: application/json\r\n\r\n"

printf "{\n"
printf "direction=$DIR,time=$TIME,pointX=$XPOINT,pointY=$YPOINT\n"
printf "}"
