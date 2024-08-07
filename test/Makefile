OUTPUT_DIR := ./../out

all: create_output_dir compile run

create_output_dir:
ifeq ($(shell [ ! -d "$(OUTPUT_DIR)" ]; echo $$?), 0)
	@mkdir -p $(OUTPUT_DIR)
endif

compile:
	g++ -c test.cpp -o $(OUTPUT_DIR)/test.o -I../thirdParty/PEGTL/include
	g++ $(OUTPUT_DIR)/test.o -o $(OUTPUT_DIR)/test.out

run:
	./../out/test.out

clean:
	rm -rf $(OUTPUT_DIR)/*