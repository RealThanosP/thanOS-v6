#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

extern struct ptable ptable;
extern int syscall_counts[];
extern const int syscall_counts_len;

int sys_fork(void)
{
  return fork();
}

int sys_exit(void)
{
  exit();
  return 0; // not reached
}

int sys_wait(void)
{
  return wait();
}

int sys_kill(void)
{
  int pid;

  if (argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int sys_getpinfo(void)
{
  struct pstat *pstat;

  argptr(0, (void *)&pstat, sizeof(*pstat));

  if (pstat == 0) {
    return -1;
  }

  acquire(&ptable.lock);
  struct proc *p;
  int i;
  for (p = ptable.proc; p != &(ptable.proc[NPROC]); p++) {
    i = p - ptable.proc;

    /* if (p->state == UNUSED) { */
    /*     pstat->inuse[i] = 0; */
    /* } else { */
    /*     pstat->inuse[i] = 1; */
    /* } */
    pstat->inuse[i] = p->inuse;
    pstat->pid[i] = p->pid;
    pstat->ticks[i] = p->ticks;
    pstat->tickets[i] = p->tickets;
  }
  release(&ptable.lock);
  return 0;
}

int sys_getpid(void)
{
  return proc->pid;
}

uintp sys_sbrk(void)
{
  uintp addr;
  uintp n;

  if (arguintp(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

int sys_sleep(void)
{
  int n;
  uint ticks0;

  if (argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (proc->killed) {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Shutdown xv6
// For reasons of simplicity I just write a specific value to a specific port.
// This is a QEMU-specific hack, but it works for our purposes.
int sys_halt(void)
{
  // QEMU uses port 0x604 for ACPI power-off
  outw(0x604, 0x2000);
  // Fallback for older QEMU versions
  outw(0xB004, 0x2000);

  return 0;
}

// returns my favourite number
int sys_favnum(void)
{
  return 42;
}

// kills a random process
int sys_killrandom(void)
{
  struct proc *candidates[NPROC];
  int count = 0;
  struct proc *p;

  acquire(&ptable.lock);

  // 1. Gather all killable processes
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    // Only pick processes that are alive
    if (p->state == UNUSED || p->state == ZOMBIE)
      continue;

    // Protect init (PID 1) and do not commit suicide (current process)
    if (p->pid <= 1 || p == proc)
      continue;

    candidates[count++] = p;
  }

  // 2. If no killable process exists, safely return -1
  if (count == 0) {
    release(&ptable.lock);
    return -1;
  }

  // 3. Pick a random candidate
  // modulo count makes sure the idx is within range of how many processes we
  // have
  int chosen_idx = (int)(rand() % (unsigned int)count);
  struct proc *victim = candidates[chosen_idx];

  release(&ptable.lock);

  // 4. Kill the candidate safely
  kill(victim->pid); // no need to know if it was successful or not

  return victim->pid;
}

// System call wrapper to return a pseudo-random positive integer
int sys_rand(void)
{
  return (int)(rand() & 0x7FFFFFFF);
}

// Returns the number of times a system call has been executed since init
int sys_getcount(void)
{
  int sc;

  // Get the syscall number from the first argument
  if (argint(0, &sc) < 0)
    return -1;

  // Check if the syscall number is valid
  if (sc < 0 || sc >= syscall_counts_len)
    return -1;

  return syscall_counts[sc];
}