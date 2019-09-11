#./prog3_server 6969 9696
#./prog3_participant CF165-12 6969
#./prog3_observer CF165-12 9696

server: observer participant
	gcc -Wall -o prog3_server prog3_server.c

observer:
	gcc -Wall -o prog3_observer prog3_observer.c

participant:
	gcc -Wall -o prog3_participant prog3_participant.c

clean: 
	rm prog3_server prog3_participant prog3_observer
