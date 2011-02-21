r31 = #0x0100000 //sp

//Uses r0..r1
.DEF MULT16u_test_max = 4 //0x0FFFF
MULT16u_test: r0 = #0 //outer loop
r1 = #0
MULT16u_test_l0: push r0
push r1
push r0	//copy
push r1
+> MULT16u
pop r3	//result: not used
pop r1
pop r0
r1 ==? #MULT16u_test_max
zero? -> MULT16u_test_l1
r1 += #1
-> MULT16u_test_l0
MULT16u_test_l1: r0 ==? #MULT16u_test_max
zero? -> MULT16u_test_l2
r0 += #1
-> MULT16u_test_l0
MULT16u_test_l2: halt



//A quick test
r0 = #1234
r1 = #5678
push r0
push r1
+> MULT16u
pop r2
halt

// Unsigned multiply of 2 16-bit (unsigned) numbers.
// Uses: r0..r5
//
MULT16u: pop r0	//return address
pop r1	//operand1
pop r2	//operand2
r4 = #0		//result = r1 * r2
r3 = #1		//and mask
MULT16u_l0: r5 = r2
r5 &= r3
zero? -> MULT16u_l1
r4 += r1
MULT16u_l1: r1 <<= #1
r3 <<= #1
.DEF MULT16u_max = 0x010000
r3 ==? #MULT16u_max
!zero? ->MULT16u_l0
push r4
push r0
ret
