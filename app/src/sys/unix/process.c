#include "util/process.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util/log.h"

enum sc_process_result
sc_process_execute_p(const char *const argv[], sc_pid *pid, unsigned inherit,
                     int *pin, int *pout, int *perr) {
    bool inherit_stdout = inherit & SC_INHERIT_STDOUT;
    bool inherit_stderr = inherit & SC_INHERIT_STDERR;

    // If pout is defined, then inherit MUST NOT contain SC_INHERIT_STDOUT.
    assert(!pout || !inherit_stdout);
    // If perr is defined, then inherit MUST NOT contain SC_INHERIT_STDERR.
    assert(!perr || !inherit_stderr);

    int in[2];
    int out[2];
    int err[2];
    int internal[2]; // communication between parent and children

    if (pipe(internal) == -1) {
        perror("pipe");
        return SC_PROCESS_ERROR_GENERIC;
    }
    if (pin) {
        if (pipe(in) == -1) {
            perror("pipe");
            close(internal[0]);
            close(internal[1]);
            return SC_PROCESS_ERROR_GENERIC;
        }
    }
    if (pout) {
        if (pipe(out) == -1) {
            perror("pipe");
            // clean up
            if (pin) {
                close(in[0]);
                close(in[1]);
            }
            close(internal[0]);
            close(internal[1]);
            return SC_PROCESS_ERROR_GENERIC;
        }
    }
    if (perr) {
        if (pipe(err) == -1) {
            perror("pipe");
            // clean up
            if (pout) {
                close(out[0]);
                close(out[1]);
            }
            if (pin) {
                close(in[0]);
                close(in[1]);
            }
            close(internal[0]);
            close(internal[1]);
            return SC_PROCESS_ERROR_GENERIC;
        }
    }

    *pid = fork();
    if (*pid == -1) {
        perror("fork");
        // clean up
        if (perr) {
            close(err[0]);
            close(err[1]);
        }
        if (pout) {
            close(out[0]);
            close(out[1]);
        }
        if (pin) {
            close(in[0]);
            close(in[1]);
        }
        close(internal[0]);
        close(internal[1]);
        return SC_PROCESS_ERROR_GENERIC;
    }

    if (*pid == 0) {
        if (pin) {
            if (in[0] != STDIN_FILENO) {
                dup2(in[0], STDIN_FILENO);
                close(in[0]);
            }
            close(in[1]);
        }
        // Do not close stdin in the child process, this makes adb fail on
        // Linux

        if (pout) {
            if (out[1] != STDOUT_FILENO) {
                dup2(out[1], STDOUT_FILENO);
                close(out[1]);
            }
            close(out[0]);
        } else if (!inherit_stdout) {
            // Close stdout in the child process
            close(STDOUT_FILENO);
        }

        if (perr) {
            if (err[1] != STDERR_FILENO) {
                dup2(err[1], STDERR_FILENO);
                close(err[1]);
            }
            close(err[0]);
        } else if (!inherit_stderr) {
            // Close stderr in the child process
            close(STDERR_FILENO);
        }

        close(internal[0]);
        enum sc_process_result err;
        if (fcntl(internal[1], F_SETFD, FD_CLOEXEC) == 0) {
            execvp(argv[0], (char *const *) argv);
            perror("exec");
            err = errno == ENOENT ? SC_PROCESS_ERROR_MISSING_BINARY
                                  : SC_PROCESS_ERROR_GENERIC;
        } else {
            perror("fcntl");
            err = SC_PROCESS_ERROR_GENERIC;
        }
        // send err to the parent
        if (write(internal[1], &err, sizeof(err)) == -1) {
            perror("write");
        }
        close(internal[1]);
        _exit(1);
    }

    // parent
    assert(*pid > 0);

    close(internal[1]);

    enum sc_process_result res = SC_PROCESS_SUCCESS;
    // wait for EOF or receive err from child
    if (read(internal[0], &res, sizeof(res)) == -1) {
        perror("read");
        res = SC_PROCESS_ERROR_GENERIC;
    }

    close(internal[0]);

    if (pin) {
        close(in[0]);
        *pin = in[1];
    }
    if (pout) {
        *pout = out[0];
        close(out[1]);
    }
    if (perr) {
        *perr = err[0];
        close(err[1]);
    }

    return res;
}

bool
sc_process_terminate(pid_t pid) {
    if (pid <= 0) {
        LOGC("Requested to kill %d, this is an error. Please report the bug.\n",
             (int) pid);
        abort();
    }
    return kill(pid, SIGKILL) != -1;
}

sc_exit_code
sc_process_wait(pid_t pid, bool close) {
    int code;
    int options = WEXITED;
    if (!close) {
        options |= WNOWAIT;
    }

    siginfo_t info;
    int r = waitid(P_PID, pid, &info, options);
    if (r == -1 || info.si_code != CLD_EXITED) {
        // could not wait, or exited unexpectedly, probably by a signal
        code = SC_EXIT_CODE_NONE;
    } else {
        code = info.si_status;
    }
    return code;
}

void
sc_process_close(pid_t pid) {
    sc_process_wait(pid, true); // ignore exit code
}

ssize_t
sc_pipe_read(int pipe, char *data, size_t len) {
    return read(pipe, data, len);
}

void
sc_pipe_close(int pipe) {
    if (close(pipe)) {
        perror("close pipe");
    }
}
