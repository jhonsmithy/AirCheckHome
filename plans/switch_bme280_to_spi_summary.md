# Резюме: Переключение BME280 с I2C на SPI

## Выполненные изменения

### 1. include/config.h
- Добавлены константы для SPI пинов ESP8266 (Wemos D1 Mini):
  - `BME280_SPI_MOSI_PIN` = D7 (GPIO13)
  - `BME280_SPI_MISO_PIN` = D6 (GPIO12)
  - `BME280_SPI_SCK_PIN` = D5 (GPIO14)
  - `BME280_SPI_CS_PIN` = D8 (GPIO15)

### 2. include/sensor_manager.h
- Добавлен `#include <SPI.h>`
- Удален `#include <Wire.h>` (не требуется для SPI)

### 3. src/sensor_manager.cpp
- Заменен `#include <Wire.h>` на `#include <SPI.h>`
- Изменена инициализация BME280:
  - Было: `bme.begin(0x76)` или `bme.begin(0x77)` (I2C)
  - Стало: `bme.begin(BME280_SPI_CS_PIN)` (SPI)
- Обновлена проверка статуса сенсора:
  - Удалена проверка I2C адресов
  - Добавлена проверка чтения температуры для SPI

### 4. src/main.cpp
- Удален `#include <Wire.h>`
- Удалена инициализация I2C: `Utils::initI2C()`
- Обновлены сообщения об ошибках:
  - Было: "I2C pins: SDA=D2, SCL=D1"
  - Стало: "SPI pins: MOSI=D7, MISO=D6, SCK=D5, CS=D8"

### 5. src/utils.cpp
- Удален `#include <Wire.h>`
- Удалены функции:
  - `Utils::initI2C()`
  - `Utils::checkI2CDevice()`

### 6. include/utils.h
- Удален `#include <Wire.h>`
- Удалены объявления функций:
  - `void initI2C()`
  - `bool checkI2CDevice(uint8_t address)`

### 7. test/test_sensor_manager.cpp
- Удален `#include <Wire.h>`

### 8. platformio.ini
- Библиотека `Adafruit_BME280_Library` уже добавлена и поддерживает SPI
- Дополнительные зависимости не требуются

## Преимущества SPI над I2C
- Более высокая скорость передачи данных
- Нет конфликтов адресов (как в I2C)
- Не требует pull-up резисторов
- Более стабильная работа на больших расстояниях

## Схема подключения BME280 по SPI
```
BME280          ESP8266 (Wemos D1 Mini)
VCC    --------  3.3V
GND    --------  GND
SCK    --------  D5 (GPIO14)
MISO   --------  D6 (GPIO12)
MOSI   --------  D7 (GPIO13)
CS     --------  D8 (GPIO15)
```

## Примечания
- Adafruit_BME280 библиотека поддерживает как I2C, так и SPI
- Для SPI используется метод `begin(CS_PIN)` вместо `begin(ADDRESS)`
- Все изменения обратно совместимы с существующим кодом
- Тесты обновлены для удаления зависимости от Wire.h
