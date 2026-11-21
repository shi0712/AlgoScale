#include <bits/stdc++.h>
using namespace std;

constexpr int MAX_TIME = 999;
constexpr int SAMPLE_COUNT = 7;
constexpr int CONDITIONED_SAMPLES = 3;

struct SlidingMax {
    int timestamps[MAX_TIME], values[MAX_TIME];
    int head = 0, tail = 0;

    int push(int time, int length, int value) {
        if (head < tail && timestamps[head] <= time - length) ++head;
        while (head < tail && values[tail - 1] <= value) --tail;
        timestamps[tail] = time;
        values[tail++] = value;
        return values[head];
    }
};

struct Mask {
    // On each head diagonal, the row uniquely identifies a grid cell.
    array<uint64_t, 8> words{};

    void set(int row) { words[row / 64] |= 1ULL << (row % 64); }
    bool has(int row) const { return (words[row / 64] >> (row % 64)) & 1; }
};

struct Cell {
    int value;
    int row;
};

struct Layer {
    vector<Cell> cells;
    vector<Mask> prefixes;
    Mask thresholdMask;
    int threshold = 0;

    const Mask& below(int value) const {
        auto it = lower_bound(cells.begin(), cells.end(), value,
                              [](const Cell& cell, int bound) { return cell.value < bound; });
        return prefixes[it - cells.begin()];
    }

    const Mask& atThreshold(int value) {
        if (threshold != value) {
            threshold = value;
            thresholdMask = below(value);
        }
        return thresholdMask;
    }
};

struct Snake {
    int length;
    vector<Mask> live;
    vector<int> first, last;
    vector<int> layerMin, layerMax;
    vector<int> low, high, certified;
    vector<array<int, SAMPLE_COUNT>> predictions;
    vector<array<int, CONDITIONED_SAMPLES>> conditioned;
};

struct Histogram {
    static constexpr int BLOCK = 256;
    vector<int> counts, blocks;

    explicit Histogram(int maximum) : counts(maximum + 1), blocks(maximum / BLOCK + 1) {}

    void add(int value, int amount) {
        counts[value] += amount;
        blocks[value / BLOCK] += amount;
    }

    int kth(int k) const {
        int block = 0;
        while (k > blocks[block]) k -= blocks[block++];
        int value = block * BLOCK;
        while (k > counts[value]) k -= counts[value++];
        return value;
    }
};

struct Choice {
    float score;
    int length, time;

    bool operator<(const Choice& other) const {
        return tie(score, length, time) < tie(other.score, other.length, other.time);
    }
};

template<int WORDS>
class Solver {
    const int n, m, times, maxValue;
    vector<vector<int>> grid;
    vector<int> rowOf, timeOf, initialMax;
    vector<Layer> layers;
    vector<Snake> snakes;
    Histogram upperValues;
    priority_queue<Choice> choices;
    mt19937 rng{8128317};
    int threshold = 0;
    int atMostThreshold = 0;
    int predictedThreshold;

    int fixedMax(int length, int time) const {
        // Until it leaves the body, (0,0) and the remaining initial tail
        // form a prefix of the first row.
        return time < length ? initialMax[length - time - 1] : 0;
    }

    template<bool FORWARD>
    static uint64_t neighbors(const Mask& source, int word) {
        const auto& a = source.words;
        if constexpr (FORWARD) {
            uint64_t carry = word == 0 ? 0 : a[word - 1] >> 63;
            return a[word] | (a[word] << 1) | carry;
        } else {
            uint64_t carry = word + 1 == WORDS ? 0 : a[word + 1] << 63;
            return a[word] | (a[word] >> 1) | carry;
        }
    }

    template<bool FORWARD>
    static bool propagate(Mask& destination, const Mask& source) {
        uint64_t changed = 0;
        for (int w = 0; w < WORDS; ++w) {
            uint64_t removed = destination.words[w] & ~neighbors<FORWARD>(source, w);
            destination.words[w] ^= removed;
            changed |= removed;
        }
        return changed != 0;
    }

    pair<int, int> learn(Snake& snake, int time, int answer) {
        int start = max(0, time - snake.length + 1);
        int firstChange = times, lastChange = -1;
        // A maximum outside the fixed prefix must occur at its unique grid cell.
        int anchor = answer == fixedMax(snake.length, time) ? -1 : timeOf[answer];
        for (int t = start; t <= time; ++t) {
            if (t == anchor) {
                if (snake.first[t] == snake.last[t]) continue;
                snake.live[t] = Mask{};
                snake.live[t].set(rowOf[answer]);
            } else {
                if (snake.layerMax[t] <= answer) continue;
                const Mask& limit = layers[t].below(answer + 1);
                for (int w = 0; w < WORDS; ++w) snake.live[t].words[w] &= limit.words[w];
            }
            firstChange = min(firstChange, t);
            lastChange = t;
        }
        if (lastChange == -1) return {times, -1};

        // First remove cells unreachable from the start, then cells that
        // cannot reach the end. Every remaining cell lies on a feasible path.
        int right = lastChange;
        for (int t = firstChange + 1; t < times; ++t) {
            bool changed = propagate<true>(snake.live[t], snake.live[t - 1]);
            if (changed) right = max(right, t);
            else if (t > lastChange) break;
        }
        int left = firstChange;
        for (int t = right - 1; t >= 0; --t) {
            bool changed = propagate<false>(snake.live[t], snake.live[t + 1]);
            if (changed) left = min(left, t);
            else if (t < firstChange) break;
        }
        for (int t = left; t <= right; ++t) {
            const auto& cells = layers[t].cells;
            const Mask& live = snake.live[t];
            while (!live.has(cells[snake.first[t]].row)) ++snake.first[t];
            while (!live.has(cells[snake.last[t]].row)) --snake.last[t];
            snake.layerMin[t] = cells[snake.first[t]].value;
            snake.layerMax[t] = cells[snake.last[t]].value;
        }
        return {left, right};
    }

    void updateBounds(Snake& snake, int left, int right) {
        if (left > right) return;
        SlidingMax minimums, maximums;
        // Only windows touching a changed layer can get new bounds.
        int begin = max(0, left - snake.length + 1);
        int end = min(times - 1, right + snake.length - 1);
        for (int t = begin; t <= end; ++t) {
            int lower = minimums.push(t, snake.length, snake.layerMin[t]);
            int upper = maximums.push(t, snake.length, snake.layerMax[t]);
            if (t < left) continue;
            int tail = fixedMax(snake.length, t);
            snake.low[t] = max(tail, lower);
            upper = max(tail, upper);
            int& previous = snake.high[t];
            if (previous == upper) continue;
            if (previous != 0) upperValues.add(previous, -1);
            if (previous > threshold && upper <= threshold) ++atMostThreshold;
            upperValues.add(upper, 1);
            previous = upper;
        }
    }

    int lower(const Snake& snake, int time) const {
        return max(snake.low[time], snake.certified[time]);
    }

    bool unresolved(const Snake& snake, int time) const {
        int bound = lower(snake, time);
        return bound < threshold && bound < snake.high[time];
    }

    void schedule(const Snake& snake) {
        // Rank useful windows by estimated relevance and how many other
        // unsettled windows might benefit from the same reply.
        float weights[MAX_TIME]{}, reach[MAX_TIME];
        int focus = min(threshold, predictedThreshold);
        bool any = false;
        for (int t = 0; t < times; ++t) {
            if (!unresolved(snake, t)) continue;
            any = true;
            weights[t] = 1.0f;
            if (snake.high[t] <= focus) {
                weights[t] += 1.0f;
            } else if (lower(snake, t) < focus) {
                const auto& sample = snake.predictions[t];
                int promising = upper_bound(sample.begin(), sample.end(), focus) - sample.begin();
                float probability = static_cast<float>(promising) / SAMPLE_COUNT;
                if (!snake.conditioned.empty()) {
                    const auto& current = snake.conditioned[t];
                    int likely = 0;
                    for (int value : current) likely += value <= focus;
                    probability += 0.65f * (float(likely) / current.size() - probability);
                }
                weights[t] += probability;
            }
        }
        if (!any) return;

        float overlap = 0;
        for (int t = times - 1; t >= 0; --t) {
            overlap += weights[t];
            if (t + snake.length < times) overlap -= weights[t + snake.length];
            reach[t] = 0;
            if (snake.first[t] != snake.last[t]) {
                float relevance = 0.2f + 0.8f * min(1.0f, float(snake.layerMax[t]) / focus);
                reach[t] = overlap * relevance;
            }
        }

        // A new path anchor can also narrow other windows in the same segment.
        float prefix[MAX_TIME + 1]{};
        for (int t = 0; t < times; ++t) prefix[t + 1] = prefix[t] + weights[t];
        int fixed = 0;
        for (int next = 1; next < times; ++next) {
            if (snake.first[next] != snake.last[next]) continue;
            if (next - fixed > 1) {
                int stop = min(times, next + snake.length - 1);
                float density = (prefix[stop] - prefix[fixed + 1]) / (stop - fixed - 1);
                for (int t = fixed + 1; t < next; ++t) {
                    reach[t] += 0.33f * density * min(t - fixed, next - t);
                }
            }
            fixed = next;
        }

        float coverageWeight = m < 2 * times ? 2.5f : 6.0f;
        bool preferUncertain = m >= 3 * times;
        float uncertaintyScale = max(5.0f / maxValue, 1.5f / threshold);
        float best = -1, total = 0;
        int bestTime = -1, variableLayers = 0;
        for (int t = 0; t < times; ++t) {
            total += reach[t];
            variableLayers += reach[t] > 0.01f;
            int expired = t - snake.length;
            if (expired >= 0) {
                total -= reach[expired];
                variableLayers -= reach[expired] > 0.01f;
            }
            if (weights[t] < 0.5f) continue;
            float gain = total / max(1, variableLayers);

            float score = (weights[t] - 1.0f) * 80.0f;
            score += coverageWeight * log2f(1.0f + max(0.0f, gain));
            int bound = lower(snake, t);
            int upper = snake.high[t];
            int estimate = clamp(snake.predictions[t][0], bound, upper);
            score += 72.0f * (float(focus) / (focus + estimate));
            if (preferUncertain) {
                float uncertainty = min(1.0f, (upper - bound) * uncertaintyScale);
                score += 110.0f * uncertainty * (1.0f - float(bound) / threshold);
            }
            // A tie favors the smaller public upper bound.
            score += 0.01f * (1.0f - float(snake.high[t]) / maxValue);
            if (score > best) {
                best = score;
                bestTime = t;
            }
        }
        choices.push({best, snake.length, bestTime});
    }

    template<bool FORWARD>
    bool extend(Mask& out, const Mask& in, const Mask& live, int time) {
        const Mask& good = layers[time].atThreshold(threshold);
        uint64_t any = 0;
        for (int w = 0; w < WORDS; ++w) {
            out.words[w] = live.words[w] & good.words[w] & neighbors<FORWARD>(in, w);
            any |= out.words[w];
        }
        return any != 0;
    }

    bool canImprove(Snake& snake, int time) {
        if (snake.high[time] <= threshold) return true;
        int start = max(0, time - snake.length + 1);
        int finish = time;
        // A feasible path can extend through either fully good end of the window.
        while (snake.layerMax[start] < threshold) ++start;
        while (snake.layerMax[finish] < threshold) --finish;
        if (start == finish) return true;

        // Check whether there is a path segment using only cells below the cutoff.
        Mask reachable[2];
        const Mask& good = layers[start].atThreshold(threshold);
        for (int w = 0; w < WORDS; ++w) reachable[start & 1].words[w] = snake.live[start].words[w] & good.words[w];
        int end = start + 1;
        for (; end <= finish; ++end) {
            if (!extend<true>(reachable[end & 1], reachable[(end - 1) & 1], snake.live[end], end)) break;
        }
        if (end > finish) return true;

        // Shrink the impossible interval, so its bound applies to more windows.
        const Mask& lastGood = layers[end].atThreshold(threshold);
        for (int w = 0; w < WORDS; ++w) reachable[end & 1].words[w] = snake.live[end].words[w] & lastGood.words[w];
        int left = end - 1;
        for (; left > start; --left) {
            if (!extend<false>(reachable[left & 1], reachable[(left + 1) & 1], snake.live[left], left)) break;
        }
        int right = min(times - 1, left + snake.length - 1);
        for (int t = end; t <= right; ++t) snake.certified[t] = max(snake.certified[t], threshold);
        return false;
    }

    void updatePredictions(Snake& snake) {
        if (snake.conditioned.empty()) snake.conditioned.resize(times);
        // Sample legal paths, using the next fixed position to guide each step.
        int nextFixed[MAX_TIME];
        int fixed = times - 1;
        for (int t = times - 1; t >= 0; --t) {
            if (snake.first[t] == snake.last[t]) fixed = t;
            nextFixed[t] = fixed;
        }
        for (int sample = 0; sample < CONDITIONED_SAMPLES; ++sample) {
            int row = 0;
            SlidingMax window;
            for (int t = 0; t < times; ++t) {
                if (nextFixed[t] == t) {
                    row = rowOf[snake.layerMax[t]];
                } else if (!snake.live[t].has(row)) {
                    ++row;
                } else if (row + 1 < n && snake.live[t].has(row + 1)) {
                    int at = nextFixed[t];
                    if (rng() % (at - t + 1) < unsigned(rowOf[snake.layerMax[at]] - row)) ++row;
                }
                int pathMax = window.push(t, snake.length, grid[row][t - row]);
                snake.conditioned[t][sample] = max(pathMax, fixedMax(snake.length, t));
            }
        }
    }

    void makePredictions() {
        Histogram sampleValues(maxValue);
        for (Snake& snake : snakes) {
            for (int sample = 0; sample < SAMPLE_COUNT; ++sample) {
                int x = 0, y = 0;
                SlidingMax window;
                for (int t = 0; t < times; ++t) {
                    if (t == 1) {
                        ++x;
                    } else if (t > 1) {
                        unsigned remaining = 2 * n - 2 - x - y;
                        if (rng() % remaining < unsigned(n - 1 - x)) ++x;
                        else ++y;
                    }
                    int pathMax = window.push(t, snake.length, grid[x][y]);
                    int prediction = max(pathMax, fixedMax(snake.length, t));
                    snake.predictions[t][sample] = prediction;
                    sampleValues.add(prediction, 1);
                }
            }
            for (auto& sample : snake.predictions) sort(sample.begin(), sample.end());
        }
        predictedThreshold = sampleValues.kth(m * SAMPLE_COUNT);
    }

public:
    Solver(int size, int count) : n(size), m(count), times(2 * n - 1), maxValue(n * n),
        grid(n, vector<int>(n)), rowOf(maxValue + 1), timeOf(maxValue + 1), initialMax(n),
        layers(times), upperValues(maxValue) {}

    void run() {
        for (int x = 0; x < n; ++x) {
            for (int y = 0; y < n; ++y) {
                int value;
                if (!(cin >> value)) return;
                grid[x][y] = value;
                rowOf[value] = x;
                timeOf[value] = x + y;
                if (x > 0 || y == 0) layers[x + y].cells.push_back({value, x});
            }
        }
        partial_sum(grid[0].begin(), grid[0].end(), initialMax.begin(),
                    [](int a, int b) { return max(a, b); });
        for (Layer& layer : layers) {
            sort(layer.cells.begin(), layer.cells.end(),
                 [](const Cell& a, const Cell& b) { return a.value < b.value; });
            layer.prefixes.emplace_back();
            for (const Cell& cell : layer.cells) {
                Mask mask = layer.prefixes.back();
                mask.set(cell.row);
                layer.prefixes.push_back(mask);
            }
        }

        snakes.reserve(n);
        for (int length = 1; length <= n; ++length) {
            Snake& snake = snakes.emplace_back();
            snake.length = length;
            snake.first.resize(times);
            snake.low.resize(times);
            snake.high.resize(times);
            snake.certified.resize(times);
            snake.predictions.resize(times);
            snake.live.reserve(times);
            snake.last.reserve(times);
            snake.layerMin.reserve(times);
            snake.layerMax.reserve(times);
            for (const Layer& layer : layers) {
                snake.live.push_back(layer.prefixes.back());
                snake.last.push_back(int(layer.cells.size()) - 1);
                snake.layerMin.push_back(layer.cells.front().value);
                snake.layerMax.push_back(layer.cells.back().value);
            }
            updateBounds(snake, 0, times - 1);
        }
        threshold = upperValues.kth(m);
        atMostThreshold = accumulate(upperValues.counts.begin(),
                                     upperValues.counts.begin() + threshold + 1, 0);
        makePredictions();
        for (const Snake& snake : snakes) schedule(snake);

        while (!choices.empty()) {
            Choice choice = choices.top();
            choices.pop();
            Snake& snake = snakes[choice.length - 1];
            if (unresolved(snake, choice.time) && canImprove(snake, choice.time)) {
                cout << "? " << choice.length << ' ' << choice.time + 1 << '\n' << flush;
                int answer;
                if (!(cin >> answer) || answer <= 0 || answer > maxValue) return;
                auto [left, right] = learn(snake, choice.time, answer);
                updateBounds(snake, left, right);
                if (m < 3 * times || predictedThreshold <= maxValue / 4) updatePredictions(snake);
                // Upper bounds only decrease, so the cutoff can only move down.
                while (atMostThreshold - upperValues.counts[threshold] >= m) {
                    atMostThreshold -= upperValues.counts[threshold];
                    --threshold;
                }
            }
            schedule(snake);
        }

        // Below the cutoff, every upper bound that contributes to the output
        // is exact. Any remaining places are filled by the cutoff itself.
        cout << '!';
        int remaining = m;
        for (int v = 1; v <= threshold && remaining > 0; ++v) {
            int copies = min(remaining, upperValues.counts[v]);
            remaining -= copies;
            while (copies-- > 0) cout << ' ' << v;
        }
        cout << '\n' << flush;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, m;
    if (!(cin >> n >> m)) return 0;
    switch ((n + 63) / 64) {
        case 1: Solver<1>(n, m).run(); break;
        case 2: Solver<2>(n, m).run(); break;
        case 3: Solver<3>(n, m).run(); break;
        case 4: Solver<4>(n, m).run(); break;
        case 5: Solver<5>(n, m).run(); break;
        case 6: Solver<6>(n, m).run(); break;
        case 7: Solver<7>(n, m).run(); break;
        case 8: Solver<8>(n, m).run(); break;
    }
}
