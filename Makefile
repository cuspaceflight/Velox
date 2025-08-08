PORT=/dev/ttyACM0

build: FORCE
	idf.py build

flash:
	idf.py -p ${PORT} flash

monitor:
	idf.py -p ${PORT} monitor

flash_monitor:
	idf.py -p ${PORT} flash monitor

FORCE: ;
