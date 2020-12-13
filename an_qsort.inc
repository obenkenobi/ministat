/*
 * an_qsort(): Adapted from FreeBSD version and modified to work
 * specifically with specific types instead of raw memory buffers.
 *
 * Macros:
 *   AN_QSORT_SUFFIX(STRING): namespacify STRING and append SUFFIX
 *   AN_QSORT_TYPE: the type of elements in the buffer
 *   AN_QSORT_CMP(AN_QSORT_TYPE *a, AN_QSORT_TYPE *b) -> 0 if *a == *b, or (*a < *b) ? -1 : 1 (depending on what you want)
 *   AN_QSORT_HELPER_LINKAGE: linkage for internal functions.
 *
 * FreeBSD:
 *
 * Copyright 1992-2015 The FreeBSD Project. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */

#include <stddef.h>
#include <string.h>
#include <pthread.h>

#ifndef AN_QSORT_SUFFIX
# error "AN_QSORT_SUFFIX undefined"
#endif

#ifndef AN_QSORT_TYPE
# error "AN_QSORT_TYPE undefined"
#endif

#ifndef AN_QSORT_CMP
# error "AN_QSORT_CMP undefined"
#endif

#ifndef AN_QSORT_ENABLE_INSERTION_SORT_HEURISTIC
# define AN_QSORT_ENABLE_INSERTION_SORT_HEURISTIC 1
#endif

#ifndef AN_QSORT_HELPER_LINKAGE
# ifdef __GNUC__
#  define AN_QSORT_HELPER_LINKAGE static inline __attribute__((flatten))
# else
#  define AN_QSORT_HELPER_LINKAGE static inline
# endif
#endif

#define AN_QSORT_NAME_SUFFIX__(NAME, SUFFIX) NAME##_##SUFFIX
#define AN_QSORT_NAME_SUFFIX_(NAME, SUFFIX) AN_QSORT_NAME_SUFFIX__(NAME, SUFFIX)
#define AN_QSORT_NAME_SUFFIX(NAME) AN_QSORT_NAME_SUFFIX_(NAME, AN_QSORT_SUFFIX)

#ifndef max
#define max(a, b)					\
	({						\
		__typeof__(a) _a = (a);			\
		__typeof__(b) _b = (b);			\
		_a > _b ? _a : _b;			\
	})
#endif
#ifndef min
#define min(a, b)					\
	({						\
		__typeof__(a) _a = (a);			\
		__typeof__(b) _b = (b);			\
		_b > _a ? _a : _b;			\
	})
#endif

AN_QSORT_HELPER_LINKAGE void
AN_QSORT_NAME_SUFFIX(an_qsort_swapfunc)(AN_QSORT_TYPE *i, AN_QSORT_TYPE *j, ssize_t n)
{
	AN_QSORT_TYPE *a = i;
	AN_QSORT_TYPE *b = j;
	do {
		AN_QSORT_TYPE t;

		memcpy(&t, a, sizeof(t));
		memcpy(a++, b, sizeof(t));
		memcpy(b++, &t, sizeof(t));
	} while (--n > 0);
}

AN_QSORT_HELPER_LINKAGE AN_QSORT_TYPE *
AN_QSORT_NAME_SUFFIX(an_qsort_med3)(AN_QSORT_TYPE *a, AN_QSORT_TYPE *b, AN_QSORT_TYPE *c)
{
	return (AN_QSORT_CMP(a, b) < 0) ?
		(AN_QSORT_CMP(b, c) < 0 ? b : (AN_QSORT_CMP(a, c) < 0 ? c : a ))
		:(AN_QSORT_CMP(b, c) > 0 ? b : (AN_QSORT_CMP(a, c) < 0 ? a : c ));
}

AN_QSORT_HELPER_LINKAGE void
AN_QSORT_NAME_SUFFIX(an_qsort_swap)(AN_QSORT_TYPE *a, AN_QSORT_TYPE *b)
{
	AN_QSORT_TYPE t;

	memcpy(&t, a, sizeof(t));
	memcpy(a, b, sizeof(t));
	memcpy(b, &t, sizeof(t));
}

static void
AN_QSORT_NAME_SUFFIX(an_qsort)(AN_QSORT_TYPE *a, size_t n)
{
	AN_QSORT_TYPE *pa;
	AN_QSORT_TYPE *pb;
	AN_QSORT_TYPE *pc;
	AN_QSORT_TYPE *pd;
	AN_QSORT_TYPE *pl;
	AN_QSORT_TYPE *pm;
	AN_QSORT_TYPE *pn;
	size_t d;
	int r, swap_cnt;

loop:
	swap_cnt = 0;
	if (n < 7) {
		for (pm = a + 1; pm < a + n; pm += 1) {
			for (pl = pm; pl > a && AN_QSORT_CMP(pl - 1, pl) > 0; pl -= 1) {
				AN_QSORT_NAME_SUFFIX(an_qsort_swap)(pl, pl - 1);
			}
		}
		return;
	}
	pm = a + (n / 2);
	if (n > 7) {
		pl = a;
		pn = a + (n - 1);
		if (n > 40) {
			d = (n / 8);
			pl = AN_QSORT_NAME_SUFFIX(an_qsort_med3)(pl, pl + d, pl + 2 * d);
			pm = AN_QSORT_NAME_SUFFIX(an_qsort_med3)(pm - d, pm, pm + d);
			pn = AN_QSORT_NAME_SUFFIX(an_qsort_med3)(pn - 2 * d, pn - d, pn);
		}
		pm = AN_QSORT_NAME_SUFFIX(an_qsort_med3)(pl, pm, pn);
	}
	AN_QSORT_NAME_SUFFIX(an_qsort_swap)(a, pm);
	pa = pb = a + 1;
	pc = pd = a + (n - 1);
	for (;;) {
		while (pb <= pc && (r = AN_QSORT_CMP(pb, a)) <= 0) {
			if (r == 0) {
				swap_cnt = 1;
				AN_QSORT_NAME_SUFFIX(an_qsort_swap)(pa, pb);
				++pa;
			}
			++pb;
		}
		while (pb <= pc && (r = AN_QSORT_CMP(pc, a)) >= 0) {
			if (r == 0) {
				swap_cnt = 1;
				AN_QSORT_NAME_SUFFIX(an_qsort_swap)(pc, pd);
				--pd;
			}
			--pc;
		}
		if (pb > pc) {
			break;
		}
		AN_QSORT_NAME_SUFFIX(an_qsort_swap)(pb, pc);
		swap_cnt = 1;
		++pb;
		--pc;
	}
	if (swap_cnt == 0 && (AN_QSORT_ENABLE_INSERTION_SORT_HEURISTIC)) {  /* Switch to insertion sort */
		for (pm = a + 1; pm < a + n; ++pm) {
			for (pl = pm; pl > a && AN_QSORT_CMP(pl - 1, pl) > 0; --pl) {
				AN_QSORT_NAME_SUFFIX(an_qsort_swap)(pl, pl - 1);
			}
		}
		return;
	}
	pn = a + n;
	r = min(pa - a, pb - pa);
	if(r > 0) {
		AN_QSORT_NAME_SUFFIX(an_qsort_swapfunc)(a, pb - r, r);
	}
	r = min(pd - pc, pn - pd - 1);
	if(r > 0) {
		AN_QSORT_NAME_SUFFIX(an_qsort_swapfunc)(pb, pn - r, r);
	}
	if((r = pb - pa) > 1) {
		AN_QSORT_NAME_SUFFIX(an_qsort)(a, r);
	}
	if ((r = pd - pc) > 0) {
		/* Iterate rather than recurse to save stack space */
		a = pn - r;
		n = r;
		goto loop;
	}
}

struct AN_QSORT_NAME_SUFFIX(an_copy_thread_payload) {
	AN_QSORT_TYPE *src;
	AN_QSORT_TYPE *dest;
	size_t n;
	int depth;
};

static void*
AN_QSORT_NAME_SUFFIX(an_copy_parallel)(void* payload) {
	struct AN_QSORT_NAME_SUFFIX(an_copy_thread_payload) *copy_payload = 
		(struct AN_QSORT_NAME_SUFFIX(an_copy_thread_payload)*)payload;
	if(copy_payload->depth >= 1) {
		pthread_t l_thread, r_thread;
		struct AN_QSORT_NAME_SUFFIX(an_copy_thread_payload) l_payload, r_payload;
		size_t m = copy_payload->n/2;

		l_payload = (struct AN_QSORT_NAME_SUFFIX(an_copy_thread_payload)) 
			{
				.src = copy_payload->src, .dest = copy_payload->dest, 
				.n=m, .depth = copy_payload->depth - 1
			};
		r_payload = (struct AN_QSORT_NAME_SUFFIX(an_copy_thread_payload)) 
			{
				.src = copy_payload->src + m, .dest = copy_payload->dest + m, 
				.n= copy_payload->n - m, .depth = copy_payload->depth - 1
			};

		pthread_create(&l_thread, NULL, AN_QSORT_NAME_SUFFIX(an_copy_parallel), &l_payload);
		pthread_create(&r_thread, NULL, AN_QSORT_NAME_SUFFIX(an_copy_parallel), &r_payload);
		pthread_join(l_thread, NULL); pthread_join(r_thread, NULL);

	} else {
		for(size_t index = 0; index < copy_payload->n; index++) {
			copy_payload->dest[index] = copy_payload->src[index];
		}
	}
	return NULL;
}

static void
AN_QSORT_NAME_SUFFIX(an_merge)(AN_QSORT_TYPE *a, size_t n1, size_t n2, int depth){
	struct AN_QSORT_NAME_SUFFIX(an_copy_thread_payload) l_payload, r_payload;

	AN_QSORT_TYPE *left = (AN_QSORT_TYPE *)malloc(n1*(sizeof(AN_QSORT_TYPE)));
	AN_QSORT_TYPE *right = (AN_QSORT_TYPE *)malloc(n2*(sizeof(AN_QSORT_TYPE)));

	l_payload = (struct AN_QSORT_NAME_SUFFIX(an_copy_thread_payload)) 
		{.src = a, .dest = left, .n=n1, .depth = depth - 1};
	r_payload = (struct AN_QSORT_NAME_SUFFIX(an_copy_thread_payload)) 
		{.src = a + n1, .dest = right, .n=n2, .depth = depth - 1};

	AN_QSORT_NAME_SUFFIX(an_copy_parallel)(&l_payload);
	AN_QSORT_NAME_SUFFIX(an_copy_parallel)(&r_payload);
	
	size_t k = 0, i = 0, j = 0;
	while (i < n1 && j < n2)
	{
		if(AN_QSORT_CMP((void*)(left + i),(void*)(right +j)) <= 0) {
			a[k] = left[i];
			i++;
		} else {
			a[k] = right[j];
			j++;
		}
		k++;
	}
	if(i < n1) {
		l_payload = (struct AN_QSORT_NAME_SUFFIX(an_copy_thread_payload)) 
			{.src = left + i, .dest = a + k, .n=n1 - i, .depth = depth - 1};
		AN_QSORT_NAME_SUFFIX(an_copy_parallel)(&l_payload);
		k += n1 - i;
	}
	if(j < n2) {
		l_payload = (struct AN_QSORT_NAME_SUFFIX(an_copy_thread_payload)) 
			{.src = right + j, .dest = a + k, .n=n2 - j, .depth = depth - 1};
		AN_QSORT_NAME_SUFFIX(an_copy_parallel)(&l_payload);
		k += n2 - j;
	}
	free(left);
	free(right);
}

struct AN_QSORT_NAME_SUFFIX(an_sort_thread_payload) {
	AN_QSORT_TYPE *a;
	size_t n;
	int depth;
};

static void*
AN_QSORT_NAME_SUFFIX(an_parallel_sort_thread)(void* payload) {
	struct AN_QSORT_NAME_SUFFIX(an_sort_thread_payload) *thread_payload = payload;
	size_t n = thread_payload->n;
	if(thread_payload->depth >= 1) {
		struct AN_QSORT_NAME_SUFFIX(an_sort_thread_payload) l_payload, r_payload;
		pthread_t l_thread, r_thread;
		size_t m = n/2;
		l_payload = (struct AN_QSORT_NAME_SUFFIX(an_sort_thread_payload))
			{.a = thread_payload->a, .n = m, .depth = thread_payload->depth-1};
		r_payload = (struct AN_QSORT_NAME_SUFFIX(an_sort_thread_payload))
			{.a = thread_payload->a + m, .n = n - m, .depth = thread_payload->depth-1};
		pthread_create(&l_thread, NULL, AN_QSORT_NAME_SUFFIX(an_parallel_sort_thread), &l_payload);
		pthread_create(&r_thread, NULL, AN_QSORT_NAME_SUFFIX(an_parallel_sort_thread), &r_payload);
		pthread_join(l_thread, NULL); pthread_join(r_thread, NULL);
		AN_QSORT_NAME_SUFFIX(an_merge)(thread_payload->a, m, n-m, thread_payload->depth);
	} else {
		AN_QSORT_NAME_SUFFIX(an_qsort)(thread_payload->a, n);
	}
	return NULL;
}

static void 
AN_QSORT_NAME_SUFFIX(an_parallel_sort)(AN_QSORT_TYPE *a, size_t n, int n_threads) {
	int depth=0;
	for(int d=n_threads; d >= 1; d = d/2) depth++;
	
	struct AN_QSORT_NAME_SUFFIX(an_sort_thread_payload) sort_payload = 
		(struct AN_QSORT_NAME_SUFFIX(an_sort_thread_payload))
		{.a = a, .n = n, .depth = depth};
	AN_QSORT_NAME_SUFFIX(an_parallel_sort_thread)(&sort_payload);
}

#undef AN_QSORT_SUFFIX
#undef AN_QSORT_TYPE
#undef AN_QSORT_CMP
#undef AN_QSORT_HELPER_LINKAGE
#undef AN_QSORT_NAME_SUFFIX
#undef AN_QSORT_NAME_SUFFIX_
#undef AN_QSORT_NAME_SUFFIX__
#undef AN_QSORT_ENABLE_INSERTION_SORT_HEURISTIC
