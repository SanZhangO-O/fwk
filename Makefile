all:
	g++ main.cpp -o a.out
	./a.out

client:
	g++ client.cpp -o b.out
	./b.out


server:
	g++ server.cpp -o c.out
	./c.out