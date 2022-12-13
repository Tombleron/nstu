build:
	gcc -Wall -lpthread -ldl -std=gnu99 server.c -o server.elf
	gcc -Wall -lpthread -ldl -std=gnu99 client.c -o client.elf

build_debug:
	gcc	-DDEBUG -Wall -lpthread -ldl -std=gnu99 server.c -o server.elf
	gcc -DDEBUG -Wall -lpthread -ldl -std=gnu99 client.c -o client.elf

run:
	./server.elf 48377  & ./client.elf 48377 0 input_file1 & ./client.elf 48377 0 input_file2 & sleep 1 && ./client.elf 48377 0 shutdown

help:
	@echo -e "build\t\t\tcompile project without debug prints"
	@echo -e "build_debug\t\tcompile project with debug prints"
	@echo -e "run\t\t\trun program, it will open ['input_file1', 'input_file2', 'input_file3'] and replace all punctuation with '0'"
