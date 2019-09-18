

client:
	gcc src/defs.c src/client.c  src/client_ds.c -I inc/ -g -o client -lpthread

server:
	gcc src/defs.c src/warehouse_ds.c src/warehouse_db.c -I inc/ -g -o server -lpthread

pipes:
	gcc src/pipes.c -g -o pipes

clean:
	rm client
	rm server
	rm pipes

