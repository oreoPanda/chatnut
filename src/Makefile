#Makefile

objects-debug = chatnut-debug.o connection-debug.o connection_raw-debug.o file_operations-debug.o gui-debug.o gui_interaction-debug.o response_handlers-debug.o user-debug.o
objects = chatnut.o connection.o connection_raw.o file_operations.o gui.o gui_interaction.o response_handlers.o user.o
cflags = -Wall -Wextra -std=c99 `pkg-config --cflags gtk+-3.0`
libs = `pkg-config --libs gtk+-3.0`

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
