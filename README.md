# coroutine

A small coroutine library (based on sigaltstack(2)). There are two primitives to control coroutine execution -- resume and yield.

## Build

```
$ make -C src
```

## Run sample

```
$ ./src/sample
```

sample emulates the follwing command:

```
$ reader | filter | writer
```

Each program in the pipe is presented as a separate coroutine. The reader reads stdin, the filter modifies the input data, the writer dumps the modified data to stdout.

