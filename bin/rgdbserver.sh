#!/bin/bash
# Use for example like this: 
#   rgdbserver.sh -s /etc/hemul/thermal_kalix.ini -p 100000
#
# Hint:
# for(( ;; )); do ...; done

INSTDIR=/data/local/tmp
RDIR=${INSTDIR}/hemul

echo "adb forward \"tcp:5039\" \"tcp:5039\"; adb shell \"${RDIR}/gdbserver75 :5039 ${RDIR}/hemul ${@}\""
adb forward "tcp:5039" "tcp:5039"; adb shell "${RDIR}/gdbserver75 :5039 ${RDIR}/hemul ${@}"
