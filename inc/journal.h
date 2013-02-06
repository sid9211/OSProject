# include <fs/fs.h>
# include <inc/env.h>

# define MAX_TRANS 		((BLKSIZE-SECTSIZE)/sizeof(transaction_t))
# define MAX_HANDLES		(BLKSIZE/4)

struct transaction_s
{
	envid_t t_transEnvid;
	uint32_t t_blockno;
	uint32_t t_numhandles;
	uint32_t t_active;
	//uint8_t padding[32-16]
} __attribute__((packed));

typedef struct transaction_s transaction_t;

// log
struct journal_s
{
	uint32_t j_numactive;
	
	uint8_t padding[SECTSIZE-4];
	transaction_t j_transaction[MAX_TRANS];
}__attribute__((packed));

typedef struct journal_s journal_t;

union handlebuf 
{
	struct handle_delete
	{
	
	}__attribute__((packed)) delete_handle;	
	struct handle_create
	{
		uint32_t h_file;
	}__attribute__((packed)) create_handle;
	
	struct handle_truncate
	{
		off_t h_offset;
	}__attribute__((packed)) trunc_handle;

	struct handle_write
	{
		uint32_t h_length;
		off_t h_offset;
		char h_writedata[BLKSIZE - MAXPATHLEN -12];
	}__attribute__((packed)) write_handle;
};

struct handle_s
{
	char h_path[MAXPATHLEN];
	uint32_t h_type;
	union handlebuf h_buffer;
}__attribute__((packed));

typedef struct handle_s handle_t;

extern journal_t *journal;
extern bool transaction_replay;

enum
{
	JOURN_DELETE =1,
	JOURN_CREATE,
	JOURN_TRUNCATE,
	JOURN_WRITE,
	JOURN_MAX
};
	

int transactionGetHandleBlockNo(transaction_t *transaction, uint32_t handleNo, uint32_t **pToDiskBlock,bool allocNew);
int transactionGetHandle(transaction_t *t, uint32_t handleNo, handle_t **pToHandle,bool allocNew);
int transactionFreeHandle(transaction_t *t, uint32_t handleNo);
void addHandleToTransaction(envid_t envid, handle_t *h);
void startTransaction(envid_t envid);
void commitTransaction(envid_t envid,bool allocNew);
void clearTransaction(transaction_t *t);
void endTransaction(envid_t envid);
void checkJournal();
void transactionReplay(transaction_t *t);
