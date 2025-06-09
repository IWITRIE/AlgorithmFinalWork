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
 * 朴素线性递推求第 m 项
 */
ll linear_recurrence(const vector<ll>& f, const vector<ll>& recur) {
    if (m < (ll)f.size()) return f[m];
    
    vector<ll> dp = f;
    for (ll i = f.size(); i <= m; ++i) {
        ll val = 0;
        for (ll j = 0; j < q; ++j) {
            val = (val + recur[j] * dp[i - j - 1]) % MOD;
        }
        dp.push_back(val);
    }
    return dp[m];
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
