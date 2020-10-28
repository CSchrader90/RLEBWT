# Run-Length Encoded BWT (RLEBWT) 

C implementation to generate Burrows-Wheeler Transform in O(n) time (average case) of a text file

## Prerequisites

Linux system calls used for input/output 

## Notes 

This implementation builds several lists in one read of the input: 

**S-list**  - list of indexes for which the character following is lexicographically larger

**m-lists** - lists of indexes of characters which are m characters from previous S index

Single-level skip lists are used with the m-lists to improve speed by reducing the number of linked list
nodes to iterate through when using them to sort the S-list. 


## Running

makefile included

Run with arguments: -m, *input file name*, *Output folder name*


Will work for characters 0-127 ASCII. This can be extended by editing ALPHABET_SIZE in *buckets.c*

The last (sentinel) character of the file must have the lowest, unique ASCII value


## Further improvement

To reduce the required run-time memory (although increase execution time), additional sweeps of the input could be
made for each m-list and freeing this before considering the list for m + 1.

Groups of S-indexes which cannot be resolved to an order by using the m-lists are sorted naively with an implementation of merge sort. 
This can be altered to use the order of subsequent S indexes to improve time performance
