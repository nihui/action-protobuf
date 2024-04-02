#include <stdio.h>
int main()
{
    int r = 0;
    size_t len = sizeof(r);
    sysctlbyname("hw.logicalcpu", &r, &len, NULL, 0);
    printf("core count = %d\n", r);
    return 0;
}
