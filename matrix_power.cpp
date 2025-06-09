#include <bits/stdc++.h>
using namespace std;

using ll = long long;
const ll MOD = 998244353;
ll n, m, q;

/**
 * 快速幂
 */
ll qpow(ll base, ll exp) {
    ll res = 1;
    base %= MOD;
    while (exp) {
        if (exp & 1) res = res * base % MOD;
        base = base * base % MOD;
        exp >>= 1;
    }
    return res;
}

/**
 * Berlekamp-Massey 算法，返回最短递推式系数
 */
vector<ll> berlekamp_massey(const vector<ll>& seq) {
    vector<ll> cur, last;
    ll last_fail = -1, last_delta = 0;
    for (ll i = 0; i < (ll)seq.size(); ++i) {
        ll delta = 0;
        for (ll j = 0; j < (ll)cur.size(); ++j) {
            delta = (delta + seq[i - j - 1] * cur[j]) % MOD;
        }
        if (seq[i] == delta) continue;
        if (last_fail == -1) {
            last_fail = i;
            last_delta = (seq[i] - delta + MOD) % MOD;
            cur.assign(i + 1, 0);
            continue;
        }
        vector<ll> tmp = cur;
        ll coef = (seq[i] - delta + MOD) % MOD * qpow(last_delta, MOD - 2) % MOD;
        if (cur.size() < last.size() + i - last_fail)
            cur.resize(last.size() + i - last_fail, 0);
        cur[i - last_fail - 1] = (cur[i - last_fail - 1] + coef) % MOD;
        for (ll j = 0; j < (ll)last.size(); ++j) {
            cur[i - last_fail + j] = (cur[i - last_fail + j] - coef * last[j] % MOD + MOD) % MOD;
        }
        if ((ll)tmp.size() - i < (ll)last.size() - last_fail) {
            last = tmp;
            last_fail = i;
            last_delta = (seq[i] - delta + MOD) % MOD;
        }
    }
    return cur;
}

/**
 * 矩阵乘法
 */
vector<vector<ll>> matrix_mul(const vector<vector<ll>>& A, const vector<vector<ll>>& B) {
    int size = A.size();
    vector<vector<ll>> C(size, vector<ll>(size, 0));
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            for (int k = 0; k < size; ++k) {
                C[i][j] = (C[i][j] + A[i][k] * B[k][j]) % MOD;
            }
        }
    }
    return C;
}

/**
 * 矩阵快速幂
 */
vector<vector<ll>> matrix_power(vector<vector<ll>> base, ll exp) {
    int size = base.size();
    vector<vector<ll>> res(size, vector<ll>(size, 0));
    for (int i = 0; i < size; ++i) res[i][i] = 1; // 单位矩阵
    
    while (exp) {
        if (exp & 1) res = matrix_mul(res, base);
        base = matrix_mul(base, base);
        exp >>= 1;
    }
    return res;
}

/**
 * 使用矩阵快速幂求第 m 项
 */
ll linear_recurrence(const vector<ll>& f, const vector<ll>& recur) {
    if (m < (ll)f.size()) return f[m];
    if (q == 0) return 0;
    
    // 构造转移矩阵
    // 状态向量: [f[i-q+1], f[i-q+2], ..., f[i]]
    // 转移到: [f[i-q+2], f[i-q+3], ..., f[i+1]]
    vector<vector<ll>> trans(q, vector<ll>(q, 0));
    
    // 前q-1行：简单位移
    for (int i = 0; i < q - 1; ++i) {
        trans[i][i + 1] = 1;
    }
    
    // 最后一行：递推关系 f[i+1] = sum(recur[j] * f[i-j])
    for (int i = 0; i < q; ++i) {
        trans[q - 1][q - 1 - i] = recur[i];
    }
    
    // 计算转移矩阵的(m-q+1)次幂
    vector<vector<ll>> result = matrix_power(trans, m - q + 1);
    
    // 初始状态向量是 [f[0], f[1], ..., f[q-1]]
    // 结果是 result * 初始状态向量的最后一个分量（对应f[m]的位置）
    ll ans = 0;
    for (int i = 0; i < q; ++i) {
        ans = (ans + result[q - 1][i] * f[i]) % MOD;
    }
    return (ans + MOD) % MOD;
}

int main() {
    (void)scanf("%lld %lld", &n, &m);
    vector<ll> f(n);
    for (ll i = 0; i < n; ++i) (void)scanf("%lld", &f[i]);
    vector<ll> recur = berlekamp_massey(f);
    q = recur.size();
    for (ll i = 0; i < q; ++i) {
        printf("%lld%c", recur[i], i == q - 1 ? '\n' : ' ');
    }
    for (ll i = n; i < q * 2; ++i) {
        f.push_back(0);
        for (ll j = 0; j < q; ++j) {
            f[i] = (f[i] + recur[j] * f[i - j - 1]) % MOD;
        }
    }
    printf("%lld\n", linear_recurrence(f, recur));
    return 0;
}
