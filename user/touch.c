#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int touch()
{
    // Return flags for creating a new file with READ/WRITE permissions
    // Creates a file if not exists with O_CREATE with for read/write access with O_RDWR flag
    return O_CREATE | O_RDWR;
}

int main(int argc, char *argv[])
{
    int fd, i;

    if (argc < 2)
    {
        printf(2, "Usage: touch file1 file2 ...\n");
        exit();
    }

    for (i = 1; i < argc; i++)
    {
        fd = open(argv[i], touch());
        if (fd < 0)
        {
            printf(2, "touch: cannot create %s\n", argv[i]);
            exit();
        }
        close(fd);
    }
    exit();
}