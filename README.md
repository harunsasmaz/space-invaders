# Space Invaders

A simple OpenGL-based Space Invaders game.

For simplicity and lack of resources, CPU rendering is used.

To compile:

>g++ -Wall -std=c++11 -O0 -g -o main -lglfw -lglew -framework OpenGL main.cpp

There are 6 alien types and 1 player type added under <shapes.h>. Please, feel free to update shapes and enjoy.

You can change window size by changing values <buffer_width> and <buffer_height> in "main" function.

## Acknowledgements

I want to give special thanks to Nick Tasios and his great tutorial [here](http://nicktasios.nl/posts/space-invaders-from-scratch-part-1.html)
