# Simple Raytracer

This simple raytracer support:
 * Multi-threading
 * Single threading without PThread support
 * SSAA 2x and SSAA 4x (Super Sampling Anti-Aliasing)
 * Reflections

# Usage

```
SCENE.obj: The scene as Wavefront OBJ format
OUTPUT.bmp: The output image

Here is the optionals arguments:

--normals: Show normals
--distances: Distance renderer
--runner=mt/single: Set a custom runner, a runner is a way to call the renderer
    * 'mt' is the multi-threaded runner
    * 'single' is the mono-thread pthread-less runner
--width=100 --height=100: Set the output image size, by default the image is 100
   x 100 pixels
--threads=4: Set the number of threads for the 'mt' runner, default is 4
--aa=none/ssaa2x/ssaa4x: Set the antialiasing method (none, using SSAA 2X or 4X)
   The default is 'none'
```

# License

This work is licensed under the MIT License, see ```license``` for more details.
