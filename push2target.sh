#!/bin/bash


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

#Remove previous build to avoid letting failed builds to run by mistake
adb shell rm -f /system/lib/libmtime.so
adb shell rm -f /system/bin/hemul
adb shell rm -rf /system/etc/hemul

#Pushes to target. To be used for devs  purposes only

echo "Pushing {ANDROID_PRODUCT_OUT}/system/lib/libmtime.so ..."
adb push ${ANDROID_PRODUCT_OUT}/system/lib/libmtime.so /system/lib/libmtime.so
echo "Pushing {ANDROID_PRODUCT_OUT}/system/bin/hemul ..."
adb push ${ANDROID_PRODUCT_OUT}/system/bin/hemul /system/bin/hemul
echo "Pushing ./system/etc/hemul/* ..."
set +e
adb shell mkdir /system/etc/hemul/
set -e
FS=$(ls testrc/*.ini); for F in $FS; do echo $F; adb push $F /system/etc/hemul; done


