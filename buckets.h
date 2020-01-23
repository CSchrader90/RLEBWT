extern const int ALPHABET_SIZE;

struct bucket_node {
	unsigned int val;
	struct bucket_node *next;
};

struct m_list {
	int dist;
	struct bucket_node **list;
	struct m_list *next;
};

struct bucket_array {
	unsigned int *array;
	_Bool *bucket_edge;
};

struct bucket_node **new_bucket_list();
void add_to_bucket_list(struct bucket_node **list, unsigned int index, unsigned char bucket);
struct m_list *create_m_lists(int m);
void add_to_m_list(struct m_list *lists, unsigned int distance, unsigned int index, unsigned char bucket);
void free_m_lists(struct m_list *m);
struct bucket_array bucket_to_array(struct bucket_node **S_buckets, int num_S);
_Bool bucketContainsMultipleEntries(struct bucket_array bucket_array, int bucket_start, int bucket_end);
void sortSArray(struct bucket_array bucket_array, unsigned int bucket_start, unsigned int num_S, int m, int max_m, struct m_list *m_list);