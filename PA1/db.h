
typedef struct db {
	struct db *left;
	struct db *right;
	int num;
	char* word;
} db_t;

db_t *db_open(int size);
void db_close(db_t *db);
void db_put(db_t *db, char *key, int key_len,
			char *val, int val_len);
/* Returns NULL if not found. A malloc()ed array otherwise.
 * Stores the length of the array in *vallen */
char *db_get(db_t *db, char *key, int key_len,
			int *val_len);
void freeing(db_t *db);



