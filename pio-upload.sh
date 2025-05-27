echo ">>>> UPLOADING"
esptool.py --port COM23 --baud 460800 --chip esp32c6 write_flash --flash_size 4MB --flash_mode dio 0x0 .pio/build/main/bootloader.bin 0x8000 .pio/build/main/partitions.bin 0x10000 .pio/build/main/firmware.bin
read -p "Press Enter to continue" </dev/tty
