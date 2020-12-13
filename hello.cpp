#include <unistd.h>
#include <stdio.h>

int main()
{
    #pragma omp parallel for num_threads(4)
    for (int i = 0; i < 10; i++)
    {
        fprintf(stderr, "qaq %d\n", i);
        sleep(1);
    }

    return 0;
}
