

typedef struct {
    _Atomic volatile int* restrict ptr;
    _Alignas(16) const char * str;
} MyStruct;

union my_union {
    short int i : 4, : 4;
    _Alignas(const MyStruct) float f;
};

// this is an enum
enum my_enum {
    VAL_1,
    VAL_2,
    VAL_3
};

static inline int do_shit(int n, int m);

static void variadic(int m, ...);

extern int some_func();

int main() {
    register enum my_enum type = VAL_1;
    
    auto long signed int value;
    switch (type) {
        case VAL_1:
        case VAL_2:
            value = 1000l % 5;
            break;
        
        default:
            value = 30l;
            break;
    }
    
    /*
     This code serves no purpose but to have every kind of token in it

     woo multiple line comment
     */
    MyStruct s = {0, L"Hello there, this string literal needs to be longer than 512 characters oh no I don't know what to write here aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};
    int integer = 01000 << 10;
    s.ptr = &integer;

    MyStruct *s_ptr = &s;
    s_ptr->str = "Goodbye";
    *s_ptr = (MyStruct){L"\"Lstrings seem to be int pointers\"", "doot"};

    union my_union soviet_union;
    soviet_union.i = 0x1000 + 033242;
    int super_long_identifier_that_needs_to_be_over_512_characters_long_what_the_hell_am_i_supposed_to_write_here_a_b_c_d_e_f_g_h_i_j_k_l_m_n_o_p_q_r_s_t_u_v_w_x_y_z_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccooooooooooooooooooooooooooooooooooooooooooooosoooooooooooooooooooooooooooodfsoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo_ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss;
    goto my_cool_label;
     
    return _Alignof(long);

my_cool_label:
    super_long_identifier_that_needs_to_be_over_512_characters_long_what_the_hell_am_i_supposed_to_write_here_a_b_c_d_e_f_g_h_i_j_k_l_m_n_o_p_q_r_s_t_u_v_w_x_y_z_aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccooooooooooooooooooooooooooooooooooooooooooooosoooooooooooooooooooooooooooodfsoooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo_ssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss = (int)soviet_union.f;
}

static int do_shit(n, m) int n; int m; { // this syntax exists I guess
    double d = 1e-10 - 0xabecp10;
    int type_size = sizeof(double);
    int size = sizeof d;

    char c = '\n';

    _Bool true_bool = !(c != c) && (c <= size || c >= size || c < size || c > size || c == size || c != size);
    
    enum { LIMIT = 50000 };
    char arr[LIMIT] = {0};
    for (int i = 0, j = LIMIT; i < LIMIT && j > 0; ++i, j--) {
        arr[i] = (char)j | 5;
        arr[j] = (char)i / 4 ^ ~size;
    }

    int i = 0;
    int victim = 0xdeadbeef;
    do {
        if (i == 40) {
            continue;
        } else if (i == 1000) {
            break;
        } else {
            ++i;
        }
        victim += i;
        victim -= LIMIT;
        victim /= (int)c;
        d *= victim;
        type_size %= size;
        true_bool <<= 4;
        true_bool >>= 5;
        arr[i] &= 0x2341;
        arr[i + 1] |= 1;
        arr[0] ^= 10423;
    } while (1);

    while (1) {
        break;
    }

    return (arr[0] == 37 ? size : (arr[1] == 37 ? type_size : -123123 >> 4));
}

_Noreturn static void variadic(int m, ...) {
    unsigned char c = '\n';

    const char* func_name = __func__;
    double _Complex comp_d = 0;
    double _Imaginary im_d = _Generic(1.0, float: 0x1p+2f, double: 0X12.0P-10L);
    _Static_assert(1, "Something is wrong");
    return;
    char d = '\\', e = '\'', f = '"', g = '\0';
    int h = 's';
}

_Thread_local int g_thread;
