VERSION=1.5

all:
	clang++ -O3 -std=c++20 src/*.cpp src/search/*.cpp src/uhi/*.cpp -o hex-$(VERSION).exe