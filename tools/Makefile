# HoCo tools makefile
# Tobias Hoff

.PHONY: all clean rebuild

all: esptool2/esptool2.exe

clean:
	$(MAKE) -C esptool2 clean

rebuild: clean all

esptool2/Makefile:
	git submodule update --init esptool2

esptool2/esptool2.exe: esptool2/Makefile
	$(MAKE) -C esptool2
