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

objects-debug = chatnut-debug.o connection-debug.o connection_backend-debug.o file_operations-debug.o gui-debug.o gui_interaction-debug.o logger-debug.o response_handlers-debug.o user-debug.o
objects = chatnut.o connection.o connection_backend.o file_operations.o gui.o gui_interaction.o logger.o response_handlers.o user.o
cflags = -Wall -Wextra -std=c99
cflags_gtk = -mms-bitfields -pthread -mms-bitfields -IC:/msys64/mingw64/include/gtk-3.0 -IC:/msys64/mingw64/include/cairo -IC:/msys64/mingw64/include -IC:/msys64/mingw64/include/pango-1.0 -IC:/msys64/mingw64/include/atk-1.0 -IC:/msys64/mingw64/include/cairo -IC:/msys64/mingw64/include/pixman-1 -IC:/msys64/mingw64/include -I/mingw64/include/freetype2 -I/mingw64/include/libpng16 -I/mingw64/include/harfbuzz -I/mingw64/include/glib-2.0 -I/mingw64/lib/glib-2.0/include -IC:/msys64/mingw64/include -IC:/msys64/mingw64/include/freetype2 -IC:/msys64/mingw64/include -IC:/msys64/mingw64/include/harfbuzz -IC:/msys64/mingw64/include/libpng16 -IC:/msys64/mingw64/include/gdk-pixbuf-2.0 -IC:/msys64/mingw64/include/libpng16 -IC:/msys64/mingw64/include/glib-2.0 -IC:/msys64/mingw64/lib/glib-2.0/include -IC:/msys64/mingw64/include
libs = -lws2_32
libs_gtk = -LC:/msys64/mingw64/lib -lgtk-3 -lgdk-3 -lgdi32 -limm32 -lshell32 -lole32 -Wl,-luuid -lwinmm -ldwmapi -lz -lpangowin32-1.0 -lpangocairo-1.0 -lpango-1.0 -latk-1.0 -lcairo-gobject -lcairo -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -lintl

chatnut: $(objects)
	echo Linking $(objects)
	gcc $(objects) -o ../$@ $(libs) $(libs_gtk)

chatnut-debug: $(objects-debug)
	echo Linking $(objects-debug)
	gcc $(objects-debug) -o ../$@ $(libs) $(libs_gtk)

%.o: %.c
	echo Compiling $<
	gcc -c $< -o $@ $(cflags) $(cflags_gtk)

%-debug.o: %.c
	echo Compiling $< for debugging
	gcc -c $< -o $@ $(cflags) $(cflags_gtk) -g

clean:
	echo Removing executable
	rm -f chatnut.exe
	rm -f chatnut-debug.exe
	echo Removing objects
	rm -f *.o
	echo Removing temporary files created by gedit
	rm -f *~

# $@: target
# @<: first prerequisite
#  %: any string (%.o is any string with ending.o)
