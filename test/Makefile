.PHONY: all clean file json mysql

# 默认目标，编译所有测试
all: file_util json_util mysql

file: FileUtilTest.cc
	g++ -o $@ $^ -g -std=c++11

json: JsonUtilTest.cc
	g++ -o $@ $^ -g -std=c++11 -ljsoncpp

mysql: MySQLTest.cc
	g++ -o $@ $^ -g -std=c++11 -lmariadb -ljsoncpp

clean:
	rm -f file json mysql
	