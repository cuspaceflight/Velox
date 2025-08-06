PORT=/dev/ttyACM0

flash:
	idf.py -p ${PORT} flash

monitor:
	idf.py -p ${PORT} monitor

flash_monitor:
	idf.py -p ${PORT} flash monitor
