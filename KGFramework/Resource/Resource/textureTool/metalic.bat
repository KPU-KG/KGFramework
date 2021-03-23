copy %1 metalic.png
"%~dp0texconv.exe" -f BC4_UNORM -pow2 metalic.png
del metalic.png