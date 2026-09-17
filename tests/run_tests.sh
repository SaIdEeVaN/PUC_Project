#!/bin/sh
# End-to-end tests for the matrix calculator.
#
# Drives the compiled binary over stdin and checks what it prints.
# Usage: tests/run_tests.sh [path-to-binary]      (default ./puc)
#
# Cases assume MAX_SIZE == 5; the oversize cases feed 6x6 input.

BIN="${1:-./puc}"
LIMIT=10          # a hang must fail the run, not wedge it
SEP='@@'          # line separator for block matching; absent from all output

pass=0
fail=0
failed=''

if [ ! -x "$BIN" ]; then
    echo "error: $BIN is not executable; run 'make' first" >&2
    exit 2
fi

if command -v timeout >/dev/null 2>&1; then
    run_bin() { timeout "$LIMIT" "$BIN"; }
else
    run_bin() { "$BIN"; }
fi

# Strip prompts printed without a trailing newline, which otherwise run
# together with the next line of real output, then squeeze the result to
# one SEP-joined line so a block can be matched as a substring.
normalize() {
    sed -e 's/Enter an option: //g' \
        -e 's/Enter the number of rows and columns for the first matrix: //g' \
        -e 's/Enter the number of rows and columns for the second matrix: //g' \
        -e 's/Enter the number of rows and columns for matrix [0-9]*: //g' \
        -e 's/Enter the number of rows and columns: //g' \
        -e 's/Enter the number of matrices to multiply: //g' \
        -e 's/^[ 	]*//' \
        -e 's/[ 	]*$//' |
    grep -v '^$' |
    tr '\n' '|' |
    sed -e "s/|/$SEP/g"
}

# run_test <name> <stdin> [block:A|B|C] [has:text] [not:text] ...
#   block:  lines that must appear consecutively (joined with '|')
#   has:    substring that must appear somewhere
#   not:    substring that must not appear anywhere
run_test() {
    name=$1
    input=$2
    shift 2

    raw=$(printf '%b' "$input" | run_bin 2>&1)
    status=$?
    flat=$(printf '%s\n' "$raw" | normalize)
    problems=''

    if [ "$status" -eq 124 ]; then
        problems="timed out after ${LIMIT}s (likely an infinite loop)"
    else
        for spec in "$@"; do
            kind=${spec%%:*}
            want=${spec#*:}
            case $kind in
            block)
                want_flat=$(printf '%s' "$want" | sed -e "s/|/$SEP/g")
                case $flat in
                *"$want_flat"*) ;;
                *) problems="$problems
    missing block: $want" ;;
                esac
                ;;
            has)
                case $raw in
                *"$want"*) ;;
                *) problems="$problems
    missing text: $want" ;;
                esac
                ;;
            not)
                case $raw in
                *"$want"*) problems="$problems
    should not contain: $want" ;;
                esac
                ;;
            *)
                problems="$problems
    unknown check: $spec"
                ;;
            esac
        done
    fi

    if [ -n "$problems" ]; then
        fail=$((fail + 1))
        failed="$failed
  - $name"
        printf 'FAIL  %s%s\n' "$name" "$problems"
        printf '    --- actual ---\n'
        printf '%s\n' "$raw" | sed -e 's/^/    /' | head -25
    else
        pass=$((pass + 1))
        printf 'ok    %s\n' "$name"
    fi
}

# ---------------------------------------------------------------- core

run_test 'help lists every command' \
    '--help\n-exit\n' \
    'block:Options:|--help    Display this help message|-add      Perform matrix addition'

run_test 'addition' \
    '-add\n2 2\n1 2\n3 4\n5 6\n7 8\n-exit\n' \
    'block:The sum of the matrices is:|6 8|10 12'

run_test 'subtraction' \
    '-sub\n2 2\n5 6\n7 8\n1 2\n3 4\n-exit\n' \
    'block:The difference of the matrices is:|4 4|4 4'

run_test 'multiplication 2x3 * 3x2' \
    '-multi\n2 3\n3 2\n1 2 3\n4 5 6\n7 8\n9 10\n11 12\n-exit\n' \
    'block:The product of the matrices is:|58 64|139 154'

run_test 'multiplication rejects mismatched dimensions' \
    '-multi\n2 3\n2 2\n-exit\n' \
    'has:Matrix multiplication not possible.'

# Regression: -multi skipped the MAX_SIZE check every other command makes,
# so 6x6 overran the stack arrays.
run_test 'multiplication rejects oversize input' \
    '-multi\n6 6\n6 6\n-exit\n' \
    'has:Matrix size exceeds the maximum allowed (5 x 5).'

run_test 'chained multiplication of three matrices' \
    '-mmulti\n3\n2 2\n1 1\n0 1\n2 2\n1 0\n1 1\n2 2\n1 1\n0 1\n-exit\n' \
    'block:The result of multiple matrix multiplication is:|2 3|1 2'

run_test 'chained multiplication needs at least two matrices' \
    '-mmulti\n1\n-exit\n' \
    'has:At least two matrices are required for multiplication.'

run_test 'transpose' \
    '-trans\n2 3\n1 2 3\n4 5 6\n-exit\n' \
    'block:The transpose of the matrix is:|1 4|2 5|3 6'

run_test 'matrix properties' \
    '-props\n-exit\n' \
    'has:1. Associative: (AB)C = A(BC)'

run_test 'inverse' \
    '-inv\n2 2\n4 7\n2 6\n-exit\n' \
    'block:The inverse of the matrix is:|0.60 -0.70|-0.20 0.40' \
    'not:nan' 'not:inf'

# Regression: [[0,1],[1,0]] is its own inverse, but without partial pivoting
# the leading zero pivot was read as "singular" and elimination divided by it.
run_test 'inverse handles a zero pivot by swapping rows' \
    '-inv\n2 2\n0 1\n1 0\n-exit\n' \
    'block:The inverse of the matrix is:|0.00 1.00|1.00 0.00' \
    'not:nan' 'not:inf' 'not:-0.00' 'not:singular'

# Regression: the singular and non-square branches printed a message and
# then carried on, printing a matrix of nan.
run_test 'inverse rejects a singular matrix without printing a result' \
    '-inv\n2 2\n1 2\n2 4\n-exit\n' \
    'has:Matrix is singular and cannot be inverted.' \
    'not:nan' 'not:The inverse of the matrix is:'

run_test 'inverse rejects a non-square matrix without printing a result' \
    '-inv\n2 3\n-exit\n' \
    'has:Inverse of a matrix can only be found for a square matrix.' \
    'not:nan' 'not:The inverse of the matrix is:'

run_test 'unknown option is reported' \
    '-bogus\n-exit\n' \
    'has:Unknown option: -bogus. Use --help for usage information.'

run_test 'exit command terminates' \
    '-exit\n' \
    'has:Exiting the program.'

# Regression: on EOF, scanf left `option` unchanged and the loop spun forever.
run_test 'end of input terminates instead of looping' \
    '-add\n' \
    'not:Unknown option'

run_test 'non-numeric input is rejected, not consumed as garbage' \
    '-add\nabc\n-exit\n' \
    'has:Invalid input. Expected a number.' \
    'not:The sum of the matrices is:'

run_test 'zero and negative dimensions are rejected' \
    '-add\n0 2\n-exit\n' \
    'has:Matrix dimensions must be at least 1.' \
    'not:The sum of the matrices is:'

# -------------------------------------------------------------- report

printf '\n%d/%d passed\n' "$pass" "$((pass + fail))"
if [ "$fail" -ne 0 ]; then
    printf 'failing:%s\n' "$failed"
    exit 1
fi
exit 0
