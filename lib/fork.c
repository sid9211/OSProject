#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW         0x800


extern void _pgfault_upcall(void);


//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
        void *addr = (void *) utf->utf_fault_va;
        uint32_t err = utf->utf_err;
        int r;
	int pbits = PTE_U | PTE_P | PTE_W;
	void * va;
	unsigned int p_num;
	

        // Check that the faulting access was (1) a write, and (2) to a
        // copy-on-write page.  If not, panic.
        // Hint:
        //   Use the read-only page table mappings at vpt
        //   (see <inc/memlayout.h>).

        // LAB 4: Your code here.
	
	va = ROUNDDOWN(addr, PGSIZE);
	p_num = PPN(va);
	pte_t *ptentry = (pte_t *)(&vpt[p_num]);
	
	r = ((*ptentry & PTE_COW)&&(err & FEC_WR));
	if(r == 0)
		panic(" non-COW page causing Page Fault \n ");	

        // Allocate a new page, map it at a temporary location (PFTEMP),
        // copy the data from the old page to the new page, then move the new
        // page to the old page's address.
        // Hint:
        //   You should make three system calls.
        //   No need to explicitly delete the old page's mapping.

        // LAB 4: Your code here.
	envid_t envid1 = sys_getenvid();
	r = sys_page_alloc(envid1, (void *)PFTEMP, pbits);
	if(r < 0)
		panic("error in page allocation \n");
	memmove(PFTEMP, va, PGSIZE);	
	r = sys_page_map(envid1, (void *)PFTEMP, envid1, va, pbits);
	if(r < 0)
		panic("error in page map \n");
	
	r = sys_page_unmap(envid1, PFTEMP);
	if(r<0)
		panic(" error in unmapping page \n");

	return;
        //panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
        int r;

        // LAB 4: Your code here.
	int pbits;
	envid_t envid1;
	envid1 = thisenv->env_id;
	pte_t *ptentry;
	uintptr_t vp_addr;
	vp_addr = pn*PGSIZE;
	ptentry = (pte_t *)&(vpt[VPN(vp_addr)]);
	pbits = ((*ptentry) & 0xFFF);
	
	if(pbits & (PTE_COW|PTE_W)){
		pbits = pbits | PTE_COW;
		pbits = pbits & (~(PTE_W));
	}
	
	pbits = pbits & (~(PTE_A|PTE_D));
	
	r = sys_page_map(envid1, (void *)vp_addr, envid, (void *)vp_addr, pbits);
	if(r < 0)
		panic(" error in page map in duppage \n ");
	r = sys_page_map(envid1, (void *)vp_addr, envid1, (void *)vp_addr, pbits);
	if(r < 0)
		panic(" error in page map in duppage for same env \n ");
	
	return 0;	
	
        //panic("duppage not implemented");
       
}
//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use vpd, vpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
 
{
        // LAB 4: Your code here.
	pte_t *ptentry;
	pde_t *pdentry, *pdpentry, *pml4entry;
	
	uint8_t *va, *limit1, *limit2;
	extern unsigned char end[]; //change
	
	
///////////////////////////////////// init what is unsigned end[]?    limit2 never used
	
	set_pgfault_handler(pgfault);
	envid_t envid = sys_exofork();
	if(envid == 0){
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}
	if(envid<0){
		return envid;
	}
	
	int i,j;
	i = sys_page_alloc(envid, (void *)(UXSTACKTOP-PGSIZE), PTE_SYSCALL);

	if(i<0)
		panic("allocation of UXSTACK failed \n");
	
	i = sys_env_set_pgfault_upcall(envid,_pgfault_upcall);
	
	if(i<0)
		panic(" Setting of pgfault_upcall failed \n ");
	
	limit1 = (uint8_t*)UTEXT;
	limit2 = (uint8_t*)USTACKTOP;
	
	for(va = limit1; va < end; va += PGSIZE)
	{
		pml4entry = (pde_t *)&vpml4e[VPML4E(va)];
		if(!(*pml4entry & PTE_P))
		continue;
		pdpentry = (pde_t *)&vpde[VPDPE(va)];
		if(!(*pdpentry & PTE_P))
		continue;
		pdentry = (pde_t *)&vpd[VPD(va)];
		if(!(*pdentry & PTE_P))
		continue;
		ptentry = (pte_t *)&vpt[VPN(va)];
		if(!(*ptentry & PTE_P))
		continue;
		
		i = duppage(envid,PPN(va));
		if(i < 0)
			panic(" error from Duppage \n");
	}

	i = duppage(envid, PPN(USTACKTOP-PGSIZE));
	
	if(i < 0)
		panic(" error from duppage for exception stack \n");
	
	i = sys_env_set_status(envid,ENV_RUNNABLE);
	
	if(i < 0)
		panic(" error from syscall, for changing status \n ");

	return envid;
	
		
        panic("fork not implemented");
}

// Challenge
int sfork(void)
{
        panic("sfork not implemented");
        return -E_INVAL;
}

