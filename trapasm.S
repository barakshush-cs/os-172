#include "mmu.h"
#;As part of
#;saving the general-purpose registers, alltraps also saves %eax, which contains the
#;system call number for the kernel to inspect later.
#Now that the user mode processor registers are saved, alltraps can finishing setting
#up the processor to run kernel C code. 



  # vectors.S sends all traps here.
.globl alltraps
alltraps:
  # Build trap frame.
  pushl %ds
  pushl %es
  pushl %fs
  pushl %gs
  pushal
  
  # Set up data and per-cpu segments.
  movw $(SEG_KDATA<<3), %ax
  movw %ax, %ds
  movw %ax, %es
  movw $(SEG_KCPU<<3), %ax
  movw %ax, %fs
  movw %ax, %gs
  # Once the segments are set properly, alltraps can call the C trap handler trap
  # Call trap(tf), where tf=%esp
  pushl %esp
  call trap
  addl $4, %esp
  
  #@MyCode#
  #pushl %esp
  #call trap
  #call handling_signal
  #addl $4, %esp
  #########

  # Return falls through to trapret...
  # After trap returns, alltraps pops the argument off the stack by adding to the stack pointer (3273) and then starts execut
.globl trapret
trapret:
  popal
  #@MyCode#
  #pushl %esp
  #call handling_signal  //calling the function to handle signal from ks
  #addl $4, %esp
  #########
  popl %gs
  popl %fs
  popl %es
  popl %ds
  addl $0x8, %esp  # trapno and errcode
  iret
