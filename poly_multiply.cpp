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
 * 多项式乘法并对递推式取模 (朴素乘法)
 */
vector<ll> poly_mul(const vector<ll>& x, const vector<ll>& y, const vector<ll>& recur) {
    vector<ll> tmp(2 * q, 0), res(q, 0);
    for (ll i = 0; i < q; ++i)
        for (ll j = 0; j < q; ++j)
            tmp[i + j] = (tmp[i + j] + x[i] * y[j]) % MOD;
    for (ll i = 0; i < q - 1; ++i)
        for (ll j = 0; j < q; ++j)
            tmp[i + j + 1] = (tmp[i + j + 1] + tmp[i] * recur[j]) % MOD;
    for (ll i = 0; i < q; ++i)
        res[i] = tmp[i + q - 1];
    return res;
}

/**
 * 线性递推求第 m 项
 */
ll linear_recurrence(const vector<ll>& f, const vector<ll>& recur) {
    if (m < q * 2) return f[m];
    vector<ll> base = recur, res = recur;
    ll t = (m - q) / q, rem = (m - q) % q;
    while (t) {
        if (t & 1) res = poly_mul(res, base, recur);
        base = poly_mul(base, base, recur);
        t >>= 1;
    }
    ll ans = 0;
    for (ll i = 0; i < q; ++i) {
        ans = (ans + res[i] * f[rem + q - i - 1]) % MOD;
    }
    return (ans + MOD) % MOD;
}

int main() {
    int ret = scanf("%lld %lld", &n, &m);
    if (ret != 2) return 1;
    vector<ll> f(n);
    for (ll i = 0; i < n; ++i) {
        ret = scanf("%lld", &f[i]);
        if (ret != 1) return 1;
    }
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
