client:
	g++ -c client.cpp -o ./out/client.o
	g++ ./out/client.o -o a.out
	./a.out

server:
	g++ -c server.cpp -o ./out/server.o
	g++ ./out/server.o -o b.out
	./b.out

test:
	g++ json.cpp -o c.out -g
	./c.out