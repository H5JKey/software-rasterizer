#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

// Изображения хранятся как массивы float, каналы пикселя лежат подряд (RGBRGB...).
// Строки идут СНИЗУ ВВЕРХ: пиксель (x, y) лежит по адресу data[(y*w + x)*channels],
// y = 0 - нижняя строка картинки (так же, как в glDrawPixels и в текстурных
// координатах .obj).

// Сохраняет буфер в RGBA PNG (альфа всегда 255). Перед записью к каждому значению
// применяется x^(1/gamma) и результат переводится в [0, 255].
// channels: 1 - только красный канал, 2 - красный и зелёный, 3 и больше - RGB.
// Возвращает ненулевое значение при успехе.
int save_image_f32_png_rgb(const float* data, const char* filename, int w, int h, int channels, float gamma);

// Загружает 3-канальную (RGB) картинку; к каждому значению из [0, 1] применяется x^gamma
// (gamma = 1 - без преобразования). Возвращает массив w*h*3 float (освобождать free())
// или NULL при ошибке.
float* load_image_f32_rgb(const char* filename, int* w, int* h, float gamma);

#ifdef __cplusplus
}
#endif

#endif
