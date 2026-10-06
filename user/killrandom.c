#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char **argv)
{
    printf(1, "Killed pid %d\n", killrandom());
    exit();
}
