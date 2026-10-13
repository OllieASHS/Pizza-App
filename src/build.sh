#!/bin/bash

test -d ../build || mkdir -p ../build
pushd ../build

DEBUG_BUILD=0;
RELEASE_BUILD=0;

ARGC=($@)

ARGV="${#ARGC[@]}"

INDEX=0

FLAGS="-lSDL3 -lSDL3_mixer -lSDL3_ttf"
GCC_OPTS="-I../lib/linux64/include -L../lib/linux64/bin -Wl,-rpath,\$ORIGIN/../lib/linux64/bin"
DEBUG_OPTS="-ggdb -O0 -Wall -Wextra -Wno-unused-function -fsanitize=address"

function release
{
	gcc -o pizza ../src/sdl3_pizza_app.c -std=c11 -D_GNU_SOURCE $GCC_OPTS $FLAGS -lm && ./pizza
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
	    -*)

			case "${ARGC[$INDEX]}" in
                -D) DEBUG_BUILD=1;
                    echo "Debug build requested.";;
                -r) RELEASE_BUILD=1;
                    echo "Release build requested.";;
				*)  echo "Unrecognized option: ${ARGC[$INDEX]}";
                    exit 1;;

			esac
            ;;

        *)          echo "Unrecognized option: ${ARGC[$INDEX]}";
                    exit 1;;

	esac

    (( INDEX+=1 ))

done

if [[ ${DEBUG_BUILD} == 1 ]]
then
	debug
elif [[ ${RELEASE_BUILD} == 1 ]]
then
	release
fi

popd
