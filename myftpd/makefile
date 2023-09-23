myftpd:myftpd.o stream.o
	gcc myftpd.o stream.o -o myftpd
	
myftpd.o: myftpd.c
	gcc -c myftpd.c
	
stream: stream.c stream.h
	gcc -c stream.c 
	
clean:
	rm *.o
