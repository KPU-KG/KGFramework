copy %1 mask.png
"%~dp0texconv.exe" -f BC7_UNORM -pow2 mask.png
del mask.png