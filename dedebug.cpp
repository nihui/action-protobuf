
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#if defined _WIN32
#include <windows.h>
#elif defined __ANDROID__ || defined __linux__
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#elif defined __APPLE__
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>
#endif

bool is_being_debugged()
{
#if defined _WIN32
    return IsDebuggerPresent();
#elif defined __ANDROID__ || defined __linux__
    // https://stackoverflow.com/questions/3596781/how-to-detect-if-the-current-process-is-being-run-by-gdb
    int status_fd = open("/proc/self/status", O_RDONLY);
    if (status_fd == -1)
        return false;

    char buf[4096];
    ssize_t num_read = read(status_fd, buf, sizeof(buf) - 1);
    close(status_fd);

    if (num_read <= 0)
        return false;

    buf[num_read] = '\0';
    const char tracerPidString[] = "TracerPid:";
    const char* tracer_pid_ptr = strstr(buf, tracerPidString);
    if (!tracer_pid_ptr)
        return false;

    for (const char* characterPtr = tracer_pid_ptr + sizeof(tracerPidString) - 1; characterPtr <= buf + num_read; ++characterPtr)
    {
        if (isspace(*characterPtr))
            continue;

        return isdigit(*characterPtr) != 0 && *characterPtr != '0';
    }

    return false;
#elif defined __APPLE__
    // https://stackoverflow.com/questions/2200277/detecting-debugger-on-mac-os-x

    // Initialize the flags so that, if sysctl fails for some bizarre
    // reason, we get a predictable result.
    struct kinfo_proc info;
    info.kp_proc.p_flag = 0;

    // Initialize mib, which tells sysctl the info we want, in this case
    // we're looking for information about a specific process ID.
    int mib[4];
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = getpid();

    // Call sysctl.
    size_t size = sizeof(info);
    sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);

    // We're being debugged if the P_TRACED flag is set.
    return ((info.kp_proc.p_flag & P_TRACED) != 0);
#else
    // unknown platform :(
    fprintf(stderr, "unknown platform!\n");
    return false;
#endif
}

int main()
{
    fprintf(stderr, "hello\n");
    fprintf(stderr, "is_being_debugged = %d\n", is_being_debugged());
    return 0;
}
