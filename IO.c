#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "IO.h"
#include "buckets.h"

const int BLOCK_SIZE = 24;
const int READ_BLOCKS = 6; //How many blocks to hold in memory

struct mem_block_node *create_new_mem_block(){
	struct mem_block_node *new_mem = malloc(sizeof(struct mem_block_node));
	new_mem->data = NULL;
	new_mem->next = NULL;
	new_mem->block_num = 0;
	return new_mem;
}

struct mem_block_node *add_new_mem_block(int in_file, unsigned int block_num){

	struct mem_block_node *new = malloc(sizeof(struct mem_block_node)); 
	char *data = calloc(BLOCK_SIZE, sizeof(char));
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

//Speed up the reading in of single bits by holding READ_BLOCKS blocks in memory
char read_bytes(int in_file, struct mem_block_node *mem_block_node, unsigned int offset){
	unsigned int block_offset = offset%BLOCK_SIZE;
	unsigned int block_num    = offset/BLOCK_SIZE;

	struct mem_block_node *head = mem_block_node;
	struct mem_block_node *temp;
	char *read_buf = malloc(BLOCK_SIZE);
	_Bool found = false;
	char return_byte = '\0';

	int counter = 1;

	if(head->next == NULL){ //No data currently in memory
		head = add_new_mem_block(in_file, block_num);
		lseek(in_file, block_num*BLOCK_SIZE, SEEK_SET);
		read(in_file, head->data, BLOCK_SIZE);
		head->block_num = block_num;
		mem_block_node->next = head;
		return_byte = head->data[block_offset];
	} else {
		head = head->next;
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
			if(counter < READ_BLOCKS){
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

//Speed up writing to file by holding blocks in memory/delaying writing to file until block is full
void write_bytes(int out_file, struct mem_block_node *mem_block_node, unsigned int offset, char character) {

	unsigned int block_offset = offset%BLOCK_SIZE;
	unsigned int block_num    = offset/BLOCK_SIZE;

	struct mem_block_node *head = mem_block_node;
	struct mem_block_node *temp;
	char *write_buf = malloc(BLOCK_SIZE);

	if(head->next == NULL){ //No data currently in memory
		head = add_new_mem_block(out_file, block_num);
		head->block_num = block_num;
		head->data[block_offset] = character;
		mem_block_node->next = head;
	} else { //Data found
		head = head->next;
		if(head->block_num != block_num){
			temp = add_new_mem_block(out_file, block_num);
			lseek(out_file, head->block_num*BLOCK_SIZE, SEEK_SET);
			write(out_file, head->data, BLOCK_SIZE);
			free(head->data);
			free(head);
			mem_block_node->next = temp;
			head = temp;
		} 
		head->data[block_offset] = character;
	}
	free(write_buf);
}	

//Output data being held in memory from calls to write_bytes
void write_last_mem_block(int out_file, struct mem_block_node *mem_block_node, unsigned int num_bytes){

	struct mem_block_node *head = mem_block_node->next;
	lseek(out_file, head->block_num*BLOCK_SIZE, SEEK_SET); //set file descriptor to write at beginning if only 1 block present
	unsigned int counter = 0;

	while(head->block_num*BLOCK_SIZE + counter < num_bytes){
		write(out_file, &head->data[counter++], 1);
	}
}