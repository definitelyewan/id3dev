cmake -B build -S .-DDEDUB_SYMBOLS=ON -DBUILD_TESTS=ON -DBUILD_SHARED_LIBS=ON
cd build
make
cd tests
valgrind --leak-check=full ./id3v1_test