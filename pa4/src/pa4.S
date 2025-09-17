.text

/*****************************************************************************
 *                         Provided Example Functions
 *****************************************************************************/

/* uint8_t eq(int8_t a, int8_t b) |>
* Checks if two signed 8-bit integers are equal, returning either 1 if they are or 0 otherwise. */
.globl eq
eq:
        beq a0, a1, .equal
        mv a0, x0
        ret
.equal:
        addi a0, x0, 1
        ret

/* void ptr_eq(int8_t *a, int8_t *b, uint8_t *eq) |>
* Checks if the signed 8-bit numbers stored at two addresses are equal, storing a 1 at *eq if they are or 0 otherwise. */
.globl ptr_eq
ptr_eq:
        // save the return address
        addi sp, sp, -16 // allocate stack space; maintain 16B alignment
        sd ra, 8(sp)

        // call the comparison function
        lb a0, 0(a0)
        lb a1, 0(a1)
        call eq
        sb a0, 0(a2)

        // restore the return address
        ld ra, 8(sp)
        addi sp, sp, 16
        ret

/*****************************************************************************
 *                         Tier 1 Functions (Short)
 *****************************************************************************/

 /* uint8_t abs_val(int8_t a) |>
 * Get the absolute value of a signed 8-bit integer, returning a uint8_t. */
.globl abs_val
abs_val:
    sext.b a0, a0
    blt a0, x0, .negate
    ret
.negate:
    neg a0, a0
    ret

/* int8_t min(int8_t a, int8_t b) |>
 * Return the minimum of two signed 8-bit integers. */
.globl min
min:
    blt a0, a1, .minimum
    mv a0, a1
    ret
.minimum:
    ret

/* int8_t max(int8_t a, int8_t b) |>
* Return the maximum of two signed 8-bit integers. */
.globl max
max:
    bge a0, a1, maximum
    mv a0, a1
    ret
maximum:
    ret

/*****************************************************************************
 *                         Tier 2 Functions (Tall)
 *****************************************************************************/

/* uint16_t umul(uint8_t a, uint8_t b) |>
* Multiply two unsigned 8-bit integers into an unsigned 16-bit integer using multiply add. */
.globl umul
umul:
     li t1, 0
     mv t0, a1
umul_loop:
        beq t0, x0, done
        add t1, t1, a0
        addi t0, t0, -1 /*dec by 1*/
        bnez t0, umul_loop /* **Call umul_loop again to add** */
        mv a0, t1
     ret
done:
     mv a0, t1
     ret
        

/* uint8_t udiv(uint16_t a, uint8_t b) |>
* Divide a by b (a is a 16-bit unsigned integer and b is a 8-bit unsigned integer) and
* return the result as an unsigned 8-bit integer using divide subtract.
* This is integer division, so if there is a remainder, it is discarded.
* If b is zero, return the equivalent of UINT8_MAX. */
.globl udiv 
udiv:
        beqz a1, zeroDivide
        li t0, 0
        j udiv_loop
udiv_loop:
        bltu a0, a1, doneDiv
        sub a0, a0, a1
        addi t0, t0, 1 /*Count how many times div occurs so saves the quotient.*/
        j udiv_loop 
doneDiv:
        mv a0, t0
        ret
zeroDivide: /*incase divide by 0*/
        li a0, 255
        ret


/* int8_t median(int8_t *a, uint8_t len) |>
* Obtain the median number in an array of signed 8-bit integers.
* Note that all arrays will have an odd size and be pre-sorted. */
.globl median /*Look into this tomorrow. Segfaulting*/
median:
        srli t0, a1, 1 /*Trying udiv resulted in segfaults, right shifting unsigned by 1 == int div by 2*/
        add t1, a0, t0
        lb a0, 0(t1)
        ret

/* int8_t vec_min(int8_t *a, uint8_t len) |>
* Obtain the minimum number in an array of signed 8-bit integers. If len = 0, return 0. */
.globl vec_min
vec_min:
        beqz a1, lZero /*zero case*/
        lb t0, 0(a0)
        li t1, 0
min_loop:
        bge t1, a1, doneMin
        add t2, a0, t1
        lb t3, 0(t2)
        blt t3, t0, newMin   /*t3 or a[i] < t0. */
i_min:
        addi t1, t1, 1 /*i++*/
        j min_loop
newMin: 
        mv t0, t3
        j i_min
lZero:
        li a0, 0
        ret
doneMin:
        mv a0, t0
        ret



/* int8_t vec_max(int8_t *a, uint8_t len) |>
* Obtain the maximum number in an array of signed 8-bit integers. If len = 0, return 0. */
.globl vec_max
vec_max:
        beqz a1, lZero2 
        lb t0, 0(a0)
        li t1, 0
max_loop:
        bge t1, a1, doneMax
        add t2, a0, t1
        lb t3, 0(t2)
        bge t3, t0, newMax   /*t3 or a[i] > t0. */
i_max:
        addi t1, t1, 1 /*i++*/
        j max_loop
newMax: 
        mv t0, t3
        j i_max
lZero2:
        li a0, 0
        ret
doneMax:
        mv a0, t0
        ret

/* void range(int8_t *a, uint8_t len, int8_t *min, int8_t *max) |>
* Obtain the range of an array of signed 8-bit numbers (using vec_min and vec_max), and store it into min and max. If len = 0, store 0 in both. */
.globl range
range:
        addi sp, sp, -16 /*allocate stack space*/
        sd ra, 8(sp)
        beqz a1, lZero3
        mv a4, a0 /*mvs prevent corruption or change of original inputs, used for vec_max later.*/
        mv a5, a1 
        call vec_min
        sb a0, 0(a2)
        mv a0, a4
        mv a1, a5
        call vec_max
        sb a0, 0(a3)
        ld ra, 8(sp)
        addi sp, sp, 16
        ret
lZero3:
        li a0, 0
        sb a0, 0(a2)
        sb a0, 0(a3)
        ld ra, 8(sp)
        addi sp, sp, 16 /*deallocate stack space*/
        ret

/*****************************************************************************
 *                        Tier 3 Functions (Grande)
 *****************************************************************************/

/* void vec_add(int8_t *a, int8_t *b, uint8_t len) |>
* Add the two vectors a and b (with lengths len) element-wise and put the result into a vector a. */
.globl vec_add
vec_add:
        li t2, 0
vecloop:
        bge t2, a2, doneVec
        lb t0, 0(a0) /*add values at index i together*/
        lb t1, 0(a1)
        add t3, t0, t1
        sb t3, 0(a0) /*save new added value to a0 at index i*/
        j ivec
ivec:
        addi a0, a0, 1 /*a ptr, increment by 1*/
        addi a1, a1, 1 /*b ptr, increment by 1*/
        addi t2, t2, 1
        j vecloop
doneVec:
        ret


/* void vec_scale(uint8_t *a, uint8_t alpha, uint16_t *b, uint8_t len) |>
* Scale every element of a vector a by a factor alpha and put the result into a vector b. */
.globl vec_scale
vec_scale:
        li t2, 0
scaleLoop:
        addi sp, sp, -32
        sd ra, 24(sp) 
        sd t2, 16(sp) /*save register to stack, avoid all temp registers from being clobbered now?*/
        sd t1, 8(sp)
        sd t0, 0(sp)
        mv t1, a1 /*prevents changes in alpha/corruption*/
        bge t2, a3, doneScale
        lbu t0, 0(a0)
        mv a4, a0 /*prevent a0 from being lost/corrupted*/
        mv a0, t0 /*use t3 instead of t0 to prevent potential clobbering from umul*/
        mv a1, t1 /*load alpha onto a1 for umul use*/
        call umul
        sh a0, 0(a2) /*sh for 16 bit*/
        mv t1, a1 /*load t1 onto alpha*/
        mv a0, a4
        j iScale
iScale:
        ld t0, 0(sp) /*restore register*/ /*Does it for every mul operation now since mul is looped.*/
        ld t1, 8(sp)
        ld t2, 16(sp)
        ld ra, 24(sp)
        addi sp, sp, 32 /*free space*/
        addi a0, a0, 1
        addi a2, a2, 2 /*16 bit so +2*/
        addi t2, t2, 1
        j scaleLoop
doneScale:
        ld t0, 0(sp) /*restore register*/
        ld t1, 8(sp)
        ld t2, 16(sp)
        ld ra, 24(sp)
        addi sp, sp, 32 /*free space*/
        ret
