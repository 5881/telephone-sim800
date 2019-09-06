Easy "clone and go" repository for a libopencm3 based project.

# Instructions
 1. git clone https://github.com/libopencm3/libopencm3-template.git your-project
 2. cd your-project
 3. git submodule update --init # (Only needed once)
 4. make -C libopencm3 # (Only needed once)
 5. make -C my-project

# Directories
* my-project contains your application
* my-common-code contains something shared.

# As a template
You should replace this with your _own_ README if you are using this
as a template.

Мобильный телефон на базе модуля SIM800
микроконтроллер stm32f103c8t6
экран st7735 128x160

Возможности:
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
