A simple image editor built with GTK+ which is designed for scribbling on an image before sending it off somewhere. Give it input through stdin, draw some stuff, quit, and it will spit out file contents through stdout in (probably) whatever format you gave it. 

Works for me which is good enough.

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

Editing a screenshot from `grim`, modifying it, and then uploading it to `imgur`. See [this script](https://github.com/rei2hu/random_stuff/blob/master/swayshot).
```
slurp | grim -g - - | ieditor | curl --request POST \
  --url https://api.imgur.com/3/image \
  --header 'no u' \
  --form image=@-
```

##### Other:
If it segfaults when you right click and the backtrace points to strlen and g_strdup... yeah I don't know. I think it's fixed but you never know. I changed the method from `gdk_pixbuf_save_to_stream()` to `gdk_pixbuf_save_to_streamv()` with 2 NULL parameters and it seemed to stop it somehow.
