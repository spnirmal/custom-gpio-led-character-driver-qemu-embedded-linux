# Custom GPIO LED Character Driver with Userspace Toggle on QEMU Embedded Linux

This project demonstrates a **custom Linux character device driver** for a virtual GPIO LED, running on an **ARM QEMU versatilepb board** using **Buildroot**.

The aim was to create a **full embedded Linux workflow**, from kernel module development to userspace control:

* **Linux kernel driver development** (character device at `/dev/led`)
* **Cross-compilation with Buildroot** and module integration
* **Userspace interaction** via a C application
* **Virtual hardware testing** using QEMU

Since this is a virtual board, the LED is **simulated** — toggling logs kernel messages via `printk` instead of controlling physical GPIO pins. QEMU provides a safe and flexible environment for driver testing without requiring real hardware.

---

## System Setup

| Component         | Description                                       |
| ----------------- | ------------------------------------------------- |
| QEMU versatilepb  | ARM virtual board for kernel & driver testing     |
| Buildroot 2024.08 | Cross-compilation environment and root filesystem |
| `/dev/led` device | Character device created by the kernel module     |
| Userspace app     | Simple C program that toggles LED on user command |
| Kernel logs       | Monitored via `dmesg` to confirm ON/OFF actions   |

---

## Kernel Module Architecture

The **Linux character driver** implements:

* **Device registration**:

  * `register_chrdev()` – reserves a major number
  * `class_create()` – creates a device class
  * `device_create()` – creates `/dev/led`
* **File operations**:

  * Implements `.write()` for LED control
  * Writing `1` → LED ON (`printk` logs "LED: ON")
  * Writing `0` → LED OFF (`printk` logs "LED: OFF")
  * Any other input → warning log

Proper resource cleanup is done during module unload.

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

### 2. Kernel Configuration

Use the text-based kernel config tool:

```bash
make linux-menuconfig
```

Enable:

* Loadable module support (for `insmod`)
* GPIO support
* Character devices

After saving, the `.config` reflects these changes. Running `make` generates:

* `zImage` (kernel)
* `versatile-pb.dtb` (device tree)
* `rootfs.ext2` (root filesystem)
* Toolchain (cross compiler)

### 3. Deploy Module to RootFS

```bash
sudo mount -o loop ../buildroot-2024.08/output/images/rootfs.ext2 /mnt
sudo cp led_gpio.ko /mnt/root/
sudo umount /mnt
```

### 4. Run QEMU

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

## Driver Code 

### Device Macros & Globals

<img width="912" height="520" alt="image" src="https://github.com/user-attachments/assets/0f64cb99-d54c-42f9-a15a-6f6b98077994" />
* `DEVICE_NAME` → device node name (`/dev/led`)
* `CLASS_NAME` → sysfs class name (`/sys/class/ledctl`)
* `major_number` → dynamically assigned by kernel
* `led_class` → sysfs class handle
* `led_device` → device instance handle

### Write Operation

<img width="1726" height="900" alt="image" src="https://github.com/user-attachments/assets/8e15c718-2b38-4798-b59a-d2258c8562ce" />
* Uses `copy_from_user()` to read data
* `1` → log "LED: ON"
* `0` → log "LED: OFF"
* Otherwise → warning

In real hardware, GPIO control code would replace the `printk`.

### File Operations Table

<img width="880" height="406" alt="image" src="https://github.com/user-attachments/assets/dfb99122-1495-4ab5-8da7-01a3f7db4e7f" />
Defines supported operations (only `.write()` implemented). Example:
```bash
echo 1 > /dev/led   # calls led_write()
```

### Module Initialization & Exit

<img width="1710" height="1204" alt="image" src="https://github.com/user-attachments/assets/399e9605-378f-48c0-b386-354781c583a7" />
* Registers character device
* Creates sysfs class + `/dev/led`
* Logs assigned major number
<img width="1126" height="558" alt="image" src="https://github.com/user-attachments/assets/5d1468c3-34c5-4e34-beaa-3a857a603a12" />
* Cleans up all resources on module unload

---

## Testing & Demo

* Toggle LED using:

  ```bash
  echo 1 > /dev/led
  echo 0 > /dev/led
  ```
* Verify with kernel logs:

  ```bash
  dmesg | tail
  ```

---

## Memory & Resource Management

* Proper allocation and cleanup of kernel resources
* Minimal driver footprint (only `.write()` implemented)
* Safe userspace communication with `copy_from_user()`
* Unload cleans up `/dev/led` and sysfs entries

---

## Future Enhancements

* Add **sysfs interface** for LED control
* Extend driver for **real hardware GPIOs** (e.g., Raspberry Pi, BeagleBone)
* Implement **PWM** for brightness control
* Provide a **systemd service** for auto-loading the driver
* Explore **host-to-QEMU GPIO bridging** for external scripts

---

## Summary

This project showcases a **complete embedded Linux workflow**:

* Writing and testing a **custom kernel driver**
* Cross-compiling with **Buildroot**
* Safe interaction between **userspace and kernelspace**
* Full system testing in **QEMU virtual hardware**

It reflects professional driver development practices, resource management, and embedded Linux testing.
