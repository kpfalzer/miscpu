.DEF v1 = 0x01
.DEF v2 = v1 + 23
.DEF v3 = v2 + v1 + 0x12

//.ORG 0x0c00
r0 = #0x01
r12 += #0x040
lbl1: r12 += r13
r5 -= r4
r3 += r23
r2 = mem[r6 + #34]
lbl2: r23 = ~r23
pop r12
push r8
//
//

l2: ->lbl1
	cy? -> v3
	!zero? -> l2
l4: cy? +> l2
