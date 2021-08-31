#include <stdio.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
  printf("I'm the SEMAFORO process and my PID is: %i\n", getpid());
}
