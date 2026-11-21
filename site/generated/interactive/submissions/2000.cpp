#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>

using namespace std;

constexpr int max_vertices = 1000;

int read_response() {
    int value;
    if (!(cin >> value) || value == -1) {
        exit(0);
    }
    return value;
}

bool insert_ball(int leaf, int value) {
    cout << "? 1 " << leaf << ' ' << value << endl;
    return read_response() == 1;
}

vector<int> collect() {
    cout << "? 2" << endl;
    int count = read_response();
    vector<int> values(count);
    for (int& value : values) {
        value = read_response();
    }
    return values;
}

struct BatchVertex {
    int parent;
    int tag;
    int marker = -1;
};

class Reconstruction {
    int leaf_count;
    int root;
    int next_leaf = 0;
    int next_internal;
    int largest_value = 0;
    vector<int> parent;

    // One leaf below each terminal internal vertex covers every old internal
    // vertex. The number of representatives also bounds the overlap width.
    vector<int> old_representatives() const {
        vector<bool> needed(next_internal, true);
        for (int u = root + 1; u < next_internal; ++u) {
            needed[parent[u]] = false;
        }
        vector<int> representatives;
        for (int leaf = 0; leaf < next_leaf; ++leaf) {
            int p = parent[leaf];
            if (needed[p]) {
                representatives.push_back(leaf);
                needed[p] = false;
            }
        }
        return representatives;
    }

    static int capacity(int palette, int width) {
        return max(palette / 2, palette - width);
    }

    int choose_batch_size(int width) const {
        const int remaining = leaf_count - next_leaf;
        int best_cost = max_vertices;
        int best_size = 1;
        for (int palette = largest_value + 1; palette <= max(largest_value + 1, 2 * remaining); ++palette) {
            int left = remaining;
            int future_width = width;
            int cost = palette - 1;
            while (left > 0) {
                int batch = min(left, capacity(palette, future_width));
                if (batch == 0) {
                    cost = max_vertices;
                    break;
                }
                ++cost;
                left -= batch;
                future_width += batch;
            }
            if (cost < best_cost) {
                best_cost = cost;
                best_size = min(remaining, capacity(palette, width));
            }
        }
        return best_size;
    }

    struct Codes {
        int base = 0;
        int rounds = 0;
        bool positional = false;
    };

    Codes choose_codes(int left, int max_group, int best_cost) const {
        Codes best;
        constexpr int primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
        for (int base : primes) {
            int digits = 1;
            for (int labels = base; labels < left; labels *= base) {
                ++digits;
            }
            bool positional = max_group == 1;
            int rounds = positional ? digits : (digits - 1) * max_group + 1;
            int cost = max(largest_value, base) + rounds;
            if (cost < best_cost && (positional || rounds <= base)) {
                best = {base, rounds, positional};
                best_cost = cost;
            }
        }
        return best;
    }

    void fill_spine(const vector<int>& spine, int value) const {
        for (size_t i = 0; i < spine.size(); ++i) {
            insert_ball(spine.back(), value);
        }
    }

    void locate_by_codes(const vector<int>& spine, const vector<int>& upper, int fallback, Codes codes) {
        int left = leaf_count - next_leaf;
        int depths = static_cast<int>(upper.size());
        vector<vector<int>> candidates(depths);
        for (int d = 0; d < depths; ++d) {
            if (d != fallback && upper[d] > 0) {
                for (int i = 0; i < left; ++i) {
                    candidates[d].push_back(i);
                }
            }
        }
        largest_value = max(largest_value, codes.base);
        int place = 1;
        for (int round = 0; round < codes.rounds; ++round) {
            fill_spine(spine, codes.base);
            vector<int> tags(left);
            for (int i = 0; i < left; ++i) {
                if (codes.positional) {
                    tags[i] = i / place % codes.base;
                } else {
                    for (int x = i, power = 1; x > 0; x /= codes.base) {
                        tags[i] = (tags[i] + x % codes.base * power) % codes.base;
                        power = power * round % codes.base;
                    }
                }
                insert_ball(next_leaf + i, tags[i]);
            }
            if (codes.positional) {
                place *= codes.base;
            }

            vector<vector<bool>> observed(depths, vector<bool>(codes.base, false));
            vector<int> counts(depths, 0);
            int depth = -1;
            for (int value : collect()) {
                if (value == codes.base) {
                    ++depth;
                } else {
                    observed[depth][value] = true;
                    ++counts[depth];
                }
            }
            bool resolved = true;
            for (int d = 0; d < depths; ++d) {
                if (d == fallback) {
                    continue;
                }
                auto& leaves = candidates[d];
                erase_if(leaves, [&](int i) { return !observed[d][tags[i]]; });
                resolved &= static_cast<int>(leaves.size()) == counts[d];
            }
            if (resolved) {
                break;
            }
        }

        fill(parent.begin() + next_leaf, parent.begin() + leaf_count, spine[fallback]);
        for (int d = 0; d < depths; ++d) {
            for (int i : candidates[d]) {
                parent[next_leaf + i] = spine[d];
            }
        }
        next_leaf = leaf_count;
    }

    void finish_spine(const vector<int>& spine, vector<int> remaining_at_depth) {
        int spine_length = static_cast<int>(spine.size());
        while (next_leaf < leaf_count) {
            int left = leaf_count - next_leaf;
            int unknown = left;
            for (int count : remaining_at_depth) {
                unknown -= max(count, 0);
            }
            vector<int> upper(spine_length - 1);
            for (int d = 0; d + 1 < spine_length; ++d) {
                upper[d] = remaining_at_depth[d] >= 0 ? remaining_at_depth[d] : unknown;
            }
            auto largest_group = max_element(upper.begin(), upper.end());
            int fallback = static_cast<int>(largest_group - upper.begin());
            *largest_group = 0;
            int max_group = *max_element(upper.begin(), upper.end());
            if (max_group == 0) {
                fill(parent.begin() + next_leaf, parent.begin() + leaf_count, spine[fallback]);
                next_leaf = leaf_count;
                return;
            }

            int best_cost = max_vertices;
            int size = 1;
            for (int k = 1; k <= left; ++k) {
                int cost = max(largest_value, k) + (left + k - 1) / k;
                if (cost <= best_cost) {
                    size = k;
                    best_cost = cost;
                }
            }

            // A depth with a single leaf reveals its digits directly. For
            // larger groups use the digits as polynomial coefficients. A
            // different leaf can agree with each group member at no more
            // evaluation points than the degree.
            Codes codes = choose_codes(left, max_group, best_cost);
            if (codes.rounds > 0) {
                locate_by_codes(spine, upper, fallback, codes);
                return;
            }

            largest_value = max(largest_value, size);
            fill_spine(spine, size);
            for (int i = 0; i < size; ++i) {
                insert_ball(next_leaf + i, i);
            }
            int depth = -1;
            for (int value : collect()) {
                if (value == size) {
                    ++depth;
                } else {
                    parent[next_leaf + value] = spine[depth];
                    if (remaining_at_depth[depth] >= 0) {
                        --remaining_at_depth[depth];
                    }
                }
            }
            next_leaf += size;
        }
    }

    // While the root is the only internal vertex, a new path can only attach
    // there. Keep the first three vertices on the first long path uncolored
    // so a star never needs a positive value.
    void start_with_spine() {
        vector<int> spine;
        int occupied = 0;
        do {
            int length = 0;
            int limit = max_vertices - (leaf_count - next_leaf - 1);
            while (occupied < limit) {
                int value = leaf_count > 1 && length >= (next_leaf == 0 ? 3 : 2);
                largest_value = max(largest_value, value);
                if (!insert_ball(next_leaf, value)) {
                    break;
                }
                ++length;
                ++occupied;
            }
            int top = root;
            spine.assign(1, root);
            for (int i = next_leaf == 0 ? 2 : 1; i < length; ++i) {
                parent[next_internal] = top;
                top = next_internal++;
                spine.push_back(top);
            }
            parent[next_leaf] = top;
            spine.push_back(next_leaf++);
        } while (next_leaf < leaf_count && next_internal == root + 1);

        if (next_leaf == leaf_count) {
            collect();
            return;
        }

        // A second success from a new leaf would introduce another internal
        // branch. Until then, every new leaf attaches to the known spine.
        for (int leaf = next_leaf; leaf < leaf_count; ++leaf) {
            insert_ball(leaf, 0);
            if (next_internal < max_vertices && insert_ball(leaf, 0)) {
                collect();
                return;
            }
        }

        vector<int> values = collect();
        int spine_length = static_cast<int>(spine.size());
        int first_colored_depth = 3;
        if (spine_length <= 4 && leaf_count - next_leaf >= 6) {
            // The initial zero prefix hides the counts on a short spine.
            // One low-value sweep often determines the common attachment.
            largest_value = max(largest_value, 1);
            fill_spine(spine, 1);
            for (int leaf = next_leaf; leaf < leaf_count; ++leaf) {
                insert_ball(leaf, 0);
            }
            values = collect();
            first_colored_depth = 0;
        }

        vector<int> counts(spine.size(), -1);
        for (int d = first_colored_depth; d + 1 < spine_length; ++d) {
            counts[d] = 0;
        }
        int depth = first_colored_depth - 1;
        for (int value : values) {
            if (value == 1) {
                ++depth;
            } else if (depth >= first_colored_depth && depth + 1 < spine_length) {
                ++counts[depth];
            }
        }
        finish_spine(spine, move(counts));
    }

    vector<int> mark_overlap(int marker_count, const vector<int>& representatives) const {
        vector<bool> in_old_prefix(next_internal, false);
        vector<bool> in_overlap(next_internal, false);
        vector<int> frontier_color(next_internal, -1);
        vector<int> endpoint(marker_count, -1);
        in_old_prefix[root] = true;
        in_overlap[root] = true;
        frontier_color[root] = 0;
        endpoint[0] = root;
        int colors_used = 1;

        vector<int> chain;
        for (int leaf : representatives) {
            chain.clear();
            int p = leaf;
            while (!in_old_prefix[p]) {
                in_old_prefix[p] = true;
                chain.push_back(p);
                p = parent[p];
            }
            if (!in_overlap[p]) {
                continue;
            }

            int previous_color = frontier_color[p];
            int color = previous_color >= 0 ? previous_color : colors_used;
            if (color == marker_count) {
                // Any extension here would increase the overlap width.
                continue;
            }

            int filled = 0;
            int length = static_cast<int>(chain.size());
            while (filled < length && insert_ball(leaf, filled == 0 ? color : 0)) {
                ++filled;
            }
            if (filled == length) {
                continue;
            }

            // The chain is stored leaf-first. The first new ball stopped
            // immediately below the deepest vertex already filled by the batch.
            for (int i = filled; i < length; ++i) {
                in_overlap[chain[i]] = true;
            }
            int q = chain[filled];
            if (previous_color >= 0) {
                frontier_color[p] = -1;
            } else {
                ++colors_used;
            }
            frontier_color[q] = color;
            endpoint[color] = q;
        }
        return endpoint;
    }

    static vector<BatchVertex> decode_batch(const vector<int>& values, int marker_count) {
        vector<BatchVertex> batch;
        vector<int> ancestors;
        for (int value : values) {
            if (value < marker_count) {
                // Old-only branches are all visited before any batch child.
                // The first ball of this block is at the root of such a branch.
                if (batch.back().marker < 0) {
                    batch.back().marker = value;
                }
                continue;
            }
            // Tags decrease down the tree, and earlier sibling subtrees have
            // strictly smaller tags. The closest previous tag at least this
            // large therefore belongs to this vertex's parent.
            while (!ancestors.empty() && batch[ancestors.back()].tag < value) {
                ancestors.pop_back();
            }
            int p = ancestors.empty() ? -1 : ancestors.back();
            ancestors.push_back(static_cast<int>(batch.size()));
            batch.push_back({p, value});
        }
        return batch;
    }

    void add_batch(int size, const vector<int>& representatives) {
        int marker_count = min(size, static_cast<int>(representatives.size()));
        int highest_tag = marker_count + size - 1;
        largest_value = max(largest_value, highest_tag);
        int occupied = 0;
        for (int i = 0; i < size; ++i) {
            int limit = max_vertices - (leaf_count - i - 1);
            while (occupied < limit && insert_ball(next_leaf + i, highest_tag - i)) {
                ++occupied;
            }
        }

        vector<int> endpoints;
        if (marker_count > 0) {
            endpoints = mark_overlap(marker_count, representatives);
        }
        vector<BatchVertex> batch = decode_batch(collect(), marker_count);
        int vertices = static_cast<int>(batch.size());
        vector<int> mapped(vertices, -1);
        mapped[0] = root;

        // Each maximal common vertex has an old-only branch. Walk from each
        // matched endpoint towards the root to identify the entire intersection.
        vector<bool> marked_below(vertices, false);
        for (int u = vertices - 1; u > 0; --u) {
            int p = batch[u].parent;
            if (!marked_below[u] && batch[u].marker >= 0) {
                int old = endpoints[batch[u].marker];
                for (int v = u; mapped[v] < 0; v = batch[v].parent) {
                    mapped[v] = old;
                    old = parent[old];
                }
            }
            if (marked_below[u] || batch[u].marker >= 0) {
                marked_below[p] = true;
            }
        }

        // The last vertex of each insertion chain is its original leaf.
        vector<bool> seen_tag(size, false);
        for (int u = vertices - 1; u > 0; --u) {
            int i = highest_tag - batch[u].tag;
            if (!seen_tag[i]) {
                seen_tag[i] = true;
                mapped[u] = next_leaf + i;
            }
        }
        for (int u = 1; u < vertices; ++u) {
            if (mapped[u] < 0) {
                mapped[u] = next_internal++;
            }
            parent[mapped[u]] = mapped[batch[u].parent];
        }
        next_leaf += size;
    }

public:
    explicit Reconstruction(int leaves)
        : leaf_count(leaves), root(leaves), next_internal(leaves + 1), parent(max_vertices, -1) {}

    void solve() {
        if (2 <= leaf_count && leaf_count <= 5) {
            add_batch(leaf_count, {});
        } else {
            start_with_spine();
        }
        while (next_leaf < leaf_count) {
            vector<int> representatives = old_representatives();
            int size = choose_batch_size(static_cast<int>(representatives.size()));
            add_batch(size, representatives);
        }

        auto relabel = [this](int u) {
            if (u < root) {
                return u;
            }
            return u == root ? next_internal - 1 : u - 1;
        };
        cout << "! " << next_internal;
        for (int u = 0; u < next_internal; ++u) {
            if (u != root) {
                cout << ' ' << relabel(parent[u]);
            }
        }
        cout << endl;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int leaves = read_response();
    Reconstruction(leaves).solve();
    return 0;
}
