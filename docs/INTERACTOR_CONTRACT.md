# Interactive judge contract

This document defines the mandatory judge and measurement contract for every
AlgoScale Interactive problem. It is part of the benchmark specification, not
an implementation suggestion.

## 1. One point, one instance

Every UOJ test point contains exactly one interactive instance.

- A point must not contain a batch of test cases.
- A submission starts a fresh contestant process and a fresh interactor for
  every point.
- The point's metric value $x_i$ is measured only on that instance.
- The point threshold $T_i$ is derived only from that instance's actual scale.

This rule makes every UOJ point an independent scoring unit and avoids hidden
aggregation choices.

## 2. Problem-level metric definition

Every problem must define one primary non-negative metric before any model is
evaluated. The metric must be stated in the enhanced statement and recorded in
the benchmark manifest.

Common metrics are:

- the number of queries sent by the contestant;
- the number of contestant moves;
- a problem-specific resource cost, such as $K+B$.

The metric does not have to be a simple query count, but it must be:

1. deterministic from the protocol transcript;
2. independent of wall-clock time and machine performance;
3. computable exactly by the interactor;
4. monotone in the resources that the problem intends to minimize; and
5. described without exposing hidden data.

The metric name and meaning must not change without changing `dataVersion`.

## 3. What counts as an interaction

Unless a problem-specific metric explicitly says otherwise, apply these rules:

- Count a chargeable command once the interactor has read a complete,
  syntactically valid command of that type.
- Repeated queries count again.
- A valid query whose answer is uninformative or whose operation has no effect
  still counts.
- A final-answer command does not count.
- Whitespace, flushing, and ordinary output delimiters do not count.
- Debug output or an unknown command is a protocol violation, not a free
  interaction.
- An out-of-range argument is a protocol violation. Its partial metric is kept
  only for auditing; the point score is zero.
- If the original task assigns different costs to different commands, the
  interactor must implement that exact cost rather than replacing it with the
  number of output lines.

For a non-additive metric such as $K+B$, the interactor updates the underlying
state after each valid command and reports the final value of the defined cost.

## 4. Thresholds

For point $i$, $T_i$ is fixed before evaluation:

- use the original task's limit for this instance's actual scale;
- for an IOI-style task, use the original full-score line that applies to this
  scale;
- if the original limit is a formula, evaluate it on this point's hidden input;
- if the original limit is a constant, use that constant; and
- document any conversion from the original protocol.

Thresholds must be stored in the benchmark source data. They must never be
inferred from model submissions or adjusted after seeing results.

Under AlgoScale scoring, $x_i \ge T_i$ receives zero even if the contestant
eventually produces the correct answer. The interactor may terminate at the
original hard limit; that result is also zero.

## 5. Correctness and protocol validity

A point is eligible for a positive score only when all of the following hold:

- the final answer or terminal game state is correct;
- every contestant command follows the published grammar;
- every argument is within its published range;
- the contestant obeys all state-dependent restrictions;
- the interaction terminates through the published success path; and
- UOJ reports no time, memory, runtime, or judge failure.

The following are contestant failures and score zero:

- a wrong final answer;
- an illegal move or query;
- extra tokens where the grammar forbids them;
- unexpected EOF or contestant termination before success;
- output after a terminal command when the protocol forbids it;
- exceeding the protocol's hard resource limit; and
- TLE, MLE, RE, idleness, or output-limit failure.

An interactor assertion failure, malformed judge input, unavailable hidden
answer, or internal exception is a judge failure. It must not be recorded as a
model failure and must be rerun after the package is fixed.

## 6. Adaptive interactors

Adaptive interactors are allowed only when the original task is adaptive.

- Every response must remain consistent with the entire transcript and the
  published rules.
- The adversarial policy must be identical for all competitors.
- Any randomness must use a fixed seed stored with the point.
- The seed and hidden state must never be sent to the contestant.
- Replaying the same transcript against the same point and interactor version
  must produce the same responses and metric value.

Non-adaptive tasks must fix their hidden instance before the first contestant
command.

## 7. Required machine-readable result

On every normal termination, the interactor must emit exactly one diagnostic
record through the judge result message, not through the contestant-facing
channel.

Success example:

```text
ALGOSCALE_METRIC_V1 status=ok metric=queries value=42
```

Protocol or answer failure example:

```text
ALGOSCALE_METRIC_V1 status=wa metric=queries value=17 reason=invalid_argument
```

Composite metric example:

```text
ALGOSCALE_METRIC_V1 status=ok metric=K_plus_B value=23 K=7 B=16
```

Rules for this record:

- `ALGOSCALE_METRIC_V1` is the literal version marker.
- `status` is `ok` only after complete protocol-valid success.
- `metric` must match the problem manifest.
- `value` is the exact non-negative integer used as $x_i$.
- Optional audit fields must use ASCII `key=value` tokens without spaces.
- `reason` must use a stable short code and must not reveal hidden data.
- There must be exactly one version marker in the result message.

With a testlib-style interactor, the success path should end conceptually as:

```cpp
quitf(_ok, "ALGOSCALE_METRIC_V1 status=ok metric=queries value=%lld", queries);
```

Contestant-caused failures use `_wa`; judge/package failures use `_fail`.

If UOJ kills the process before the interactor can emit a record, the verdict
is sufficient to assign zero. An Accepted point with a missing, duplicate, or
malformed metric record is an instrumentation error and must not be published.

## 8. UOJ API collection

The current background-submission API does not expose a dedicated interaction
count. It returns the per-point judge details, and the interactor's result
message appears inside that point's `<res>` element.

The collector must:

1. poll the submission until it reaches a terminal state;
2. parse `result.details`;
3. match configured positive test IDs exactly;
4. read the UOJ verdict and the single `ALGOSCALE_METRIC_V1` record;
5. verify the metric name and non-negative value;
6. store the raw submission ID, point ID, verdict, protocol validity, and value;
7. reject results from a different `dataVersion`; and
8. preserve the original judgment artifact for audit.

An Accepted point without a valid record is `instrumentation_error`, not a
zero-scoring model result. A non-Accepted point without a record is a normal
zero-scoring result unless UOJ identifies it as a judge failure.

## 9. Scoring handoff

For a successful point, the collector maps `value` to the leaderboard's
`interactionCount` field. The field name is historical: it represents the
problem's declared interaction metric, including a composite cost when needed.

Repeated submissions for one competitor are merged pointwise by the minimum
eligible value. Direct, Agent, and Combined records are then computed according
to `INTERACTIVE_SCORING.md`.

Reference solutions validate points and thresholds but never establish a
leaderboard record $P_i$.

## 10. Package manifest

Every interactive problem must provide metadata equivalent to:

```json
{
  "schemaVersion": 1,
  "problemId": 80,
  "dataVersion": "sha256:...",
  "metric": {
    "name": "queries",
    "description": "Number of valid query commands",
    "finalAnswerCounts": false
  },
  "points": [
    {"testId": 1, "threshold": 100},
    {"testId": 2, "threshold": 250}
  ]
}
```

`dataVersion` must cover at least the hidden point input, interactor source or
binary, metric definition, threshold table, and protocol-relevant settings.

## 11. Required package review

Before a problem enters the benchmark, test all of these cases:

- the reference solution succeeds on every point;
- a deliberately wrong final answer is rejected;
- every command type accepts its boundary values;
- an out-of-range value is rejected;
- an unknown command and extra output are rejected;
- repeated queries are charged repeatedly;
- EOF before a final answer is rejected;
- the metric at $T_i-1$, $T_i$, and the hard-limit boundary behaves as specified;
- success produces exactly one parseable metric record;
- a judge-side fault produces Judge Failed rather than Wrong Answer;
- replay is deterministic; and
- the interactor is fast enough that it does not dominate contestant time.

The enhanced statement, interactor, manifest, collector fixture, and reference
solution must agree on command grammar, terminal behavior, metric, and limit.

## 12. Publication and audit

For every published submission retain:

- model, evaluation mode, and harness;
- UOJ submission ID;
- problem and data versions;
- point verdict and protocol-valid flag;
- exact metric value;
- the raw UOJ judgment artifact; and
- whether the point result was selected as that competitor's pointwise best.

Do not publish hidden inputs, hidden answers, adaptive state, or secrets through
the metric message or public leaderboard data.
