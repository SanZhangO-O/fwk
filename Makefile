client:
	protoc base.proto --cpp_out=./out
	g++ -c ./out/base.pb.cc -o ./out/base.pb.o -lprotobuf
	g++ -c client.cpp -o ./out/client.o -lprotobuf
	g++ ./out/base.pb.o ./out/client.o -o a.out -lprotobuf
	./a.out

server:
	protoc base.proto --cpp_out=./out
	g++ -c ./out/base.pb.cc -o ./out/base.pb.o -lprotobuf
	g++ -c server.cpp -o ./out/server.o -lprotobuf
	g++ ./out/base.pb.o ./out/server.o -o b.out -lprotobuf
	./b.out
