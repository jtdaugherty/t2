
t2
--

`t2` is an interactive ray tracer. It is written in C and OpenCL C and
runs on OpenCL-compatible devices.

Features:
* Progressive rendering with keyboard and mouse input for camera
  navigation
* Thin lens camera with depth of field (see lens radius setting)
* Monte Carlo rendering (see sample root setting)

Building
--------

Right now `t2` only builds on OS X. To install dependencies and build:
```
$ brew install glfw3 --without-shared-library
$ brew install glew
$ brew install freetype2
$ make
```

Running
-------

Run with defaults:
```
$ ./t2
```
or get help to learn about settings to change:
```
$ ./t2 -h
```

Keyboard Controls
-----------------

* `Esc`, `q` - Quit
* `a` - Move left (strafe)
* `d` - Move right (strafe)
* `w` - Move forward
* `s` - Move backward
* `-`/`+` - Decrease/increase trace depth
* `r`/`R` - Decrease/increase lens radius
* `o` - Toggle overlay display
* `t`/`T` - Decrease/increase sample root
* `Space` - Pause/resume progressive sampling (useful when batch sizes
  result in jerky movement)

Mouse Controls
--------------

* Hold left mouse button and move: change camera viewing direction
  (left-right only)
