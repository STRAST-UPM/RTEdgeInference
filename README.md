# RTEdgeInference

Guía de uso del runtime de inferencia en el edge para ESP32 y STM32.

## Contenido

- `edge/common`: runtime compartido.
- `edge/platform/esp32`: aplicación ESP-IDF.
- `edge/platform/stm32`: firmware STM32 con FreeRTOS y TFLite Micro.
- `examples`: configuración, un modelo TFLite de ejemplo y unas pocas entradas de prueba.
- `tools`: generación de los archivos que se compilan dentro del firmware y monitor serie STM32.

Los ejemplos son una pequeña muestra de `exp11`; no incluyen resultados, trazas ni el resto de sus modelos/datos.

## Requisitos

- GNU Make, Python 3 y `pip`.
- Para ESP32: ESP-IDF 6.0, activado en la terminal mediante su `export.sh`.
- Para STM32: ESP-IDF activado (se usa para resolver TFLite Micro), CMake, Arm GNU Toolchain con newlib/libstdc++ y OpenOCD.
- `xxd` para incluir el modelo TFLite en el firmware.

Instala los módulos Python:

```sh
python3 -m pip install -r requirements.txt
```

La primera resolución de dependencias necesita acceso a Internet. ESP-IDF obtiene los componentes ESPressif y CMake obtiene CMSIS/FreeRTOS para STM32.

## Compilar

ESP32:

```sh
make build PLATFORM=esp32
```

STM32:

```sh
make build PLATFORM=stm32 STM32_HW_MODEL=STM32_HW_MODEL_H743ZI2
```

El proyecto usa por defecto `examples/config.yaml`, `examples/models` y `examples/data/events_serial.txt`. Para probar otro modelo, indica `MODEL_DIR` (debe contener `.tflite` y `models_props.csv`); puedes cambiar también `CONFIG_FILE` y `EVENTS_FILE`.

## Flashear y monitorizar

ESP32:

```sh
make run PLATFORM=esp32 PORT=/dev/ttyUSB0
```

STM32 mediante ST-Link:

```sh
make run PLATFORM=stm32 PORT=/dev/ttyACM0
```

El puerto serie y el baudrate dependen de la placa y del adaptador; se pueden indicar con `PORT` y `BAUD`.
Para flashear sin abrir el monitor, usa `make flash PLATFORM=esp32 PORT=...` o `make flash PLATFORM=stm32`.

## Ejemplos

- `examples/models/*.tflite`: modelo de demostración compilado dentro del firmware.
- `examples/models/models_props.csv`: tiempo estimado, umbral y eventos asociados al modelo.
- `examples/data/*-inout.csv`: vectores de entrada junto con la clase esperada.
- `examples/data/events_serial.txt`: secuencias de eventos usadas por el modo de lectura desde memoria.

`make clean PLATFORM=esp32` o `make clean PLATFORM=stm32` elimina únicamente la salida de compilación de esa plataforma.
