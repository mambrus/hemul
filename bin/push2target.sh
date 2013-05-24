#!/bin/bash
set -e

INSTDIR=/data/local/tmp
RDIR=${INSTDIR}/hemul

set +e
adb shell mkdir ${INSTDIR}
adb shell mkdir ${RDIR}
set -e

#Remove any previous build to avoid to avoid running the wrong binary by
#mistake

adb shell rm -f ${RDIR}/libmtime.so
adb shell rm -f ${RDIR}/hemul

#Push gdbserver (make conditional. TBD)
adb push bin/gdbserver75 ${RDIR}/gdbserver75

#Push the binaries
adb push libs/armeabi/hemul ${RDIR}/hemul
if [ "X${LIB_DYNAMIC}" != "X" ]; then
	echo "Pushing libs/armeabi/hemul/libmtime.so ..."
	adb push libs/armeabi/hemul/libhemullib.so ${RDIR}/libhemullib.so
fi

FS=$(ls testrc/*.ini); for F in $FS; do echo $F; adb push $F ${RDIR}; done


