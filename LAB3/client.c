#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include <sys/mman.h>

#define BUF_SIZE 128

typedef struct {
    int len;
    char signal;
    char data[BUF_SIZE];
} shared_t;

long long number = 0;
int negative = 0;
int has_digits = 0;
int too_big = 0;

void say(int fd, const char *msg) {
    ssize_t r = write(fd, msg, strlen(msg));
    (void)r;
}

void print_error(const char *msg) {
    say(2, msg);
    exit(1);
}

int check_prime(long long n) {
    if (n < 2) return 0;
    for (long long i = 2; i * i <= n; i++) {
        if (n % i == 0) {
            return 0;
        }
    }
    return 1;
}

void wait_server(sem_t *sem, pid_t parent) {
    while (1) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 100000000;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec += 1;
            ts.tv_nsec -= 1000000000;
        }
        if (sem_timedwait(sem, &ts) == 0) {
            return;
        }
        if (errno == ETIMEDOUT && getppid() != parent) {
            print_error("Server process is gone. :(\n");
        }
    }
}

int end_of_number(int filefd) {
    int stop = 0;
    if (has_digits) {
        if (too_big) {
            say(2, "Number is too big, skipped. :(\n");
        } else {
            if (negative) number = -number;
            if (number <= 1 || check_prime(number)) {
                stop = 1;
            } else {
                char temp[32];
                char buffer[32];
                int t_pos = 0;
                long long n = number;

                while (n > 0) {
                    temp[t_pos++] = '0' + (n % 10);
                    n /= 10;
                }
                int b_pos = 0;
                while (t_pos > 0) {
                    buffer[b_pos++] = temp[--t_pos];
                }
                buffer[b_pos++] = '\n';
                if (write(filefd, buffer, b_pos) == -1) print_error("Write to file failed. :(\n");
            }
        }
    }
    number = 0;
    negative = 0;
    has_digits = 0;
    too_big = 0;
    return stop;
}

int main(int argc, char *argv[]) {
    if (argc < 5) print_error("client got less than 5 arguments. :(\n");

    pid_t parent = getppid();
    int filefd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (filefd == -1) print_error("File not opened. :(\n");

    int shm_fd = shm_open(argv[2], O_RDWR, 0);
    if (shm_fd == -1) print_error("Shared memory not opened. :(\n");
    shared_t *shm = mmap(NULL, sizeof(shared_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) print_error("mmap failed. :(\n");
    close(shm_fd);

    sem_t *sem_data = sem_open(argv[3], 0);
    sem_t *sem_done = sem_open(argv[4], 0);
    if (sem_data == SEM_FAILED || sem_done == SEM_FAILED) print_error("Semaphores not opened. :(\n");

    while (1) {
        wait_server(sem_data, parent);
        int len = shm->len;
        if (len == 0) {
            if (end_of_number(filefd)) shm->signal = 'X';
            break;
        }

        int stop = 0;
        for (int i = 0; i < len && !stop; i++) {
            char c = shm->data[i];
            if (c == '-') {
                negative = 1;
            } else if (c >= '0' && c <= '9') {
                if (number > (LLONG_MAX - 9) / 10) {
                    too_big = 1;
                } else {
                    number = number * 10 + (c - '0');
                }
                has_digits = 1;
            } else if (c == ' ' || c == '\n' || c == '\r') {
                stop = end_of_number(filefd);
            }
        }

        if (stop) shm->signal = 'X';
        sem_post(sem_done);
        if (stop) break;
    }

    munmap(shm, sizeof(shared_t));
    sem_close(sem_data);
    sem_close(sem_done);
    close(filefd);
    return 0;
}
