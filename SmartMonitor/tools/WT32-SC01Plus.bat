set /p usedcom=Televersement de "data" via port COM
echo Utilisation de COM%usedcom%
.\bin\mkspiffs.exe -c ../data -p 256 -b 4096 -s 0x160000 SmartMonitor.spiffs.bin
.\bin\esptool.exe --chip esp32s3 --port com%usedcom% --baud 921600 --after no_reset write_flash -z 0x290000 SmartMonitor.spiffs.bin
del SmartMonitor.spiffs.bin
.\bin\esptool.exe --chip esp32s3 --port COM%usedcom% --baud 921600 --before default_reset --after no_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0    ./bin/S3.bootloader.bin 0x8000 ./bin/S3.partitions.bin 0xe000 ./bin/boot_app0.bin 0x10000 ../SmartMonitor.SC01PLus.ino.bin 

pause
