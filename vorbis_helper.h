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

inline int lookup1_values(int dimension, int entry)
{
    return (int)powf((float)entry, 1.0f / dimension);
}

inline int low_neighbor(uint16_t *v, int x)
{
    int ret = x;
    uint16_t max = 0;
    for(int i = 0; i < x; i++) {
        if(max < v[i] && v[i] < v[x]) {
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
        if(v[x] < v[i] && v[i] < min) {
            ret = i;
            min = v[i];
        }
    }
    return ret;
}
