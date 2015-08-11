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
