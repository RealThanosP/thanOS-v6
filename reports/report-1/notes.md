# Άσκηση 1 

## Απάντηση

### Αρχεία ορισμού κλήσεων συστήματος
Τα αρχεία στον πηγαίο κώδικα του xv6 όπου ορίζονται οι κλήσεις συστήματος για τα προγράμματα χρήστη είναι:
- `include/user.h`: Περιέχει τις δηλώσεις (function prototypes) των διαθέσιμων κλήσεων συστήματος για το user space (π.χ. `open`, `read`, `write`, `close`, κ.λπ.).
- `include/syscall.h`: Ορίζει τους αριθμούς/κωδικούς των κλήσεων συστήματος (π.χ. `#define SYS_open 15`).
- `ulib/usys.S`: Περιέχει τα assembly stubs που καλούνται από το user space και εκτελούν το trap προς τον πυρήνα (`int $T_SYSCALL`).

### Ανάλυση του προγράμματος `cat`
Επιλέγουμε το πρόγραμμα `cat`, το οποίο ανοίγει αρχεία και δείχνει τι περιέχει στο τερματικό του xv6. 

1. Το `cat` λειτουργεί για πολλαπλά αρχεία, επομένως η παρακάτω διαδικασία συμβαίνει για πολλαπλά αρχεία σειριακά.
2. Αρχικά προσπαθεί να ανοίξει ένα αρχείο με ένα `SYS_CALL(open)` με κωδικό 15 στο όνομα του αρχείου που ζήτησε ο χρήστης.
3. Στη συνέχεια χρησιμοποιεί ένα `SYS_CALL(read)` με κωδικό 5 για να διαβάσει τα περιεχόμενα του αρχείου που ανοίξαμε με την προηγούμενη κλήση.
4. Έπειτα χρησιμοποιεί ένα `SYS_CALL(write)` για να γράψει όλα τα περιεχόμενα που πέρασαν από την ανάγνωση του προηγούμενου βήματος, χρησιμοποιώντας έναν buffer μεγέθους 512, στον file descriptor 1 (standard output) ο οποίος είναι η έξοδος του shell.
5. Τα βήματα 3 και 4 συνεχίζονται μέχρι ο buffer να περάσει από όλο το αρχείο, δηλαδή μέχρι η συνάρτηση `read` να επιστρέψει 0, που σημαίνει ότι διάβασε 0 bytes του αρχείου.
6. Στη συνέχεια, τέλος καλείται το `SYS_CALL(close)` με κωδικό 21 το οποίο κλείνει το αρχείο και το απελευθερώνει από τη μνήμη.
7. Τέλος, αφού ολοκληρωθεί όλο το cat από τη συνάρτηση `cat()`, γίνεται ένα τελικό `SYS_CALL(exit)` για να γίνει έξοδος από την τρέχουσα διεργασία και επιστροφή στο shell.

# Άσκηση 2

## Περιγραφή
Υλοποίηση δύο νέων προγραμμάτων χρήστη στο xv6:
1. `touch`: Δημιουργία νέων αρχείων.
2. `shutdown`: Τερματισμός του xv6 και επιστροφή στο host λειτουργικό σύστημα μέσω νέας κλήσης συστήματος `halt`.

---
## Πρόγραμμα `touch` (`user/touch.c`)

Δημιουργεί νέα κενά αρχεία ανοίγοντάς τα με τα κατάλληλα flags δημιουργίας (`O_CREATE | O_RDWR`) μέσω της κλήσης συστήματος `open`.

### Κώδικας
```c
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int touch()
{
    // Επιστρέφει flags για δημιουργία αρχείου (αν δεν υπάρχει) με δικαιώματα R/W
    return O_CREATE | O_RDWR;
}

int main(int argc, char *argv[])
{
    int fd, i;

    if (argc < 2) {
        printf(2, "Usage: touch file1 file2 ...\n");
        exit();
    }

    for (i = 1; i < argc; i++) {
        fd = open(argv[i], touch());
        if (fd < 0) {
            printf(2, "touch: cannot create %s\n", argv[i]);
            exit();
        }
        close(fd);
    }
    exit();
}
```

### Λειτουργία
- Ελέγχει αν δόθηκαν ορίσματα (`argc < 2`). Αν όχι, εμφανίζει μήνυμα χρήσης στον fd 2 (stderr).
- Επαναλαμβάνει σειριακά για κάθε όρισμα:
  - Καλεί `open(argv[i], O_CREATE | O_RDWR)` (`SYS_open`, κωδικός 15). Αν το αρχείο δεν υπάρχει, δημιουργείται αυτόματα.
  - Κλείνει άμεσα τον περιγραφέα με `close(fd)` (`SYS_close`, κωδικός 21).
- Τερματίζει τη διεργασία καλώντας `exit()` (`SYS_exit`, κωδικός 2).

## Ενσωμάτωση στο `Makefile`

Για να γίνει compile το εκτελέσιμο και να προστεθεί στο αρχικό filesystem (`fs.img`), προστέθηκε στη μεταβλητή `USER_PROGS`:

```makefile
USER_PROGS=\
	... lotteryschedtest\ touch\
```

# Άσκηση 3
*Needs structured answer*

The uptime system call `SYS_CALL(uptime)` which has:
- code 14
- ran by the `sys_uptime(void)` function in the `kernel/sysproc.c` file
- it is implemented like:
```c
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
```
- it returns all the clock interrupts that have occured since the start of the shell in xv6. *As I understand* this clock is a software clock keeping time from there. *I think*.
- The implementation essentially just locks the cpu, so ticks do not increment, so we can get the ticks into xticks variable and the release it and just return the ticks.

# Άσκηση 4
The implementation of the running the system calls is:
```c
void syscall(void)
{
  int num;

  num = proc->tf->eax;
  if (num > 0 && num < NELEM(syscalls) && syscalls[num])
  {
    proc->tf->eax = syscalls[num]();
  }
  else
  {
    cprintf("%d %s: unknown sys call %d\n",
            proc->pid, proc->name, num);
    proc->tf->eax = -1;
  }
}
```
so what it does. 
- It takes the number stored in *Accumulator* of the process currently running 
IF
- the accumulator is >0 
- **and** the accumulators is smaller than the number of system calls available 
- **and** the accumulator is a number that matches a system call
Then it makes the system call

ELSE 
It just prints that the system call is unknown in the console output. And it sets the accumulator to -1 so in the next cycle no system call will be made.

# Άσκηση 5

For this the process is quite something. In order to create a systemcall we first have to include it in the `include/syscall.h` header file and give it a code number, for me it's 
```c
// include/syscall.h

SYS_favnum 29
```
likewise we have to add the system call to the `ulib/usys.S` assembly wrapper with
```assembly
// ulib/usys.S

SYSCALL(favnum)
```
then we need to go inside the kernel and add our system call function as a process in `kernel/sysproc.c`. We implement like this:
```c
// kernel/sysproc.c

// returns my favourite number
int sys_favnum(void)
{
  return 42;
}
```
Then we need to add this function in the system calls list in `kernel/syscall.c` as 
```c
// kernel/syscall.c
// ...
// other external declerations here

// We externally declare the function that exists in sysproc.c just so the compiler does not yell at ass
extern int sys_fork(void);
extern int sys_favnum(void);

static int (*syscalls[])(void) = {
    [SYS_fork] sys_fork,
	// other sys calls here
	// ...
	[SYS_favnum] sys_favnum,
}
```
Now we have created a system call! In order to see the results of it we have to make it accessible to the user, same implementation with the way we added the **touch** command into a **favnum** command available to the user, and include it in the `USER_PROGS` list on the `Makefile`. There we can implement `user/favnum.c` as:
```c
#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
    printf(1, "%d\n", favnum());
    exit();
}
```


# Άσκηση 6

## Εκφώνηση
Προσθέστε μία κλήση συστήματος η οποία θα εκτελεί λειτουργία «shutdown». Η κλήση που θα φτιάξετε πρέπει να έχει το εξής πρωτότυπο: `void halt(void)`. Έπειτα, φτιάξτε ένα πρόγραμμα χρήστη με όνομα `shutdown` το οποίο θα καλεί την κλήση συστήματος που φτιάξατε, ώστε χρήστες του xv6 να μπορούν πλέον να κάνουν shutdown το σύστημα χωρίς να χρειάζεται να το κάνουν από το host μηχάνημα.

Σε αυτή τη κλήση συστήματος θα πρέπει να επικοινωνήσετε άμεσα με το hardware της εικονικής μηχανής (στην περίπτωσή μας το QEMU). Για την απενεργοποίηση του συστήματος το hardware περιμένει σε κάποια συγκεκριμένη διεύθυνση (port) να υπάρξει μία συγκεκριμένη τιμή. Οι αριθμοί για αυτά τα δύο ορίζονται από τον κατασκευαστή. Οδηγίες για την τιμή που πρέπει να στείλετε σε ποια πόρτα έχουν καταγραφεί για διάφορους emulators εδώ. Εξηγήστε τη λειτουργία της συνάρτησης `outw` την οποία θα χρησιμοποιήσετε.

---

## Απάντηση

Για να επιτευχθεί ο τερματισμός της εικονικής μηχανής και η επιστροφή στο host terminal, υλοποιήθηκε νέα κλήση συστήματος (`halt`) στον πυρήνα, καθώς το xv6 δεν παρέχει εντολή power-off εξ ορισμού.

### Νέα Κλήση Συστήματος `sys_halt` (`kernel/sysproc.c`)
Αξιοποιεί την I/O θύρα ACPI power-off του QEMU (port `0x604` / fallback `0xB004`):

```c
// Shutdown xv6
int sys_halt(void)
{
  // QEMU ACPI power-off
  outw(0x604, 0x2000);
  // Fallback για παλαιότερες εκδόσεις QEMU / Bochs
  outw(0xB004, 0x2000);

  return 0;
}
```

### Εξήγηση της συνάρτησης `outw` (`include/x86.h`)
Η συνάρτηση `outw` είναι υλοποιημένη ως `static inline` στο αρχείο `include/x86.h`:

```c
static inline void
outw(ushort port, ushort data)
{
  asm volatile("out %0,%1" : : "a" (data), "d" (port));
}
```

#### Λειτουργία & Επίπεδο CPU:
- **Εντολή I/O (`outw`)**: Στην αρχιτεκτονική x86 υπάρχει ξεχωριστός χώρος διευθύνσεων I/O (Port-Mapped I/O - PMIO), ανεξάρτητος από τη διευθυνσιοδότηση της κύριας μνήμης RAM. Η πρόσβαση στις θύρες I/O γίνεται αποκλειστικά μέσω των assembly εντολών `in` και `out`.
- **Καταχωρητές & Inline Assembly**:
  - Το όρισμα `data` (16-bit word) δεσμεύεται στον καταχωρητή `%ax` (`"a"` constraint).
  - Το όρισμα `port` (16-bit I/O port address) δεσμεύεται στον καταχωρητή `%dx` (`"d"` constraint).
  - Η assembly εντολή `out %ax, %dx` αποστέλλει τα 16 bits της τιμής `data` στη διεύθυνση I/O `port`.
- **Προνόμια εκτέλεσης (Privilege Levels - Ring 0 vs Ring 3)**:
  - Οι εντολές `in` / `out` είναι **προνομιακές (privileged instructions)**. Επιτρέπεται να εκτελεστούν μόνο όταν το τρέχον επίπεδο προνομίων του επεξεργαστή (CPL) είναι Ring 0 (Kernel Mode).
  - Εάν επιχειρούσε ένα πρόγραμμα χρήστη (User Space - Ring 3) να εκτελέσει απευθείας την `outw`, η CPU θα παρήγαγε αμέσως σφάλμα γενικής προστασίας (**General Protection Fault - `#GP exception`**).
  - Γι' αυτόν τον λόγο, η αλληλεπίδραση με τις θύρες hardware του QEMU πρέπει απαραίτητα να περάσει μέσα από κλήση συστήματος (system call) στον πυρήνα.

#### Ανάλυση διευθύνσεων (Ports) και μαγικών τιμών (Magic Values):
- **Port `0x604` (QEMU ACPI)**: Στο προσομοιούμενο chipset του QEMU (PIIX4), η βάση του ACPI Power Management I/O space ορίζεται στη διεύθυνση `0x600`. Η θύρα στο offset `+0x04` (`0x604`) αντιστοιχεί στον καταχωρητή ελέγχου **`PM1a_CNT`** (Power Management 1 Control Register).
- **Τιμή `0x2000` (`SLP_EN`)**: Σύμφωνα με το ACPI specification:
  - Το **bit 13** είναι το flag `SLP_EN` (Sleep Enable).
  - Τα **bits 10-12** καθορίζουν το `SLP_TYP` (Sleep Type, όπου `000` σηματοδοτεί την κατάσταση `S5` / Soft-Off).
  - Γράφοντας `0x2000` (δυαδικό: `0010 0000 0000 0000`) στον καταχωρητή `PM1a_CNT`, δίνεται εντολή στο κύκλωμα ACPI του QEMU να διακόψει την τροφοδοσία και να κλείσει την εικονική μηχανή.
- **Port `0xB004`**: Χρησιμοποιείται ως fallback διεύθυνση ACPI για παλαιότερες εκδόσεις του QEMU (< 2.0) καθώς και για τον Bochs emulator.

### Διασύνδεση του System Call στο σύστημα
- **`include/syscall.h`**: Ορισμός μοναδικού opcode:
  ```c
  #define SYS_halt 28
  ```
- **`include/user.h`**: Δήλωση του prototype για τα user programs (όπως ορίζεται στην εκφώνηση):
  ```c
  void halt(void);
  ```
- **`ulib/usys.S`**: Προσθήκη assembly stub (παράγει trap `int $T_SYSCALL` με `%eax = SYS_halt`):
  ```assembly
  SYSCALL(halt)
  ```
- **`kernel/syscall.c`**: Καταχώριση στον πίνακα κλήσεων συστήματος:
  ```c
  extern int sys_halt(void);

  static int (*syscalls[])(void) = {
      ...
      [SYS_halt] sys_halt,
  };
  ```

### Κώδικας Προγράμματος Χρήστη (`user/shutdown.c`)
```c
#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
    printf(1, "Shutting down xv6... Goodbye!\n");
    halt();

    // Εάν για οποιονδήποτε λόγο αποτύχει το halt
    exit();
}
```

### Ενσωμάτωση στο `Makefile`
Για να γίνει compile το εκτελέσιμο `shutdown` και να ενσωματωθεί στο filesystem (`fs.img`) του xv6, προστέθηκε στη λίστα `USER_PROGS`:
```makefile
USER_PROGS=\
	...
	shutdown\
```

# Άσκηση 7
So the idea is to create an array of size of how many system calls we have and we increment each index based on what code has been run.  

So we edit `kernel/syscall.c`
```c
// kernel/syscall.c
int syscall_counts[NELEM(syscalls)] = {0};

void syscall(void)
{
  int num;

  num = proc->tf->eax;
  if (num > 0 && num < NELEM(syscalls) && syscalls[num])
  {
    syscall_counts[num]++; // Increment the system call that is run
    proc->tf->eax = syscalls[num]();
  }
  else
  {
    cprintf("%d %s: unknown sys call %d\n",
            proc->pid, proc->name, num);
    proc->tf->eax = -1;
  }
}
```
so now a system call is made that is documented in the `syscall_counts` array.

Now we need to actually implement `sys_getcount(void)` function in the `kernel/syscall.c` and then the process of using it as a user command is the same as before.
We implement like this
```c
// kernel/syscall.c
#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf(2, "Usage: %s <syscall_num>\n", argv[0]);
        exit();
    }

    int syscall_num = atoi(argv[1]);
    int count = getcount(syscall_num);

    if (count < 0)
    {
        printf(2, "syscount: invalid syscall number %d\n", syscall_num);
        exit();
    }

    printf(1, "System call %d execution count: %d\n", syscall_num, count);
    exit();
}
```
now we can test with favnum, as it is called only when the user runs the command favnum. We can see it works from the logs:
```bash
$ syscount 29    
System call 29 execution count: 0
$ favnum
42
$ syscount 29
exec syscount failed
$ syscount 29
System call 29 execution count: 1
```

# Άσκηση 7
Basically, we need to build a random number generator with a sudo-radom generator using ticks + a random number state var and doing Xorshift32.

SOOO we declare `unsigned int rand(void)` in `kernel/proc.h` and implement it int `kernel/proc.c`.
```c
static unsigned int rand_state = 1;

// Generates a 32-bit pseudo-random unsigned integer
unsigned int rand(void)
{
  unsigned int x = ticks ^ rand_state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  rand_state = x;
  return x;
}
```
we can now use this to have a random process id generated. Now we need to create a system call that uses this function with the `int kill(int pid)` to kill a random process and expose it to the user using the same process as before. 

So in `kernel/sysproc.c` we implement:
```c
// kills a random process
int sys_killrandom(void)
{
  struct proc *candidates[NPROC];
  int count = 0;
  struct proc *p;

  acquire(&ptable.lock);

  // 1. Gather all killable processes
  for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
  {
    // Only pick processes that are alive
    if (p->state == UNUSED || p->state == ZOMBIE)
      continue;

    // Protect init (PID 1) and do not commit suicide (current process)
    if (p->pid <= 1 || p == proc)
      continue;

    candidates[count++] = p;
  }

  // 2. If no killable process exists, safely return -1
  if (count == 0)
  {
    release(&ptable.lock);
    return -1;
  }

  // 3. Pick a random candidate
  // modulo count makes sure the idx is within range of how many processes we have
  int chosen_idx = (int)(rand(ticks) % (unsigned int)count);
  struct proc *victim = candidates[chosen_idx];

  release(&ptable.lock);

  // 4. Kill the candidate safely
  kill(victim->pid); // no need to know if it was successful or not

  return victim->pid;
}
```
we could just go with a simpler way but this is bulletproof and guaranteed to kill a process and protects from kernel panic in case the random pid is 1.
