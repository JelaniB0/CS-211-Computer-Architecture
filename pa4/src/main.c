#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

// PROVIDED
uint8_t eq(int8_t a, int8_t b);
void ptr_eq(int8_t *a, int8_t *b, uint8_t *eq);

// SHORT
uint8_t abs_val(int8_t a);
int8_t min(int8_t a, int8_t b);
int8_t max(int8_t a, int8_t b);

// TALL
uint16_t umul(uint8_t a, uint8_t b);
uint8_t udiv(uint16_t a, uint8_t b);
int8_t median(int8_t *a, uint8_t len);
int8_t vec_min(int8_t *a, uint8_t len);
int8_t vec_max(int8_t *a, uint8_t len);
void range(int8_t *a, uint8_t len, int8_t *min, int8_t *max);

// GRANDE
void vec_add(int8_t *a, int8_t *b, uint8_t len);
void vec_scale(uint8_t *a, uint8_t alpha, uint16_t *b, uint8_t len);

/*****************************************************************************
 *                        Main function and tests
 *****************************************************************************/
 int main()
{
    // eq
    assert(0 == eq(1, 2));
    assert(0 == eq(-1, 1));
    assert(1 == eq(1, 1));
    assert(1 == eq(3, 3));
    
    // eq_ptr
    int8_t x = 1, 
           y = 2;
    uint8_t z;
    ptr_eq(&x, &y, &z);
    assert(z == 0);
    
    x = 2;
    ptr_eq(&x, &y, &z);
    assert(z == 1);

    // abs_val
    assert(4 == abs_val(4));
    assert(4 == abs_val(-4));
    // TODO: add your own test cases
    assert(3 == abs_val(-3));

    // min
    assert(2 == min(4, 2));
    // TODO: add your own test cases

    // max
    assert(4 == max(4, 2));
    // TODO: add your own test cases

    // umul
    assert(8 == umul(4, 2));
    // TODO: add your own test cases

    // udiv
    assert(6 == udiv(12, 2));
    assert(2 == udiv(5, 2));

    // TODO: add your own test cases

    // median
    int8_t median_test0[] = {0, 3, 5, 6, 7, 9, 10};
    assert(6 == median(median_test0, sizeof(median_test0) / sizeof(median_test0[0])));
    int8_t median_test1[] = {0, 3, 5, 6, 7, 9, 10, 12, 15};
    assert(7 == median(median_test1, sizeof(median_test1) / sizeof(median_test1[0])));

    // vec_max
    int8_t vec_max_test[] = {0, 3, 7, 2, 4, 6, 10, 2};
    assert(10 == vec_max(vec_max_test, sizeof(vec_max_test) / sizeof(vec_max_test[0])));
    // TODO: add your own test cases

    // vec_min
    int8_t vec_min_test[] = {0, 3, 7, 2, 4, 6, 10, 2};
    assert(0 == vec_min(vec_min_test, sizeof(vec_min_test) / sizeof(vec_min_test[0])));
    // TODO: add your own test cases

    // range
    int8_t range_test[] = {-2, 3, 7, 2, 4, 6, 15, 2},
           min = 0,
           max = 0;
    range(range_test, sizeof(range_test) / sizeof(range_test[0]), &min, &max);
    assert(-2 == min);
    assert(15 == max);
    // TODO: add your own test cases

    // vec_add
    int8_t vec_add_test0_a[] = {0, 3, 5, 6, 7, 9, 10};
    #define LEN_ADD (sizeof(vec_add_test0_a) / sizeof(vec_add_test0_a[0]))
    int8_t vec_add_test0_b[] = {1, 2, 3, 4, 5, 6, 7};
    int8_t vec_add_test0_a_original[LEN_ADD];
    memcpy(vec_add_test0_a_original, vec_add_test0_a, LEN_ADD);
    vec_add(vec_add_test0_a, vec_add_test0_b, LEN_ADD);
    for(size_t i = 0; i < LEN_ADD; i++)
        assert(vec_add_test0_a[i] == vec_add_test0_b[i] + vec_add_test0_a_original[i]);
    // TODO: add your own test cases

    // vec_scale
    uint8_t vec_scale_test0_a[] = {5, 4, 0, 10, 5, 5, 5, 67, 2, 6, 7, 5, 7, 87, 3, 7, 2, 5 ,6 ,7 ,8 ,8 ,10, 0 ,0};
    #define LEN_SCALE (sizeof(vec_scale_test0_a) / sizeof(vec_scale_test0_a[0]))
    uint16_t vec_scale_test0_b[LEN_SCALE] = {0};
    uint8_t alpha = 2;
    vec_scale(vec_scale_test0_a, alpha, vec_scale_test0_b, LEN_SCALE);
    for(size_t i = 0; i < LEN_SCALE; i++){
        printf("%u\n", vec_scale_test0_b[i]);
        assert(vec_scale_test0_b[i] == vec_scale_test0_a[i] * alpha);
    }
    // TODO: add your own test cases

    return EXIT_SUCCESS;
}
