#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>
#include <assert.h>

#include "coroutine.h"

/* DETACHED is not a state, it is a flag. */
#define DETACHED	0x01
/* Coroutine states. */
#define RUNNING		0x02
#define	YIELDED		0x04
#define TERMINATED	0x08

struct Coroutine {
	int		state;
	sigjmp_buf	env;
	void		(*entry)(void *opaque);
	void		*opaque;
	void		*resume;
	void		*sp;
	Coroutine	*callee;
};

static Coroutine coro_main, *current = &coro_main;

static void yield(Coroutine *co, int state, void *resume)
{
	co->resume = resume;
	co->state  = state | (co->state & DETACHED);
	
	if (sigsetjmp(current->env, 0) == 0)
		siglongjmp(current->callee->env, 1);
}

void coroutine_yield(void *resume)
{
	assert(current != &coro_main);
	assert(current->state & RUNNING);
	yield(current, YIELDED, resume);
}

static void coroutine_start(Coroutine *co)
{
	if (sigsetjmp(co->env, 0) == 0)
		siglongjmp(co->callee->env, 1);

	co->entry(co->opaque);
	yield(co, TERMINATED, NULL);
}

static void sigusr1(int signo)
{
	(void)signo;
	if (sigsetjmp(current->env, 0) == 0)
		return;
	/* We come here not in signal context. */
	coroutine_start(current);
}

void *coroutine_resume(Coroutine *co)
{
	void *resume = co->resume;

	if (co->state & TERMINATED)
		goto out;

	assert(co->state & YIELDED);

	co->state  = RUNNING | (co->state & DETACHED);
	co->callee = current;
	current    = co;
	if (sigsetjmp(co->callee->env, 0) == 0)
		siglongjmp(co->env, 1);	
	resume     = co->resume;
	current    = co->callee;
	co->callee = NULL;

	if ((co->state & TERMINATED) && (co->state & DETACHED))
		coroutine_destroy(&co);
out:
	return resume;
}

Coroutine *
coroutine_create(size_t _stacksz, void (*entry)(void *), void *opaque)
{
	Coroutine *co;
	struct sigaction sa, osa;
	stack_t stack, ostack;
	sigset_t mask, omask;
	size_t stacksz = _stacksz ? _stacksz : CO_STACK_SIZE;

	co = malloc(sizeof(*co));
	if (!co)
		return NULL;

	memset(co, 0, sizeof(*co));
	co->state  = YIELDED;
	co->entry  = entry;
	co->opaque = opaque;
	co->sp     = malloc(stacksz);
	if (!co->sp) {
		free(co);
		return NULL;
	}
	/* Block SIGUSR1, we need it to bootstrap a new coroutine. */
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigprocmask(SIG_BLOCK, &mask, &omask);

	stack.ss_sp    = co->sp;
	stack.ss_size  = stacksz;
	stack.ss_flags = 0;
	if (sigaltstack(&stack, &ostack) < 0) {
		free(co->sp);
		free(co);
		sigprocmask(SIG_SETMASK, &omask, NULL);
		return NULL;
	}

	/* Replace SIGUSR1, set the alternative stack for the handler. */	
	memset(&sa, 0, sizeof(sa));
	sigfillset(&sa.sa_mask);
	sa.sa_handler = sigusr1;
	sa.sa_flags   = SA_ONSTACK;
	sigaction(SIGUSR1, &sa, &osa);
	
	raise(SIGUSR1);
	sigfillset(&mask);
	sigdelset(&mask, SIGUSR1);
	/* Allow only SIGUSR1 for a while, let the handler save
	 * the execution environment with sigsetjmp in sigusr1(). */
	co->callee = current;
	current    = co;
	sigsuspend(&mask);
	current    = co->callee;
	co->callee = NULL;
	/* Disable the alternative stack and restore the old stack
	 * if it was active. */	
	sigaltstack(NULL, &stack);
	stack.ss_flags |= SS_DISABLE;
	if (sigaltstack(&stack, NULL) < 0)
		abort();
	if (!(ostack.ss_flags & SS_DISABLE))
		sigaltstack(&ostack, NULL);

	/* Restore the previous handler and the signal mask. */
	sigaction(SIGUSR1, &osa, NULL);
	sigprocmask(SIG_SETMASK, &omask, NULL);

	co->callee = current;
	current    = co;
	/* Jump back to sigusr1() but now not in signal context. */
	if (sigsetjmp(co->callee->env, 0) == 0)
		siglongjmp(co->env, 1);
	current    = co->callee;
	co->callee = NULL;

	return co;
}

Coroutine *coroutine_self(void)
{
	return current;
}

void coroutine_detach(Coroutine *co)
{
	co->state |= DETACHED;
}

void coroutine_destroy(Coroutine **co)
{
	free((*co)->sp);
	free((*co));
	*co = NULL;
}

const char *coroutine_state(const Coroutine *co)
{
	return co->state & RUNNING ? "running" :
	       co->state & YIELDED ? "yielded" :
	       co->state & TERMINATED ? "terminated" : "unknown";
}

