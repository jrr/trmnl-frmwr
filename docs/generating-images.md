```
magick -size 800x120 xc:"rgb(0,0,0)" \
         -size 800x120 xc:"rgb(85,85,85)" \
         -size 800x120 xc:"rgb(170,170,170)" \
         -size 800x120 xc:"rgb(255,255,255)" \
         -append -depth 2 -colors 4 \
         grayscale_test.png
```

```
xxd -i grayscale_test.png > grayscale_test.h
```