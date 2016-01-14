inline int ilog(int32_t x)
{
    if(x <= 0) return 0;

    int i;
    for(i = 0; i < 32; i++) {
        if(!x) break;
        x >>= 1;
    }
    return i;
}

inline int integer_power(int x, int n)
{
    int ret = 1;
    while(n > 0) {
        if(n & 1) {
            ret *= x;
            n ^= 1;
        } else {
            ret *= ret;
            n >>= 1;
        }
    }
    return ret;
}

inline int lookup1_values(int dimension, int entry)
{
    //return (int)powf((float)entry, 1.0f / dimension);
    int ret;
    for(ret = 1; integer_power(ret, dimension) <= entry; ret++);
    return ret - 1;
}

inline int low_neighbor(uint16_t *v, int x)
{
    int ret = x;
    uint16_t max = 0;
    for(int i = 0; i < x; i++) {
        if(max <= v[i] && v[i] < v[x]) {
            ret = i;
            max = v[i];
        }
    }
    return ret;
}

inline int high_neighbor(uint16_t *v, int x)
{
    int ret = x;
    uint16_t min = 65535;
    for(int i = 0; i < x; i++) {
        if(v[x] < v[i] && v[i] <= min) {
            ret = i;
            min = v[i];
        }
    }
    return ret;
}

inline int render_point(int x0, int y0, int x1, int y1, int X)
{
    int off = abs(y1 - y0) * (X - x0) / (x1 - x0);

    if(y1 < y0) {
        return y0 - off;
    } else {
        return y0 + off;
    }
}

inline int max(int a, int b)
{
    return (a > b) ? a : b;
}

inline int min(int a, int b)
{
    return (a < b) ? a : b;
}

inline int16_t clamp16s(int x)
{
    if(x < -32768) return -32768;
    if(x > 32767) return 32767;
    return x;
}
