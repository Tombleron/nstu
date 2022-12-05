man("Vlad").
man("Ivan").
man("Anatoly II").
man("Evgeniy").
man("Zahar").
man("Sergey").
man("Mikhail").
man("Viktor").
man("Anatoly I").

woman("Tatyana").
woman("Nataly").
woman("Pelageya").
woman("Lidiya").
woman("Ekaterina").
woman("Alexandra").

parent("Anatoly II", "Zahar").
parent("Tatyana", "Zahar").
parent("Anatoly II", "Ivan").
parent("Tatyana", "Ivan").
parent("Evgeniy", "Vlad").
parent("Nataly", "Vlad").
parent("Anatoly I", "Tatyana").
parent("Ekaterina", "Tatyana").
parent("Viktor", "Anatoly II").
parent("Alexandra", "Anatoly II").
parent("Mikhail", "Evgeniy").
parent("Lidiya", "Evgeniy").
parent("Sergey", "Nataly").
parent("Pelageya", "Nataly").

married("Anatoly II", "Tatyana").
married("Anatoly I", "Ekaterina").
married("Viktor", "Alexandra").
married("Nataly", "Evgeniy").
married("Sergey", "Pelageya").
married("Mikhail", "Lidiya").

father(D, C):- parent(D, C), man(D).
mother(M, C):- parent(M, C), woman(M).

child(C, P):- parent(P, C).

son(S, P):- child(S, P), man(S).
daughter(D, P):- child(D, P), woman(D).

sister(S, B):- parent(P, S), parent(P, B), woman(S).
brother(B, S):- parent(P, S), parent(P, B), man(B).

grandchild(G, P):- child(G, H), child(H, P).