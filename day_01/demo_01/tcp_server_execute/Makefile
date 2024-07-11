.PHONY:all
all:tcp_client tcp_server

tcp_client:TcpClient.cc
	g++ -o $@ $^ -std=c++14 -lpthread
tcp_server:Main.cc
	g++ -o $@ $^ -std=c++14 -lpthread

.PHONY:clean
clean:
	rm -f tcp_server tcp_client