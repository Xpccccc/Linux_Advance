.PHONY:all
all:tcp_client tcp_server

# LDFLAG=-DSELF=1 # 触发SELF=1

tcp_client:TcpClient.cc
	g++ -o $@ $^ $(LDFLAG) -std=c++14 -lpthread -ljsoncpp
tcp_server:Main.cc
	g++ -o $@ $^ $(LDFLAG) -std=c++14 -lpthread -ljsoncpp

.PHONY:clean
clean:
	rm -f tcp_server tcp_client