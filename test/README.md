# Allocator tests

Run all tests in Docker from the repository root:

```sh
bash ./run.sh
```

The runner defaults to the custom allocator (`TEST_USE_SYSTEM_MALLOC=0`).
Select libc with the command below. In system mode, `test_bin_index` is
skipped and `test_new_delete` uses standard C++ new/delete.

```sh
TEST_USE_SYSTEM_MALLOC=1 ./run.sh
```

Run selected tests:

```sh
bash ./run.sh test_data_integrity test_malloc_free
```

On Linux with GCC installed, use `sh test/run.sh` directly.
The runner compiles each test independently, keeps assertions enabled, and
executes each in a separate process with a 20-second timeout. Compilation
has a 60-second timeout. It continues after failures and exits nonzero if
any test fails. Binaries and logs are temporary; failed cases print the
last 12 log lines. The multi-translation-unit case compiles and links both
fixture files together.

The tests from `main.cpp` now live in `test_sbrk.cpp`, `test_malloc_free.cpp`,
`test_memory_mmap.cpp`, `test_malloc_additional.cpp`, `test_bin_index.cpp`, `test_mallinfo.cpp`,
`test_mallopt.cpp`, and `test_new_delete.cpp`. The bin-index and mallinfo cases retain
the original diagnostic demonstrations.

| Finding in BUGS.md | Regression test |
| --- | --- |
| P1.2 4 GiB mapping truncation | `test_p1_mmap_4gib` |
| P1.3 signed metadata at 2 GiB | `test_p1_metadata_2gib` |
| P1.4 size overflow | `test_p1_size_overflow` |
| P1.5 concurrent allocation | `test_p1_concurrent` |
| P2.6 mmap count limit | `test_p2_mmap_limit` |
| P2.7 retained trim padding | `test_p2_trim_pad` |
| P2.8 trim return value | `test_p2_trim_result` |
| P2.9 multi-file integration | `test_p2_multi_file` |
| P2.10 stress validation | `test_data_integrity` |

The teaching allocator retains its sbrk design. Safe coexistence with
libc's heap allocator is outside its supported scope and is not tested.

Regressions assert correct behavior and may fail until allocator bugs are
fixed. Concurrency failures can depend on scheduling and memory layout;
a passing run does not establish their absence. Large
allocation tests accept allocation failure, but a returned pointer must
have metadata covering the requested payload and support freeing. They
touch only the first and last payload bytes, avoiding physical allocation
of multiple GiB.

`test_data_integrity` uses a fixed seed and stores each allocation's size and
fill byte. It checks every byte of every live allocation after every
operation and before final cleanup, aborting on the first mismatch.

Historical Docker result before the heap-isolation test was removed,
on 2026-11-06 with `abseil-dev:latest`: **9 passed, 9 failed**.
All eight original cases and `test_data_integrity` passed. The five P1 cases
and four P2 allocator/integration cases failed, reproducing the unresolved
findings. No allocator implementation changes are included in this split.
