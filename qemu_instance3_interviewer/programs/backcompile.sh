cc `pkg-config --static --cflags kcgi libpq libsodium` -c -o $1.o $1.c
cc -static -o $1.cgi $1.o `pkg-config --static --libs kcgi libpq libsodium`
rm $1.o
