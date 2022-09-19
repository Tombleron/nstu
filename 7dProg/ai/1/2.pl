car("CX-5").
car("Camry").
car("Urus").
car("Aventador").
car("Zonda R").
car("LaFerarri").

color("CX-5", "black").
color("Camry", "black").
color("Urus", "yellow").
color("Aventador", "purple").
color("Zonda R", "grey blue").
color("LaFerarri", "red").

hpower("CX-5", 200).
hpower("Camry", 280).
hpower("Urus", 640).
hpower("Aventador", 690).
hpower("Zonda R", 750).
hpower("LaFerarri", 963).

cost("CX-5", 2700000).
cost("Camry", 3200000).
cost("Urus", 20000000).
cost("Aventador", 30000000).
cost("Zonda R", 82000000).
cost("LaFerarri", 999999999).


fcolor(C):- color(X, C), write(X), write(" is "), write(C),nl,fail.

fhp(H):- hpower(X, HP), HP >= H, write(X),nl,fail.

menu:- repeat, nl,


	write("1: Display cars"), nl,
	write("2: Find cars by color"), nl,
	write("3: Find cars by horse power"), nl,

	read(N), do(N).
	
do(1):- car(X), write(X), write(" is a "), color(X, C), write(C), write(" color car that have "), hpower(X, H), write(H), write(" horsepower and costs "), cost(X, M), write(M) ,nl,fail.
do(2):- write("Enter color: "), read(C), fcolor(C).
do(3):- write("Enter minimal horsepower: "), read(C), fhp(C).
