copy %1 color.png
"%~dp0texconv.exe" -f DXT1 color.png
del color.png