test:hyperloglogcount.o murmurhash.o
	g++ -o test hyperloglogcount.o murmurhash.o test.cpp -std=c++11

hyperloglogcount.o: hyperloglogcount.cpp hyperloglogcount.h
	g++ -c hyperloglogcount.cpp 
murmurhash.o:murmurhash.c murmurhash.h
	g++ -c murmurhash.c

clean:
	rm -rf test hyperloglogcount.o murmurhash.o
