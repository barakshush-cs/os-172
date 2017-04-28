1.4.
=============================================================================
A signal should be handled (if at all) whenever control for the receiving 
process is passed from the kernel space to the user space. 
=============================================================================
Before restoring the process context (see trapret at trapasm.S), 
the kernel first checks for the pending signals of that process.
(Where we check for the pendig signals?)
=============================================================================
If signal handling is required, the kernel must create a signal-handling 
stack frame which is a (user) stack frame for the appropriate 
signal handler execution.
The signalhandling stack frame should contain the 
1. values of the CPU registers, stored before the execution of the signal handler in the trapframe 
   of the current process, 
2. the signal number and a return address that points to an invocation 
   of the system call int sigreturn(void). 

The values in the signal-handling stack frame should be arranged so that they will 
be used as follows:
		1. The signal handler should receive a single argument – the signal number
		2. After the signal handler termination an invocation of the sigreturn 
		   system call should be performed (If you like, you can do this in a manner 
		   similar to the implicit invocation of the exit system call you implemented 
		   in the first assignment)
		3. As a response to the sigreturn system call the kernel should restore the trapframe which is
		   stored in the signal-handling stack frame of the calling process

Finally, the kernel should change the trapframe in the process’ control block so that 
it will execute the signal handler. 
This way, when the process returns from kernel space to user space, the code for the
signal handler is executed. 
Note that in case several signals are pending, the kernel should prepare signal-handling 
stack frame and execute the signal handler of a single signal.