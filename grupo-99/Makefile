all: server client

server: bin/aurrasd

client: bin/aurras

bin/aurrasd: obj/aurrasd.o obj/auxAurrasd.o
	gcc -g obj/aurrasd.o obj/auxAurrasd.o -o bin/aurrasd

obj/aurrasd.o: src/aurrasd.c include/auxAurrasd.h
	gcc -Wall -g -c src/aurrasd.c -o obj/aurrasd.o 

obj/auxAurrasd.o: src/auxAurrasd.c include/auxAurrasd.h
	gcc -Wall -g -c src/auxAurrasd.c -o obj/auxAurrasd.o

bin/aurras: obj/aurras.o obj/auxAurras.o
	gcc -g obj/aurras.o obj/auxAurras.o -o bin/aurras

obj/aurras.o: src/aurras.c include/auxAurras.h
	gcc -Wall -g -c src/aurras.c -o obj/aurras.o

obj/auxAurras.o: src/auxAurras.c include/auxAurras.h
	gcc -Wall -g -c src/auxAurras.c -o obj/auxAurras.o

clean:
	rm obj/* tmp/* bin/aurras bin/aurrasd

test:
	bin/aurras
	bin/aurras status
	bin/aurras samples/sample-1-so.m4a output.mp3 alto eco rapido
