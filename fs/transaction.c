#include<inc/journal.h>
#include<inc/error.h>
#include<inc/string.h>
#include<inc/lib.h>
#include<inc/assert.h>
//#include "replayTransaction.c"

journal_t *journal;

bool transaction_replay;

int transactionGetHandleBlockNo(transaction_t *t, uint32_t handleNo, uint32_t **pToDiskBlock,bool allocNew)
{
	int r;
	uint32_t *va;

	if(handleNo >= MAX_HANDLES)
		return -E_INVAL;


	assert(t != NULL);

	if(t->t_blockno == 0){
		if(!allocNew)
			return -E_INVAL;

		r = alloc_block();
		if(r < 0){
			return r;
		}

		memset(diskaddr(r), 0, BLKSIZE);
		t->t_blockno = r;
	}
	
	va = diskaddr(t->t_blockno);

	*pToDiskBlock = &(va[handleNo]);

	return 0;
}

int transactionGetHandle(transaction_t *t, uint32_t handleNo, handle_t **pToHandle,bool allocNew)
{
	int r;
	uint32_t *pHandleDiskBlock;

	if(handleNo > MAX_HANDLES -1)
		return -E_INVAL;
	
	r = transactionGetHandleBlockNo(t,handleNo,&pHandleDiskBlock,allocNew);

	if(r<0)
		return r;

	if(*pHandleDiskBlock == 0){
		if(!allocNew)
			return -E_INVAL;

		r = alloc_block();
		if(r<0)
			return r;

		memset(diskaddr(r), 0, BLKSIZE);
		*pHandleDiskBlock = r;
	}
	
	*pToHandle = diskaddr(*pHandleDiskBlock);

	return 0;
}

int transactionFreeHandle(transaction_t *t, uint32_t handleNo)
{

	int r;
	uint32_t *pDiskNo;

	r = transactionGetHandleBlockNo(t, handleNo, &pDiskNo,0);
	
	if(r < 0)
		return r;

	if(*pDiskNo!=0){
		free_block(*pDiskNo);
		*pDiskNo = 0;
	}

	return 0;
}

void addHandleToTransaction(envid_t envid, handle_t *h)
{
	uint32_t i,dataLength;
	handle_t *tempHandle;
	transaction_t *t;
	off_t offset1,offset2;
	int r;
	//if(transaction_replay) return;

	t = envs[ENVX(envid)].env_transaction;
	assert(t!=NULL);

	i = t->t_numhandles;
	t->t_numhandles++;

	//cprintf("----------------------------------------numhandles %d\n",t->t_numhandles);
	tempHandle = NULL;
	r = transactionGetHandle(t,i,&tempHandle,1);
	if(r<0) return ;
	assert(tempHandle != NULL);
	memmove(tempHandle,h,sizeof(handle_t));

}

			

void startTransaction(envid_t envid)
{
	transaction_t *t;

	int i, nextFreeTransaction;

	//if(transaction_replay) return;

	t = envs[ENVX(envid)].env_transaction;

	if(t != NULL) return; //Already has an associated transaction

	for(i = 0; i< MAX_TRANS; i++)
		if(journal->j_transaction[i].t_transEnvid > 0){
			nextFreeTransaction = i;
			break;
		}

	nextFreeTransaction++;

	//cprintf("-------------------------------------------nextfreetransaction %d\n",nextFreeTransaction);
	if(nextFreeTransaction >= MAX_TRANS) return ;

	sys_set_transaction(envid, &journal->j_transaction[nextFreeTransaction]);
	t = envs[ENVX(envid)].env_transaction; //can modify and set here as well

	t->t_transEnvid = envid;

}

void commitTransaction(envid_t envid,bool allocNew)
{

	uint32_t i;
	handle_t *tempHandle;
	transaction_t *t;
	int r;

	//if(transaction_replay) return;

	t = envs[ENVX(envid)].env_transaction;

	if((t == NULL) && allocNew){
		startTransaction(envid);
		t = envs[ENVX(envid)].env_transaction;
	}

	assert(t!=NULL);
	
	if(t->t_numhandles == 0){
		assert(t->t_blockno == 0);
		return;
	}
	
	for(i = 0; i< t->t_numhandles; i++){
		tempHandle = NULL;
		//cprintf("------------------------------------ val i %d\n",i);
		r = transactionGetHandle(t,i,&tempHandle,0);
		if(r<0) return;
		//cprintf("path %s type %d",tempHandle->h_path,tempHandle->h_type);
		flush_block(tempHandle);
	}

	flush_block(diskaddr(t->t_blockno));

	t->t_active = 1;
	flush_sector(t);
	//cprintf("------------------------------------------------------- t_active %d\n",t->t_active);

}

void clearTransaction(transaction_t *t)
{
	uint32_t i;
	int r;

	assert(t!=NULL);

	//cprintf("---------------------------------------------------------- in clear transaction\n");
	t->t_active = 0;
	t->t_transEnvid = 0;
	flush_sector(t);
	
	for(i = 0;i< t->t_numhandles; i++){
		r = transactionFreeHandle(t,i);
	}

	if(t->t_blockno) free_block(t->t_blockno);

	t->t_blockno = 0;
	t->t_numhandles = 0;
	flush_sector(t);
}


void endTransaction(envid_t envid)
{
	transaction_t *t;

	//if(transaction_replay) return;

	t = envs[ENVX(envid)].env_transaction;
	assert(t!=NULL);

	clearTransaction(t);

	sys_set_transaction(envid,NULL);
}
