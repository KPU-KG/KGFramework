copy %1 metalicsmooth.png
"%~dp0texconv.exe" -f BC7_UNORM -pow2 metalicsmooth.png
del metalicsmooth.png