A simple image editor built with GTK+ which is designed for scribbling on an image before sending it off somewhere. Give it input through stdin, draw some stuff, quit, and it will spit out file contents through stdout in (probably) whatever format you gave it.

##### Building:
```
meson build
cd build
ninja
```

##### Usage:
* Left mouse button/Primary mouse button: draw
* Right mouse button/Secondary mouse button: output results and exit
* Mouse wheel: change colors

##### Example uses:
Editing a picture and saving it to another file
```
cat ./reimu.png | ieditor > ./editedreimu.png
```

Editing a screenshot from `grim`, modifying it, and then uploading it to `imgur`.
```
slurp | grim -g - - | ieditor | curl --request POST \
  --url https://api.imgur.com/3/image \
  --header 'no u' \
  --form image=@-
```
