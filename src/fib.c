#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "coroutine.h"

static void fib(void *opaque)
{
	unsigned long a = 0, b = 1, c;
	unsigned long m = 0, n = (uintptr_t)opaque;

	for (;;) {
		if (n && m++ >= n)
			break;
		coroutine_yield(&a);
		c = a + b;
		a = b;
		b = c;
	}
}

int main(void)
{
	Coroutine *co_fib;
	unsigned long *n, m = 0;

	/* Generate N numbers. */
	co_fib = coroutine_create(0, fib, (void *)(uintptr_t)20);
	coroutine_detach(co_fib);

	/* fib returns a pointer that points to a value on its own stack. */
	while ((n = coroutine_resume(co_fib)))
		printf("%5lu: %20lu\n", ++m, *n);

	return EXIT_SUCCESS;
}

