#include "types.h"
#include "user.h"
#include "pstat.h"
int main(int argc, char **argv)
{
  int tickets[3] = {1, 2, 3}, pid[3], start[3], count = 0;
  int i, j;
  struct pstat p;

  if (argc != 1 && argc != 4) {
    printf(2, "usage: schedtest [tickets1 tickets2 tickets3]\n");
    exit();
  }
  for (i = 1; i < argc; i++) {
    tickets[i - 1] = atoi(argv[i]);
    if (tickets[i - 1] <= 0) {
      printf(2, "tickets must be positive\n");
      exit();
    }
  }
  for (i = 0; i < 3; i++) {
    if (settickets(tickets[i]) < 0 || (pid[i] = fork()) < 0)
      break;
    if (pid[i] == 0)
      for (;;)
        ; // Just wait
    count++;
  }
  if (count != 3 || getpinfo(&p) < 0)
    goto done;
  for (i = 0; i < 3; i++) {
    for (j = 0; j < NPROC && p.pid[j] != pid[i]; j++)
      ;
    if (j == NPROC)
      goto done;
    start[i] = p.ticks[j];
  }
  sleep(600);
  if (getpinfo(&p) < 0)
    goto done;
  printf(1, "PID TICKETS TICKS\n");
  for (i = 0; i < 3; i++) {
    for (j = 0; j < NPROC && p.pid[j] != pid[i]; j++)
      ;
    if (j < NPROC)
      printf(1, "%d %d %d\n", pid[i], tickets[i], p.ticks[j] - start[i]);
  }
done:
  for (i = 0; i < count; i++)
    kill(pid[i]);
  for (i = 0; i < count; i++)
    wait();
  exit();
}
