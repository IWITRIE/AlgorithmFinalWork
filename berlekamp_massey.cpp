#include <bits/stdc++.h>
using namespace std;

using ll = long long;
const ll MOD = 998244353;
const ll G = 3; // Primitive root for NTT
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

vector<int> rev; // For NTT bit-reversal permutation

/**
 * 准备NTT位逆序置换数组
 */
void ntt_prepare_rev(int limit, int l) {
    rev.resize(limit);
    for (int i = 0; i < limit; ++i) {
        rev[i] = (rev[i >> 1] >> 1) | ((i & 1) << (l - 1));
    }
}

/**
 * NTT 变换
 */
void ntt_transform(vector<ll>& a, bool inv) {
    int limit = a.size();
    for (int i = 0; i < limit; ++i) {
        if (i < rev[i]) swap(a[i], a[rev[i]]);
    }
    for (int mid = 1; mid < limit; mid <<= 1) {
        ll wn_base = qpow(G, (MOD - 1) / (mid << 1));
        if (inv) wn_base = qpow(wn_base, MOD - 2);
        for (int j = 0; j < limit; j += (mid << 1)) {
            ll w = 1;
            for (int k = 0; k < mid; ++k, w = w * wn_base % MOD) {
                ll x = a[j + k];
                ll y = w * a[j + mid + k] % MOD;
                a[j + k] = (x + y) % MOD;
                a[j + mid + k] = (x - y + MOD) % MOD;
            }
        }
    }
    if (inv) {
        ll limit_inv = qpow(limit, MOD - 2);
        for (int i = 0; i < limit; ++i) {
            a[i] = a[i] * limit_inv % MOD;
        }
    }
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
 * 多项式乘法并对递推式取模 (NTT优化)
 */
vector<ll> poly_mul(const vector<ll>& x, const vector<ll>& y, const vector<ll>& recur) {
    int n_poly = q; // Degree of input polynomials is q-1, size is q
    if (n_poly == 0) return {}; // Handle empty recurrence

    int limit = 1, l = 0;
    while (limit < (n_poly + n_poly - 1)) { // Need space for degree up to 2*(n_poly-1)
        limit <<= 1;
        l++;
    }
    if (limit == 0 && (n_poly + n_poly -1) > 0) { // if n_poly = 1, 2*n_poly-1 = 1. limit becomes 1.
         limit = 1; l = 0; // ensure limit is at least 1 if product is not identically zero
    } else if (n_poly + n_poly -1 == 0 && n_poly == 1) { // product of two constants
        limit = 1; l = 0;
    }


    ntt_prepare_rev(limit, l);

    vector<ll> poly_a(limit, 0), poly_b(limit, 0);
    for (int i = 0; i < n_poly; ++i) poly_a[i] = x[i];
    for (int i = 0; i < n_poly; ++i) poly_b[i] = y[i];

    ntt_transform(poly_a, false);
    ntt_transform(poly_b, false);

    for (int i = 0; i < limit; ++i) {
        poly_a[i] = poly_a[i] * poly_b[i] % MOD;
    }

    ntt_transform(poly_a, true);

    // poly_a now contains coefficients of x*y
    // The original code's tmp vector was size 2*q
    // Copy result from poly_a to a tmp-like structure for reduction
    vector<ll> tmp(2 * n_poly, 0); 
    for (int i = 0; i < min(limit, 2 * n_poly); ++i) {
        tmp[i] = poly_a[i];
    }
    
    // Original reduction logic (unchanged)
    // This part assumes recur is c_0, ..., c_{q-1} for P(x)=x^q - sum c_j x^{q-1-j}
    // And seems to perform a specific type of reduction.
    // The loop for (ll i = 0; i < q - 1; ++i) uses tmp[i] which are lower degree coefficients of product.
    // This reduction logic is preserved as per existing code's structure.
    for (ll i = 0; i < n_poly - 1; ++i) { // Original loop was q-1, which is n_poly-1
        if (i >= tmp.size()) continue; // Safety break, should not happen with tmp size 2*n_poly
        for (ll j = 0; j < (ll)recur.size(); ++j) { // recur.size() is q (or n_poly)
            if (i + j + 1 < tmp.size()) {
                 tmp[i + j + 1] =  (tmp[i + j + 1] + tmp[i] * recur[j]) % MOD;
            }
        }
    }
    
    vector<ll> res(n_poly);
    // Original result extraction (unchanged)
    // res[i] = tmp[i + q - 1]
    for (ll i = 0; i < n_poly; ++i) {
        if (i + n_poly - 1 < tmp.size()) {
            res[i] = tmp[i + n_poly - 1];
        } else {
            res[i] = 0; // Should not happen if tmp is large enough
        }
    }
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