# README

## Описание

Этот пример демонстрирует использование модуля DNN Super Resolution из OpenCV для увеличения разрешения изображений с помощью различных алгоритмов (FSRCNN, EDSR, ESPCN, LapSRN) и классических методов масштабирования (bilinear, bicubic).

## Требования

* OpenCV версии 4.x с модулем `dnn_superres` (включённым при сборке через opencv\_contrib).
* Компилятор C++ (GCC, Clang, MSVC).
* CMake (опционально) или `pkg-config` для сборки.

## Сборка

### Через g++ и pkg-config

```bash
g++ main.cpp -o superres `pkg-config --cflags --libs opencv4`
```

### Через CMake

Создайте `CMakeLists.txt` в той же папке:

```cmake
cmake_minimum_required(VERSION 3.5)
project(DnnSuperResExample)
find_package(OpenCV REQUIRED COMPONENTS dnn_superres imgproc highgui)
add_executable(superres main.cpp)
target_link_libraries(superres ${OpenCV_LIBS})
```

Сборка:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Использование

```bash
./superres <путь к изображению> <алгоритм> <масштаб> [<путь к модели>]
```

* `<путь к изображению>` — входное изображение (JPEG, PNG и т.п.).
* `<алгоритм>`:

    * `bilinear` — билинейная интерполяция.
    * `bicubic` — бикубическая интерполяция.
    * `edsr`, `espcn`, `fsrcnn`, `lapsrn` — методы на основе нейросетей.
* `<масштаб>` — множитель (2, 3 или 4).
* `[<путь к модели>]` — путь к файлу `.pb` (обязательно для нейросетевых алгоритмов).

При отсутствии корректных аргументов будет выведена справка по использованию.

## Предварительно обученные модели

Скачать модели можно из репозитория OpenCV Contrib:
[https://github.com/opencv/opencv\_contrib/tree/master/modules/dnn\_superres/samples/models](https://github.com/opencv/opencv_contrib/tree/master/modules/dnn_superres/samples/models)

Примеры:

* `FSRCNN_x2.pb` для `fsrcnn` с масштабом 2.
* `EDSR_x4.pb` для `edsr` с масштабом 4.

## Пример

```bash
./superres input.jpg fsrcnn 2 models/FSRCNN_x2.pb
```

После выполнения:

* В консоли появится сообщение `Upsampling succeeded.`
* Окно покажет результирующее изображение.
* Результат сохранится в файл `saved.jpg`.

## Лицензия

Исходный код распространяется под лицензией OpenCV (см. LICENSE в корне репозитория OpenCV).
