
typedef struct db {
	struct db *left;
	struct db *right;
	int num;
	char* word;
} db_t;

db_t *db_open(int size);
void db_close(db_t *db);
void db_put(db_t *db, char *key, int key_len,int value);
int db_get(db_t *db, char *key, int key_len);
void save_and_free(db_t *db,int idx);
int findw(db_t *db,char* key,int idx);


