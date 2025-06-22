# Super‑Resolution Demo (OpenCV DNN SuperRes)

Этот репозиторий демонстрирует увеличение разрешения изображений с помощью
модуля **`dnn_superres`** из OpenCV. Приложение поддерживает как классические
методы интерполяции (bilinear, bicubic), так и ИИ‑модели (EDSR, ESPCN, FSRCNN,
LapSRN). Так же добавлена **опциональная оценка качества**: программа может
автоматически рассчитывать **PSNR** и **SSIM**.

---

## Возможности

| Возможность                | Описание                                                                                                                   |
|----------------------------|----------------------------------------------------------------------------------------------------------------------------|
| Простое масштабирование    | Увеличение «как есть» входного Low‑Res изображения до выбранного масштаба любым поддерживаемым алгоритмом.                 |
| Режим *metrics*            | Программа принимает High‑Res изображение как «истинное», сама понижает его резолюцию, восстанавливает и выводит PSNR+SSIM. |
| Поддерживаемые алгоритмы   | **bilinear**, **bicubic**, **edsr**, **espcn**, **fsrcnn**, **lapsrn**.                                                    |
| Масштаб                    | 2x/3x/4x/8x (8x — только LapSRN).                                                                                          |
| Вывод                      | Изображение сохраняется как `restored_<algo>.png` или `restored_<algo>_m.png` (при `--metrics`).                           |

---

## Требования

* **OpenCV 4.x** (собранный с `opencv_contrib` и флагом `-Dopencv_dnn_superres=ON`).
* Компилятор C++17/20 (GCC, Clang, MSVC).
* **CMake ≥ 3.20** *или* `pkg-config`.

---

## Сборка

### 1. Быстрый запуск через `g++`

```bash
# Linux / macOS
 g++ -std=c++20 main.cpp -o superres $(pkg-config --cflags --libs opencv4)

# Windows (MinGW)
 g++ -std=c++20 main.cpp -o superres.exe -IC:/opencv/include -LC:/opencv/x64/mingw/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_dnn_superres
```

### 2. CMake

```cmake
cmake_minimum_required(VERSION 3.20)
project(SuperResDemo LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
find_package(OpenCV REQUIRED)
add_executable(
        superres 
        main.cpp
        metrics/metrics.cpp
        metrics/metrics.h
)
target_link_libraries(superres PRIVATE ${OpenCV_LIBS})
```

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

---

## Использование

```bash
# Классический апскейл
./superres <image> <algorithm> <scale> [model_path]

# Апскейл + метрики (PSNR, SSIM)
./superres <image> <algorithm> <scale> [model_path] --metrics   # или -m
```

| Аргумент      | Значение                                                                                   |
|---------------|--------------------------------------------------------------------------------------------|
| **`<image>`** | Путь к входному изображению. В режиме `--metrics` это High‑Res GT.                         |
| **`<algorithm>`** | `bilinear` · `bicubic` · `edsr` · `espcn` · `fsrcnn` · `lapsrn`                            |
| **`<scale>`** | 2/3/4/8                                                                                    |
| **`[model_path]`** | Файл`.pb` /`.onnx` для нейросетевых моделей. Не требуется для bilinear/bicubic.            |
| **`--metrics`** | (опционально) Включает расчёт PSNR и SSIM. Программа сама уменьшается → восстанавливается. |

### Примеры

```bash
# FSRCNN 2x, обычный режим
./superres low.png fsrcnn 2 models/FSRCNN_x2.pb

# EDSR 4x, c метриками
./superres lena.png edsr 4 models/EDSR_x4.pb --metrics
```

---

## Метрики качества

* **PSNR** (Peak Signal‑to‑Noise Ratio)
* **SSIM** (Structural Similarity) – рассчитывается по каждому цветному каналу (B, G, R).

> Примечание: показатели имеют смысл **только** в режиме `--metrics`, когда Ground Truth известен.


## Дополнительная информация об upscale-алгоритмах

Ниже представлен обзор шести алгоритмов, поддерживаемых в этом примере, с классификацией по использованию ИИ и рекомендациями по выбору.

### 1. Классические методы (без ИИ)

* **bilinear** (двухлинейная интерполяция): усреднение по двум направлениям (горизонтали и вертикали). Очень быстро, но даёт размытые края и артефакты «мыло».
* **bicubic** (бикубическая интерполяция): учитывает 16 ближайших пикселей и аппроксимирует их кубическими полиномами. Лучше сглаживает границы, но всё ещё размывает мелкие детали.

### 2. Методы на основе AI

### EDSR

* **Скачивание модели:** [здесь](https://github.com/Saafke/EDSR_Tensorflow/tree/master/models)
* **Размер модели:** \~38.5МБ (квантованная версия; оригинал — \~150МБ)
* **Обучение:** 3 дня, batch size 16
* **Поддерживаемые масштабы:** x2, x3, x4
* **Скорость:** < 3с на изображении 256×256 на Intel i7-9700K CPU
* **Преимущество:** высокая точность
* **Недостаток:** медленная и большой размер файла
* **Реализация:** [здесь](https://github.com/Saafke/EDSR_Tensorflow)
* **Оригинальная статья:** [здесь](https://arxiv.org/pdf/1707.02921.pdf)

### ESPCN

* **Скачивание модели:** [здесь](https://github.com/fannymonori/TF-ESPCN/tree/master/export)
* **Размер модели:** \~100КБ
* **Обучение:** \~100 итераций, batch size 32
* **Поддерживаемые масштабы:** x2, x3, x4
* **Скорость:** < 0.01с на изображении 256×256 на Intel i7-9700K CPU
* **Преимущество:** маленький размер и высокая скорость
* **Недостаток:** уступает по визуальному качеству более современным моделям
* **Реализация:** [здесь](https://github.com/fannymonori/TF-ESPCN)
* **Оригинальная статья:** [здесь](https://arxiv.org/pdf/1609.05158.pdf)

### FSRCNN

* **Скачивание модели:** [здесь](https://github.com/Saafke/FSRCNN_Tensorflow/tree/master/models)
* **Размер модели:** \~40КБ (версии FSRCNN-small — \~9КБ)
* **Обучение:** \~30 итераций, batch size 1
* **Поддерживаемые масштабы:** x2, x3, x4
* **Скорость:** < 0.01 с на изображении 256×256 на Intel i7-9700K CPU
* **Преимущество:** быстрая, компактная и при этом точная
* **Недостаток:** не достигает передового уровня точности
* **Реализация:** [здесь](https://github.com/Saafke/FSRCNN_Tensorflow)
* **Оригинальная статья:** [здесь](http://mmlab.ie.cuhk.edu.hk/projects/FSRCNN.html)

### LapSRN

* **Скачивание модели:** [здесь](https://github.com/fannymonori/TF-LapSRN/tree/master/export)
* **Размер модели:** от 1 до 5МБ
* **Обучение:** \~50 итераций, batch size 32
* **Поддерживаемые масштабы:** x2, x4, x8 (мультиуровневая суперрезолюция одним проходом)
* **Скорость:** < 0.1с на изображении 256×256 на Intel i7-9700K CPU
* **Преимущество:** поддержка нескольких масштабов в одном проходе
* **Недостаток:** медленнее ESPCN и FSRCNN, точность ниже, чем у EDSR
* **Реализация:** [здесь](https://github.com/fannymonori/TF-LAPSRN)
* **Оригинальная статья:** [здесь](https://arxiv.org/pdf/1704.03915.pdf)


Подробную информацию про бенчмарки и источники можно найти [здесь](https://github.com/opencv/opencv_contrib/tree/master/modules/dnn_superres)

### Рекомендации по выбору

* **Для скорости** (онлайн-видео, мобильные приложения): FSRCNN или ESPCN.
* **Для максимальной чёткости** (если есть ресурсы на GPU): EDSR или LapSRN.
* **Для простой предпросмотра** без ИИ: bilinear или bicubic.