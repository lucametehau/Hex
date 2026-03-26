VERSION=1.5-v4

all:
	clang++ -O3 -std=c++20 src/*.cpp src/search/*.cpp src/uhi/*.cpp -o hex-$(VERSION).exe

datagen:
	clang++ -O3 -std=c++20 src/datagen/main.cpp src/search/*.cpp -o hex-datagen.exe

shuffle:
	clang++ -O3 -std=c++20 src/datagen/shuffle.cpp -o shuffle.exe