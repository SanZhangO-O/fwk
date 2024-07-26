OUTPUT_DIR := ./../out

all: create_output_dir compile

create_output_dir:
ifeq ($(shell [ ! -d "$(OUTPUT_DIR)" ]; echo $$?), 0)
	@mkdir -p $(OUTPUT_DIR)
endif

compile:
	g++ -c json.cpp -o $(OUTPUT_DIR)/json.o
	g++ $(OUTPUT_DIR)/json.o -o $(OUTPUT_DIR)/json.out

clean:
	rm -rf $(OUTPUT_DIR)/*