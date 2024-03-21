#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "umutex.h"

int
sys_fork(void)
{
  return fork();
}

int sys_clone(void)
{
  int fn, stack, arg;
  argint(0, &fn);
  argint(1, &stack);
  argint(2, &arg);
  return clone((void (*)(void*))fn, (void*)stack, (void*)arg);
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
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

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  if (n == 0) {
    yield();
    return 0;
  }
  acquire(&tickslock);
  ticks0 = ticks;
  myproc()->sleepticks = n;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  myproc()->sleepticks = -1;
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// minit
int 
sys_minit(void){
  // take a look at how sleeplock.c is written. sleeplock.c is for kernel space, we are doing similar logic for user space
  mutex* m;
  // ensure atmocity in these sys functions when mutating lock?
  argptr(0, (void*)&m, sizeof(mutex*));
  initlock(&m->lk, "sleep lock");
  m->state = 0; // is lock in use
  m->ownership = 0; // who owns this lock - maybe pid?
  
  // each thread has a proc struct associated with it. do we need lock variables in proc struct?
  return 0;
}

// macquire
int 
sys_macquire(void){
  // proc.c contains sleep and wakeup functions which will need to be used
  mutex* m;
  // ensure atmocity in these sys functions when mutating lock?
  argptr(0, (void*)&m, sizeof(mutex*));

  acquire(&m->lk); // acquires the spinlock that protects the sleeplock
  while (m->state) { // is lock already in use?
    sleep(m, &m->lk); // if this lock is already held, this thread is put to "sleep"
  }
  m->state = 1;
  m->ownership = myproc()->pid;
  release(&m->lk);
  
  return 0;
}

// mrelease
int 
sys_mrelease(void){
  mutex* m;
  argptr(0, (void*)&m, sizeof(mutex*));
  // only can release if we own this lock
  // release lock, mark as not used, wake other threads?
  acquire(&m->lk);
  m->state = 0;
  m->ownership = 0;
  wakeup(m); // wakes up a thread using "chan" field in proc struct
  release(&m->lk);
  
  return 0;
}

int 
sys_nice(void){
  int inc;
  argint(0, &inc);
  struct proc *p = myproc();
  // can we increment nice val? needs to stay in range -20 to 19
  if (p->nice + inc > 19 || p->nice + inc < -20){
    return -1;
  } 
  // do we worry about atomicity between if statement and reassigning nice val?
  p->nice = p->nice + inc; // reassign nice val of this proc
  return 0;
}