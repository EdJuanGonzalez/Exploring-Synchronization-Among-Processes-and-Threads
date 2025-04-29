all: threads producer consumer

threads: threads.c 
	gcc threads.c -o threads -Wall -Werror

producer: producer.c
	gcc producer.c -o producer -Wall -Werror

consumer: consumer.c
	gcc consumer.c -o consumer -Wall -Werror -lpthread

clean:
	rm -rf *.o threads.out producer.out consumer.out threads producer consumer