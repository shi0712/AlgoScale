#include <bits/stdc++.h>
using namespace std;

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int r, c;
    if(!(cin >> r >> c)) return 0;
    vector<string> mp(r);
    for(int i=0;i<r;i++) cin >> mp[i];

    const int dr[4] = {-1, 0, 1, 0};
    const int dc[4] = {0, 1, 0, -1};
    // 0: up, 1: right, 2: down, 3: left
    // left: (d+3)%4, right: (d+1)%4

    vector<vector<int>> cellIdx(r, vector<int>(c, -1));
    vector<int> cellR, cellC;
    int ncell = 0;
    for(int i=0;i<r;i++){
        for(int j=0;j<c;j++){
            if(mp[i][j]=='.'){
                cellIdx[i][j] = ncell++;
                cellR.push_back(i);
                cellC.push_back(j);
            }
        }
    }

    int N = ncell * 4;
    vector<int> dstate(N, 0), transL(N), transR(N), transF(N, -1);

    for(int cell=0; cell<ncell; cell++){
        for(int dir=0; dir<4; dir++){
            int s = cell*4 + dir;
            int rr = cellR[cell] + dr[dir];
            int cc = cellC[cell] + dc[dir];
            int cnt = 0;
            while(rr>=0 && rr<r && cc>=0 && cc<c && mp[rr][cc]=='.'){
                cnt++;
                rr += dr[dir];
                cc += dc[dir];
            }
            dstate[s] = cnt;

            transL[s] = cell*4 + (dir+3)%4;
            transR[s] = cell*4 + (dir+1)%4;

            if(cnt > 0){
                int nr = cellR[cell] + dr[dir];
                int nc = cellC[cell] + dc[dir];
                int nid = cellIdx[nr][nc];
                transF[s] = nid*4 + dir;
            }
        }
    }

    // Compute behavioral equivalence classes using Hopcroft-like refinement.
    int SINK = N;
    int M = N + 1;
    vector<int> output(M);
    for(int s=0;s<N;s++) output[s] = dstate[s];
    output[SINK] = -1;

    vector<array<int,3>> tr(M);
    for(int s=0;s<N;s++){
        tr[s][0] = transL[s];
        tr[s][1] = transR[s];
        if(dstate[s] > 0) tr[s][2] = transF[s];
        else tr[s][2] = SINK;
    }
    for(int a=0;a<3;a++) tr[SINK][a] = SINK;

    unordered_map<int,int> outId;
    vector<vector<int>> blocks;
    vector<int> color(M);
    for(int s=0;s<M;s++){
        int o = output[s];
        auto it = outId.find(o);
        if(it == outId.end()){
            int id = (int)blocks.size();
            outId[o] = id;
            blocks.push_back({});
            it = outId.find(o);
        }
        int id = it->second;
        blocks[id].push_back(s);
        color[s] = id;
    }

    int numBlocks = (int)blocks.size();
    vector<int> blockSize(numBlocks);
    for(int i=0;i<numBlocks;i++) blockSize[i] = (int)blocks[i].size();

    deque<int> W;
    vector<char> inW(numBlocks, 0);
    for(int i=0;i<numBlocks;i++){
        W.push_back(i);
        inW[i] = 1;
    }

    vector<vector<int>> pre[3];
    for(int a=0;a<3;a++) pre[a].assign(M, {});
    for(int s=0;s<M;s++){
        for(int a=0;a<3;a++){
            pre[a][tr[s][a]].push_back(s);
        }
    }

    int MAXB = M + 10;
    vector<int> mark(MAXB, -1), head(MAXB, -1), cnt(MAXB, 0);
    vector<int> nxt(M, -1);
    vector<int> stateMark(M, 0);
    int token = 0, stoken = 0;

    while(!W.empty()){
        int A = W.front();
        W.pop_front();
        inW[A] = 0;
        if(blockSize[A] == 0) continue;

        vector<int> Astates = blocks[A];

        for(int a=0;a<3;a++){
            ++token;
            vector<int> affected;

            for(int t : Astates){
                for(int s : pre[a][t]){
                    int B = color[s];
                    if(mark[B] != token){
                        mark[B] = token;
                        cnt[B] = 0;
                        head[B] = -1;
                        affected.push_back(B);
                    }
                    cnt[B]++;
                    nxt[s] = head[B];
                    head[B] = s;
                }
            }

            for(int B : affected){
                int sz = blockSize[B];
                if(sz == 0) continue;
                int ct = cnt[B];
                if(ct == 0 || ct == sz) continue;

                vector<int> X;
                X.reserve(ct);
                ++stoken;
                for(int s = head[B]; s != -1; s = nxt[s]){
                    X.push_back(s);
                    stateMark[s] = stoken;
                }

                int oldSize = sz;
                int newId = (int)blocks.size();
                blocks.emplace_back();
                blockSize.push_back(0);
                inW.push_back(0);

                vector<int> rest;
                rest.reserve(oldSize - ct);
                for(int s : blocks[B]){
                    if(stateMark[s] != stoken) rest.push_back(s);
                }

                if(ct <= oldSize - ct){
                    for(int s : X) color[s] = newId;
                    blocks[newId] = move(X);
                    blocks[B] = move(rest);
                    blockSize[newId] = ct;
                    blockSize[B] = oldSize - ct;
                }else{
                    for(int s : rest) color[s] = newId;
                    blocks[newId] = move(rest);
                    blocks[B] = move(X);
                    blockSize[newId] = oldSize - ct;
                    blockSize[B] = ct;
                }

                if(inW[B]){
                    if(!inW[newId] && blockSize[newId] > 0){
                        W.push_back(newId);
                        inW[newId] = 1;
                    }
                }else{
                    int smaller = (blockSize[B] <= blockSize[newId]) ? B : newId;
                    if(!inW[smaller] && blockSize[smaller] > 0){
                        W.push_back(smaller);
                        inW[smaller] = 1;
                    }
                }
            }
        }
    }

    vector<int> stable(N);
    for(int s=0;s<N;s++) stable[s] = color[s];

    int curD;
    cin >> curD;

    vector<int> B;
    B.reserve(N);
    for(int s=0;s<N;s++){
        if(dstate[s] == curD) B.push_back(s);
    }

    unordered_map<uint64_t, char> syncCache[5];
    const int CHECK_LIMIT = 500000;

    auto encodeArr = [&](int arr[], int m)->uint64_t{
        sort(arr, arr+m);
        uint64_t code = 0;
        for(int i=0;i<m;i++){
            code |= ((uint64_t)(uint32_t)arr[i]) << (16*i);
        }
        return code;
    };

    auto decodeCode = [&](uint64_t code, int m, int arr[]){
        for(int i=0;i<m;i++){
            arr[i] = (int)((code >> (16*i)) & 0xFFFF);
        }
    };

    auto allSameCellArr = [&](int arr[], int m)->bool{
        int cell0 = arr[0] / 4;
        for(int i=1;i<m;i++){
            if(arr[i] / 4 != cell0) return false;
        }
        return true;
    };

    auto checkSync = [&](const vector<int>& G)->int{
        int m = (int)G.size();
        if(m == 0) return 1;
        bool same = true;
        int cell0 = G[0] / 4;
        for(int s : G){
            if(s / 4 != cell0){ same = false; break; }
        }
        if(same) return 1;
        if(m > 4) return 0;

        int arr[4];
        for(int i=0;i<m;i++) arr[i] = G[i];
        uint64_t start = encodeArr(arr, m);

        auto &mp0 = syncCache[m];
        auto it = mp0.find(start);
        if(it != mp0.end()) return it->second;

        unordered_set<uint64_t> vis;
        vector<uint64_t> q;
        vis.reserve(1024);
        q.push_back(start);
        vis.insert(start);

        for(size_t qi=0; qi<q.size(); qi++){
            uint64_t code = q[qi];
            int cur[4];
            decodeCode(code, m, cur);
            int dcur = dstate[cur[0]];

            for(int a=0;a<3;a++){
                if(a == 2 && dcur == 0) continue;

                int ns[4];
                for(int i=0;i<m;i++){
                    int s = cur[i];
                    if(a == 0) ns[i] = transL[s];
                    else if(a == 1) ns[i] = transR[s];
                    else ns[i] = transF[s];
                }

                uint64_t nc = encodeArr(ns, m);
                if(!vis.insert(nc).second) continue;

                int check[4];
                decodeCode(nc, m, check);
                if(allSameCellArr(check, m)){
                    mp0[start] = 1;
                    return 1;
                }

                q.push_back(nc);
                if((int)q.size() > CHECK_LIMIT){
                    return 2;
                }
            }
        }

        mp0[start] = 0;
        return 0;
    };

    const int SYNC_LIMIT = 2500000;

    auto findSyncWord = [&](const vector<int>& G)->pair<int, vector<int>>{
        int m = (int)G.size();
        if(m == 0) return {1, {}};
        bool same = true;
        int cell0 = G[0] / 4;
        for(int s : G){
            if(s / 4 != cell0){ same = false; break; }
        }
        if(same) return {1, {}};
        if(m > 4) return {0, {}};

        int arr[4];
        for(int i=0;i<m;i++) arr[i] = G[i];
        uint64_t start = encodeArr(arr, m);

        unordered_map<uint64_t, pair<uint64_t,int>> parent;
        vector<uint64_t> q;
        parent.reserve(4096);
        q.push_back(start);
        parent[start] = {start, -1};

        for(size_t qi=0; qi<q.size(); qi++){
            uint64_t code = q[qi];
            int cur[4];
            decodeCode(code, m, cur);
            int dcur = dstate[cur[0]];

            for(int a=0;a<3;a++){
                if(a == 2 && dcur == 0) continue;

                int ns[4];
                for(int i=0;i<m;i++){
                    int s = cur[i];
                    if(a == 0) ns[i] = transL[s];
                    else if(a == 1) ns[i] = transR[s];
                    else ns[i] = transF[s];
                }

                uint64_t nc = encodeArr(ns, m);
                if(parent.count(nc)) continue;

                parent[nc] = {code, a};

                int check[4];
                decodeCode(nc, m, check);
                if(allSameCellArr(check, m)){
                    vector<int> rev;
                    uint64_t curCode = nc;
                    while(curCode != start){
                        auto [par, act] = parent[curCode];
                        rev.push_back(act);
                        curCode = par;
                    }
                    reverse(rev.begin(), rev.end());
                    return {1, rev};
                }

                q.push_back(nc);
                if((int)q.size() > SYNC_LIMIT){
                    return {2, {}};
                }
            }
        }

        return {0, {}};
    };

    const int DIST_LIMIT = 2000000;

    auto packPair = [&](int a, int b)->uint64_t{
        return ((uint64_t)(uint32_t)a << 32) | (uint32_t)b;
    };

    auto findDistWord = [&](int s0, int t0)->vector<int>{
        if(stable[s0] == stable[t0]) return {};

        struct Node{
            int s, t;
            int parent;
            signed char act;
        };

        vector<Node> nodes;
        nodes.reserve(1024);
        unordered_map<uint64_t,int> idx;
        idx.reserve(4096);

        nodes.push_back({s0, t0, -1, -1});
        idx[packPair(s0, t0)] = 0;

        for(size_t qi=0; qi<nodes.size(); qi++){
            int s = nodes[qi].s;
            int t = nodes[qi].t;
            int dcur = dstate[s];

            for(int a=0;a<3;a++){
                if(a == 2 && dcur == 0) continue;

                int ns, nt;
                if(a == 0){
                    ns = transL[s];
                    nt = transL[t];
                }else if(a == 1){
                    ns = transR[s];
                    nt = transR[t];
                }else{
                    if(dstate[s] == 0 || dstate[t] == 0) continue;
                    ns = transF[s];
                    nt = transF[t];
                }

                if(ns < 0 || nt < 0) continue;

                int os = dstate[ns];
                int ot = dstate[nt];

                if(os != ot){
                    vector<int> rev;
                    rev.push_back(a);
                    int cur = (int)qi;
                    while(nodes[cur].parent != -1){
                        rev.push_back(nodes[cur].act);
                        cur = nodes[cur].parent;
                    }
                    reverse(rev.begin(), rev.end());
                    return rev;
                }

                if(ns == nt) continue;

                uint64_t key = packPair(ns, nt);
                if(idx.count(key)) continue;

                if((int)nodes.size() > DIST_LIMIT) return {};

                int id = (int)nodes.size();
                idx[key] = id;
                nodes.push_back({ns, nt, (int)qi, (signed char)a});
            }
        }

        return {};
    };

    vector<int> plan;
    int planIdx = 0;
    vector<int> syncPlan;
    int syncIdx = 0;

    const int MAXC = 50000;
    vector<int> markCol(MAXC, -1);
    int tokenCol = 0;
    vector<int> idxCol(MAXC, -1);

    while(true){
        // Build stable-class groups in the current belief.
        ++tokenCol;
        vector<vector<int>> groups;
        groups.reserve(16);

        int firstCell = -1;
        bool allSameCell = true;

        for(int s : B){
            int cell = s / 4;
            if(firstCell == -1) firstCell = cell;
            else if(firstCell != cell) allSameCell = false;

            int col = stable[s];
            if(markCol[col] != tokenCol){
                markCol[col] = tokenCol;
                idxCol[col] = (int)groups.size();
                groups.push_back({});
            }
            groups[idxCol[col]].push_back(s);
        }

        if(allSameCell){
            cout << "yes " << cellR[firstCell] + 1 << ' ' << cellC[firstCell] + 1 << '\n';
            cout.flush();
            return 0;
        }

        bool anySync = false;
        int syncGroupIdx = -1;
        for(int i=0;i<(int)groups.size();i++){
            int st = checkSync(groups[i]);
            if(st != 0){
                anySync = true;
                syncGroupIdx = i;
                break;
            }
        }

        if(!anySync){
            cout << "no\n";
            cout.flush();
            return 0;
        }

        int action = -1;

        if((int)groups.size() == 1){
            plan.clear();
            planIdx = 0;

            if(syncIdx >= (int)syncPlan.size()){
                auto res = findSyncWord(B);
                if(res.first == 0){
                    cout << "no\n";
                    cout.flush();
                    return 0;
                }else if(res.first == 1){
                    syncPlan = move(res.second);
                    syncIdx = 0;
                }else{
                    syncPlan.clear();
                    syncIdx = 0;
                }
            }

            if(syncIdx < (int)syncPlan.size()){
                action = syncPlan[syncIdx++];
            }else{
                action = (curD > 0 ? 2 : 0);
            }

            if(action == 2 && curD == 0){
                action = 0;
                syncPlan.clear();
                syncIdx = 0;
            }
        }else{
            syncPlan.clear();
            syncIdx = 0;

            array<vector<int>, 105> gL, gR;

            for(int s : B){
                int nsL = transL[s];
                int nsR = transR[s];
                gL[dstate[nsL]].push_back(nsL);
                gR[dstate[nsR]].push_back(nsR);
            }

            auto evalGroups = [&](array<vector<int>,105>& g, int &nonEmpty)->tuple<int,int,long long>{
                nonEmpty = 0;
                int worst = 0;
                int maxSize = 0;
                long long totalWorlds = 0;

                for(int o=0;o<105;o++){
                    if(g[o].empty()) continue;
                    nonEmpty++;

                    vector<uint64_t> keys;
                    keys.reserve(g[o].size());
                    for(int s : g[o]){
                        uint64_t key = ((uint64_t)(uint32_t)stable[s] << 32) | (uint32_t)(s / 4);
                        keys.push_back(key);
                    }
                    sort(keys.begin(), keys.end());
                    keys.erase(unique(keys.begin(), keys.end()), keys.end());

                    int worlds = (int)keys.size();

                    int cell0 = (int)(keys[0] & 0xFFFFFFFFULL);
                    bool same = true;
                    for(uint64_t key : keys){
                        int cell = (int)(key & 0xFFFFFFFFULL);
                        if(cell != cell0){ same = false; break; }
                    }

                    bool allMulti = true;
                    for(int i=0;i<(int)keys.size(); ){
                        int j = i;
                        uint32_t col = (uint32_t)(keys[i] >> 32);
                        int firstC = (int)(keys[i] & 0xFFFFFFFFULL);
                        int cellCnt = 1;
                        i++;
                        while(i<(int)keys.size() && (uint32_t)(keys[i] >> 32) == col){
                            int cc = (int)(keys[i] & 0xFFFFFFFFULL);
                            if(cc != firstC) cellCnt++;
                            i++;
                        }
                        if(cellCnt < 2){
                            allMulti = false;
                            break;
                        }
                    }

                    int score;
                    if(same || allMulti) score = 1;
                    else score = worlds;

                    worst = max(worst, score);
                    maxSize = max(maxSize, (int)g[o].size());
                    totalWorlds += worlds;
                }

                return {worst, maxSize, totalWorlds};
            };

            int neL = 0, neR = 0;
            auto [worstL, maxSizeL, totalL] = evalGroups(gL, neL);
            auto [worstR, maxSizeR, totalR] = evalGroups(gR, neR);

            bool infL = (neL > 1);
            bool infR = (neR > 1);

            if(infL || infR){
                bool chooseL;
                if(infL && infR){
                    auto scoreL = make_tuple(1 + worstL, maxSizeL, totalL);
                    auto scoreR = make_tuple(1 + worstR, maxSizeR, totalR);
                    chooseL = (scoreL < scoreR);
                }else{
                    chooseL = infL;
                }

                action = (chooseL ? 0 : 1);
                plan.clear();
                planIdx = 0;

                cout << (action == 0 ? "left\n" : "right\n");
                cout.flush();

                int obs;
                if(!(cin >> obs)) return 0;

                vector<int> newB;
                newB.reserve(B.size());
                for(int s : B){
                    int ns = (action == 0 ? transL[s] : transR[s]);
                    if(dstate[ns] == obs) newB.push_back(ns);
                }
                B.swap(newB);
                curD = obs;
                continue;
            }

            // No informative turn at the current cell.
            if(planIdx >= (int)plan.size()){
                int otherIdx = -1;
                int sIdx = syncGroupIdx;
                if(sIdx < 0 || sIdx >= (int)groups.size()) sIdx = 0;

                for(int i=0;i<(int)groups.size();i++){
                    if(i == sIdx) continue;
                    if(otherIdx == -1 || groups[i].size() > groups[otherIdx].size()){
                        otherIdx = i;
                    }
                }

                if(otherIdx != -1){
                    int s0 = groups[sIdx][0];
                    int t0 = groups[otherIdx][0];
                    vector<int> word = findDistWord(s0, t0);
                    if(!word.empty()){
                        plan = move(word);
                        planIdx = 0;
                    }else{
                        plan.clear();
                        planIdx = 0;
                    }
                }
            }

            if(planIdx < (int)plan.size()){
                action = plan[planIdx++];
            }else{
                action = (curD > 0 ? 2 : 0);
            }

            if(action == 2 && curD == 0){
                action = 0;
                plan.clear();
                planIdx = 0;
            }
        }

        if(action == 0){
            cout << "left\n";
        }else if(action == 1){
            cout << "right\n";
        }else{
            cout << "step\n";
        }
        cout.flush();

        int obs;
        if(!(cin >> obs)) return 0;

        vector<int> newB;
        newB.reserve(B.size());

        for(int s : B){
            int ns;
            if(action == 0) ns = transL[s];
            else if(action == 1) ns = transR[s];
            else ns = transF[s];

            if(ns >= 0 && dstate[ns] == obs){
                newB.push_back(ns);
            }
        }

        B.swap(newB);
        curD = obs;
    }

    return 0;
}
