#include <err.h>
#include <stdlib.h>
#include <string.h>

#include "antialias.h"
#include "image.h"

/*
** Downscale image using a downscale factor
*/
static void downscale_image_aa(struct rgb_image *input,
                               struct rgb_image *output, size_t factor)
{
    for (size_t y = 0; y < output->height; y++)
    {
        for (size_t x = 0; x < output->width; x++)
        {
            size_t average_r = 0;
            size_t average_g = 0;
            size_t average_b = 0;

            // Do the average of a chunk of pixels
            for (size_t y_c = 0; y_c < factor; y_c++)
            {
                for (size_t x_c = 0; x_c < factor; x_c++)
                {
                    struct rgb_pixel currentpixel = rgb_image_get(
                        input, (x * factor) + x_c, (y * factor) + y_c);
                    average_r += currentpixel.r;
                    average_g += currentpixel.g;
                    average_b += currentpixel.b;
                }
            }

            //-------------------
            // Average pixel
            struct rgb_pixel average_pixel;
            average_pixel.r = (average_r / factor) & 0xFF;
            average_pixel.g = (average_g / factor) & 0xFF;
            average_pixel.b = (average_b / factor) & 0xFF;

            rgb_image_set(output, x, y, average_pixel);
        }
    }
}

/*
** Downscale image using Bilinear downscale
*/
static void downscale_image_bilinear(struct rgb_image *input,
                                     struct rgb_image *output)
{
    float Xratio = (((float)input->width - 1) / output->width);
    float Yratio = (((float)input->height - 1) / output->height);
    float x2, y2;
    struct rgb_pixel A, B, C, D;
    size_t x, y, index;
    float blue, green, red;

    for (size_t j = 0; j < output->height; j++)
    {
        for (size_t i = 0; i < output->width; i++)
        {
            x = (size_t)(Xratio * i);
            y = (size_t)(Yratio * j);
            x2 = (Xratio * i) - x;
            y2 = (Yratio * j) - y;
            index = (y * input->width + x);
            A = input->data[index];
            B = input->data[index + 1];
            C = input->data[index + input->width];
            D = input->data[index + input->width + 1];

            red = (A.r * (1 - y2) * (1 - x2) + B.r * x2 * (1 - y2)
                   + C.r * y2 * (1 - x2) + D.r * y2 * x2);
            green = (A.g * (1 - y2) * (1 - x2) + B.g * x2 * (1 - y2)
                     + C.g * y2 * (1 - x2) + D.g * y2 * x2);

            blue = ((A.b) * (1 - y2) * (1 - x2) + (B.b) * x2 * (1 - y2)
                    + (C.b) * y2 * (1 - x2) + (D.b) * y2 * x2);

            output->data[i + j * output->width].r = red;
            output->data[i + j * output->width].g = green;
            output->data[i + j * output->width].b = blue;
        }
    }
}

/*
** Select the correct Anti-alias setting from argument parser
*/
enum aa_type select_alias_opt(char *option)
{
    if (strcmp(option, "=ssaa2x") == 0)
        return ANTIALIAS_SSAA_2X;
    else if (strcmp(option, "=ssaa4x") == 0)
        return ANTIALIAS_SSAA_4X;
    else if (strcmp(option, "=none") == 0)
        return ANTIALIAS_NONE;

    // Wrong option
    return ANTIALIAS_UNKNOWN;
}

/*
** Modify the context of the rendering based on the anti-aliasing selected
*/
void preprocess_antialias(enum aa_type aalias_type, struct rgb_image **image)
{
    // Dereference image
    struct rgb_image *image_p = *image;

    // Used to upscale the image when SSAA is used
    size_t upscale_factor = 0;
    size_t width = image_p->width;
    size_t height = image_p->height;

    // Check if the antialiasing requested is valid
    if (aalias_type == ANTIALIAS_UNKNOWN)
        errx(4, "Invalid anti-aliasing technique requested");

    // Exit when we don't want antialiasing
    if (aalias_type == ANTIALIAS_NONE)
        return;

    if (aalias_type == ANTIALIAS_SSAA_2X)
        upscale_factor = SSAA_2X_UPSCALE_FACTOR;
    else if (aalias_type == ANTIALIAS_SSAA_4X)
        upscale_factor = SSAA_4X_UPSCALE_FACTOR;

    // Logging
    warnx("AA - Using SSAA");
    warnx("AA - Using %lix upscale factor (from %lix%li to %lix%li)",
          upscale_factor, width, height, width * upscale_factor,
          height * upscale_factor);

    // Free the old image and create another with the required dimension
    free(image_p);
    *image = rgb_image_alloc(width * upscale_factor, height * upscale_factor);
}

/*
** Select and apply run a post processing antialiasing
*/
void postprocess_antialias(enum aa_type aalias_type, struct rgb_image **image)
{
    // Dereference image
    struct rgb_image *image_p = *image;
    size_t downscale_factor = 0;
    size_t width = image_p->width;
    size_t height = image_p->height;

    // Exit when we don't want antialiasing
    if (aalias_type == ANTIALIAS_NONE)
        return;

    // Downscale image - Antialiasing
    if (aalias_type == ANTIALIAS_SSAA_2X)
        downscale_factor = SSAA_2X_UPSCALE_FACTOR;
    else if (aalias_type == ANTIALIAS_SSAA_4X)
        downscale_factor = SSAA_4X_UPSCALE_FACTOR;

    // Allocate a new image
    struct rgb_image *downscaled
        = rgb_image_alloc(width / downscale_factor, height / downscale_factor);

    // Logging
    warnx("AA - Downscaling image (from %lix%li to %lix%li)...", width, height,
          width / downscale_factor, height / downscale_factor);

    // Do the downscale
    // downscale_image_aa(image_p, downscaled, downscale_factor);
    downscale_image_bilinear(image_p, downscaled);

    // Logging
    warnx("AA - Completed downscale");

    // Free the old upsampled image
    free(image_p);
    // Replace with the downsampled image
    *image = downscaled;
}
