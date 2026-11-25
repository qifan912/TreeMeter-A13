#ifndef TREE_MATH_H
#define TREE_MATH_H
#include <math.h>
#include "tree_config.h"
static int tree_angle_valid(float a)
{
    return a >= -TREE_LIMIT_DEG && a <= TREE_LIMIT_DEG;
}
static int tree_horizontal(unsigned int mm, float angle, float *distance)
{
    if (mm < 200U || mm > 20000U || !tree_angle_valid(angle)) return 0;
    *distance = (float)mm * 0.001f * cosf(angle * 0.0174532925199433f);
    return *distance >= 0.2f && *distance <= 20.0f;
}
static int tree_height(float distance, float top, float base, float *height)
{
    float h;
    if (!(distance >= 0.2f && distance <= 20.0f) ||
        !tree_angle_valid(top) || !tree_angle_valid(base) || !(top > base)) return 0;
    h = distance * (tanf(top * 0.0174532925199433f) -
                    tanf(base * 0.0174532925199433f));
    if (!(h > 0.0f && h < 250.0f)) return 0;
    *height = h;
    return 1;
}
#endif
