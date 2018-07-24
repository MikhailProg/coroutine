#ifndef COROUTINE_H
#define COROUTINE_H

#ifndef CO_STACK_SIZE
#  define CO_STACK_SIZE		262144
#endif

typedef struct Coroutine Coroutine;

/* Create a coroutine with the entry point and the opaque argument. */
Coroutine *
coroutine_create(size_t stacksz, void (*entry)(void *), void *opaque);

/* Run or continue the coroutine execution, it returns a pointer
 * passed from coroutine_yield. */
void *coroutine_resume(Coroutine *co);

/* Yield the current coroutine and return execution to coroutine_resume. */
void coroutine_yield(void *resume);

Coroutine *coroutine_self(void);

void coroutine_destroy(Coroutine **co);

void coroutine_detach(Coroutine *co);

const char *coroutine_state(const Coroutine *co);

#endif /* COROUTINE_H */
