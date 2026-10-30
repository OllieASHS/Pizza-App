#!/bin/bash

test -d ../build || mkdir -p ../build
pushd ../build

DEBUG_BUILD=0;
RELEASE_BUILD=0;

ARGC=($@)

ARGV="${#ARGC[@]}"

GCC_BUILD=0;
WIN64_BUILD=0;

INDEX=0

FLAGS="-lSDL3 -lSDL3_mixer -lSDL3_ttf"
GCC_OPTS="-I../lib/linux64/include -L../lib/linux64/bin -Wl,-rpath,\$ORIGIN/../lib/linux64/bin"
DEBUG_OPTS="-ggdb -O0 -Wall -Wextra -Wno-unused-function -fsanitize=address"
MINGW64_OPTS="-o pizza.exe -I../lib/win64/SDL3/include -I../lib/win64/SDL3/include/SDL3 -I../lib/win64/SDL3/include/SDL3_mixer -I../lib/win64/SDL3/include/SDL3_ttf -L../lib/win64/SDL3/lib -lmingw32"

function release
{
	if [[ ${GCC_BUILD} == 1 ]]
	then
		gcc -o pizza ../src/sdl3_pizza_app.c -std=c11 -D_GNU_SOURCE $GCC_OPTS $FLAGS -lm && ./pizza
	elif [[ ${WIN64_BUILD} == 1 ]]
	then
		x86_64-w64-mingw32-gcc -std=c11 -D_GNU_SOURCE ../src/sdl3_pizza_app.c $MINGW64_OPTS $FLAGS -mwindows && wine pizza.exe
	fi
}

function debug
{
	gcc -o pizza ../src/sdl3_pizza_app.c -std=c11 -D_GNU_SOURCE -D_DEBUG $DEBUG_OPTS $GCC_OPTS $FLAGS -lm && ./pizza
}

if [[ "${ARGC[$INDEX]}" == "" ]]
then
	echo "Release build by default."
	release
	exit 0
fi


while [ "${ARGC[$INDEX]}" != "" ]
do

	case "${ARGC[$INDEX]}" in
        --debug)    DEBUG_BUILD=1;
                    echo "Debug build requested.";;
        --release)  RELEASE_BUILD=1;
                    echo "Release build requested.";;
        --windows)  WIN64_BUILD=1;
                    echo "mingw 64...";;
        --linux)    GCC_BUILD=1;
                    echo "gcc 64...";;
	    -*)

			case "${ARGC[$INDEX]}" in
                -D) DEBUG_BUILD=1;
                    echo "Debug build requested.";;
                -r) RELEASE_BUILD=1;
                    echo "Release build requested.";;
				-W) WIN64_BUILD=1;
                    echo "mingw 64...";;
                -L) GCC_BUILD=1;
                    echo "gcc 64...";;
				*)  echo "Unrecognized option: ${ARGC[$INDEX]}";
                    exit 1;;

			esac
            ;;

        *)          echo "Unrecognized option: ${ARGC[$INDEX]}";
                    exit 1;;

	esac

    (( INDEX+=1 ))

done

if [[ ${GCC_BUILD} == 0 && ${WIN64_BUILD} == 0 ]]
then
	echo "Specify build tool. --windows or -W for mingw64, --linux or -L for gcc";
	exit 0
fi

if [[ ${GCC_BUILD} == 1 && ${WIN64_BUILD} == 1 ]]
then
	echo "Can't have both dingus.";
	exit 0
fi

if [[ ${DEBUG_BUILD} == 1 ]]
then
	debug
elif [[ ${RELEASE_BUILD} == 1 ]]
then
	release
fi

popd
