#include<inc/journal.h>
#include<inc/assert.h>
#include<inc/error.h>
//#include<inc/init.h>

int handleReplay(handle_t *h, uint32_t handleno)
{
	struct File *f;
	int r;

	f = NULL;
	if(h->h_type == JOURN_CREATE){
		//cprintf("\n Hi, inside create\n");
		r = file_create(h->h_path,
			&f);
		if(r>=0)file_flush(f);
		return 0;
	}
	else{
		r = file_open(h->h_path,&f);
		if(r<0) return r;
		assert(f!=NULL);
	}

	if(h->h_type == JOURN_WRITE){
		//cprintf("\n Hi, inside write\n");
		r = file_write(f,
			h->h_buffer.write_handle.h_writedata,
			h->h_buffer.write_handle.h_length,
			h->h_buffer.write_handle.h_offset);
		if(r<0) return r;
		file_flush(f);
	}
	else if(h->h_type == JOURN_TRUNCATE){
		r = file_set_size(f, h->h_buffer.trunc_handle.h_offset);
		if(r<0) return r;
		file_flush(f);
	}
	else if(h->h_type == JOURN_DELETE){
		r = file_remove(h->h_path);
		if(r<0) return r;
	}
	return 0;
}

void transactionReplay(transaction_t *t)
{
	uint32_t i;
	handle_t *tempHandle;
	int r;

	assert(t!=NULL);
	if(t->t_blockno == 0)
		return;

	for(i = 0; i< t->t_numhandles;i++){
		//cprintf("-------------------------------------- inside transaction Replay i %d  numhandles %d\n",i,t->t_numhandles);
		tempHandle = NULL;
		r = transactionGetHandle(t,i,&tempHandle,0);
		if(r<0) return;

	assert(tempHandle != NULL);
	r = handleReplay(tempHandle,i);

	//cprintf("--------------------------------------------------------------------- value of r %d",r);
	if(r<0) return;
	}
}

void checkJournal()
{
        int i;
	cprintf("--------------------------------------------inside checkJournal\n");
        transaction_replay = 1;

        for(i = 0;i< MAX_TRANS;i++){
                if(journal->j_transaction[i].t_active > 0)
                        transactionReplay(&journal->j_transaction[i]);
		//cprintf("------------------------------------------------- i %d, active %d\n",i,journal->j_transaction[i].t_active);
                clearTransaction(&journal->j_transaction[i]);
        }
        transaction_replay = 0;
}
