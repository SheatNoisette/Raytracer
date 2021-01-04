#include "bmp.h"
#include "camera.h"
#include "image.h"
#include "normal_material.h"
#include "obj_loader.h"
#include "phong_material.h"
#include "rendering.h"
#include "scene.h"
#include "sphere.h"
#include "triangle.h"
#include "vec3.h"

/*
** Single thread runner, not using pthread. This is a minimal runner
*/
int runner_singlethread(struct rgb_image *image, struct scene *scene,
                        render_mode_f renderer)
{
    // Apply the renderer to every pixels of the image
    for (size_t y = 0; y < image->height; y++)
        for (size_t x = 0; x < image->width; x++)
            renderer(image, scene, x, y);

    // Success!
    return 0;
}
