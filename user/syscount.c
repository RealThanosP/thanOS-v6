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