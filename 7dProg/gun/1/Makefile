build:
	gcc -Wall main.c -o main.out

build_debug:
	gcc -Wall -DDEBUG main.c -o main.out

run:
	./main.out input_file 0
	cat input_file.out

help:
	@echo -e "build\t\t\tcompile project without debug prints"
	@echo -e "build_debug\t\tcompile project witth debug prints"
	@echo -e "run\t\t\trun program, it will open 'input_file' and replace all punctuation with '0'"