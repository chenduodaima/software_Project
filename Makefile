main:main.cc
	g++ -o $@ $^ -ljsoncpp -lmariadb -std=c++11

.PHONY:clean
clean:
	rm -rf main