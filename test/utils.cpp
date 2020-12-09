/** @file   utils.cpp
 *  @brief  Unit-test utilities.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2019-02-03 create new.
 */

#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <cerrno>
#include <ctime>

#include "utils.hpp"

int msleep(long msec)
{
    struct timespec req, rem = {msec / 1000, (msec % 1000) * 1000000};
    int ret;

    do {
        req = rem;
        ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &req, &rem);
    } while ((ret != 0) && (errno == EINTR));

    return ret;
}

int64_t getuptime(int64_t base)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return -1;
    }
    return (ts.tv_sec * 1000 + (ts.tv_nsec / 1000000)) - base;
}

ssize_t file_read(const char *path, void *buf, size_t count)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    ssize_t read_sz = read(fd, buf, count);

    close(fd);

    return read_sz;
}

ssize_t file_read(std::string path, void *buf, size_t count)
{
    return file_read(path.c_str(), buf, count);
}

ssize_t file_write(const char *path, void *buf, size_t count)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0) {
        return -1;
    }

    ssize_t written_sz = write(fd, buf, count);

    close(fd);

    return written_sz;
}

ssize_t file_write(std::string path, void *buf, size_t count)
{
    return file_write(path.c_str(), buf, count);
}
