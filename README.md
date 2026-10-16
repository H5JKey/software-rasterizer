# Software Rasterizer

## Сборка под Linux

Задание написано на C++ (C++23). Нужны компилятор (g++ или clang), CMake не ниже
3.10 и GLFW 3.3+.
На Debian/Ubuntu:

```bash
sudo apt install build-essential cmake libglfw3-dev g++
```

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
./build/main_cli [mesh.obj] [texture.png|jpg] [output.png]
```

Загружает меш, применяет к нему текстуру (при наличии), рендерит сцену, состоящую из этого меша
и после сохраняет изображение в указанный файл.


### main_app

```bash
./build/main_app [width] [height]
```

Открывает окно и каждый кадр и рендерит сцену в реальном времени состояющую из нескольких подвижных
объектов. Выводит в консоль FPS, а также воемя затраченное на вывод изображения и сам процесс растеризации.

## Соглашения

