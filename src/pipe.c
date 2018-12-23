#include <stdio.h>
#include <stdlib.h>

#include "coroutine.h"

static void reader(void *opaque)
{
	char line[1024];

	(void)opaque;
	while (fgets(line, sizeof(line), stdin) != NULL)
		coroutine_yield(line);
}

static void filter(void *opaque)
{
	Coroutine *co_reader = opaque;
	char buf[1024];
	char *p;
	int n = 0;

	while ((p = coroutine_resume(co_reader)) != NULL) {
		snprintf(buf, sizeof(buf), "%3d | %s", ++n, p);
		coroutine_yield(buf);
	}
}

static void writer(void *opaque)
{
	Coroutine *co_filter = opaque;
	char *p;

	while ((p = coroutine_resume(co_filter)) != NULL)
		printf("%s", p);
}

int main(void)
{
	Coroutine *co_reader, *co_filter, *co_writer;

	/* Do it like pipe: reader | filter | writer */
	co_reader = coroutine_create(0, reader, NULL);
	co_filter = coroutine_create(0, filter, co_reader);
	co_writer = coroutine_create(0, writer, co_filter);

	coroutine_resume(co_writer);

	coroutine_destroy(&co_reader);
	coroutine_destroy(&co_filter);
	coroutine_destroy(&co_writer);

	return EXIT_SUCCESS;
}

