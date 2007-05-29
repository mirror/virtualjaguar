#! /bin/sh
#
#  Sets up the Allegro package for building under Unix, converting text
#  files from CR/LF to LF format.


echo "Converting NeoPocott files to Unix format..."

find . -type f "(" \
   -name "*.c*" -o -name "*.cfg" -o \
   -name "*.h" -o -name "*.s" -o \
   -name "*.txt" -o -name "*.asm" -o \
   -name "Makefile*" -o -name "readme.*" \
   ")" \
   -exec sh -c "echo {};
		mv {} _tmpfile;
		tr -d \\\r < _tmpfile > {};
		touch -r _tmpfile {}; 
		rm _tmpfile" \;

echo "Done!"
