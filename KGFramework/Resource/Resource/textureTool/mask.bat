copy %1 mask.png
"%~dp0texconv.exe" -f DXT5 -pow2 mask.png
del mask.png