build:
	gcc main.c -lm -Wall -O3 -o main.elf

build_debug:
	gcc main.c -lm -DDEBUG -Wall -O3 -o main.elf

run:
	./main.elf 10 10 450

help:
	@echo -e "build\t\t\tcompile project without debug prints"
	@echo -e "build_debug\t\tcompile project with debug prints"
	@echo -e "run\t\t\trun program"
