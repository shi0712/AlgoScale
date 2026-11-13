The standard implementation constructs spanning trees in both graphs, tests fundamental paths, contracts cyclic biconnected edge blocks, and checks strongly connected components of the resulting dependency graph. It runs in near-linear `O((n+m) log n)` time and linear space.

The original official approach explicitly enumerates auxiliary-graph paths and is quadratic in the graph size on dense cases. The strengthened range is `sum n,sum m <= 10000`.
