
CXXFLAGS = -stdlib=libc++ -std=c++11 -W -Wall -O3 -march=native -ffast-math
CXX = clang++

spatial: spatial.cc

.PHONY: clean test

test: spatial
	./spatial

clean:
	rm -f spatial

