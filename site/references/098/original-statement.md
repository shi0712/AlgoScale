This is an interactive problem.

The jury has a hidden tree with $n$ vertices, numbered from 1 to $n$ . The tree has $n - 1$ edges, the $i$ -th edge connects vertices $s _ { i }$ and $f _ { i }$ . You do not know the edges, but you can ask jury questions about the tree.

Each query you make must contain one vertex $v$ and a binary string $b$ of length $n - 1$ . The jury does the following things for the query:

• The tree is considered as a rooted tree with vertex $v$ being the root.   
• Each edge of the tree becomes directed. More specifically, if $b _ { i } = 1$ , then the $i$ -th edge becomes directed from $s _ { i }$ to $f _ { i }$ . Otherwise, it is directed from $f _ { i }$ to $s _ { i }$ .   
• The answer to the query is the number of edges that are directed towards the root. In other words, the answer to the query is the number of tree edges for which the distance between its start to $\boldsymbol { v }$ is larger than the distance between its end and $v$ . Here the distance between two vertices is the number of edges on the simple undirected path between these vertices.

Your task is to guess all $s _ { i }$ and $f _ { i }$ in no more than 10 000 queries.

# Interaction Protocol

The first line contains the only integer $n$ ( $2 \leq n \leq 5 0 0$ ) — the number of vertices in the tree.

After reading $n$ , you can make no more than 10 000 queries of form $^ { 6 6 } ?$ $v$ $b ^ { \prime }$ , where $v$ is the vertex number ( $1 \leq v \leq n ,$ ), and $b$ is a binary string of length $n - 1$ .

For each query, the jury responds with a line containing a single integer, denoting the number of edges that are directed towards $v$ .

When you are ready to tell all $s _ { i }$ and $f _ { i }$ , print “! $ ! s _ { 1 } f _ { 1 } s _ { 2 } f _ { 2 } \ldots s _ { n - 1 } f _ { n - 1 } ? .$ .

You may assume that the tree itself and all $s _ { i }$ and $f _ { i }$ are defined before the start of interaction (i.e. tha the interactor is not adaptive).

# Example

<table><tr><td>standard input</td><td>standard output</td></tr><tr><td>3</td><td>？101</td></tr><tr><td>2</td><td></td></tr><tr><td>2</td><td>？200</td></tr><tr><td></td><td>？311</td></tr><tr><td>1</td><td>！2321</td></tr></table>