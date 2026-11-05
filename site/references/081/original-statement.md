## Problem Description

This is an interactive problem.

You are given a map made of unit squares. Every square is either open or occupied by a wall. Initially, you are placed in an unknown open square and face an unknown cardinal direction.

At every step, you observe the distance to the next wall in the direction you currently face. This distance is the number of open squares strictly between your current square and that wall. Squares outside the map are considered walls.

You may turn left, turn right, or move one square forward. Your goal is to determine your exact current position. If no strategy can always determine the position, report that fact instead.

Open squares are indistinguishable from one another, as are wall squares. You only need to determine the current square, not the direction you face.

## Input Format

Read from standard input.

First read two integers $r$ and $c$, the number of rows and columns in the map.

Then read $r$ strings of length $c$. A dot (`.`) denotes an open square, and a number sign (`#`) denotes a wall.

At the beginning of every interaction round, read one integer $d$: the distance to the nearest wall in the direction you currently face.

## Output Format

Write to standard output.

After reading the observation for a round, print exactly one of the following commands:

- `left` - turn $90$ degrees to the left;
- `right` - turn $90$ degrees to the right;
- `step` - move one square forward;
- `yes i j` - report that your current position is row $i$, column $j$;
- `no` - report that it is impossible to determine your position reliably.

Never use `step` when the square directly ahead is a wall. After printing `yes i j` or `no`, the interaction ends and the program must terminate. Otherwise, flush the output and begin the next round by reading another observation.

## Examples

### Sample Interaction 1

| Judge sends | Program sends |
| --- | --- |
| `3 3` | |
| `##.` | |
| `#..` | |
| `...` | |
| `1` | |
| | `right` |
| `1` | |
| | `step` |
| `0` | |
| | `left` |
| `0` | |
| | `right` |
| `0` | |
| | `right` |
| `1` | |
| | `yes 2 2` |

### Sample Interaction 2

| Judge sends | Program sends |
| --- | --- |
| `3 5` | |
| `##.##` | |
| `###.#` | |
| `.#.##` | |
| `0` | |
| | `left` |
| `0` | |
| | `no` |

### Sample Interaction 3

| Judge sends | Program sends |
| --- | --- |
| `2 1` | |
| `#` | |
| `.` | |
| `0` | |
| | `yes 2 1` |

## Explanation / Notes

Each table is one complete interaction in chronological order. A blank cell means that the other side sends the next message. The observations are not ordinary batch input and cannot be read in advance.

## Constraints

- $1\le r,c\le100$.
- Every map cell is either `.` or `#`.
- At least one map cell is open.
- The initial state is an open square paired with one of the four cardinal directions.
- Every observation satisfies $0\le d\le99$.
- Every reported coordinate satisfies $1\le i\le r$ and $1\le j\le c$.
- At most $100000$ interaction rounds may be used, including the final round containing `yes i j` or `no`.
- The program must never step into a wall.
- Time limit: **5 seconds**.
