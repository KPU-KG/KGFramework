copy %1 rough.png
"%~dp0texconv.exe" -f BC4_UNORM -pow2 rough.png
del rough.png