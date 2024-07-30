OUTPUT_DIR := ./../out

all: create_output_dir compile run

create_output_dir:
ifeq ($(shell [ ! -d "$(OUTPUT_DIR)" ]; echo $$?), 0)
	@mkdir -p $(OUTPUT_DIR)
endif

compile:
	g++ -c json.cpp -o $(OUTPUT_DIR)/json.o -I../thirdParty/PEGTL/include
	g++ $(OUTPUT_DIR)/json.o -o $(OUTPUT_DIR)/json.out

run:
	./../out/json.out

clean:
	rm -rf $(OUTPUT_DIR)/*