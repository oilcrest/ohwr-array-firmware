#!/bin/bash

if [ $# -ne 6 ]; then
	echo "Usage: $0 TARGET CROSS_COMPILE CPU SOC BOARD APP" >/dev/stderr
	exit 1;
fi

TARGET=$1
CROSS_COMPILE=$2
CPU=$3
SOC=$4
BOARD=$5
APP=$6

echo "Configuring for TARGET=$TARGET APP=$APP BOARD=$BOARD, SOC=$SOC, CPU=$CPU"
echo "TARGET = $TARGET"                > genconfig.mk
echo "CROSS_COMPILE = $CROSS_COMPILE" >> genconfig.mk
echo "CPU = $CPU"                     >> genconfig.mk
echo "SOC = $SOC"                     >> genconfig.mk
echo "BOARD = $BOARD"                 >> genconfig.mk
echo "APP = $APP"                     >> genconfig.mk

