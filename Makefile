aod:main.cc
	g++ -o $@ $^ -ljsoncpp -lmariadb -std=c++14 -L../cpphttplib/

.PHONY:clean
clean:
	rm -rf aod