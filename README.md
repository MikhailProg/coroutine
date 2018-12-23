# coroutine

A small coroutine library (based on sigaltstack(2)). There are two primitives to control coroutine execution -- resume and yield.

## Build

```
$ make -C src
```

## Run pipe sample

```
$ ./src/pipe
```

pipe emulates the follwing command:

```
$ reader | filter | writer
```

Each program in the pipe is presented as a separate coroutine. The reader reads stdin, the filter modifies the input data, the writer dumps the modified data to stdout.

```
$ ./src/fib
```

fib produces 20 first Fibonacci numbers. In this case a coroutine acts like a generator.

