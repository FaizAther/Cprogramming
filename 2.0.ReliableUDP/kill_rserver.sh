#!/bin/bash

ANS=""

if test $# -eq 1
then
	echo "Killing pids `pgrep RClient.exe`"
	pkill RClient.exe
	exit
fi


echo "Kill RServers? Y/N"
read ANS

if test $ANS = "Y"
then
	echo "Killing pids `pgrep RUSHBSvr`"
	pkill RUSHBSvr
fi
