#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
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
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
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
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
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
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


/*@My:sytem calls*/
/*SIGNAL*/
int
sys_signal(void){
 int signal_Number;
 sighandler_t sigHandler;

 cprintf("sysproc.c : sys_signal nom  for process %d \n",proc->pid);

 if(argint(0,&signal_Number) < 0){   
   return (-1);
 }
 /*test sig_num validity*/
 if (signal_Number<0||signal_Number>NUMSIG-1)
 {
  cprintf("Iligal sig_Num: %d \n",signal_Number);
  return (-1);
 }

 if(argint(1,(int*)&sigHandler) < 0){
   return (-1);
 }
 if (0==sigHandler)
 {
  cprintf("signal handlerm is NULL, defult handlers will be used\n");
  return (-1);
 }
 return (int)(signal(signal_Number, (sighandler_t)sigHandler));
}

int
sys_sigsend(void){

 int pid;
 int sig_Num;
 //cprintf("sysproc.c : sys_sigsend nom.  for process %d \n",proc->pid);

 if((argint(0,&pid) < 0) || (argint(1,&sig_Num) < 0) || (argint(1,&sig_Num) > NUMSIG)){
   return (-1);
 }

 return sigsend(pid,sig_Num);

}

int 
sys_sigreturn(void){
 //cprintf("sys_sigreturning pid: %d \n",proc->pid);
 return (sigreturn());
}

int 
sys_alarm(void){
  int tick;
 
  //cprintf("Alarm tick: ");
  if(argint(0, &tick) < 0)
    return 0;
  //cprintf("%d\n",tick);
  
  return setAlarmTicks(tick);
}
int 
sys_getick(void){
  return ticks;
}