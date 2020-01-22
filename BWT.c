#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "file.h"
#include "BWT.h"
#include "buckets.h"


void displayBuckets(struct bucket_node **S_buckets){
	struct bucket_node *head;
	for(int i =0; i<ALPHABET_SIZE; i++){
		head = S_buckets[i];
		if(head->next != NULL){
			printf("Bucket for %c:\n", i);
			while(head->next != NULL){
				printf("%d\n", head->val);
				head = head->next;
			}
		}
	}
}

void displayBucketArray(struct bucket_array array, unsigned int num_chars){
	unsigned int element = 0;
	while(element < num_chars){
		if(array.bucket_edge[element])
			printf("New bucket:\n");
		printf("%d\n", array.array[element]);
		element++;
	}
}

//Generate suffix array and use to derive BWT in linear time
void create_BWT(char *inputFileName, char *inputFolderName){

	char *B_File = create_file_name(inputFolderName, inputFileName, ".b");
	int in_file = open("test.txt", O_RDONLY);
//	int out_file = open(B_File, O_RDWR);
	long unsigned int num_chars = lseek(in_file, 0, SEEK_END); //number of characters
	lseek(in_file, 0, SEEK_SET);

	char *buf_start = malloc(sizeof(char));
	char *buf_end   = malloc(sizeof(char));
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

		for(seq_idx=start_idx; seq_idx<end_idx; seq_idx++){
			if(distance>0){
				add_to_m_list(m_lists, distance, start_idx, *buf_start); 
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

//	displayBuckets(S_buckets);
	struct bucket_array S_array = bucket_to_array(S_buckets, num_S);
	free(S_buckets);
	displayBucketArray(S_array, num_S);

	//Sort the buckets of S indexes with m-lists
	sortSArray(S_array, 0, num_S, 1, max_dist, m_lists);

	free_m_lists(m_lists);
	free(B_File);
	free(buf_start);
	free(buf_end);
}