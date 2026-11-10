#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define BUF_SIZE 128

typedef struct {
    int len;
    char signal;
    char data[BUF_SIZE];
} shared_t;

char shm_name[64];
char sem_data_name[64];
char sem_done_name[64];
volatile sig_atomic_t interrupted = 0;

void on_sigint(int sig) {
    (void)sig;
    interrupted = 1;
}

void say(int fd, const char *msg) {
    ssize_t r = write(fd, msg, strlen(msg));
    (void)r;
}

void cleanup() {
    shm_unlink(shm_name);
    sem_unlink(sem_data_name);
    sem_unlink(sem_done_name);
}

void print_error(const char *msg) {
    say(2, msg);
    cleanup();
    exit(1);
}

int wait_client(sem_t *sem, pid_t pid) {
    while (!interrupted) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 100000000;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec += 1;
            ts.tv_nsec -= 1000000000;
        }
        if (sem_timedwait(sem, &ts) == 0) {
            return 0;
        }
        if (errno == ETIMEDOUT && waitpid(pid, NULL, WNOHANG) == pid) {
            return -1;
        }
    }
    return -1;
}

int main() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigint;
    sigaction(SIGINT, &sa, NULL);

    snprintf(shm_name, sizeof(shm_name), "/lab3_shm_%d", getpid());
    snprintf(sem_data_name, sizeof(sem_data_name), "/lab3_data_%d", getpid());
    snprintf(sem_done_name, sizeof(sem_done_name), "/lab3_done_%d", getpid());

    say(1, "Write a name of out file\n");
    char filename[256];
    int pos = 0;
    char a;
    while (read(0, &a, 1) > 0 && a != '\n' && a != '\r') {
        if (pos == 255) print_error("File name is too long. :(\n");
        filename[pos++] = a;
    }
    filename[pos] = '\0';
    if (interrupted) print_error("Interrupted. Bye!\n");
    if (pos == 0) print_error("Empty file name. :(\n");

    int shm_fd = shm_open(shm_name, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (shm_fd == -1) print_error("Shared memory not created. :(\n");
    if (ftruncate(shm_fd, sizeof(shared_t)) == -1) print_error("ftruncate failed. :(\n");
    shared_t *shm = mmap(NULL, sizeof(shared_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) print_error("mmap failed. :(\n");
    close(shm_fd);

    sem_t *sem_data = sem_open(sem_data_name, O_CREAT | O_EXCL, 0600, 0);
    sem_t *sem_done = sem_open(sem_done_name, O_CREAT | O_EXCL, 0600, 0);
    if (sem_data == SEM_FAILED || sem_done == SEM_FAILED) print_error("Semaphores not created. :(\n");

    pid_t pid = fork();
    if (pid == -1) print_error("Something gone wrong with fork. :(\n");

    if (pid == 0) {
        char *args[] = {"./client", filename, shm_name, sem_data_name, sem_done_name, NULL};
        execve("./client", args, NULL);
        say(2, "Something gone wrong with execve. :(\n");
        _exit(1);
    }

    say(1, "Now u can write numbers\n");
    int reason = 0;
    while (1) {
        ssize_t bytes_read = read(0, shm->data, BUF_SIZE);
        if (bytes_read < 0) {
            if (errno == EINTR && !interrupted) continue;
            if (interrupted) {
                reason = 3;
                break;
            }
            print_error("Read failed. :(\n");
        }
        if (bytes_read == 0) break;

        shm->len = bytes_read;
        sem_post(sem_data);
        if (wait_client(sem_done, pid) == -1) {
            reason = interrupted ? 3 : 2;
            break;
        }
        if (shm->signal == 'X') {
            reason = 1;
            break;
        }
    }

    shm->len = 0;
    sem_post(sem_data);
    int status;
    while (waitpid(pid, &status, 0) == -1 && errno == EINTR);
    if (reason == 0 && shm->signal == 'X') reason = 1;

    if (reason == 1) {
        say(1, "From parent: terminate signal 'X' reached. Exiting!\n");
    } else if (reason == 2) {
        say(1, "From parent: process stopped without signal. Exiting!\n");
    } else if (reason == 3) {
        say(1, "From parent: interrupted. Bye!\n");
    } else {
        say(1, "Nothing to read. Bye!\n");
    }

    munmap(shm, sizeof(shared_t));
    sem_close(sem_data);
    sem_close(sem_done);
    cleanup();
    return 0;
}
