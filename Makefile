compile: client server

client:
	g++ -c client.cpp -o ./out/client.o
	g++ ./out/client.o -o a.out

server:
	g++ -c server.cpp -o ./out/server.o
	g++ ./out/server.o -o b.out

runClient:
	./a.out

runServer:
	./b.out
