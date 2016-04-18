#Makefile

#Copyright (C) 2016 Jonas Fuglsang-Petersen

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

objects-debug = chatnut-debug.o connection-debug.o connection_backend-debug.o file_operations-debug.o gui-debug.o gui_interaction-debug.o response_handlers-debug.o user-debug.o
objects = chatnut.o connection.o connection_backend.o file_operations.o gui.o gui_interaction.o response_handlers.o user.o
cflags = -Wall -Wextra -std=c99 `pkg-config --cflags gtk+-3.0`
libs = `pkg-config --libs gtk+-3.0` -lws2_32

chatnut: $(objects)
	echo "Linking "$(objects)
	gcc $(objects) -o $@ $(libs)

chatnut-debug: $(objects-debug)
	echo "Linking "$(objects-debug)
	gcc $(objects-debug) -o $@ $(libs)

%.o: %.c
	echo "Compiling "$<
	gcc -c $< -o $@ $(cflags)

%-debug.o: %.c
	echo "Compiling "$<" for debugging"
	gcc -c $< -o $@ $(cflags) -g

clean:
	echo "Removing executable"
	rm -f chatnut
	rm -f chatnut-debug
	echo "Removing objects"
	rm -f *.o
	echo "Removing temporary files created by gedit"
	rm -f *~

# $@: target
# @<: first prerequisite
#  %: any string (%.o is any string with ending.o)
