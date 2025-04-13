#/bin/bash

BUSID=$1

if [ -z $BUSID ]; then
  echo "Requires BUSID\n"
  usbipd.exe list
  exit 0
fi

echo "Attaching USB: $BUSID"
python3 ./wudo/wsl-sudo.py usbipd.exe bind --force --busid ${BUSID}
usbipd.exe attach --wsl --busid ${BUSID}

echo "Do some epic building here"
lsusb
pio device list
