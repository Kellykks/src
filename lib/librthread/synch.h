/*	$OpenBSD: synch.h,v 1.1 2017/05/27 14:20:39 mpi Exp $ */
/*
 * Copyright (c) 2017 Martin Pieuchot
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/atomic.h>
#include <sys/time.h>
#include <sys/futex.h>

REDIRECT_SYSCALL(futex);

#include <machine/lock.h>	/* for SPINLOCK_SPIN_HOOK */
#define SPINLOCK_SPIN_COUNT	128

static inline int
_wake(volatile uint32_t *p, int n)
{
	return futex(p, FUTEX_WAKE, n, NULL, NULL);
}

static inline void
_wait(volatile uint32_t *p, int val)
{
	while (*p != (uint32_t)val)
		futex(p, FUTEX_WAIT, val, NULL, NULL);
}

static inline int
_twait(volatile uint32_t *p, int val, clockid_t clockid, const struct timespec *abs)
{
	struct timespec rel;

	if (abs == NULL)
		return futex(p, FUTEX_WAIT, val, NULL, NULL);

	if (abs->tv_nsec >= 1000000000 || clock_gettime(clockid, &rel))
		return (EINVAL);

	rel.tv_sec = abs->tv_sec - rel.tv_sec;
	if ((rel.tv_nsec = abs->tv_nsec - rel.tv_nsec) < 0) {
		rel.tv_sec--;
		rel.tv_nsec += 1000000000;
	}
	if (rel.tv_sec < 0)
		return (ETIMEDOUT);

	return futex(p, FUTEX_WAIT, val, &rel, NULL);
}

static inline int
_requeue(volatile uint32_t *p, int n, int m, volatile uint32_t *q)
{
	return futex(p, FUTEX_REQUEUE, n, (void *)(long)m, q);
}
