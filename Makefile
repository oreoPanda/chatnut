#Makefile

make = mingw32-make

all:
	cd src && $(make) --quiet chatnut

default:
	cd src && $(make) --quiet chatnut

debug:
	cd src && $(make) --quiet chatnut-debug

clean:
	rm -f *~
	rm -f .*~
	cd src && $(make) --quiet clean
