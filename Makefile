obj-m += tp2-5.o
all:
	make -C /usr/src/linux-headers-3.16.0-4-686-pae M=$(PWD) modules

clean:
	make -C /usr/src/linux-headers-3.16.0-4-686-pae M=$(PWD) clean
