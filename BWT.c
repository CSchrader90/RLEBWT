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
#include "BWT.h"
#include "buckets.h"

//Generate suffix array and use to derive BWT in linear time
void create_BWT(char *inputFileName, char *outputFolderName){

	char *output_file_name = create_file_name(outputFolderName, inputFileName, ".bwt");

	int in_file = open(inputFileName, O_RDONLY);
	int out_file = open(output_file_name, O_RDWR | O_CREAT);

	long unsigned int num_chars = lseek(in_file, 0, SEEK_END); //number of characters
	lseek(in_file, 0, SEEK_SET);

	unsigned char *buf_start = malloc(sizeof(char));
	unsigned char *buf_end   = malloc(sizeof(char));
	unsigned char *buf_temp  = malloc(sizeof(char)*2);

	unsigned int start_idx = 0, end_idx = 0;
	int seq_idx;
	int distance = 0;
	int max_dist = 0;
	int num_S = 1;

	struct bucket_node **S_buckets = new_bucket_list();
	struct m_list *m_lists = create_m_lists(1);

	read(in_file, buf_start, 1);
	while(read(in_file, buf_end, 1)){
		end_idx++;

		//Find sequences of repeated characters
		if(*buf_start == *buf_end){
			while(end_idx<num_chars && *buf_start == *buf_end){
				read(in_file, buf_end, 1);
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

	//Sort the buckets of S indexes with m-lists
	sortBy_m(S_array, 0, num_S, 1, max_dist, m_lists);

	//Sort last S-indexes using order of buckes in its own list
	sortBy_S(S_array, num_S, max_dist);

	//If there are still buckets of S with multiple values
	unsigned int bucket_start = 0;
	unsigned int bucket_end   = bucket_start;
	while(bucket_start < num_S - 1 && bucket_overfull(S_array, 0, num_S)){
		while(bucket_start < num_S - 1 && S_array.bucket_edge[bucket_start + 1] == true)
			bucket_start++;
		bucket_end = findBucketEnd(S_array, bucket_start, num_S);
		lexo_merge_sort(S_array.array, bucket_start, bucket_end, bucket_end - bucket_start + 1, num_chars, in_file);
		bucket_start = bucket_end + 1;
	}

	//Add type L indexes to front of buckets
	struct bucket_node **bucket_final = new_bucket_list();
	struct bucket_node *final_list_head;
	char *buf_S = malloc(sizeof(unsigned char)*2);
	unsigned int S_counter = 1;
	unsigned char next_bucket;

	lseek(in_file, num_chars - 2, SEEK_SET);
	read(in_file, buf_temp, 1);

	add_to_bucket_list(bucket_final, num_chars - 2, *buf_temp);

	lseek(in_file, S_array.array[S_counter], SEEK_SET);
	read(in_file, buf_S, 1);//Determine character the current S_array bucket represents

	_Bool found = false;
	unsigned char final_list_char = '\0';
	while(final_list_char < ALPHABET_SIZE && !found){
		if(bucket_final[final_list_char]->next!=NULL)
			found = true;
		else 
			final_list_char++;
	}

	while(S_counter < num_S || final_list_char < ALPHABET_SIZE){

		if(S_counter < num_S && final_list_char < ALPHABET_SIZE){
			next_bucket = (final_list_char > *buf_S) ?  *buf_S : final_list_char;
		}
		else if(S_counter >= num_S){
			next_bucket = final_list_char;
		}
		else {
			next_bucket = *buf_S;
		}

		if(next_bucket == final_list_char){ //If the next bucket has elements in final_list
			final_list_head = bucket_final[final_list_char];
			while(final_list_head->next!= NULL){
				if(final_list_head->val > 0){
					lseek(in_file, final_list_head->val - 1, SEEK_SET);
					read(in_file, buf_temp, 2);
					while(buf_temp[0]==buf_temp[1])
						read(in_file, &buf_temp[1], 1);

					if(buf_temp[1] < buf_temp[0]) //if T[i-1] is type L
						add_to_bucket_list(bucket_final, final_list_head->val - 1, *buf_temp);
					
				}
				final_list_head = final_list_head->next;
			}
			final_list_char++;

		} else {

			if(S_array.array[S_counter] > 0){
				lseek(in_file, S_array.array[S_counter] - 1, SEEK_SET);
				read(in_file, buf_temp, 2);	
				
				if(buf_temp[1] < buf_temp[0]){ //if T[i-1] is type L
					add_to_bucket_list(bucket_final, S_array.array[S_counter] - 1, buf_temp[0]);
					S_counter++;
				}				
			}

			lseek(in_file, S_array.array[S_counter], SEEK_SET);
			read(in_file, buf_S, 1);

			while(*buf_S == next_bucket && S_counter < num_S){
				if(S_array.array[S_counter] > 0){
					lseek(in_file, S_array.array[S_counter] - 1, SEEK_SET);
					read(in_file, buf_S, 2);

					if(buf_S[1] < buf_S[0]) //if T[i-1] is type L
						add_to_bucket_list(bucket_final, S_array.array[S_counter] - 1, buf_S[0]);
									
				}

				S_counter++;
				if(S_counter < num_S){
					lseek(in_file, S_array.array[S_counter], SEEK_SET);
					read(in_file, buf_S, 1);
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
		lseek(in_file, S_array.array[i], SEEK_SET);
		read(in_file, buf_temp, 1);
		add_to_bucket_list(bucket_final, S_array.array[i], *buf_temp);
	}

	struct bucket_node *head;
	int temp_idx;
	for(int i = 0; i <ALPHABET_SIZE; i ++){
		head = bucket_final[i];
		while(head->next != NULL){
			temp_idx = head->val;
			temp_idx--;
			if(temp_idx < 0)
				lseek(in_file, num_chars - 1, SEEK_SET);
			else
				lseek(in_file, head->val - 1, SEEK_SET);
			read(in_file, buf_temp, 1);
			head = head -> next;
			write(out_file, buf_temp, 1);
		}
	}


	free_bucket_list(bucket_final);
	free_bucket_list(S_buckets);
	free_bucket_array(S_array);
	free_m_lists(m_lists);
	free(output_file_name);
	free(buf_start);
	free(buf_end);
	free(buf_temp);
	free(buf_S);
}