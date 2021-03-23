copy %1 color.png
"%~dp0texconv.exe" -f DXT1 -pow2 color.png
del color.png