## Problem Description

This is an interactive problem.

A mischievous robot is navigating an infinitely large 2D grid. However, its movement is confined to the first quadrant, meaning its coordinates $( x , y )$ must always satisfy $x > 0$ and $y > 0$ . Initially, the robot is located at grid coordinates $( s _ { x } , s _ { y } )$ . All grid cells are initially white.

The game proceeds in discrete time steps. In each time step, the following sequence occurs:

1. Your Move: You choose integer coordinates $( x _ { m } , y _ { m } )$ such that $1 \leq x _ { m } \leq 1000$ and $1 \leq y _ { m } \leq 1000$ . You then mark this cell black. A cell, once marked black, remains black for the rest of the game. You can mark any cell within the range, including the cell where the robot is currently located.

2. Robot’s Move: The robot, currently at position $( r _ { x } , r _ { y } )$ , observes the grid (including the cell you just marked black). It knows the locations of all currently black cells. It then moves to a new position $( n _ { x } , n _ { y } )$ . The new position $( n _ { x } , n _ { y } )$ must be one of the 8 cells immediately adjacent (horizontally, vertically, or diagonally) to $( r _ { x } , r _ { y } )$ . That is, $| n _ { x } - r _ { x } | \le 1$ and $| n _ { y } - r _ { y } | \le 1$ , and $( n _ { x } , n _ { y } ) \neq ( r _ { x } , r _ { y } )$ . Additionally, the robot must stay within the first quadrant, so $n _ { x } > 0$ and $n _ { y } > 0$ . The interactor (controlling the robot) will choose one valid move for the robot. The robot’s strategy is unknown to you.

3. Outcome Check: After the robot moves to $( n _ { x } , n _ { y } )$ , the system checks if the cell $( n _ { x } , n _ { y } )$ is black.

* If $( n _ { x } , n _ { y } )$ is black, the robot explodes.   
* If $( n _ { x } , n _ { y } )$ is white, the game continues to the next time step, with the robot now at $( n _ { x } , n _ { y } )$ .

Your goal is to make the robot explode.

## Input Format

Read the two initial coordinates of the robot.

## Output Format

```text
x y
```

Mark one cell black. After the robot moves, read its new coordinates, or 0 0 if it exploded. Exit immediately on 0 0. There is no separate final answer. Every turn counts. The robot observes the marked cells before choosing its next legal move.

Fewer turns earn a higher score. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `1 1` | |
| | `2 2` |
| `2 1` | |

## Constraints

- Both initial coordinates lie in [1,20]. You may mark cells whose two coordinates are in [1,1000]. The robot stays in the positive quadrant.
- Time limit: **10 seconds**. Memory limit: **1024 MB**.
