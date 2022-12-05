build_static:
	gcc -c -fPIC -Wall lib.c
	gcc -shared -o libname.so lib.o
	gcc -g -Wall -o main.out main.c -L. -lname
	rm lib.o

build_static_debug:
	gcc -Wall -DDEBUG -c lib.c
	gcc -Wall -DDEBUG -c main.c
	gcc -o main.out main.o lib.o
	rm lib.o main.o

build_dynamic:
	gcc -Wall -fPIC -c lib.c
	gcc -shared lib.o -o lib.so
	gcc -Wall -DDYNAMIC main.c -o main.out
	rm lib.o

build_dynamic_debug:
	gcc -Wall -DDEBUG -fPIC -c lib.c
	gcc -shared lib.o -o lib.so
	gcc -Wall -DDEBUG -DDYNAMIC main.c -o main.out
	rm lib.o

run:
	LD_LIBRARY_PATH=. ./main.out input_file 0

help:
	@echo -e "build_static\t\t\tcompile project with static library without debug prints"
	@echo -e "build_static_debug\t\tcompile project with static library with debug prints"
	@echo -e "build_dynamic\t\t\tcompile project with dynamic library without debug prints"
	@echo -e "build_dynamic_debug\t\tcompile project with dynamic library with debug prints"
	@echo -e "run\t\t\t\trun program, it will open 'input_file' and replace all punctuation with '0'"