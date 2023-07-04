

#define A_VAR

#define B_VAR 10

#if B_VAR == 10 && (1 << 3) == -(-8) && (8 >> 3) == +1 && 60 / 2 == 30 && 2 * 100 == 200 && 3 + 2 == 5 && 3 - 2 == 1

char b = 100;

#elif defined(A_VAR)

int a = 10000;

#endif

#if !defined(D_VAR) && ~0u

const char* not_d_var = "not d";

#endif

#if 100 == 1 || 100 % 2 != 0 || 100 < 5 || 125 <= 43 || 7 > 102 || 8 >= 7878

int we_are_in_grave_danger = 2034243;

#elif defined(A_VAR) ? B_VAR == 10 : 0

int the_world_is_in_order = 60;

#endif

// Test nested ifs

#define OUTER_VAR 134

#define INNER_1 1231

#define INNER_2 1234

#if OUTER_VAR == 134

#   if INNER_1 == 10

int inner_if = 234;

#   elif INNER_2 == 1234

int inner_elif = 23;

#   else
int inner_else = 23;
#   endif

#else

#   if INNER_1 == 1231
int else_inner_if = 234;
#   else 
int else_inner_else = 2;
#   endif
#endif

