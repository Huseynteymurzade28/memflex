obj-m += mymemory.o
mymemory-objs := src/kernel.o src/memory.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f tests/test_memory

run: all
	sudo insmod mymemory.ko
	sudo dmesg | tail -n 30
	sudo rmmod mymemory

test:
	gcc -I tests/mocks -I src tests/test_memory.c -o tests/test_memory
	./tests/test_memory
