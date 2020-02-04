#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "buckets.h"

const int ALPHABET_SIZE  = 128;

struct bucket_node **new_bucket_list(){

	struct bucket_node **list = malloc(sizeof(struct bucket_node *)*ALPHABET_SIZE);
	for(int i = 0; i < ALPHABET_SIZE; i++){
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

void free_bucket_list(struct bucket_node **list){

	struct bucket_node *temp;
	for(int i = 0; i < ALPHABET_SIZE; i++){
		while(list[i]->next!= NULL){
			temp = list[i]->next;
			free(list[i]);
			list[i] = temp;
		}
		free(list[i]);
	}
	free(list);
}

struct m_list *create_m_lists(int m){

	struct m_list *First = malloc(sizeof(struct m_list));
	First->dist = m;
	First->list = new_bucket_list();
	First->next = NULL;

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
	struct m_list *temp;
	while(m->next!=NULL){
		free_bucket_list(m->list);
		temp = m->next;
		free(m);
		m = temp;
	}
	free_bucket_list(m->list);
	free(m);
}

struct bucket_array bucket_to_array(struct bucket_node **S_buckets, int num_S){

	struct bucket_array array;
	struct bucket_node *head;

	array.array = malloc(sizeof(unsigned int)*num_S);
	array.bucket_edge = malloc(sizeof(_Bool)*num_S);

	int index = 0;

	for(int i = 0; i < ALPHABET_SIZE; i++){
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

void free_bucket_array(struct bucket_array array){
	free(array.array);
	free(array.bucket_edge);
}

_Bool distFound(_Bool *foundinS, int bucket_size){
	for(int i = 0; i < bucket_size; i++){
		if(!foundinS[i])
			return false;
	}
	return true;
}

_Bool bucket_overfull(struct bucket_array bucket_array, int bucket_start, int bucket_size){
	int j = 0;
	while(j < bucket_size){
		if(bucket_array.bucket_edge[bucket_start + j] == true){
			j++;
		} else {
			return true;
		}
	}
	return false;
}

unsigned int findBucketEnd(struct bucket_array bucket_array, unsigned int bucket_start, unsigned int num_elems){
	unsigned int bucket_end;
	bucket_end = bucket_start;
	while(bucket_end < num_elems - 1 && bucket_array.bucket_edge[bucket_end + 1] == false){
		bucket_end = bucket_end + 1;
	}
	return bucket_end;
}

_Bool lexo_greater(unsigned int *array, int in_file, unsigned int first, unsigned int second, unsigned int sub_array_size, unsigned int num_chars){
	int counter = 0;
	char buf1 = '\0';
	char buf2 = '\0'; 
	_Bool continue_checking = true;
	_Bool return_value;

	while(continue_checking && first + counter < num_chars && second + counter <= num_chars){
		lseek(in_file, first + counter, SEEK_SET);
		read(in_file, &buf1, 1);
		lseek(in_file, second + counter, SEEK_SET);
		read(in_file, &buf2, 1);

		if(buf2 > buf1){
			return_value = false;
			continue_checking = false;
		}
		else if (buf2 < buf1){
			return_value = true;
			continue_checking = false;
		}
		else
			counter++;
	}
	return return_value;
}

void lexo_merge(unsigned int *array, unsigned int ls, unsigned int le, unsigned int rs, unsigned int re, unsigned int sub_array_size, unsigned int num_chars, int in_file){

	unsigned int temp_array_size = re - ls + 1;
	unsigned int *temp_array = malloc(sizeof(unsigned int)*temp_array_size);
	unsigned int l_count = ls;
	unsigned int r_count = rs;
	unsigned int temp_idx = 0;


	while(l_count<=le && r_count<=re)	//while elements in both lists
	{	

		if(lexo_greater(array, in_file, array[l_count], array[r_count], sub_array_size, num_chars)){
			temp_array[temp_idx++] = array[r_count++];
		}
		else{
			temp_array[temp_idx++] = array[l_count++];
		}
	}
	
	while(l_count<=le)	//copy remaining elements of the first list
		temp_array[temp_idx++]=array[l_count++];
		
	while(r_count<=re)	//copy remaining elements of the second list
		temp_array[temp_idx++]=array[r_count++];

	for(int i = 0; i < temp_array_size; i++)
		array[ls + i] = temp_array[i];

	free(temp_array);
}

void lexo_merge_sort(unsigned int *array, unsigned int bucket_start, unsigned int bucket_end, unsigned int sub_array_size, unsigned int num_chars,int in_file){

	if(bucket_start < bucket_end){
		unsigned int mid_point = bucket_start + (bucket_end - bucket_start)/2;
		lexo_merge_sort(array, bucket_start, mid_point, sub_array_size, num_chars, in_file);		//left recursion
		lexo_merge_sort(array, mid_point + 1, bucket_end, sub_array_size, num_chars, in_file);    //right recursion
		lexo_merge(array, bucket_start, mid_point, mid_point + 1, bucket_end, sub_array_size, num_chars, in_file);	
	}
}

void sortBy_S(struct bucket_array bucket_array, unsigned int num_S, unsigned int dist){
	unsigned int bucket_start = 0;
	unsigned int bucket_end, bucket_size;
	unsigned int tempWriteIndex = 0;
	int cand_dist = 1; //candidate distance

	struct bucket_array tempArray;

	while(bucket_start < num_S - 1){

		while(bucket_start < num_S - 1 && bucket_array.bucket_edge[bucket_start + 1] == true)
			bucket_start++;

		bucket_end = findBucketEnd(bucket_array, bucket_start, num_S);
		bucket_size = bucket_end - bucket_start + 1;

		if(bucket_size > 1){	
			tempArray.array = malloc(sizeof(unsigned int)*bucket_size);
			tempArray.bucket_edge = malloc(sizeof(_Bool)*bucket_size);
			tempWriteIndex = 0;
			
			_Bool *foundinS = malloc(sizeof(_Bool)*bucket_size);
			for(int i = 0; i < bucket_size; i ++)
				foundinS[i] = false;
			
			//Find distance to check (all S-indexes can be resolved to an order from other S-indexes at equal distance)
			_Bool end_loop = false;
			for(int i = 1; i <= dist && !end_loop; i++){
				cand_dist = i;
				for(int j = 0; j < bucket_size; j++){
					for(int k = 0; k < num_S; k++){
						if(bucket_array.array[bucket_start + j] + cand_dist == bucket_array.array[k]){
							foundinS[j] = true;
						}
					}
				}
				if(distFound(foundinS, bucket_size)){
					dist = cand_dist;
					end_loop = true;
				}else{
					for(int j = 0; j < bucket_size; j++)
						foundinS[j] = false;
				}
			}

			//If S indexes are found at equal distance from all indexes in bucket
			if(distFound(foundinS, bucket_size)){
				for(int i = 0; i < num_S; i++){
					for(int j = 0; j < bucket_size; j++){
						if(bucket_array.array[bucket_start + j] + dist == bucket_array.array[i]){
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
			}
			free(tempArray.array);
			free(tempArray.bucket_edge);
			free(foundinS);
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
		while(bucket_end < num_S - 1 && !bucket_array.bucket_edge[bucket_end + 1]){
			bucket_end++;
		}
		bucket_size = bucket_end - bucket_start + 1;

		if(bucket_size > 1){
			tempArray.array = malloc(sizeof(unsigned int)*bucket_size);
			tempArray.bucket_edge = malloc(sizeof(_Bool)*bucket_size);
			_Bool *tempArraySet = malloc(sizeof(_Bool)*bucket_size);
			for(int i = 0; i < bucket_size; i++)
				tempArraySet[i] = false;

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
							tempArraySet[j] = true;
							bucket_boundary = false;
						}
					}
					head = head->next;
				}
			}

			if(distFound(tempArraySet, bucket_size)){
				//Replace elements of bucket array with those in tempArray
				for(int i = 0; i < bucket_size; i++){
					bucket_array.array[bucket_start + i] = tempArray.array[i];
					bucket_array.bucket_edge[bucket_start + i] = tempArray.bucket_edge[i];
				}
			} else if(m < max_m){
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

						sortBy_m(bucket_array, sub_start, sub_end + 1, m + 1, max_m, m_head->next);
						sub_start = sub_end + 1;
					}
				}
			}
			free(tempArray.array);
			free(tempArray.bucket_edge);
			free(tempArraySet);
		}
		bucket_start = bucket_end + 1;
		bucket_end = bucket_start;
	}
}