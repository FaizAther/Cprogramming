#!/bin/bash

RUN_FILE="file.0.txt"
RUN_CHKSUM=""
RUN_ENC=""
RUN_MISB=""

if test $# -eq 0
then
        exit
fi

if test $# -gt 0
then
        if test `expr $1 % 2` -eq 0
        then
                RUN_FILE="file.1.txt"
        elif test `expr $1 % 3` -eq 0
        then
                RUN_FILE="file.2.txt"
        elif test `expr $1 % 5` -eq 0
        then
                RUN_FILE="file.n.txt"
        fi
fi

if test $# -gt 1
then
        if test $2 = "-c"
        then
                RUN_CHKSUM=$2
        fi
        if test $2 = "-e"
        then
                RUN_ENC=$2
        fi
        if test $2 = "-m"
        then
                RUN_MISB=$2
        fi
fi

if test $# -gt 2
then
        if test $3 = "-e"
        then
                RUN_ENC=$3
        fi
        if test $3 = "-c"
        then
                RUN_CHKSUM=$3
        fi
        if test $3 = "-m"
        then
                RUN_MISB=$3
        fi
fi

if test $# -gt 3
then
        if test $4 = "-e"
        then
                RUN_ENC=$4
        fi
        if test $4 = "-c"
        then
                RUN_CHKSUM=$4
        fi
        if test $4 = "-m"
        then
                RUN_MISB=$4
        fi
fi

if test $# -gt 4
then
        stat $1 1>/dev/null 2>/dev/null
        if test $? -eq 0
        then
                RUN_FILE=$1
        fi
fi

echo "./RClient.exe -p${@: -1} -n$1 -r $RUN_FILE $RUN_CHKSUM $RUN_ENC $RUN_MISB > logs/$1.client &"
echo "./RClient.exe -p${@: -1} -n$1 -r $RUN_FILE $RUN_CHKSUM $RUN_ENC $RUN_MISB > logs/$1.client &" > logs/$1.client
./RClient.exe -p${@: -1} -n$1 -r $RUN_FILE $RUN_CHKSUM $RUN_ENC $RUN_MISB >> logs/$1.client &
