#ifndef ANTIALIAS_H
#define ANTIALIAS_H

#include "image.h"

// Macros for antialias
#define SSAA_2X_UPSCALE_FACTOR 2
#define SSAA_4X_UPSCALE_FACTOR 4

/*
** Anti-aliasing method supported
*/
enum aa_type
{
    ANTIALIAS_SSAA_2X,
    ANTIALIAS_SSAA_4X,
    ANTIALIAS_NONE,
    ANTIALIAS_UNKNOWN
};

/*
** Anti-aliasing algorithm selected in the argument parser
*/
enum aa_type select_alias_opt(char *option);

/*
** Modify the context of the rendering based on the anti-aliasing selected
*/
void preprocess_antialias(enum aa_type aalias_type, struct rgb_image **image);

/*
** Select and apply run a post processing antialiasing
*/
void postprocess_antialias(enum aa_type aalias_type, struct rgb_image **image);

#endif
