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
 * 高斯消元法求解最短线性递推式
 * 尝试从长度1开始，逐步增加递推式长度，直到找到满足条件的最短递推式
 */
vector<ll> gaussian_elimination_recurrence(const vector<ll>& seq) {
    int seq_size = seq.size();
    
    // 尝试不同的递推式长度
    for (int len = 1; len <= seq_size / 2; ++len) {
        if (seq_size < 2 * len) continue;
        
        // 构造线性方程组 A * x = b
        // 其中 x = [c1, c2, ..., c_len] 是递推式系数
        // s[i] = c1*s[i-1] + c2*s[i-2] + ... + c_len*s[i-len]
        vector<vector<ll>> A(seq_size - len, vector<ll>(len, 0));
        vector<ll> b(seq_size - len);
        
        // 构造方程组
        for (int i = 0; i < seq_size - len; ++i) {
            for (int j = 0; j < len; ++j) {
                A[i][j] = seq[len + i - 1 - j];
            }
            b[i] = seq[len + i];
        }
        
        // 高斯消元求解
        vector<vector<ll>> augmented(seq_size - len, vector<ll>(len + 1));
        for (int i = 0; i < seq_size - len; ++i) {
            for (int j = 0; j < len; ++j) {
                augmented[i][j] = A[i][j];
            }
            augmented[i][len] = b[i];
        }
        
        // 前向消元
        int rank = 0;
        vector<int> pivot_col(seq_size - len, -1);
        
        for (int col = 0; col < len && rank < seq_size - len; ++col) {
            // 寻找主元
            int pivot_row = -1;
            for (int row = rank; row < seq_size - len; ++row) {
                if (augmented[row][col] != 0) {
                    pivot_row = row;
                    break;
                }
            }
            
            if (pivot_row == -1) continue; // 该列全为0
            
            // 交换行
            if (pivot_row != rank) {
                swap(augmented[rank], augmented[pivot_row]);
            }
            
            pivot_col[rank] = col;
            
            // 消元
            ll pivot = augmented[rank][col];
            ll pivot_inv = qpow(pivot, MOD - 2);
            
            for (int j = 0; j <= len; ++j) {
                augmented[rank][j] = augmented[rank][j] * pivot_inv % MOD;
            }
            
            for (int row = 0; row < seq_size - len; ++row) {
                if (row != rank && augmented[row][col] != 0) {
                    ll factor = augmented[row][col];
                    for (int j = 0; j <= len; ++j) {
                        augmented[row][j] = (augmented[row][j] - factor * augmented[rank][j] % MOD + MOD) % MOD;
                    }
                }
            }
            rank++;
        }
        
        // 检查是否有解
        bool has_solution = true;
        for (int row = rank; row < seq_size - len; ++row) {
            if (augmented[row][len] != 0) {
                has_solution = false;
                break;
            }
        }
        
        if (!has_solution) continue;
        
        // 检查解的唯一性（满秩）
        if (rank < len) continue;
        
        // 提取解
        vector<ll> solution(len, 0);
        for (int row = 0; row < rank; ++row) {
            if (pivot_col[row] != -1) {
                solution[pivot_col[row]] = augmented[row][len];
            }
        }
        
        // 验证解的正确性
        bool valid = true;
        for (int i = len; i < seq_size; ++i) {
            ll expected = 0;
            for (int j = 0; j < len; ++j) {
                expected = (expected + solution[j] * seq[i - 1 - j]) % MOD;
            }
            if (expected != seq[i]) {
                valid = false;
                break;
            }
        }
        
        if (valid) {
            return solution;
        }
    }
    
    // 如果没有找到合适的递推式，返回平凡递推式
    return vector<ll>(1, 1);
}

/**
 * 多项式乘法并对递推式取模 (NTT优化)
 */
vector<ll> poly_mul(const vector<ll>& x, const vector<ll>& y, const vector<ll>& recur) {
    int n_poly = q;
    if (n_poly == 0) return {};

    int limit = 1, l = 0;
    while (limit < (n_poly + n_poly - 1)) {
        limit <<= 1;
        l++;
    }
    if (limit == 0 && (n_poly + n_poly -1) > 0) {
         limit = 1; l = 0;
    } else if (n_poly + n_poly -1 == 0 && n_poly == 1) {
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

    vector<ll> tmp(2 * n_poly, 0); 
    for (int i = 0; i < min(limit, 2 * n_poly); ++i) {
        tmp[i] = poly_a[i];
    }
    
    for (ll i = 0; i < n_poly - 1; ++i) {
        if (i >= tmp.size()) continue;
        for (ll j = 0; j < (ll)recur.size(); ++j) {
            if (i + j + 1 < tmp.size()) {
                 tmp[i + j + 1] =  (tmp[i + j + 1] + tmp[i] * recur[j]) % MOD;
            }
        }
    }
    
    vector<ll> res(n_poly);
    for (ll i = 0; i < n_poly; ++i) {
        if (i + n_poly - 1 < tmp.size()) {
            res[i] = tmp[i + n_poly - 1];
        } else {
            res[i] = 0;
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
    vector<ll> recur = gaussian_elimination_recurrence(f);
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
