#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include "IO.h"
#include "buckets.h"

const int BLOCK_SIZE = 4096;
const int MAX_BLOCKS = 6;

struct mem_block_node *create_new_mem_block(){
	struct mem_block_node *new_mem = malloc(sizeof(struct mem_block_node));
	new_mem->data = NULL;
	new_mem->next = NULL;
	new_mem->block_num = 0;
	return new_mem;
}

struct mem_block_node *add_new_mem_block(int in_file, unsigned int block_num){

	struct mem_block_node *new = malloc(sizeof(struct mem_block_node)); 
	char *data = malloc(BLOCK_SIZE);
	new->block_num = block_num;
	new->next = NULL;
	lseek(in_file, block_num*BLOCK_SIZE, SEEK_SET);
	read(in_file, data, BLOCK_SIZE);
	new->data = data;
	return new;
}

void free_mem_block_list(struct mem_block_node *list){
	struct mem_block_node *temp = list;
	while(list->next != NULL){
		temp = list->next;
		free(list->data);
		free(list);
		list = temp;
	}
	free(list->data);
	free(list);
}

//offset needs to be checked to be in range before passed in
char read_bytes(int in_file, struct mem_block_node *mem_block_node, unsigned int offset){
	unsigned int block_offset = offset%BLOCK_SIZE;
	unsigned int block_num    = offset/BLOCK_SIZE;

	struct mem_block_node *head = mem_block_node;
	struct mem_block_node *temp;
	char *read_buf = malloc(BLOCK_SIZE);
	_Bool found = false;
	char return_byte = '\0';

	int counter = 1;
	int num_held_blocks = 0;

	while(head->next !=NULL){
		num_held_blocks++;
		head = head->next;
	}

	if(num_held_blocks == 0){ //No data currently in memory
		head = add_new_mem_block(in_file, block_num);
		lseek(in_file, block_num*BLOCK_SIZE, SEEK_SET);
		read(in_file, head->data, BLOCK_SIZE);
		head->block_num = block_num;
		mem_block_node->next = head;
		return_byte = head->data[block_offset];
	} else {
		head = mem_block_node->next;
		while(!found && head->next != NULL){
			if(head->block_num == block_num){
				found = true;
			} else {
				counter++;
				head = head->next;

			}
		}

		if(head->block_num != block_num){
			temp = add_new_mem_block(in_file, block_num);
			if(counter < MAX_BLOCKS){
				head->next = temp;
				head = head->next;
			} else {
				head = mem_block_node;
				while(head->next->next!=NULL){
					head = head->next;
				}
				free(head->next->data);
				free(head->next);
				head->next = NULL;
				temp->next = mem_block_node->next;
				mem_block_node->next = temp;
				head = temp;
			}
		} 
		return_byte = head->data[block_offset];
	}
	free(read_buf);
	return return_byte;
}