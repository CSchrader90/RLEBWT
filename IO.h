extern const int BLOCK_SIZE;
extern const int MAX_BLOCKS;

struct mem_block_node {
	unsigned int block_num;
	char *data;
	struct mem_block_node *next;
};

struct mem_block_node *create_new_mem_block();
struct mem_block_node *add_new_mem_block(int in_file, unsigned int block_num);
void free_mem_block_list(struct mem_block_node *list);
char read_bytes(int in_file, struct mem_block_node *mem_block_node, unsigned int offset);