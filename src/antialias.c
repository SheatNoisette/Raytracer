#include <err.h>
#include <stdlib.h>
#include <string.h>

#include "antialias.h"
#include "image.h"

/*
** Downscale image using a downscale factor
*/
void downscale_image_aa(struct rgb_image *input, struct rgb_image *output,
                        size_t factor)
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
    downscale_image_aa(image_p, downscaled, downscale_factor);

    // Logging
    warnx("AA - Completed downscale");

    // Free the old upsampled image
    free(image_p);
    // Replace with the downsampled image
    *image = downscaled;
}
