#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "buckets.h"

const int ALPHABET_SIZE  = 128;

struct bucket_node **new_bucket_list(){

	struct bucket_node **list = malloc(sizeof(struct bucket_node)*ALPHABET_SIZE);
	for(int i = 0; i<ALPHABET_SIZE; i++){
		list[i] = malloc(sizeof(struct bucket_node));
		list[i]->next = NULL;
	}

	return list;
}

void add_to_bucket_list(struct bucket_node **list, unsigned int index, unsigned char bucket){

	struct bucket_node *head = list[bucket];
	while(head->next!=NULL)
		head = head->next;
	head->val = index;
	head->next = malloc(sizeof(struct bucket_node));
	head = head->next;
	head->next = NULL;
}

struct m_list *create_m_lists(int m){

	struct m_list *First = malloc(sizeof(struct m_list));
	First->next = NULL;
	First->dist = m;
	First->list = new_bucket_list();
	return First;
}


void add_to_m_list(struct m_list *lists, unsigned int distance, unsigned int index, unsigned char bucket){

	struct m_list *cur_list = lists;
	while(cur_list-> dist < distance){
		if(cur_list->next==NULL){
			cur_list->next = create_m_lists(cur_list->dist+1);
		}
		cur_list = cur_list->next;
	}
	add_to_bucket_list(cur_list->list, index, bucket);
}

void free_m_lists(struct m_list *m){
	struct bucket_node *temp;
	struct bucket_node **list = m->list;

	while(m->next!=NULL){
		list = m->list;
		for(int i = 0; i < ALPHABET_SIZE; i ++){
			temp = list[i];
			while(list[i]->next!=NULL){
				temp = list[i]->next;
				free(list[i]);
				list[i] = temp;
			}
		}
		m = m->next;
	}
}


struct bucket_array bucket_to_array(struct bucket_node **S_buckets, int num_S){

	struct bucket_array array;
	struct bucket_node *head;

	array.array = malloc(sizeof(unsigned int)*num_S);
	array.bucket_edge = malloc(sizeof(_Bool)*num_S);

	int index = 0;

	for(int i=0; i<ALPHABET_SIZE; i++){
		head = S_buckets[i];
		if(head->next!=NULL){
			array.array[index] = head->val;
			array.bucket_edge[index++] = true;
			head = head->next;
			while(head->next!=NULL){
				array.array[index] = head->val;
				array.bucket_edge[index++] = false;
				head = head->next;
			}
		}
	}

	return array;
}


_Bool bucketContainsMultipleEntries(struct bucket_array bucket_array, int bucket_start, int bucket_end){
	for(int i = bucket_start; i <= bucket_end; i++){
		if(!bucket_array.bucket_edge[i])
			return true;
	}
	return false;
}

void sortBy_S(struct bucket_array bucket_array, unsigned int num_S, unsigned int dist){
	unsigned int bucket_start = 0;
	unsigned int bucket_end, bucket_size;
	unsigned int tempWriteIndex;

	struct bucket_array tempArray;

	while(bucket_start < num_S - 1){

		while(bucket_start < num_S - 1 && bucket_array.bucket_edge[bucket_start + 1] == true)
			bucket_start++;

		bucket_end = bucket_start;
		while(bucket_end < num_S - 1 && bucket_array.bucket_edge[bucket_end + 1] == false){
			bucket_end = bucket_end + 1;
		}

		bucket_size = bucket_end - bucket_start + 1;
		if(bucket_size > 1){	
			tempArray.array = malloc(sizeof(unsigned int)*bucket_size);
			tempArray.bucket_edge = malloc(sizeof(_Bool)*bucket_size);

			tempWriteIndex = 0;
			for(int i = 0; i < num_S; i ++){
				for(int j = 0; j < bucket_size; j++){
					if(bucket_array.array[bucket_start +j] + dist == bucket_array.array[i]){
						tempArray.array[tempWriteIndex] = bucket_array.array[bucket_start + j];
						tempArray.bucket_edge[tempWriteIndex] = true;
						tempWriteIndex++;
					}
				}
			}

			for(int k = 0; k < bucket_size; k++){
				bucket_array.array[bucket_start + k] = tempArray.array[k];
				bucket_array.bucket_edge[bucket_start + k] = tempArray.bucket_edge[k];
			}

			free(tempArray.array);
			free(tempArray.bucket_edge);
		}

		bucket_start = bucket_end + 1;
	}


}


void sortBy_m(struct bucket_array bucket_array, unsigned int bucket_start, unsigned int num_S, int m, int max_m, struct m_list *m_list){

	unsigned int bucket_size;
	unsigned int bucket_end = bucket_start;
	struct m_list *m_head = m_list;
	struct bucket_node *head = *m_head->list;
	struct bucket_array tempArray;

	_Bool bucket_boundary = true;

	while(bucket_end < num_S - 1 && m <= max_m){

		//Find the last element of current bucket
		while(bucket_end < num_S-1 && !bucket_array.bucket_edge[bucket_end + 1]){
			bucket_end++;
		}
		bucket_size = bucket_end-bucket_start + 1;

		if(bucket_size > 1){
			tempArray.array = malloc(sizeof(unsigned int)*bucket_size);
			tempArray.bucket_edge = malloc(sizeof(_Bool)*bucket_size);
			int tempArrayIndex = 0; 

			//For each letter i
			for(int i = 0; i<ALPHABET_SIZE; i ++){
				bucket_boundary = true;
				head = m_head->list[i];
				//Indexes beginning with character i
				while(head->next != NULL){
					for(int j = 0; j < bucket_size; j++){
						if(bucket_array.array[j + bucket_start] + m == head->val){
							tempArray.array[tempArrayIndex] = bucket_array.array[j+ bucket_start];
							tempArray.bucket_edge[tempArrayIndex++] = bucket_boundary;
							bucket_boundary = false;
						}
					}
					head = head->next;
				}
			}

			//Replace elements of bucket array with those in tempArray
			for(int i = 0; i < bucket_size; i++){
				bucket_array.array[bucket_start + i] = tempArray.array[i];
				bucket_array.bucket_edge[bucket_start + i] = tempArray.bucket_edge[i];
			}

			if(m + 1 <= max_m){
				m_head = m_head->next;
				//For each sub-bucket
				int sub_start = bucket_start; //start of sub-bucket
				int sub_end   = bucket_end;

				while(sub_start < bucket_end){

					if(bucket_array.bucket_edge[sub_start+1] == true){
						sub_start++;
					} else {
						//Find end of bucket
						sub_end = sub_start + 1;
						while(sub_end < bucket_end && bucket_array.bucket_edge[sub_end + 1] == false)
							sub_end++;
						sortBy_m(bucket_array, sub_start, sub_end + 1, m + 1, max_m, m_head);
						sub_start = sub_end + 1;
					}
				}
			}
			free(tempArray.array);
			free(tempArray.bucket_edge);
		}
		bucket_start = bucket_end + 1;
		bucket_end = bucket_start + 1;
	}
}
