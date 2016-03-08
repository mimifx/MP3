obj-m:=button.o
KDIR:=/home/OK6410/kernel/linux-ok6410/
all:
	make -C $(KDIR) M=$(PWD) modules CROSS_COMPILE=arm-linux- ARCH=arm
clean:
	rm *.ko *.o *.mod.c *.order *.symvers

