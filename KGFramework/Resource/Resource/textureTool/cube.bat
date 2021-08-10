copy %1 cube.dds
"%~dp0texconv.exe" -f DXT1 -pow2 cube.dds
pause