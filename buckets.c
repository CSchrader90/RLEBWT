#include <stdbool.h>
#include <stdlib.h>
#include "IO.h"
#include "buckets.h"

const int ALPHABET_SIZE = 128;
//SKIP_DISTANCE to give a skip distance proportional to frequency of characters in English text
//when scanning through m-lists
const int SKIP_DISTANCE[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2000,\
	1,1,1,1,1,1,1,1,1,1,1,1,1,160, 1, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 1, 1, 1, 1, 1, 1, 1, \
	33, 13,15, 9, 8, 11, 5, 12, 21, 2, 3, 7, 11, 7, 22, 12, 1, 8, 19, 50, 3, 2, 16, 1, 2, 1, 1, 1, \
	1, 1, 1, 1, 960, 364, 430, 261, 230, 330, 134, 345, 599, 42, 71, 198, 314, 187, 627, 355, 18, \
	232, 549, 1313, 97, 67, 451, 37, 63, 4, 1, 1, 1, 1, 1};
const int SKIP_DISTANCE_MULTIPLIER = 4935;

//Forward declarations of skip list functions for use by m-list functions
struct skip_list *new_skip_list();
void add_to_skip_list(bucket_node **list, struct skip_list *skip_list, unsigned int index, unsigned char bucket);
void free_skip_list(struct skip_list *skip_list);

//Bucket list functions
bucket_node **new_bucket_list(){

	bucket_node **list = malloc(sizeof(bucket_node *)*ALPHABET_SIZE);
	for(int i = 0; i < ALPHABET_SIZE; i++){
		list[i] = malloc(sizeof(bucket_node));
		list[i]->next = NULL;
		list[i]->last = list[i];
	}
	return list;
}

void add_to_bucket_list(bucket_node **list, unsigned int index, unsigned char bucket){

	bucket_node *head = list[bucket]->last;
	head->val = index;
	head->next = malloc(sizeof(bucket_node));
	head = head->next;
	head->next = NULL;
	list[bucket]->last = head;
}

void free_bucket_list(bucket_node **list){

	bucket_node *temp;
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

//m-list functions
struct m_list *create_m_lists(int m){

	struct m_list *new = malloc(sizeof(struct m_list));
	new->dist = m;
	new->list = new_bucket_list();
	new->next = NULL;
	new->skip_list = new_skip_list();

	return new;
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
	add_to_skip_list(cur_list->list, cur_list->skip_list, index, bucket);
}

void free_m_lists(struct m_list *m){
	struct m_list *temp;
	while(m->next!=NULL){
		free_bucket_list(m->list);
		free_skip_list(m->skip_list);
		temp = m->next;
		free(m);
		m = temp;
	}
	free_bucket_list(m->list);
	free_skip_list(m->skip_list);
	free(m);
}

//skip-list functions for optimisation of m-lists
struct skip_list *new_skip_list(){
	struct skip_list *new = malloc(sizeof(struct skip_list));
	skip_list_node **list = malloc(sizeof(skip_list_node *)*ALPHABET_SIZE);
	unsigned int *count   = malloc(sizeof(unsigned int)*ALPHABET_SIZE);
	for(int i = 0; i < ALPHABET_SIZE; i++){
		list[i] = malloc(sizeof(skip_list_node));
		list[i]-> next = NULL;
		list[i]-> m_list_node = NULL;
		list[i]-> index = 0;
		count[i] = 0;
	}
	new->list = list;
	new->count = count;

	return new;
}

void add_to_skip_list(bucket_node **list, struct skip_list *skip_list, unsigned int index, unsigned char bucket){
	if(++skip_list->count[bucket] == SKIP_DISTANCE[bucket]*SKIP_DISTANCE_MULTIPLIER){
		skip_list_node *head = skip_list->list[bucket];

		while(head->next != NULL){
			head = head->next;
		}

		skip_list_node *new = malloc(sizeof(skip_list_node));
		new -> index = 0;
		new -> m_list_node = NULL;
		new -> next = NULL;

		head -> index = index;
		head -> m_list_node = list[bucket]->last;
		head -> next = new;

		skip_list->count[bucket] = 0;
	}
}

void free_skip_list(struct skip_list *skip_list){
	skip_list_node *temp;
	skip_list_node *head;
	for(int i = 0; i < ALPHABET_SIZE; i++){
		head = skip_list->list[i];
		while(head->next != NULL){
			temp = head->next;
			free(head);
			head = temp;
		}
		free(head);

	}
	free(skip_list->list);
	free(skip_list->count);
	free(skip_list);
}

//bucket arrays for intuitive representation of S buckets
struct bucket_array new_bucket_array(unsigned int size){
	struct bucket_array new;
	new.array = malloc(sizeof(unsigned int)*size);
	new.bucket_edge = malloc(sizeof(unsigned int)*size);
	return new;
}

//convert the list of linked-lists into array form. Use a boolean array to mark edges
struct bucket_array bucket_to_array(bucket_node **S_buckets, int num_S){

	struct bucket_array array;
	bucket_node *head;

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

//an order for the indices in bucket found for dist m
_Bool distFound(_Bool *foundinS, int bucket_size){
	for(int i = 0; i < bucket_size; i++){
		if(!foundinS[i])
			return false;
	}
	return true;
}

//bucket contains indexes that are still to be resolved to an order
_Bool bucket_overfull(struct bucket_array bucket_array, int bucket_start, int bucket_size){
	int j = 0;
	while(j < bucket_size){
		if(bucket_array.bucket_edge[bucket_start + j] == true) j++; 
		else return true;
	}
	return false;
}

//Find the last element in the array that belongs to bucket beginning at bucket_start
unsigned int findBucketEnd(struct bucket_array bucket_array, unsigned int bucket_start, unsigned int num_elems){
	unsigned int bucket_end;
	bucket_end = bucket_start;
	while(bucket_end < num_elems - 1 && bucket_array.bucket_edge[bucket_end + 1] == false) bucket_end = bucket_end + 1;
	return bucket_end;
}

//Determine if substring beginning at first is lexicographically larger than that beginning at second
_Bool lexo_greater(unsigned int *array, int in_file, struct mem_block_node *reading_mem, unsigned int first, unsigned int second, unsigned int sub_array_size, unsigned int num_chars){
	int counter = 0;
	char buf1 = '\0';
	char buf2 = '\0'; 
	_Bool continue_checking = true;
	_Bool return_value;

	while(continue_checking && first + counter < num_chars && second + counter < num_chars){
		buf1 = read_bytes(in_file, reading_mem, first + counter);
		buf2 = read_bytes(in_file, reading_mem, second + counter);

		if(buf2 > buf1){
			return_value = false;
			continue_checking = false;
		}
		else if (buf2 < buf1){
			return_value = true;
			continue_checking = false;
		}
		else counter++;
	}
	return return_value;
}

//merge sub-arrays lexicographically
void lexo_merge(unsigned int *array, unsigned int ls, unsigned int le, unsigned int rs, unsigned int re, unsigned int sub_array_size, unsigned int num_chars, int in_file, struct mem_block_node *reading_mem){

	unsigned int temp_array_size = re - ls + 1;
	unsigned int *temp_array = malloc(sizeof(unsigned int)*temp_array_size);
	unsigned int l_count = ls;
	unsigned int r_count = rs;
	unsigned int temp_idx = 0;

	while(l_count<=le && r_count<=re)	//while elements in both lists
	{	
		if(lexo_greater(array, in_file, reading_mem, array[l_count], array[r_count], sub_array_size, num_chars)) temp_array[temp_idx++] = array[r_count++];
		else temp_array[temp_idx++] = array[l_count++];
	}
	
	while(l_count<=le)	temp_array[temp_idx++]=array[l_count++];
		
	while(r_count<=re)	temp_array[temp_idx++]=array[r_count++];

	for(int i = 0; i < temp_array_size; i++) array[ls + i] = temp_array[i];
	free(temp_array);
}

//merge sort indexes
void lexo_merge_sort(unsigned int *array, unsigned int bucket_start, unsigned int bucket_end, unsigned int sub_array_size, unsigned int num_chars, int in_file, struct mem_block_node *reading_mem){

	if(bucket_start < bucket_end){
		unsigned int mid_point = bucket_start + (bucket_end - bucket_start)/2;
		lexo_merge_sort(array, bucket_start, mid_point, sub_array_size, num_chars, in_file, reading_mem);	  //left recursion
		lexo_merge_sort(array, mid_point + 1, bucket_end, sub_array_size, num_chars, in_file, reading_mem);    //right recursion
		lexo_merge(array, bucket_start, mid_point, mid_point + 1, bucket_end, sub_array_size, num_chars, in_file, reading_mem);	
	}
}

//sort buckets by the m-distances
void sortBy_m(struct bucket_array bucket_array, unsigned int bucket_start, unsigned int num_S, int m, int max_m, struct m_list *m_list){

	unsigned int bucket_size;
	unsigned int bucket_end = bucket_start;
	struct m_list *m_head = m_list;
	bucket_node *head;
	skip_list_node *skip_list_head;
	struct bucket_array tempArray;

	_Bool bucket_boundary = true;

	while(bucket_end < num_S - 1 && m <= max_m){
		//Find the last element of current bucket
		while(bucket_end < num_S - 1 && !bucket_array.bucket_edge[bucket_end + 1]){
			bucket_end++;
		}
		bucket_size = bucket_end - bucket_start + 1;

		if(bucket_size > 1){
			tempArray = new_bucket_array(bucket_size);
			_Bool *tempArraySet = malloc(sizeof(_Bool)*bucket_size);
			for(int i = 0; i < bucket_size; i++)
				tempArraySet[i] = false;

			int tempArrayIndex = 0; 

			//For each letter i
			for(int i = 0; i < ALPHABET_SIZE; i++){
				bucket_boundary = true;
				head = m_head->list[i];
				skip_list_head = m_head->skip_list->list[i];

				//Indexes beginning with character i
				for(int j = 0; j < bucket_size; j++){
					//Use skip lists to reduce scanning through m-lists
					if(skip_list_head -> index > bucket_array.array[j + bucket_start]) skip_list_head = m_head->skip_list->list[i];
					while(skip_list_head->next != NULL && skip_list_head->next->index < bucket_array.array[j + bucket_start] + m){
						head = skip_list_head->m_list_node;
						skip_list_head = skip_list_head->next;
					}
					while(head->next != NULL){
						if(bucket_array.array[j + bucket_start] + m == head->val){
							tempArray.array[tempArrayIndex] = bucket_array.array[j+ bucket_start];
							tempArray.bucket_edge[tempArrayIndex++] = bucket_boundary;
							tempArraySet[j] = true;
							bucket_boundary = false;
						}
						head = head->next;
					}
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
			free_bucket_array(tempArray);
			free(tempArraySet);
		}
		bucket_start = bucket_end + 1;
		bucket_end = bucket_start;
	}
}