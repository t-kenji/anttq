AntTQ (Embeded Task Queue) {#mainpage}
==========================

Introduction
------------

AntTQ is Task Queue for embedded.

Dependencies
------------

- libpthread
- (optional) Catch2 (for test)

How to build
------------

```
$ make
```

How to test
-----------

```
$ make test
```

run clang static analyer
------------------------

```
$ scan-build --use-cc=`which clang` make NODEBUG=1 EXTRA_CFLAGS=-fblocks EXTRA_LIBS=-lBlocksRuntime
```
