## cplr - the C piler

The C piler *cplr* is a tool for executing C code from the shell and is meant to break the barrier between the shell and the core language of UNIX, which it does by making the compiler a tool like sed, grep and awk.

### But why!?

The practical purpose of *cplr* is to allow the compilation and execution of short C programs for testing, system information retrieval, live coding and interactive development of small C sequences and programs.

```
$ cplr 'puts("hello!")'
hello!
$ cplr 'printf("%d\n", getpagesize())'
4096
```

### Tell me more!

*cplr* itself is a simple **compiler front-end** that does not have any comprehension of the C language or even data types, but this is why it works well because it does not try to do something hard. Instead we follow the UNIX philosophy and do just one thing: act as the interactive equivalent of *cc*.

The current backend of *cplr* is based on **TinyCC**, which gives it a language level of C99 or better and the ability to interact with almost any C library on a normal Linux system including large and complicated ones. You can run Gtk or Python in this tool without having to expect any issues.

### Usage examples

A basic use is to retrieve type information:
```
$ cplr -i linux/kvm.h 'printf("%zu\n", sizeof(struct kvm_regs))'
144
```

You can also use *cplr* to create ad-hoc executables:
```
$ cplr -o realpath 'puts(realpath(argv[1],NULL))'
$ ./realpath .
/home/user
```

Small C libraries can be used directly from source:
```
$ cplr -s linenoise.c -i linenoise.h 'printf("Line: %s\n", linenoise("% "))'
% hello
Line: hello
```

Thanks to pkg-config you can easily call any library:
```
$ cplr -P tinfo -i term.h 'setupterm(NULL,1,NULL)' 'printf("%d\n", tigetnum("colors"))'
256
```

Further examples can be found [here](doc/examples.md).

### Building cplr

We build using CMake. You need readline. We bring our own copy of TinyCC.

```
$ git clone https://github.com/promovicz/cplr.git
$ cd cplr
$ git submodule init
$ git submodule update
$ cmake .
$ make
$ make install
```

Alternatively you can build a docker image containing cplr with

```
$ docker build -t cplr -f Dockerfile.debian .
```

### Future possibilities

 * C REPL (experimental form exists)
 * Garbage collection (in progress)
 * Crash handlers (easy)
 * Backtrace support
 * Bounds checking
 * Instruction trace
 * Call trace
 * Other line editors (not readline)
 * Internal disassembler (capstone)
 * Support other compilers using dlopen
 * Support launching into gdb/lldb
 * Source code and assembly dumping
 * Editor and pager integration
 * Live process code injection (researching)
 * Live kernel code injection (needs gcc)
 * Support for C++ and ObjC (needs gcc/clang)
