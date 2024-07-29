OUTPUT_DIR := ./../out

all: create_output_dir compile run

create_output_dir:
ifeq ($(shell [ ! -d "$(OUTPUT_DIR)" ]; echo $$?), 0)
	@mkdir -p $(OUTPUT_DIR)
endif

compile:
	g++ -c xml.cpp -o $(OUTPUT_DIR)/xml.o -I../thirdParty/PEGTL/include
	g++ $(OUTPUT_DIR)/xml.o -o $(OUTPUT_DIR)/xml.out

run:
	./../out/xml.out

clean:
	rm -rf $(OUTPUT_DIR)/*