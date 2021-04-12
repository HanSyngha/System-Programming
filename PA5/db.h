#define SECTOR_SIZE 512
#define BUF_SIZE (SECTOR_SIZE * 8)

typedef struct db {
	struct db *left;
	struct db *right;
	int num;
	char* word;
} db_t;

db_t *db_open(int size);
void db_close(db_t *db);
void db_put(db_t *db, char *key, int keylen, char *val, int vallen);
void db_put_with_no_log(db_t *db, char *key, int keylen, char *val, int vallen);
/* Returns NULL if not found. A malloc()ed array otherwise.
 * Stores the length of the array in *vallen */
char *db_get(db_t *db, char *key, int keylen, int *vallen);
void save_and_free(db_t *db,int idx);
int findw(db_t *db,char* key,int idx);
void recovery(db_t *db);
void* log_buf;
int log_fd, dir_fd;
