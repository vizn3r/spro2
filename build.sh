#/bin/bash

pio run --verbose
pio.exe run --disable-auto-clean -t nobuild -t upload -d "Z:$(pwd)" --verbose
