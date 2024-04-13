cmake -S . -B ./build -DBUILD_ID3_TESTS=ON -DBUILD_SHARED_LIBS=ON -DDEBUG_ID3_SYMBOLS=ON
cd build
make
cd tests
leaks --atExit --list -- ./id3dev_test
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./id3dev_test