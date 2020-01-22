#include <stdio.h>
#include <stdlib.h>

#define ALPHABET_SIZE 128

struct Node {
	int val;
	struct Node *next;
};

struct Node **createNodeList(){

	struct Node **list = malloc(sizeof(struct Node)*ALPHABET_SIZE);
	for(int i = 0; i<ALPHABET_SIZE; i++){
		list[i] = malloc(sizeof(struct Node));
		list[i]->next = NULL;
	}

	return list;
}


void add_to_bucket_list(struct Node **list, unsigned int index, unsigned char bucket){
	struct Node *head = list[bucket];
	while(head->next!=NULL)
		head = head->next;
	head->val = index;
	head->next = malloc(sizeof(struct Node));
	head = head->next;
	head->next = NULL;
}

int main(int argc, char *argv[]){


	struct Node **buckets = createNodeList();
	add_to_bucket_list(buckets, 1, 'c');
	add_to_bucket_list(buckets, 2, 'c');
	add_to_bucket_list(buckets, 3, 'c');


	while(buckets['c']->next != NULL){
		printf("Value in list for c is %d\n", buckets['c']->val);
		buckets['c'] = buckets['c']->next;
	}


	return 0;
}