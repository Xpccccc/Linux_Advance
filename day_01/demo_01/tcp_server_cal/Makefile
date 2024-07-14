.PHONY:all
all:tcp_client tcp_server

tcp_client:TcpClient.cc
	g++ -o $@ $^ -std=c++14 -lpthread -ljsoncpp
tcp_server:Main.cc
	g++ -o $@ $^ -std=c++14 -lpthread -ljsoncpp

.PHONY:clean
clean:
	rm -f tcp_server tcp_client