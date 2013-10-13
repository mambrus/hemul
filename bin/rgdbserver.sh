#!/bin/bash
# Use for example like this: 
#   rgdbserver.sh -p 100000 /tmp/one_log.txt
#
# Hint:
# for(( ;; )); do ...; done

INSTDIR=/data/local/tmp
RDIR=${INSTDIR}/hemul

echo "adb forward \"tcp:6039\" \"tcp:6039\"; adb shell \"${RDIR}/gdbserver75 :6039 ${RDIR}/hemul ${@}\""
adb forward "tcp:6039" "tcp:6039"; adb shell "${RDIR}/gdbserver75 :6039 ${RDIR}/hemul ${@}"
