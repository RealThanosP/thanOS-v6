#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
    int r = rand();

    // If a max value is provided as an argument, bound the output to [0, max - 1]
    if (argc > 1)
    {
        int max = atoi(argv[1]);
        if (max > 0)
            r = r % max;
    }

    printf(1, "%d\n", r);
    exit();
}