all: RLEFM.c
	gcc -std=c99 -Wall -Werror -g -o RLEFM RLEFM.c file.h file.c IO.h IO.c buckets.h buckets.c BWT.h BWT.c 