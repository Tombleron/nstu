build:
	gcc -Wall -lpthread -std=gnu99 main.c -o main.elf

build_debug:
	gcc -Wall -DDEBUG -lpthread -std=gnu99 main.c -o main.elf

run:
	./main.elf 0 input_file1 input_file2 input_file3 

help:
	@echo -e "build\t\t\tcompile project without debug prints"
	@echo -e "build_debug\t\tcompile project with debug prints"
	@echo -e "run\t\t\trun program, it will open ['input_file1', 'input_file2', 'input_file3'] and replace all punctuation with '0'"
