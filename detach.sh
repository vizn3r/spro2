#/bin/bash

BUSID=$1

if [ -z $BUSID ]; then
  echo "Requires BUSID\n"
  usbipd.exe list
  exit 0
fi

usbipd.exe detach --busid ${BUSID}
echo "Detached USB"
