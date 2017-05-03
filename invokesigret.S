#include "syscall.h"
#include "traps.h"

.globl sigreturn_ass_call_start
.globl sigreturn_ass_call_end

sigreturn_ass_call_start:
  movl $SYS_sigreturn, %eax
  int $T_SYSCALL
sigreturn_ass_call_end:
