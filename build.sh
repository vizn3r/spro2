#/bin/bash

pio run
pio.exe run --disable-auto-clean -t nobuild -t upload -d "Z:\home\vizn3r\spro2\main"

#BUSID=$1
#
#if [ -z $BUSID ]; then
#  echo "Requires BUSID\n"
#  usbipd.exe list
#  exit 0
#fi
#
#usbipd.exe detach --busid ${BUSID}
#echo "Detached USB"
#
#echo "Attaching USB: $BUSID"
#python3 ./wudo/wsl-sudo.py usbipd.exe bind --force --busid ${BUSID}
#usbipd.exe attach --wsl --busid ${BUSID}
#
#sleep 1s
#
#echo "\bBuilding..."
#pio run --target upload --upload-port /dev/$(ls /dev | grep ttyACM)
#
#usbipd.exe detach --busid ${BUSID}
#echo "Detached USB"
