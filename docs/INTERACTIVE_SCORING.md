# Interactive leaderboard scoring

Interactor behavior, metric counting, threshold derivation, and the
machine-readable UOJ result contract are defined in
[`INTERACTOR_CONTRACT.md`](INTERACTOR_CONTRACT.md). Every scored point contains
exactly one interactive instance.

AlgoScale publishes three Interactive leaderboard views:

- **Direct** uses records established by Direct submissions only.
- **Agent** uses records established by Agent submissions only.
- **Combined** places Direct and Agent configurations in one table and uses records shared by both modes.

A competitor is identified by `modelKey + mode + harness`. Direct entries have no harness. The same model in Direct mode and Agent mode remains two separate competitors. Different Agent harnesses also remain separate.

## Point score

For test point $i$, let $T_i$ be the original task's limit for that point's actual scale. For an IOI task, use the original full-score line for the corresponding scale. Let $P_i$ be the best eligible interaction count in the current leaderboard view.

An eligible result is correct, protocol-valid, and has $x<T_i$. Reference solutions validate the package and do not compete for $P_i$.

$$
s_i(x)=
\begin{cases}
0, & \text{invalid or }x\ge T_i,\\
100\left(\dfrac{T_i-x}{T_i-P_i}\right)^2, & P_i<x<T_i,\\
100, & x=P_i<T_i.
\end{cases}
$$

If no eligible result exists for a point, $P_i$ is undefined and every competitor receives zero for that point.

If problem $j$ has $m_j$ points, its score is the mean of its point scores:

$$
S_j=\frac{1}{m_j}\sum_{i=1}^{m_j}s_i.
$$

If the suite has $n$ Interactive problems, the leaderboard score is the mean of the problem scores:

$$
S=\frac{1}{n}\sum_{j=1}^{n}S_j.
$$

This gives every problem equal weight regardless of how many judge points it contains.

## Repeated submissions

Repeated submissions for the same competitor are combined pointwise. For every point, use the minimum interaction count among that competitor's correct and protocol-valid results. The source submission ID of every selected point remains in the generated leaderboard for auditing.

Scores are recomputed from the raw point results whenever a new record is established. No historical score is treated as authoritative.

## Ranking

Rows are sorted by unrounded overall score only. Equal scores share a rank, using competition ranking such as `1, 2, 2, 4`. Record count and valid-point count are descriptive and never break ties.

## Source data

The authoritative source is `site/data/interactive-results.json`:

```json
{
  "schemaVersion": 1,
  "updated": "2026-11-20",
  "status": "Preliminary",
  "problems": [
    {
      "problemId": 80,
      "title": "Example",
      "dataVersion": "sha256:...",
      "points": [
        {"testId": 1, "threshold": 100, "metric": "queries"}
      ]
    }
  ],
  "submissions": [
    {
      "submissionId": "uoj-1001",
      "model": "Example Model",
      "modelKey": "example-model",
      "mode": "agent",
      "harness": "Example Harness",
      "problemId": 80,
      "dataVersion": "sha256:...",
      "points": [
        {
          "testId": 1,
          "verdict": "Accepted",
          "protocolValid": true,
          "interactionCount": 42
        }
      ]
    }
  ]
}
```

`dataVersion` must change when test data, the interactor, the interaction metric, or a threshold changes. Submissions from a stale data version are rejected by the builder rather than mixed with current results.

Generate the public boards with:

```powershell
python scripts/build_interactive_leaderboard.py
```

The output is `site/data/interactive-leaderboard.json`. It is derived data and must not be edited by hand.
