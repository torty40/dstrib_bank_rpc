# Build with modern tirpc
CXX=g++
CC=gcc
CFLAGS+= -O2 -Wall -Wextra -I/usr/include/tirpc -D_GNU_SOURCE
LDFLAGS+= -ltirpc -lsqlite3

BIN=client vb_server bank1_server bank2_server
RPC_GEN=rpcgen

all: $(BIN)

# --- RPC codegen ---
# Bank1 interface
bank1_if.h bank1_if_clnt.c bank1_if_svc.c bank1_if_xdr.c: rpc/bank1_if.x
	$(RPC_GEN) -C -M -h -o bank1_if.h rpc/bank1_if.x
	$(RPC_GEN) -C -M -l -o bank1_if_clnt.c rpc/bank1_if.x
	$(RPC_GEN) -C -M -m -o bank1_if_svc.c rpc/bank1_if.x
	$(RPC_GEN) -C -M -c -o bank1_if_xdr.c rpc/bank1_if.x

# Bank2 interface
bank2_if.h bank2_if_clnt.c bank2_if_svc.c bank2_if_xdr.c: rpc/bank2_if.x
	$(RPC_GEN) -C -M -h -o bank2_if.h rpc/bank2_if.x
	$(RPC_GEN) -C -M -l -o bank2_if_clnt.c rpc/bank2_if.x
	$(RPC_GEN) -C -M -m -o bank2_if_svc.c rpc/bank2_if.x
	$(RPC_GEN) -C -M -c -o bank2_if_xdr.c rpc/bank2_if.x

# Virtual bank interface
vb_if.h vb_if_clnt.c vb_if_svc.c vb_if_xdr.c: rpc/vb_if.x
	$(RPC_GEN) -C -M -h -o vb_if.h rpc/vb_if.x
	$(RPC_GEN) -C -M -l -o vb_if_clnt.c rpc/vb_if.x
	$(RPC_GEN) -C -M -m -o vb_if_svc.c rpc/vb_if.x
	$(RPC_GEN) -C -M -c -o vb_if_xdr.c rpc/vb_if.x

# --- Libraries with SQLite helpers ---
bankdb.o: src/bankdb.c src/bankdb.h
	$(CC) $(CFLAGS) -c src/bankdb.c -o $@

# --- Servers and client ---
bank1_server: bank1_if_svc.c bank1_if_xdr.c src/bank1_impl.c bankdb.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

bank2_server: bank2_if_svc.c bank2_if_xdr.c src/bank2_impl.c bankdb.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

vb_server: vb_if_svc.c vb_if_xdr.c vb_if_clnt.c bank1_if_clnt.c bank2_if_clnt.c src/vb_impl.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

client: vb_if_clnt.c vb_if_xdr.c src/client.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(BIN) *.o *_if_*.c *_if.h
	rm -f bank1.db bank2.db
	rm -f nohup.out

# --- Test automation ---
test: all
	@echo "Starting rpcbind if needed..."
	- sudo service rpcbind start || true

	@echo "Launching BANK1 and BANK2 servers..."
	- pkill bank1_server || true
	- pkill bank2_server || true
	- pkill vb_server || true
	nohup ./bank1_server & sleep 0.5
	nohup ./bank2_server & sleep 0.5
	nohup ./vb_server & sleep 0.5

	@echo "Running client script..."
	@echo "credit A12345 100" > test_input.txt
	@echo "credit B12345 100" >> test_input.txt
	@echo "transfer A12345 B12345 25" >> test_input.txt
	@echo "transfer B12345 A12345 10" >> test_input.txt
	@echo "transfer B12345 A12345 120" >> test_input.txt
	@./client < test_input.txt

	@echo; echo "=== BANK1 DB ==="
	@sqlite3 bank1.db "select account, balance from accounts order by account;"
	@echo; echo "=== BANK2 DB ==="
	@sqlite3 bank2.db "select account, balance from accounts order by account;"
