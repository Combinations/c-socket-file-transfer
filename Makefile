LDFLAGS= -L/usr/local/opt/zlib/lib
CPPFLAGS= -I/usr/local/opt/zlib/include

run: 


	clear
	gcc -o tfs server/tfs.c libs/DieWithError.c server/HandleTCPClient.c $(LDFLAGS) $(CPPFLAGS)
	gcc -o tfc client/tfc.c libs/DieWithError.c client/HandleTCPServer.c

	./tfs 0.0.0.0 8080

