#include <stdio.h>
#include <sys/sysctl.h>
#include <sys/types.h>

int main()
{
    int r = 0;
    size_t len = sizeof(r);
    sysctlbyname("hw.logicalcpu", &r, &len, NULL, 0);
    printf("core count = %d\n", r);
    return 0;
}
