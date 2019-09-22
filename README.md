# Мобильный телефон на базе модуля SIM800
микроконтроллер stm32f103c8t6
экран st7735 128x160

# Возможности:
- совершать звонки
- принимать звонки
- отправлять смс (ru/eng)
- принимать смс (ru/eng)
- телефонная книга с функцией быстрого набора
- 8мб flash памяти 
- вывод картинок bmp из flash памяти
- запись файлов в память по протоколу xmodem через UART


Проэкт предоставляется как есть без каких либо гарантий

Код написан на C с использованием библиотеки libopencm3

# Instructions
 
 1. $sudo pacman -S openocd arm-none-eabi-binutils arm-none-eabi-gcc arm-none-eabi-newlib arm-none-eabi-gdb
 2. $git clone https://github.com/5881/telephone-sim800.git
 3. $cd telephone-sim800
 4. $git submodule update --init # (Only needed once)
 5. $make -C libopencm3 # (Only needed once)
 6. $make -C mobile
 7. $cd mobile
 8. $make flash

Александр Белый 2019
