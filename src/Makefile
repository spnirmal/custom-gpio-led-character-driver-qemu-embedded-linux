obj-m += led_gpio.o

all:
	make ARCH=arm CROSS_COMPILE=arm-buildroot-linux-gnueabi- -C /home/nirmal/Desktop/buildroot-2024.08/output/build/linux-6.6.32 M=$(PWD) modules

clean:
	make ARCH=arm CROSS_COMPILE=arm-buildroot-linux-gnueabi- -C /home/nirmal/Desktop/buildroot-2024.08/output/build/linux-6.6.32 M=$(PWD) clean
