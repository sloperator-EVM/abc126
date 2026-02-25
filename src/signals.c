#include "winrun/loader.h"

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_debug = 0;

static void crash_handler(int sig, siginfo_t *info, void *ucontext) {
    (void)ucontext;
    fprintf(stderr, "winrun: caught signal %d at address %p\n", sig, info->si_addr);

    if (g_debug) {
        void *trace[64];
        int size = backtrace(trace, 64);
        backtrace_symbols_fd(trace, size, fileno(stderr));
    }

    _Exit(128 + sig);
}

void install_signal_handlers(bool debug) {
    g_debug = debug ? 1 : 0;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = crash_handler;
    sa.sa_flags = SA_SIGINFO | SA_RESTART;

    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
}
