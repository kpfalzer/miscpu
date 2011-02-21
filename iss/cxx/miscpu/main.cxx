/**
 * The MIT License
 * 
 * Copyright (c) 2010  Karl W. Pfalzer
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
**/

#include <stdlib.h>
#include <iostream>
#include "xyzzy/assert.hxx"
#include "xyzzy/array.hxx"
#include "miscpu.hxx"

using xyzzy::PTArray;
using namespace miscpu;
using std::cout;
using std::endl;

static void status(const MiscCpu &cpu) {
    cout << "Info: cpu ran " << cpu.getPerfMon()->getInstructionCnt()
         << " instructions" << endl;
}

int main(int argc, char** argv) {
    if (1 < argc) {
        MiscCpu cpu(argv[1]);   //mem.hex
        cpu.run();
		cpu.dumpRegs(0,7); cpu.dumpRegs(31);
        status(cpu);
    } else {
        MiscCpu cpu;

        const int cMemSz = cpu.getMemDepth();
        const unsigned cSpIx = cpu.getNumRegs() - 1;
        unsigned n = 2; //loop count

        {
            TInt32 instructs[] = {
                /*00*/cpu.instruction(OpCode::eLoadil, cSpIx),        //load sp w/ m[pc+1]
                /*01*/cMemSz,                                         // sp-value
                /*02*/cpu.instructioni(OpCode::eLoadi, 3u, n),        //r[3] = n
                /*03*/cpu.instructioni(OpCode::eLoadi, 0u, 0),        //l1: r[0] = 0;
                /*04*/cpu.instructioni(OpCode::eLoadi, 1u, 2),        //r[1] = 1;
                /*05*/cpu.instructioni(OpCode::eLoadi, 2u, n),        //r[2] = n;
                /*06*/cpu.instructioni(OpCode::eAddi,  1u, 1),        //lo: r[1] += 1;
                /*07*/cpu.instructioni(OpCode::eSubi,  2u, 1),        //r[2] -= 1;
                /*08*/cpu.instructionb(OpCode::eBr,  MiscCpu::eNotZero, -3), //->lo ? !=0
                /*09*/cpu.instructioni(OpCode::eSubi,  3u, 1),        //r[3] -= 1;
                /*10*/cpu.instructionb(OpCode::eBr,  MiscCpu::eNotZero, -8), //->l1 ? !=0
                //write result to this next location, which we jump over
                /*11*/cpu.instructionb(OpCode::eBr,  MiscCpu::eUncond, 1),    //->l2
                /*12*/0, //@12: we will write here
                /*13*/cpu.instructioni(OpCode::eLoadi, 4, 20),        //l2: r[4] = 20
                /*14*/cpu.instruction(OpCode::eStore, 1, 4, -8),      //mem[r[4]-8] = r[1]
                /*15*/cpu.instruction(OpCode::ePush, 1),    //push r[1]
                /*16*/cpu.instructioni(OpCode::eLoadi, 1, 666),   //r[1]=666
                /*17*/cpu.instruction(OpCode::ePop, 1),   //pop r[1]
                /*18*/cpu.instruction(OpCode::eHalt),
                /*19*/-1
            };
            PTArray<TInt32> instrAr(&instructs[0], -1);
            cpu.loadMemory(instrAr);
        }
        cpu.run();
        status(cpu);
    }


    return (EXIT_SUCCESS);
}

