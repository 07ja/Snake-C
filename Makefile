build:
	gcc -Wall -std=c99 snake.c -o game

run:
	./game

clean:
	rm game