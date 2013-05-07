#!/bin/bash

#This is a temporary script taking care of build order dependency found. It
#will be removed when problem is solved.

set -e

#set this in bash_profile. It defaults to kalix currently
USE_COMBO=${USE_COMBO-"1 kalix 3"}

OPATH=$(pwd)

# cd $(gettop); doesn't work. Try guessing instead
if [ "X${T}" != "X" ]; then
	cd ${T}
else
	cd ../..
fi
source ./build/envsetup.sh

if [ "X$(echo $ANDROID_PRODUCT_OUT | sed -E 's/.*\///')" == "X" ] ||
   [ "X$(echo $ANDROID_PRODUCT_OUT | sed -E 's/.*\///')" == "Xgeneric" ]
then
	choosecombo ${USE_COMBO}
fi
cd ${OPATH}

#Paranoia since mm doesn't return fail...
rm -f ${ANDROID_PRODUCT_OUT}/obj/lib/libhemullib.so
rm -f ${ANDROID_PRODUCT_OUT}/obj/SHARED_LIBRARIES/libhemullib_intermediates/LINKED/libhemullib.so
rm -f ${ANDROID_PRODUCT_OUT}/symbols/system/lib/libhemullib.so
rm -f ${ANDROID_PRODUCT_OUT}/system/lib/libhemullib.so
rm -rf ${ANDROID_PRODUCT_OUT}/obj/SHARED_LIBRARIES/libhemullib_intermediates

rm -f ${ANDROID_PRODUCT_OUT}/obj/EXECUTABLES/hemul_intermediates/LINKED/hemul
rm -f ${ANDROID_PRODUCT_OUT}/obj/EXECUTABLES/hemul_intermediates/hemul
rm -f ${ANDROID_PRODUCT_OUT}/symbols/system/bin/hemul
rm -f ${ANDROID_PRODUCT_OUT}/system/bin/hemul


( cd libmtime; ( mm 2>&1 ) | grcat conf.android ) && \
( mm  2>&1 ) | grcat conf.android 

