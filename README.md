# Software Rasterizer

```

## Сборка под Linux

Сборка (из корня шаблона):

```bash
cmake -S . -B build
cmake --build build -j
```

По умолчанию собирается **Release** (`-O3 -march=native`). Для отладки
соберите отдельную Debug-версию в другую папку:

```bash
cmake -S . -B build_debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build_debug -j
```

Измерять скорость растеризатора имеет смысл только в Release-сборке.

## Сборка под Windows


Сборка CMake(из корня шаблона):

```bash
cmake --build build -j --config Release
```

## Запуск


### main_cli

```bash
./build/main_cli <mesh.obj> 
```

### main_app

```bash
./build/main_app [width] [height]
```

