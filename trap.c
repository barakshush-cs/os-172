#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(proc->killed)
      exit();
    proc->tf = tf;
    syscall();
    if(proc->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpunum() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
      Alarm();
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpunum(), tf->cs, tf->eip);
    lapiceoi();
    break;

  //PAGEBREAK: 13
  default:
    if(proc == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpunum(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            proc->pid, proc->name, tf->trapno, tf->err, cpunum(), tf->eip,
            rcr2());
    proc->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(proc && proc->state == RUNNING && tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit();
}
/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/
/********************************************************************************************************/
/*@My code*/
void defaultSigHandler(int sigNum){
cprintf("A signal %d was accepted by process %d",sigNum,proc->pid);
}


/*check the pending var to see if theres a signal waiting to be executed, if there are,
 set to 0 the sig_bit in the handkers array,store the current trap in tfToRestore var, 
 put the suitable handler to exacute,set to 1 the procHandlingSigNow var */


void checkSignals(struct trapframe *tf){
  int esp, size, ret;
  //sighandler_t sigHandlDefult=(sighandler_t)defaultSigHandler;
  if (proc == 0) /*there is no proc*/
    return; 
  if (proc->ignoreSignal)/*   */
    return; 
  if ((tf->cs & 3) != DPL_USER)/*not going back to user mode*/
    return;
  
  for(int sigIndx=0;sigIndx<NUMSIG; sigIndx++){
    // cprintf("Checking sigggnal %d\n", sigIndx);
    if(proc->pending[sigIndx] == 1)
    {
          // cprintf(" Sigggnal %d exists\n", sigIndx);

      proc->pending[sigIndx] = 0;  
      if(proc->sighandlers[sigIndx] == 0)
      {
        defaultSigHandler(sigIndx);/*no extern handler - go to def handler*/
        return;
      }
      else{
        proc->ignoreSignal = 1;

        /*1. back up trap frame*/
        //memmove(&proc->backUpTf, proc->tf, sizeof(struct trapframe));
        //proc->tf->esp -= sizeof(struct trapframe);
        //memmove((void*)proc->tf->esp, proc->tf, sizeof(struct trapframe));        
        /*2. copy invocation to sigreturn*/
        esp = proc->tf->esp; 
        size = (uint)&sigreturn_ass_call_end - (uint)&sigreturn_ass_call_start;
        esp -= size;
        ret = esp;
        memmove((void*)esp, sigreturn_ass_call_start, size);
        esp -= sizeof(struct trapframe);
        memmove((void*)esp, proc->tf, sizeof(struct trapframe)); 

        /*2. set parameters for the handlers*/
        esp -= 4;
        *(int*)esp = sigIndx;
        esp -= 4;
        *(int*)esp = ret; // sigreturn address
        /*3. Makes trapret to resume to the relevant signal handler */
        proc->tf->eip = (uint)proc->sighandlers[sigIndx];
        proc->tf->esp = esp;
        return;
      }           
    }
  }
}

