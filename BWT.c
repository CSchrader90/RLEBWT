#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include "file.h"
#include "IO.h"
#include "BWT.h"
#include "buckets.h"

//Generate suffix array and use to derive BWT in linear time
void create_BWT(char *inputFileName, char *outputFolderName){

	char *output_s_file    = create_file_name(outputFolderName, inputFileName, ".s");
	char *output_b_file    = create_file_name(outputFolderName, inputFileName, ".b");

	int in_file = open(inputFileName, O_RDONLY);
	int b_file  = open(output_b_file, O_RDWR);
	if(b_file == -1){
		b_file  = open(output_b_file, O_RDWR | O_CREAT);
	}
	int s_file  = open(output_s_file, O_RDWR );
	if(s_file == -1){
		s_file = open(output_s_file, O_RDWR | O_CREAT);
	}

	long unsigned int num_chars = lseek(in_file, 0, SEEK_END); //number of characters
	lseek(in_file, 0, SEEK_SET);

	struct mem_block_node *reading_mem = create_new_mem_block();

	unsigned char *buf_start = malloc(sizeof(char));
	unsigned char *buf_end   = malloc(sizeof(char));
	unsigned char *buf_temp  = malloc(sizeof(char)*2);
	char *buf_S = malloc(sizeof(unsigned char)*2);

	unsigned int start_idx = 0, end_idx = 0;
	int seq_idx;
	int distance = 0;
	int max_dist = 0;
	int num_S = 1;

	bucket_node **S_buckets = new_bucket_list();
	struct m_list *m_lists = create_m_lists(1);
	unsigned int input_counter = 0;
	*buf_start = read_bytes(in_file, reading_mem, input_counter++);
	while(input_counter < num_chars){
		*buf_end = read_bytes(in_file, reading_mem, input_counter++);
		end_idx++;

		//Find sequences of repeated characters
		if(*buf_start == *buf_end){
			while(end_idx<num_chars && *buf_start == *buf_end){
				*buf_end = read_bytes(in_file, reading_mem, input_counter++);
				end_idx++;
			}
		}
		//Add indexes from sequences to lists
		for(seq_idx=start_idx; seq_idx<end_idx; seq_idx++){
			if(distance>0){
				add_to_m_list(m_lists, distance, seq_idx, *buf_start);
			}
			if(*buf_start < *buf_end){
				add_to_bucket_list(S_buckets, seq_idx, *buf_start);
				num_S++;
				distance = 1;
			} else {
				if(distance > 0)
					distance++;
			}

			if(distance>max_dist)
				max_dist = distance;

		}
		*buf_start = *buf_end;
		start_idx = end_idx;
	}
	add_to_bucket_list(S_buckets, end_idx, *buf_end);
	if(distance>0){
		add_to_m_list(m_lists, distance, start_idx, *buf_start); 
	}

	struct bucket_array S_array = bucket_to_array(S_buckets, num_S);
	free_bucket_list(S_buckets);

	//Sort the buckets of S indexes with m-lists
	sortBy_m(S_array, 0, num_S, 1, max_dist, m_lists);
	free_m_lists(m_lists);

	//If there are still buckets of S with multiple values
	unsigned int bucket_start = 0;
	unsigned int bucket_end   = bucket_start;
	while(bucket_start < num_S - 1 && bucket_overfull(S_array, 0, num_S)){
		while(bucket_start < num_S - 1 && S_array.bucket_edge[bucket_start + 1] == true)
			bucket_start++;
		bucket_end = findBucketEnd(S_array, bucket_start, num_S);
		lexo_merge_sort(S_array.array, bucket_start, bucket_end, bucket_end - bucket_start + 1, num_chars, in_file, reading_mem);
		bucket_start = bucket_end + 1;
	}

	//Add type L indexes to front of buckets
	bucket_node **bucket_final = new_bucket_list();
	bucket_node *final_list_head;
	unsigned int S_counter = 1;
	unsigned char next_bucket;

	buf_temp[0] = read_bytes(in_file, reading_mem, num_chars - 2);
	add_to_bucket_list(bucket_final, num_chars - 2, *buf_temp);

	//Determine character the current S_array bucket represents
	buf_S[0] = read_bytes(in_file, reading_mem, S_array.array[S_counter]);
	
	_Bool found = false;
	unsigned char final_list_char = '\0';
	while(final_list_char < ALPHABET_SIZE && !found){
		if(bucket_final[final_list_char]->next!=NULL)
			found = true;
		else 
			final_list_char++;
	}
	
	while(S_counter < num_S || final_list_char < ALPHABET_SIZE){

		if(S_counter < num_S && final_list_char < ALPHABET_SIZE)
			next_bucket = (final_list_char > *buf_S) ?  *buf_S : final_list_char;
		else if(S_counter >= num_S)
			next_bucket = final_list_char;
		else 
			next_bucket = *buf_S;

		if(next_bucket == final_list_char){ //If the next bucket has elements in final_list
			final_list_head = bucket_final[final_list_char];
			while(final_list_head->next!= NULL){
				if(final_list_head->val > 0){
					input_counter = final_list_head->val - 1;

					buf_temp[0] = read_bytes(in_file, reading_mem, input_counter++);
					buf_temp[1] = read_bytes(in_file, reading_mem, input_counter++);

					while(buf_temp[0]==buf_temp[1])
						buf_temp[1] = read_bytes(in_file, reading_mem, input_counter++);

					if(buf_temp[1] < buf_temp[0]) //if T[i-1] is type L
						add_to_bucket_list(bucket_final, final_list_head->val - 1, *buf_temp);
					
				}
				final_list_head = final_list_head->next;
			}
			final_list_char++;

		} else {

			if(S_array.array[S_counter] > 0){
				input_counter = S_array.array[S_counter] - 1;
				buf_temp[0] = read_bytes(in_file, reading_mem, input_counter++);
				buf_temp[1] = read_bytes(in_file, reading_mem, input_counter++);
				
				if(buf_temp[1] < buf_temp[0]){ //if T[i-1] is type L
					add_to_bucket_list(bucket_final, S_array.array[S_counter] - 1, buf_temp[0]);
					S_counter++;
				}				
			}

			buf_S[0] = read_bytes(in_file, reading_mem, S_array.array[S_counter]);

			while(*buf_S == next_bucket && S_counter < num_S){
				if(S_array.array[S_counter] > 0){
					input_counter = S_array.array[S_counter] - 1;
					buf_S[0] = read_bytes(in_file, reading_mem, input_counter++);
					buf_S[1] = read_bytes(in_file, reading_mem, input_counter++);


					if(buf_S[1] < buf_S[0]) //if T[i-1] is type L
						add_to_bucket_list(bucket_final, S_array.array[S_counter] - 1, buf_S[0]);
									
				}

				S_counter++;
				if(S_counter < num_S){
					buf_S[0] = read_bytes(in_file, reading_mem, S_array.array[S_counter]);
				}
			}

			found = false;
			final_list_char = next_bucket + 1;
			while(final_list_char < ALPHABET_SIZE && !found){
				if(bucket_final[final_list_char]->next!=NULL)
					found = true;
				else 
					final_list_char++;
			}	
		}

	}

	//Append S indexes
	for(int i = 0; i < num_S; i++){
		buf_temp[0] = read_bytes(in_file, reading_mem, S_array.array[i]);
		add_to_bucket_list(bucket_final, S_array.array[i], *buf_temp);
	}
	free_bucket_array(S_array);

	//Write to file
	bucket_node *head;
	int temp_idx;
	int bit_count = 0;
	unsigned char b_out = '\0';
	unsigned char prev_char = '\0';

	for(int i = 0; i < ALPHABET_SIZE; i ++){
		head = bucket_final[i];
		while(head->next != NULL){
			temp_idx = head->val;
			if(temp_idx < 1)
				input_counter = num_chars - 1;
			else
				input_counter = head->val - 1;
			*buf_temp = read_bytes(in_file, reading_mem, input_counter++);
			if(bit_count == 8){
				write(b_file, &b_out, 1);
				bit_count = 0;
				b_out = '\0';
			}

			if(*buf_temp != prev_char){
				b_out |= 1 << (7 -bit_count);
				write(s_file, buf_temp, 1);
			} 
			bit_count++;
			prev_char = *buf_temp;
			head = head -> next;
		}
	}

	while(bit_count < 8){
		b_out |= 1 << (7 -bit_count);
		bit_count++;
	}
	write(b_file, &b_out, 1);

	free_bucket_list(bucket_final);
	free(output_s_file);
	free(output_b_file);
	free(buf_start);
	free(buf_end);
	free(buf_temp);
	free(buf_S);
	free_mem_block_list(reading_mem);

	close(in_file);
	close(b_file);
	close(s_file);
}