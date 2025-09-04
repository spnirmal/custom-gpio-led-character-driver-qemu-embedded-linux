# Custom GPIO LED Character Driver with Userspace Toggle on QEMU Embedded Linux

This project demonstrates a **custom Linux character device driver** for a virtual GPIO LED, running on an **ARM QEMU versatilepb board** using **Buildroot**.

I wanted to create a **full embedded Linux workflow**, from kernel module development to userspace control, showing:

* **Linux kernel driver development** (character device, `/dev/led`)
* **Cross-compilation with Buildroot** and module integration
* **Userspace interaction** via a C application
* **Virtual hardware testing** using QEMU

---

## System Setup

| Component         | Description                                          |
| ----------------- | ---------------------------------------------------- |
| QEMU versatilepb  | ARM virtual board for testing kernel and drivers     |
| Buildroot 2024.08 | Cross-compilation environment and root filesystem    |
| `/dev/led` device | Character device created by the kernel module        |
| Userspace app     | Simple C application that toggles LED on user command|
| Kernel logs       | Monitored via `dmesg` to confirm LED ON/OFF actions  |

The LED here is **virtual**; the kernel logs messages via `printk` instead of controlling physical GPIO pins.QEMU is used in this project instead of physical hardware to provide a fully emulated ARM environment, enabling rapid testing of the kernel module, safe experimentation with GPIO operations, and debugging of userspace interactions. This approach allows us to focus on developing the driver, working with Buildroot, and exploring embedded Linux, rather than spending time on hardware setup.

---

## Kernel Module Architecture

The project implements a **Linux character driver**:

* **Device registration**:

  * `register_chrdev()` – reserves a major number
  * `class_create()` – creates a device class
  * `device_create()` – creates `/dev/led`
* **File operations**:

  * Only implements `.write()` for simplicity
  * Writing `1` → LED ON (`printk` logs “LED: ON”)
  * Writing `0` → LED OFF (`printk` logs “LED: OFF”)

> Memory safety is ensured using `copy_from_user()` and proper cleanup is performed during module unload.

---

## Build & Deployment

### 1. Buildroot Setup

```bash
wget https://buildroot.org/downloads/buildroot-2024.08.tar.gz
tar xvf buildroot-2024.08.tar.gz
cd buildroot-2024.08
make qemu_arm_versatile_defconfig
make menuconfig   # Enable loadable module support + GPIO drivers
make -j$(nproc)
```

### 2. Build Kernel Module

```bash
cd driver
make
```

### 3. Deploy to RootFS

```bash
sudo mount -o loop ../buildroot-2024.08/output/images/rootfs.ext2 /mnt
sudo cp led_gpio.ko /mnt/root/
sudo umount /mnt
```

### 4. Run on QEMU

```bash
qemu-system-arm -M versatilepb \
  -kernel output/images/zImage \
  -dtb output/images/versatile-pb.dtb \
  -drive file=output/images/rootfs.ext2,if=scsi \
  -append "root=/dev/sda console=ttyAMA0,115200" \
  -serial stdio
```

Inside QEMU:

```bash
insmod /root/led_gpio.ko
echo 1 > /dev/led
echo 0 > /dev/led
dmesg | tail
```

---

## Memory & Resource Management

Even though this is running in a virtual environment, good practices were followed:

* Proper allocation and cleanup of **kernel resources** (device, class, major number)
* Minimal kernel footprint — driver only implements `.write()`
* Userspace app uses simple file writes without memory leaks
* Module unloading removes all device nodes and frees kernel objects

---

## Testing & Demo

* **Toggle LED** by writing `1`/`0` to `/dev/led`
* **Check kernel logs** using `dmesg` to see ON/OFF messages
* Userspace C app demonstrates **automatic toggling every second**

> Screenshots of kernel log output and QEMU terminal can be added here to make the demo more visual.

---

## Future Enhancements

* Add **sysfs interface** for advanced control
* Integrate with **real hardware GPIO** on Raspberry Pi or BeagleBone
* Implement **PWM support** for LED brightness control
* Create a **systemd service** for automatic driver loading
* Create a connection between host kernel and QEMU , so that a script running in Host can control the GPIO

---

## Summary

This project illustrates a **complete embedded Linux workflow**:

* Writing and testing a **custom kernel driver**
* Cross-compiling using Buildroot
* Safe **userspace interaction**
* Virtual hardware testing with **QEMU**

It demonstrates professional practices for **driver development**, resource management, and system testing.

