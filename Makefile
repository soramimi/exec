

all: a.out

a.out: main.o
	g++ $^ -o $@

main.o: main.cpp

clean:
	rm *.out
	rm *.user
	rm *.o
