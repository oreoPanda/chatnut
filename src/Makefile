#Makefile

objects = chatnut.o connection.o connection_raw.o file_operations.o gui.o gui_interaction.o response_handlers.o user.o
cflags = -Wall -Wextra -std=c99 `pkg-config --cflags gtk+-3.0`
libs = `pkg-config --libs gtk+-3.0`

chatnut: $(objects)
	echo "Linking "$(objects)
	gcc $(objects) -o $@ $(libs)

%.o: %.c
	echo "Compiling "$<
	gcc -c $< -o $@ $(cflags)

clean:
	echo "Removing executable"
	rm -f chatnut
	echo "Removing objects"
	rm -f *.o
	echo "Removing temporary files created by gedit"
	rm -f *~

# $@: target
# @<: first prerequisite
#  %: any string (%.o is any string with ending.o)
