## Problem Description

A bracket sequence has $n$ positions. Each position contains an opening or closing parenthesis and has one of three fixed colors: lime, gray, or blue.

One change flips a parenthesis between `(` and `)`. Colors and positions cannot change. Find the minimum number of flips needed so that both of these statements hold:

1. Removing all lime brackets leaves a balanced bracket sequence.
2. Removing all gray brackets leaves a balanced bracket sequence.

Report impossibility if no choice of orientations satisfies both conditions. A balanced sequence may be empty; otherwise it is formed by concatenating balanced sequences or enclosing a balanced sequence in a matching opening and closing pair. The two brackets of such a pair need not have the same color.

## Input Format

Read from standard input.

Read $n$, a string $s$ of $n$ parentheses, then $n$ integers $c_1,\ldots,c_n$. Colors 0, 1, and 2 mean lime, gray, and blue, respectively.

## Output Format

Write to standard output.

Print the minimum number of changes, or `-1` when impossible.

## Examples

### Example

Input:

```text
3
()(
0 2 1
```

Output:

```text
-1
```

## Explanation / Notes

Removing the lime bracket leaves positions 2 and 3, so position 2 must be an opening bracket. Removing the gray bracket leaves positions 1 and 2, so position 2 must be a closing bracket. These requirements conflict, and the answer is `-1`.

The full sequence itself need not be balanced: the two required color-deleted subsequences must be balanced.

## Constraints

- One test case.
- $1\le n\le500000$.
- $|s|=n$, and every character is `(` or `)`.
- $0\le c_i\le2$ for every position.
- Only bracket orientations may change; order and colors are fixed.
- Empty subsequences are balanced.
- Time limit: 1.5 seconds; memory limit: 256 MB.
