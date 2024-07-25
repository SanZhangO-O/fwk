OUTPUT_DIR := ./out

create_output_dir:
ifeq ($(shell [ ! -d "$(OUTPUT_DIR)" ]; echo $$?), 0)
	@mkdir -p $(OUTPUT_DIR)
endif

compile: create_output_dir client server

client:
	g++ -c client.cpp -o $(OUTPUT_DIR)/client.o
	g++ $(OUTPUT_DIR)/client.o -o $(OUTPUT_DIR)/a.out

server:
	g++ -c server.cpp -o $(OUTPUT_DIR)//server.o
	g++ $(OUTPUT_DIR)/server.o -o $(OUTPUT_DIR)/b.out

runClient:
	./$(OUTPUT_DIR)/a.out

runServer:
	./$(OUTPUT_DIR)/b.out

clean:
	rm -rf $(OUTPUT_DIR)/*