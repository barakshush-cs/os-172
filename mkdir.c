#include "types.h"
#include "stat.h"
#include "user.h"

typedef void (*sighandler_t)(int);

void
testSignals(int sigNum){
 printf(1,"test_handle:process %d sent signal number %d to this running process\n", getpid(),sigNum);
}


int
main(int argc, char *argv[])
{
  int i;
  sighandler_t sigHandlFunc=(sighandler_t)testSignals;
  for(i=0;i<32;i++){
    printf(1,"test  for1: i=%d\n",i);
    signal(i,sigHandlFunc);
    }

  if(argc < 2){
    printf(2, "Usage: mkdir files...\n");
    exit();
  }

  for(i = 1; i < argc; i++){
    if(mkdir(argv[i]) < 0){
      printf(2, "mkdir: %s failed to create\n", argv[i]);
      break;
    }
  }

  exit();
}
