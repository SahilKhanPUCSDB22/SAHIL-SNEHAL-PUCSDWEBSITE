cc `pkg-config --static --cflags kcgi libpq kcgi-html ` -c -o $1.o $1.c
cc -static -o $1.cgi $1.o `pkg-config --static --libs kcgi libpq kcgi-html`
install -m 0555 $1.cgi /usr/local/www/apache24/cgi-bin/
rm $1.cgi $1.o
