## Problem Description

This is an interactive problem.

Mark is administering an online exam consisting of $n$ true-false questions. However, he has lost all answer keys. He needs a way to retrieve the answers before his client gets infuriated.

Fortunately, he has access to the grading system. Thus, for each query, you can input the answers to all $n$ questions, and the grading system will output how many of them are correct.

Help Mark determine the answer keys.

Note that answer keys are fixed in advance and will not change depending on your queries.

## Input Format

Read n.

## Output Format

```text
s
```

Print a length-n answer sheet made of T and F. Read the number of correct answers. A reply equal to n ends the interaction. The successful sheet also counts. The answer key is fixed. Exit on −1.

Fewer submitted sheets earn a higher score. End every output line with a newline and flush standard output. If the judge closes the stream, terminate immediately.

## Examples

The following table shows messages in chronological order. An excerpt may end before the final answer.

| Judge sends | Program sends |
| --- | --- |
| `2` | |
| | `FF` |
| `1` | |
| | `FT` |
| `2` | |

## Constraints

- 1 ≤ n ≤ 1000.
- Time limit: **10 seconds**. Memory limit: **256 MB**.
