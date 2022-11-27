#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
#define MAX_PG_ACCESS_NUM 512
#ifndef CHAR_BITS_NUM
#define CHAR_BITS_NUM 8
#endif
int
sys_pgaccess(void)
{
  uint64 va;   // virtual address of the first user page to check
  int npages;     // number of pages to check
  uint64 buf;  // a user address to a buffer to store the results into a bitmask

  if (argaddr(0, &va) < 0 ||
      argint(1, &npages) < 0 ||
      argaddr(2, &buf) < 0 ||
      npages < 0 ||
      npages > MAX_PG_ACCESS_NUM) {
    return -1;
  }

  // init temp rlt buf
  int npages_roundup = (npages + CHAR_BITS_NUM -1) & ~(CHAR_BITS_NUM -1);
  char temp[npages_roundup / CHAR_BITS_NUM];
  memset(temp, 0, sizeof(temp));

  // walk
  pagetable_t pagetable = myproc()->pagetable;
  pte_t *pte;
  for (int i = 0; i < npages; va += PGSIZE, i++) {
    pte = walk(pagetable, va, 0);
    if (pte == 0)
      continue;
    if (*pte & PTE_A) {
      temp[i / CHAR_BITS_NUM] |= (1 << (i % CHAR_BITS_NUM));
    }
    *pte &= ~PTE_A;
  }

  return copyout(pagetable, buf, temp, sizeof(temp));
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
