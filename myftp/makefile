myftp:myftp.o stream.o
	gcc myftp.o stream.o -o myftp
	
myftp.o: myftp.c
	gcc -c myftp.c
	
stream: stream.c stream.h
	gcc -c stream.c
	
clean: 
	rm *.o
