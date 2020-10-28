all: RLEBWT.c
	gcc -Wall -Werror -g -o RLEBWT RLEBWT.c file.h file.c IO.h IO.c buckets.h buckets.c BWT.h BWT.c 
