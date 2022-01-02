#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
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
  backtrace();

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

uint64 sys_sigalarm(void)
{
  int n;
  struct proc *p = myproc();
  if(argint(0, &n)<0){
    return -1;
  }
  p->nticks = n;
  p->passedticks = n;
  uint64 va;
  if(argaddr(1, &va)<0){
    return -1;
  }
  p->retaddr = va;
  return 0;
}

uint64 sys_sigreturn(void)
{
  // return 0;
  struct proc *p = myproc();

  p->trapframe->kernel_sp = p->trapframe->bakkernel_sp;
  p->trapframe->epc = p->trapframe->bakepc;
  p->trapframe->kernel_hartid = p->trapframe->bakkernel_hartid;
  p->trapframe->ra = p->trapframe->bakra;
  p->trapframe->sp = p->trapframe->baksp;
  p->trapframe->gp = p->trapframe->bakgp;
  p->trapframe->tp = p->trapframe->baktp;
  p->trapframe->t0 = p->trapframe->bakt0;
  p->trapframe->t1 = p->trapframe->bakt1;
  p->trapframe->t2 = p->trapframe->bakt2;
  p->trapframe->s0 = p->trapframe->baks0;
  p->trapframe->s1 = p->trapframe->baks1;
  p->trapframe->a0 = p->trapframe->baka0;
  p->trapframe->a1 = p->trapframe->baka1;
  p->trapframe->a2 = p->trapframe->baka2;
  p->trapframe->a3 = p->trapframe->baka3;
  p->trapframe->a4 = p->trapframe->baka4;
  p->trapframe->a5 = p->trapframe->baka5;
  p->trapframe->a6 = p->trapframe->baka6;
  p->trapframe->a7 = p->trapframe->baka7;
  p->trapframe->s2 = p->trapframe->baks2;
  p->trapframe->s3 = p->trapframe->baks3;
  p->trapframe->s4 = p->trapframe->baks4;
  p->trapframe->s5 = p->trapframe->baks5;
  p->trapframe->s6 = p->trapframe->baks6;
  p->trapframe->s7 = p->trapframe->baks7;
  p->trapframe->s8 = p->trapframe->baks8;
  p->trapframe->s9 = p->trapframe->baks9;
  p->trapframe->s10 = p->trapframe->baks10;
  p->trapframe->s11 = p->trapframe->baks11;
  p->trapframe->t3 = p->trapframe->bakt3;
  p->trapframe->t4 = p->trapframe->bakt4;
  p->trapframe->t5 = p->trapframe->bakt5;
  p->trapframe->t6 = p->trapframe->bakt6;
  p->inproc = 0;
  return 0;
}

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
