#include "debog.h"

#define B_STACKTRACE_IMPL
#include "third-party-thing/b_stacktrace.h"

#include <signal.h>


void my_handler(int signal)
{
    printf("signal: %d\n", signal);
    puts(b_stacktrace_get_string());
    exit(1);
}


void debog_init(void)
{
    signal(SIGSEGV, my_handler);
    signal(SIGABRT, my_handler);
}

