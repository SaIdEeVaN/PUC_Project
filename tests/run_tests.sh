#!/bin/sh
# End-to-end tests for the matrix calculator.
#
# Drives the compiled binary over stdin and checks what it prints.
# Usage: tests/run_tests.sh [path-to-binary]      (default ./puc)
#
# The size limit is read from src/matrix.h rather than hardcoded, so raising
# MAX_SIZE does not silently invalidate the oversize cases.

BIN="${1:-./puc}"
LIMIT=10          # a hang must fail the run, not wedge it
SEP='@@'          # line separator for block matching; absent from all output

pass=0
fail=0
failed=''

# Track the compiled-in limit instead of duplicating it here.
HEADER="$(dirname "$0")/../src/matrix.h"
MAX_SIZE=$(sed -n 's/^#define MAX_SIZE \([0-9][0-9]*\).*/\1/p' "$HEADER")
if [ -z "$MAX_SIZE" ]; then
    echo "error: could not read MAX_SIZE from $HEADER" >&2
    exit 2
fi
OVERSIZE=$((MAX_SIZE + 1))

# repeat_row <value> <count> -> "v v v ... " (trailing space)
repeat_row() {
    _i=0
    _row=''
    while [ "$_i" -lt "$2" ]; do
        _row="${_row}${1} "
        _i=$((_i + 1))
    done
    printf '%s' "$_row"
}

# filled_matrix <value> <n> -> n rows of n values, each row \n-terminated
filled_matrix() {
    _i=0
    _out=''
    while [ "$_i" -lt "$2" ]; do
        _out="${_out}$(repeat_row "$1" "$2")\n"
        _i=$((_i + 1))
    done
    printf '%s' "$_out"
}

# diagonal_matrix <value> <n> -> n x n with <value> on the diagonal
diagonal_matrix() {
    _i=0
    _out=''
    while [ "$_i" -lt "$2" ]; do
        _j=0
        _row=''
        while [ "$_j" -lt "$2" ]; do
            if [ "$_i" -eq "$_j" ]; then
                _row="${_row}${1} "
            else
                _row="${_row}0 "
            fi
            _j=$((_j + 1))
        done
        _out="${_out}${_row}\n"
        _i=$((_i + 1))
    done
    printf '%s' "$_out"
}

# identity_matrix <n>
identity_matrix() {
    diagonal_matrix 1 "$1"
}

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
# so oversized input overran the stack arrays.
run_test 'multiplication rejects oversize input' \
    "-multi\n$OVERSIZE $OVERSIZE\n$OVERSIZE $OVERSIZE\n-exit\n" \
    "has:Matrix size exceeds the maximum allowed ($MAX_SIZE x $MAX_SIZE)."

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

# ------------------------------------------------------ at the limit

ones=$(filled_matrix 1 "$MAX_SIZE")
twos_row=$(repeat_row 2 "$MAX_SIZE" | sed "s/ *$//")

run_test "addition at the maximum size (${MAX_SIZE}x${MAX_SIZE})" \
    "-add\n$MAX_SIZE $MAX_SIZE\n${ones}${ones}-exit\n" \
    "block:The sum of the matrices is:|${twos_row}"

run_test "determinant of the ${MAX_SIZE}x${MAX_SIZE} identity is 1" \
    "-det\n$MAX_SIZE $MAX_SIZE\n$(identity_matrix "$MAX_SIZE")-exit\n" \
    'has:The determinant of the matrix is: 1'

# 100^10 = 1e20, which overflows a signed 64-bit result. The elimination and
# the result are MatrixDet (__int128 where available), so this is exact. This
# is the one case that a long long fallback build cannot satisfy, by design.
if [ "$MAX_SIZE" -ge 10 ]; then
    run_test 'determinant wider than 64 bits is exact' \
        "-det\n10 10\n$(diagonal_matrix 100 10)-exit\n" \
        'has:The determinant of the matrix is: 100000000000000000000'
fi

run_test "inverse of the ${MAX_SIZE}x${MAX_SIZE} identity is the identity" \
    "-inv\n$MAX_SIZE $MAX_SIZE\n$(identity_matrix "$MAX_SIZE")-exit\n" \
    'has:The inverse of the matrix is:' \
    'not:nan' 'not:singular' 'not:-0.00'

# --------------------------------------------------------- determinant

run_test 'determinant of a 2x2 matrix' \
    '-det\n2 2\n4 7\n2 6\n-exit\n' \
    'has:The determinant of the matrix is: 10'

run_test 'determinant of a 3x3 matrix' \
    '-det\n3 3\n6 1 1\n4 -2 5\n2 8 7\n-exit\n' \
    'has:The determinant of the matrix is: -306'

run_test 'determinant of a singular matrix is zero' \
    '-det\n2 2\n1 2\n2 4\n-exit\n' \
    'has:The determinant of the matrix is: 0'

run_test 'determinant rejects a non-square matrix' \
    '-det\n2 3\n-exit\n' \
    'has:Determinant is only defined for a square matrix.'

# -------------------------------------------------------------- report

printf '\n%d/%d passed\n' "$pass" "$((pass + fail))"
if [ "$fail" -ne 0 ]; then
    printf 'failing:%s\n' "$failed"
    exit 1
fi
exit 0
