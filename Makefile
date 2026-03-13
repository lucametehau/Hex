VERSION=1.1

all:
	g++ -O3 -std=c++20 src/*.cpp src/search/*.cpp src/uhi/*.cpp -o hex-$(VERSION)