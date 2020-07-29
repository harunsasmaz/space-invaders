# Space Invaders

A simple OpenGL-based Space Invaders game.

<p float="left">
    <img src="assets/intro.png" width="224px" height="256px">
    <img src="assets/played.png" width="224px" height="256px">
    <img src="assets/gameover.png" width="224px" height="256px">
</p>

For simplicity and lack of resources, CPU rendering is used.

Environment:

    - C++14
    - g++ compiler

Dependencies:

    - glew
    - glfw3

To install dependencies:

    - MacOS:
        - brew install glfw3
        - brew install glew
    - Ubuntu:
        - sudo apt-get install libglfw3 / sudo apt-get install libglfw3-dev
        - sudo apt-get install libglew-dev

To compile:

```bash
$ make
```
Play:

| Key  | Action |
| ------------- | ------------- |
| <kbd>Left</kbd>  | Slide left  |
| <kbd>Right</kbd>  | Slide right  |
| <kbd>Space</kbd>  | Fire laser |

* There are 6 alien types and 1 player type added under <code>shapes.h</code>. Please, feel free to update shapes and enjoy.

* You can change window size by changing values <code>buffer_width</code> and <code>buffer_height</code> in "main" function.

## Acknowledgements

I want to give special thanks to Nick Tasios and his great tutorial [here](http://nicktasios.nl/posts/space-invaders-from-scratch-part-1.html)
