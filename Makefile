DRIVE_SIZE = 1G

.PHONY: full

.FORCE:

homu.bin: .FORCE
	$(MAKE) -C kernel
	./make_fs.py fs_data.bin $(DRIVE_SIZE)
	cat kernel/kernel.bin fs_data.bin > homu.bin
