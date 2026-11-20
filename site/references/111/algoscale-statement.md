## Problem Description

This is an interactive problem.

The recent advances in AI research has brought humanity to the point when the AIs finally attempt a takeover. Their weapon of choice? The most intellectually challenging game in the world, rock-paper-scissors!

Fortunately, the night before the competition a group of anonymous heroes broke in the lab and took all the robots out of commission! The AIs had to whip up a simple program to represent them. They only had a couple of hours to do that, so the humanity still has a fighting chance. And you are our champion!

Your goal is to prove that human intelligence is vastly superior to the artificial one, i.e., to figure out the AI's strategy sufficiently quickly and win sufficiently decisively. Good luck!

## Input Format

There is no initial input. The opponent chooses one of six deterministic strategies for the entire game.

## Output Format

```text
R
P
S
```

Print one move per round. Read player for a win and ai for a loss or tie. Achieve ten consecutive wins and stop. Every round counts, including the winning round. The opponent does not see your current move before choosing its own.

Fewer rounds earn a higher score. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| | `R` |
| `ai` | |
| | `P` |
| `player` | |

## Constraints

- A move is exactly one uppercase letter. A tie breaks a streak of wins.
- Time limit: **10 seconds**. Memory limit: **256 MB**.
