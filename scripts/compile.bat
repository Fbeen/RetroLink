@echo off

cd ..

if not exist build mkdir build

echo Compiling...

toolchain\SDCC\bin\sdcc -c -mmcs51 --model-large --xram-size 0x0800 --xram-loc 0x0600 --code-size 0xEFFF -Isrc -Iinclude -DFREQ_SYS=48000000 src\main.c -o build\main.rel
toolchain\SDCC\bin\sdcc -c -mmcs51 --model-large --xram-size 0x0800 --xram-loc 0x0600 --code-size 0xEFFF -Isrc -Iinclude -DFREQ_SYS=48000000 src\util.c -o build\util.rel
toolchain\SDCC\bin\sdcc -c -mmcs51 --model-large --xram-size 0x0800 --xram-loc 0x0600 --code-size 0xEFFF -Isrc -Iinclude -DFREQ_SYS=48000000 src\hardware.c -o build\hardware.rel
toolchain\SDCC\bin\sdcc -c -mmcs51 --model-large --xram-size 0x0800 --xram-loc 0x0600 --code-size 0xEFFF -Isrc -Iinclude -DFREQ_SYS=48000000 src\USBHost.c -o build\USBHost.rel
toolchain\SDCC\bin\sdcc -c -mmcs51 --model-large --xram-size 0x0800 --xram-loc 0x0600 --code-size 0xEFFF -Isrc -Iinclude -DFREQ_SYS=48000000 src\console.c -o build\console.rel
toolchain\SDCC\bin\sdcc -c -mmcs51 --model-large --xram-size 0x0800 --xram-loc 0x0600 --code-size 0xEFFF -Isrc -Iinclude -DFREQ_SYS=48000000 src\config.c -o build\config.rel
toolchain\SDCC\bin\sdcc -c -mmcs51 --model-large --xram-size 0x0800 --xram-loc 0x0600 --code-size 0xEFFF -Isrc -Iinclude -DFREQ_SYS=48000000 src\hid_mouse.c -o build\hid_mouse.rel
toolchain\SDCC\bin\sdcc -c -mmcs51 --model-large --xram-size 0x0800 --xram-loc 0x0600 --code-size 0xEFFF -Isrc -Iinclude -DFREQ_SYS=48000000 src\hid_joystick.c -o build\hid_joystick.rel
toolchain\SDCC\bin\sdcc -c -mmcs51 --model-large --xram-size 0x0800 --xram-loc 0x0600 --code-size 0xEFFF -Isrc -Iinclude -DFREQ_SYS=48000000 src\retro_mouse.c -o build\retro_mouse.rel
toolchain\SDCC\bin\sdcc -c -mmcs51 --model-large --xram-size 0x0800 --xram-loc 0x0600 --code-size 0xEFFF -Isrc -Iinclude -DFREQ_SYS=48000000 src\retro_joystick.c -o build\retro_joystick.rel
toolchain\SDCC\bin\sdcc -c -mmcs51 --model-large --xram-size 0x0800 --xram-loc 0x0600 --code-size 0xEFFF -Isrc -Iinclude -DFREQ_SYS=48000000 src\led.c -o build\led.rel

echo Linking...

toolchain\SDCC\bin\sdcc build\main.rel build\util.rel build\hardware.rel build\USBHost.rel build\console.rel build\config.rel build\hid_mouse.rel build\hid_joystick.rel build\retro_mouse.rel build\retro_joystick.rel build\led.rel -mmcs51 --model-large --xram-size 0x0800 --xram-loc 0x0600 --code-size 0xEFFF -o build\CH559USB.ihx

echo Creating HEX...

toolchain\SDCC\bin\packihx build\CH559USB.ihx > build\CH559USB.hex
toolchain\SDCC\bin\hex2bin -c build\CH559USB.hex

echo Flashing...

tools\chflasher.exe build\CH559USB.bin

rmdir /s /q build
cd scripts

echo Done.
