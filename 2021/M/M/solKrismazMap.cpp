// Model solution, but stores DP states in a map, instead of an array
// This trades-off lookup time with not having to represent states where the answer is 0.
//
// Time complexity: O(q^5 + n) with a low constant factor

#include <bits/stdc++.h>


using namespace std;


const int MOD = 1000 * 1000 * 1000 + 7;
const int Q = 101;

int binom[Q][Q];
int revs[Q];


void fixHigh(int &x) { if (x >= MOD) x -= MOD; }
void fixLow(int &x) { if (x < 0) x += MOD; }


int expo(int a, long long n) {
    int ans = 1;

    while (n) {
        if (n & 1LL) {
            ans = ((long long) ans * a) % MOD;
        }

        n >>= 1;
        a = ((long long) a * a) % MOD;
    }

    return ans;
}


int revMod(int a) {
    return expo(a, MOD - 2);
}


void prepare() {
    for (int i = 0; i < Q; i++) {
        binom[i][0] = 1;
    }

    for (int i = 1; i < Q; i++) for (int j = 1; j <= i; j++) {
        binom[i][j] = binom[i-1][j] + binom[i-1][j-1];
        fixHigh(binom[i][j]);
    }

    for (int i = 1; i < Q; i++) {
        revs[i] = revMod(i);
    }
}


vector <int> sumPrefPow(int n, int k) {
    vector <int> sum(k + 1, 0);
    sum[0] = n;

    for (int a = 1; a <= k; a++) {
        sum[a] = expo(n + 1, a + 1) - 1;
        fixLow(sum[a]);

        for (int b = 0; b < a; b++) {
            sum[a] = (sum[a] - (long long) binom[a + 1][b] * sum[b]) % MOD;
            fixLow(sum[a]);
        }

        sum[a] = ((long long) sum[a] * revs[a + 1]) % MOD;
    }

    return sum;
}


vector <int> sumSeqPow(int l, int r, int k) {
    auto res = sumPrefPow(r, k);

    if (l > 1) {
        auto resSub = sumPrefPow(l - 1, k);

        for (int i = 0; i <= k; i++) {
            res[i] -= resSub[i];
            fixLow(res[i]);
        }
    }

    if (l == 0) {
        res[0]++;
        fixHigh(res[0]);
    }

    return res;
}


int sumSeqProd(int l, int r, int alpha, int a, int b) {
    int res = 0, alphaPow = 1;
    auto sums = sumSeqPow(l, r, a + b);

    for (int i = b; i >= 0; i--) {
        int here = ((long long) sums[a + i] * alphaPow) % MOD;
        here = ((long long) here * binom[b][i]) % MOD;

        if (i % 2) {
            res -= here;
            fixLow(res);
        } else {
            res += here;
            fixHigh(res);
        }

        alphaPow = ((long long) alphaPow * alpha) % MOD;
    }

    return res;
}


map <pair<int,int>,int> convolveDP(map <pair<int,int>,int> &dp1, map <pair<int,int>,int> &dp2) {
    map <pair<int,int>,int> res;

    for (auto &e1 : dp1) for (auto &e2 : dp2) {
        pair <int,int> eCombined = {
            e1.first.first + e2.first.first, e1.first.second + e2.first.second
        };

        res[eCombined] = (res[eCombined] + (long long) e1.second * e2.second) % MOD;
    }

    return res;
}


int mapValue(int x, int thresh) {
    if (x == -1) {
        return -1;
    } else if (x < thresh) {
        return 0;
    } else if (x > thresh) {
        return 2;
    } else {
        return 1;
    }
}


struct Testcase {
    int n, m;
    vector <int> arr;

    vector <pair<int,int>> range;
    vector <vector<int>> children;
    vector <int> knownResult;

    Testcase(int n, int m, const vector <int> &arr): n(n), m(m), arr(arr) {}

    int buildTree(int l, int r) {
        int s = r - l + 1, id = range.size();
        range.push_back({l, r});

        if (s <= 2) {
            children.emplace_back();
            knownResult.push_back(*min_element(arr.begin() + l, arr.begin() + r + 1));
        } else {
            int sDiv = s / 3, sRem = s % 3;

            int len1 = sDiv + (sRem > 0);
            int len2 = sDiv + (sRem > 1);

            children.emplace_back();
            knownResult.emplace_back();

            children[id] = {
                buildTree(l, l + len1 - 1),
                buildTree(l + len1, l + len1 + len2 - 1),
                buildTree(l + len1 + len2, r)
            };

            vector <int> knownResults;
            for (int j : children[id]) {
                knownResults.push_back(knownResult[j]);
            }

            sort(knownResults.begin(), knownResults.end());
            knownResult[id] = knownResults[0] == -1 ? -1 : knownResults[1];
        }

        return id;
    }

    vector <map<pair<int,int>,int>> countSolutionsSingleValue(int value, int thresh) {
        vector <map<pair<int,int>,int>> sols(3);

        int v = mapValue(value, thresh);
        if (v == -1) {
            sols[0][{1, 0}] = 1;
            sols[1][{0, 0}] = 1;
            sols[2][{0, 1}] = 1;
        } else {
            sols[v][{0, 0}] = 1;
        }

        return sols;
    }

    vector <map<pair<int,int>,int>> countSolutions(int id, int thresh) {
        if (knownResult[id] != -1) {
            vector <map<pair<int,int>,int>> ans(3);
            ans[mapValue(knownResult[id], thresh)][{0, 0}] = 1;

            return ans;
        }

        if (children[id].empty()) {
            auto [l, r] = range[id];

            if (l == r) {
                return countSolutionsSingleValue(arr[l], thresh);
            } else {
                auto solsL = countSolutionsSingleValue(arr[l], thresh);
                auto solsR = countSolutionsSingleValue(arr[r], thresh);

                vector <map<pair<int,int>,int>> solsCombined(3);
                for (int res1 = 0; res1 < 3; res1++) {
                    for (int res2 = 0; res2 < 3; res2++) {
                        int resCombined = min(res1, res2);
                        for (auto &e : convolveDP(solsL[res1], solsR[res2])) {
                            solsCombined[resCombined][e.first] = (solsCombined[resCombined][e.first] + e.second) % MOD;
                        }
                    }
                }

                return solsCombined;
            }
        } else {
            vector <vector<map<pair<int,int>,int>>> childSols;
            for (int j : children[id]) {
                childSols.push_back(countSolutions(j, thresh));
            }

            vector <map<pair<int,int>,int>> sols(3);
            for (int res1 = 0; res1 <= 2; res1++) {
                for (int res2 = 0; res2 <= 2; res2++) {
                    auto solsTwo = convolveDP(childSols[0][res1], childSols[1][res2]);

                    for (int res3 = 0; res3 <= 2; res3++) {
                        vector <int> results = {res1, res2, res3};
                        sort(results.begin(), results.end());

                        int resCombined = results[1];
                        for (auto &e : convolveDP(solsTwo, childSols[2][res3])) {
                            sols[resCombined][e.first] = (sols[resCombined][e.first] + e.second) % MOD;
                        }
                    }
                }
            }

            return sols;
        }
    }

    int solve() {
        vector <int> unknown;
        map <int,int> valueToCount;

        for (int i = 0; i < n; i++) {
            if (arr[i] == -1) {
                unknown.push_back(i);
            } else {
                valueToCount[arr[i]]++;
            }
        }

        int k = (n - 1) / 2;
        int q = unknown.size();

        vector <int> values;
        for (auto &e : valueToCount) {
            values.push_back(e.first);
        }

        vector <pair<int,int>> ranges;
        for (int v : values) {
            ranges.push_back({v, v});
        }

        for (int i = -1; i < (int) values.size(); i++) {
            int l = i > -1 ? values[i] + 1 : 0;
            int r = i < (int) values.size() - 1 ? values[i + 1] - 1 : m - 1;

            if (l <= r) {
                ranges.push_back({l, r});
            }
        }

        sort(ranges.begin(), ranges.end());

        int root = buildTree(0, n - 1);

        int ans = 0, cntLess = 0, cntGreater = n - q;
        for (auto &[l, r] : ranges) {
            int cntEq = (l == r ? valueToCount[l] : 0);

            cntGreater -= cntEq;

            int maxLess = k - cntLess;
            int maxGreater = (n - k - 1) - cntGreater;

            if (maxLess >= 0 && maxGreater >= 0) {
                auto solutions = countSolutions(root, l)[1];
                for (auto &sol : solutions) {
                    auto [x, y] = sol.first;

                    if (x > maxLess || y > maxGreater) {
                        continue;
                    }

                    int numWays = sumSeqProd(l, r, m - 1, x, y);
                    ans = (ans + (long long) sol.second * numWays) % MOD;
                }
            }

            cntLess += cntEq;
        }

        return ans;
    }
};


int main() {
    ios_base::sync_with_stdio(false);
    prepare();

    int t;
    cin >> t;

    while (t--) {
        int n, m;
        cin >> n >> m;

        vector <int> arr(n);
        for (int &ai : arr) {
            cin >> ai;
        }

        cout << Testcase(n, m, arr).solve() << '\n';
    }

    return 0;
}
