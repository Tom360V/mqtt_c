rm -rf bld
mkdir bld
cd bld
cmake .. -G "MSYS Makefiles" -Wno-dev
cmake ..
make
