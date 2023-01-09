set /p usedcom=Téléversement de "data" via port COM
echo Utilisation de COM%usedcom%
C:\Users\Admin\AppData\Local\Arduino15\packages\esp32\tools\mkspiffs\0.2.3\mkspiffs.exe -c ../data -p 256 -b 4096 -s 1507328 SmartMonitor.spiffs.bin
C:\Users\Admin\AppData\Local\Arduino15\packages\esp32\tools\esptool_py\3.0.0\esptool.exe --chip esp32 --baud 921600 --port com%usedcom% --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0x00290000 SmartMonitor.spiffs.bin
del SmartMonitor.spiffs.bin
C:\Users\Admin\AppData\Local\Arduino15\packages\esp32\tools\esptool_py\3.0.0\esptool.exe --chip esp32 --port COM%usedcom% --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0xe000 ../bin/WT32-SC01/boot_app0.bin 0x1000 ../bin/WT32-SC01/bootloader_qio_80m.bin 0x10000 ../bin/WT32-SC01/SmartMonitorWT32-SC01.ino.bin 0x8000 ../bin/WT32-SC01/SmartMonitorWT32-SC01.partitions.bin 

pause