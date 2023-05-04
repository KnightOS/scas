typedef struct {
    unsigned int capacity;
    unsigned int length;
    void **items;
} list_t;

list_t *create_list(void);
void list_free(list_t *list);
void list_add(list_t *list, void *item);
void list_del(list_t *list, unsigned int index);
void list_cat(list_t *list, list_t *source);
