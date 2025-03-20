all : ../lib/can.o ../lib/can_infra.o

CC=/home/teuto/Documents/10.SEC/1.PSE/tools-master/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc


../lib/can.o: src/can.c
	$(CC) -c $^ -o $@

../lib/can_infra.o: src/can_infra.c
	$(CC) -c $^ -o $@

clean:
	rm -rf ../lib/*