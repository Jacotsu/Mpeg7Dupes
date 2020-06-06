#ifndef MACROS
#define MACROS

#define FFMAX(a,b) ((a) > (b) ? (a) : (b))

#define XSTR(A) STR(A)
#define STR(A) #A

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

// i = destination integer
// n = source double
#define double2int(i, d) \
    {double t = ((d) + 6755399441055744.0); i = *((int *)(&t));}


#endif
