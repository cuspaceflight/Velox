build: FORCE
	idf.py build

flash:
	idf.py flash

monitor:
	idf.py monitor

flash_monitor:
	idf.py flash monitor

menuconfig:
	idf.py menuconfig

FORCE: ;
