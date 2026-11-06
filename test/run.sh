#!/bin/sh
set -u
use_system_malloc=${TEST_USE_SYSTEM_MALLOC:-0}
case "$use_system_malloc" in
    0|1) ;;
    *) echo 'TEST_USE_SYSTEM_MALLOC must be 0 or 1' >&2; exit 2 ;;
esac
cd "$(dirname "$0")/.."
ulimit -c 0
build_dir=$(mktemp -d)
trap 'rm -rf "$build_dir"' EXIT HUP INT TERM
passed=0
failed=0
skipped=0

run_test() {
    name=$1
    shift
    if [ "$use_system_malloc" -eq 1 ]; then
        case "$name" in
            test_bin_index|test_p1_free_list_query)
                printf 'SKIP %s (custom allocator only)\n' "$name"
                skipped=$((skipped + 1))
                return
                ;;
        esac
        set -- -DTEST_USE_SYSTEM_MALLOC=1 "$@"
    fi
    log="$build_dir/$name.log"
    if timeout 60 g++ -std=c++17 -O0 -g -UNDEBUG -pthread "$@" -o "$build_dir/$name" >"$log" 2>&1; then
        timeout 20 "$build_dir/$name" >"$log" 2>&1
        status=$?
        if [ "$status" -eq 0 ]; then
            printf 'PASS %s\n' "$name"
            passed=$((passed + 1))
            return
        fi
        printf 'FAIL %s (exit %s)\n' "$name" "$status"
    else
        printf 'FAIL %s (compile/link)\n' "$name"
    fi
    tail -n 12 "$log"
    failed=$((failed + 1))
}

# Optional arguments select test names without .cpp; otherwise run everything.
if [ "$#" -eq 0 ]; then
    for source in test/*.cpp; do
        run_test "$(basename "$source" .cpp)" "$source"
    done
    run_test test_p2_multi_file test/test_p2_multi_file/test_allocate.cpp test/test_p2_multi_file/test_release.cpp
else
    for name in "$@"; do
        case "$name" in
            test_p2_multi_file) run_test "$name" test/test_p2_multi_file/test_allocate.cpp test/test_p2_multi_file/test_release.cpp ;;
            *[!a-z0-9_]*) printf 'Invalid test name: %s\n' "$name"; failed=$((failed + 1)) ;;
            *) run_test "$name" "test/$name.cpp" ;;
        esac
    done
fi
printf '\n%s passed, %s failed, %s skipped\n' "$passed" "$failed" "$skipped"
[ "$failed" -eq 0 ]
