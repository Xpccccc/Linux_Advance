.PHONY:all
all:udp_client udp_server

udp_client:UdpClient.cc
	g++ -o $@ $^ -std=c++14 -lpthread
udp_server:Main.cc
	g++ -o $@ $^ -std=c++14 -lpthread

.PHONY:clean
clean:
	rm -f udp_server udp_client