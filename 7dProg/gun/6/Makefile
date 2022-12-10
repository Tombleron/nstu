build:
	gcc -Wall -lrt -std=gnu99 server.c -o server.elf
	gcc -Wall -lrt -std=gnu99 child.c -o child.elf

build_debug:
	gcc -Wall -DDEBUG server.c -o server.elf
	gcc -Wall -DDEBUG child.c -o child.elf

run:
	./server.elf & ./child.elf 0 input_file1 & ./child.elf 0 input_file2 & ./child.elf 0 input_file3 

help:
	@echo -e "build\t\t\tcompile project without debug prints"
	@echo -e "build_debug\t\tcompile project with debug prints"
	@echo -e "run\t\t\trun program, it will open ['input_file1', 'input_file2', 'input_file3'] and replace all punctuation with '0'"
