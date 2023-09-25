cmake -B build -S .-DDEDUB_SYMBOLS=ON -DBUILD_TESTS=ON
cd build
make
cd tests
./id3v1_test