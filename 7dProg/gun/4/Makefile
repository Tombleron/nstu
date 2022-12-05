build:
	gcc -Wall main.c -o main.out
	gcc -Wall child.c -o child.out

build_debug:
	gcc -Wall -DDEBUG main.c -o main.out
	gcc -Wall -DDEBUG child.c -o child.out

run:
	./main.out 0 input_file1 input_file2 input_file3 & ./child.out

help:
	@echo -e "build\t\t\tcompile project without debug prints"
	@echo -e "build_debug\t\tcompile project with debug prints"
	@echo -e "run\t\t\trun program, it will open ['input_file1', 'input_file2', 'input_file3'] and replace all punctuation with '0'"