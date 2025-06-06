/*
 * Copyright (c) 2015-2017, Ieshen Zheng <ieshen.zheng at 163 dot com>
 * Copyright (c) 2020, Nick <heronr1 at gmail dot com>
 * Copyright (c) 2020-2021, Bjorn Svensson <bjorn.a.svensson at est dot tech>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <errno.h>
#include <fcntl.h>
#include <alloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifndef _WIN32
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/time.h>
#endif

#ifdef HI_HAVE_BACKTRACE
#include <execinfo.h>
#endif

#include "hiutil.h"
#include "win32.h"

#ifndef _WIN32
int hi_set_blocking(int sd) {
    int flags;

    flags = fcntl(sd, F_GETFL, 0);
    if (flags < 0) {
        return flags;
    }

    return fcntl(sd, F_SETFL, flags & ~O_NONBLOCK);
}

int hi_set_nonblocking(int sd) {
    int flags;

    flags = fcntl(sd, F_GETFL, 0);
    if (flags < 0) {
        return flags;
    }

    return fcntl(sd, F_SETFL, flags | O_NONBLOCK);
}

int hi_set_reuseaddr(int sd) {
    int reuse;
    socklen_t len;

    reuse = 1;
    len = sizeof(reuse);

    return setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuse, len);
}

/*
 * Disable Nagle algorithm on TCP socket.
 *
 * This option helps to minimize transmit latency by disabling coalescing
 * of data to fill up a TCP segment inside the kernel. Sockets with this
 * option must use readv() or writev() to do data transfer in bulk and
 * hence avoid the overhead of small packets.
 */
int hi_set_tcpnodelay(int sd) {
    int nodelay;
    socklen_t len;

    nodelay = 1;
    len = sizeof(nodelay);

    return setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &nodelay, len);
}

int hi_set_linger(int sd, int timeout) {
    struct linger linger;
    socklen_t len;

    linger.l_onoff = 1;
    linger.l_linger = timeout;

    len = sizeof(linger);

    return setsockopt(sd, SOL_SOCKET, SO_LINGER, &linger, len);
}

int hi_set_sndbuf(int sd, int size) {
    socklen_t len;

    len = sizeof(size);

    return setsockopt(sd, SOL_SOCKET, SO_SNDBUF, &size, len);
}

int hi_set_rcvbuf(int sd, int size) {
    socklen_t len;

    len = sizeof(size);

    return setsockopt(sd, SOL_SOCKET, SO_RCVBUF, &size, len);
}

int hi_get_soerror(int sd) {
    int status, err;
    socklen_t len;

    err = 0;
    len = sizeof(err);

    status = getsockopt(sd, SOL_SOCKET, SO_ERROR, &err, &len);
    if (status == 0) {
        errno = err;
    }

    return status;
}

int hi_get_sndbuf(int sd) {
    int status, size;
    socklen_t len;

    size = 0;
    len = sizeof(size);

    status = getsockopt(sd, SOL_SOCKET, SO_SNDBUF, &size, &len);
    if (status < 0) {
        return status;
    }

    return size;
}

int hi_get_rcvbuf(int sd) {
    int status, size;
    socklen_t len;

    size = 0;
    len = sizeof(size);

    status = getsockopt(sd, SOL_SOCKET, SO_RCVBUF, &size, &len);
    if (status < 0) {
        return status;
    }

    return size;
}
#endif

int _hi_atoi(uint8_t *line, size_t n) {
    int value;

    if (n == 0) {
        return -1;
    }

    for (value = 0; n--; line++) {
        if (*line < '0' || *line > '9') {
            return -1;
        }

        value = value * 10 + (*line - '0');
    }

    if (value < 0) {
        return -1;
    }

    return value;
}

void _hi_itoa(uint8_t *s, int num) {
    uint8_t c;
    uint8_t sign = 0;

    if (s == NULL) {
        return;
    }

    uint32_t len, i;
    len = 0;

    if (num < 0) {
        sign = 1;
        num = abs(num);
    } else if (num == 0) {
        s[len++] = '0';
        return;
    }

    while (num % 10 || num / 10) {
        c = num % 10 + '0';
        num = num / 10;
        s[len + 1] = s[len];
        s[len] = c;
        len++;
    }

    if (sign == 1) {
        s[len++] = '-';
    }

    s[len] = '\0';

    for (i = 0; i < len / 2; i++) {
        c = s[i];
        s[i] = s[len - i - 1];
        s[len - i - 1] = c;
    }
}

int hi_valid_port(int n) {
    if (n < 1 || n > UINT16_MAX) {
        return 0;
    }

    return 1;
}

int _uint_len(uint32_t num) {
    int n = 0;

    if (num == 0) {
        return 1;
    }

    while (num != 0) {
        n++;
        num /= 10;
    }

    return n;
}

void hi_stacktrace(int skip_count) {
#ifdef HI_HAVE_BACKTRACE
    void *stack[64];
    char **symbols;
    int size, i, j;

    size = backtrace(stack, 64);
    symbols = backtrace_symbols(stack, size);
    if (symbols == NULL) {
        return;
    }

    skip_count++; /* skip the current frame also */

    for (i = skip_count, j = 0; i < size; i++, j++) {
        printf("[%d] %s\n", j, symbols[i]);
    }

    hi_free(symbols);
#else
    (void)skip_count;
#endif
}

void hi_stacktrace_fd(int fd) {
#ifdef HI_HAVE_BACKTRACE
    void *stack[64];
    int size;

    size = backtrace(stack, 64);
    backtrace_symbols_fd(stack, size, fd);
#else
    (void)fd;
#endif
}

void hi_assert(const char *cond, const char *file, int line, int panic) {

    printf("File: %s Line: %d: %s\n", file, line, cond);

    if (panic) {
        hi_stacktrace(1);
        abort();
    }
    abort();
}

#ifndef _WIN32
/*
 * Send n bytes on a blocking descriptor
 */
ssize_t _hi_sendn(int sd, const void *vptr, size_t n) {
    size_t nleft;
    ssize_t nsend;
    const char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        nsend = send(sd, ptr, nleft, 0);
        if (nsend < 0) {
            if (errno == EINTR) {
                continue;
            }
            return nsend;
        }
        if (nsend == 0) {
            return -1;
        }

        nleft -= (size_t)nsend;
        ptr += nsend;
    }

    return (ssize_t)n;
}

/*
 * Recv n bytes from a blocking descriptor
 */
ssize_t _hi_recvn(int sd, void *vptr, size_t n) {
    size_t nleft;
    ssize_t nrecv;
    char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        nrecv = recv(sd, ptr, nleft, 0);
        if (nrecv < 0) {
            if (errno == EINTR) {
                continue;
            }
            return nrecv;
        }
        if (nrecv == 0) {
            break;
        }

        nleft -= (size_t)nrecv;
        ptr += nrecv;
    }

    return (ssize_t)(n - nleft);
}
#endif

/*
 * Return the current time in microseconds since Epoch
 */
int64_t hi_usec_now(void) {
    int64_t usec;
#ifdef _WIN32
    LARGE_INTEGER counter, frequency;

    if (!QueryPerformanceCounter(&counter) ||
        !QueryPerformanceFrequency(&frequency)) {
        return -1;
    }

    usec = counter.QuadPart * 1000000 / frequency.QuadPart;
#else
    struct timeval now;
    int status;

    status = gettimeofday(&now, NULL);
    if (status < 0) {
        return -1;
    }

    usec = (int64_t)now.tv_sec * 1000000LL + (int64_t)now.tv_usec;
#endif

    return usec;
}

/*
 * Return the current time in milliseconds since Epoch
 */
int64_t hi_msec_now(void) { return hi_usec_now() / 1000LL; }
