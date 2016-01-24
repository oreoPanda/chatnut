
#Makefile

all:
	cd src && make --quiet chatnut

default:
	cd src && make --quiet chatnut

clean:
	rm -f .*~
	cd src && make --quiet clean

remove:
	cd src && make --quiet remove
