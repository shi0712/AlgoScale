## Problem Description

This is an interactive problem.

While lost on the long walk from the college to the UQ Centre, you have stumbled across the entrance to a secret cave system running deep under the university. The entrance is blocked by a security system consisting of $N$ consecutive doors, each door behind the previous; and N switches, with each switch connected to a different door.

The doors are numbered $0, 1, \dots, N - 1$ in order, with door $0$ being closest to you. The switches are also numbered $0, 1, \dots, N - 1$, though you do not know which switch is connected to which door.

The switches are all located at the entrance to the cave. Each switch can either be in an up or down position. Only one of these positions is correct for each switch. If a switch is in the correct position then the door it is connected to will be open, and if the switch is in the incorrect position then the door it is connected to will be closed. The correct position may be different for different switches, and you do not know which positions are the correct ones.

You would like to understand this security system. To do this, you can set the switches to any combination, and then walk into the cave to see which is the first closed door. Doors are not transparent: once you encounter the first closed door, you cannot see any of the doors behind it.

Your task is to determine the correct position for each switch, and also which door each switch is connected to.

## Input Format

First read the number of doors $N$. The wiring and the correct switch positions are fixed before the interaction.

## Output Format

To test a combination, print:

```text
? s
```

Here `s` is a single string of $N$ bits in switch order. A zero means up and a one means down. Read the number of the first closed door, or `-1` if all doors are open. This `-1` is a valid reply.

To finish, print:

```text
! s d0 d1 ... d(N-1)
```

Here `s` gives the correct switch positions and $d_i$ is the number of the door controlled by switch $i$.

End every command with a newline and flush standard output. Each query counts once. Fewer queries earn a higher score. The final answer is free. Exit immediately after printing it, or if the input stream ends unexpectedly.

## Examples

| Judge sends | Program sends |
| --- | --- |
| `2` |  |
|  | `? 00` |
| `1` |  |
|  | `? 10` |
| `-1` |  |
|  | `! 10 1 0` |

## Constraints

- $1 \le N \le 5000$.
- Time limit: **10 seconds**. Memory limit: **256 MB**.
