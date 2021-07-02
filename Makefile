.PHONY: clean

all:
	i686-w64-mingw32-gcc -Os -Wconversion -Wsign-conversion -Wextra -Wall -Werror -pedantic -mwindows -municode -lm ./src/*.c ./deps/**/*.c -o ./bin/output -lgdi32 -lgdiplus

run: all
	wine64 ./bin/output.exe

clean:
	# if [ -d "./bin" ]; then rm ./bin/*; fi
