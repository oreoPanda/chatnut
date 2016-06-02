#Makefile

#Copyright (C) 2016 Jonas Fuglsang-Petersen

#This file is part of chatnut.

#chatnut is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.

#chatnut is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with chatnut.  If not, see <http://www.gnu.org/licenses/>.

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
