copy %1 normal.png
"%~dp0texconv.exe" -f DXT1 -pow2 normal.png
del normal.png