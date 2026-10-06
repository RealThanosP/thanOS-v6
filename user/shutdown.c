#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
    printf(1, "Shutting down xv6... Goodbye!\n");
    halt();

    // This state should never be reached, but if it is, we exit the program.
    exit();
}