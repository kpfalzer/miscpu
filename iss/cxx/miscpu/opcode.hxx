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

#if !defined(_miscpu_opcode_hxx_)
#    define  _miscpu_opcode_hxx_

#include "xyzzy/bitvec.hxx"
#include "xyzzy/assert.hxx"

using xyzzy::TBitVec;

namespace miscpu
{
    class OpCode {
    public:
        static const unsigned cOpCodeN = 5;
        typedef TBitVec<cOpCodeN> tAsBits;

        enum EOp {
            eNop = 0,
            //
            //Arithmetic
            eAdd,   // r[j] = r[j] + r[k]
            eAddi,  // r[j] = r[j] + immed
            eSub,   // r[j] = r[j] - r[k]
            eSubi,  // r[j] = r[j] - immed
            //
            //Load/store
            eLoad,  // r[j] = mem[r[k]+immed]
            eLoadr, // r[j] = r[k]
            eLoadi, // r[j] = immed
            eLoadil,// r[j] = [pc+1]   ([pc+1] is next full word)
            eStore, // mem[r[k]+immed] = r[j]
            ePush,  // mem[--sp] = r[j]
            ePop,   // r[j] = mem[sp++]
            //
            //Logical
            eLsl,   // r[j] = r[j] << r[k]  (0 fill LSBs)
            eLsli,  // r[j] = r[j] << immed (0 fill LSBs)
            eLsr,   // r[j] = r[j] >> r[k]  (0 fill MSBs)
            eLsri,  // r[j] = r[j] >> immed (0 fill LSBs)
            eAsr,   // r[j] = r[j] >> r[k]  (MSB fill MSBs)
            eAsri,  // r[j] = r[j] >> immed (MSB fill MSBs)
            eAnd,   // r[j] = r[j] & r[k]
            eOr,    // r[j] = r[j] | r[k]
            eXor,   // r[j] = r[j] ^ r[k]
            eAndi,  // r[j] = r[j] & immed
            eOri,   // r[j] = r[j] | immed
            eXori,  // r[j] = r[j] ^ immed
            eNot,   // r[j] = ~r[j]
            eCmp,   // r[j] - r[k]   //compare (update flags) but not r[j]
            eCmpi,  // r[j] - immed  //compare (update flags) but not r[j]
            //
            //Branch
            eBr,    // pc = pc + immed  (immed is signed offset)
            //NOTE: conditionals encoded in instruction
            //eBrz,   // ... if zero flag set
            //eBrc,   // ... if carry set
            //eBrnz,  // ... if zero flag not set
            //eBrnc,  // ... if carry not set
            //
            //Call (push next pc onto stack)
            eCall,  // mem[--sp] = pc+1; pc = pc + immed
            //NOTE: conditionals encoded in instruction
            //eCallz, // ... if zero flag set
            //eCallc, // ... if carry set
            //eCallnz,// ... if zero flag not set
            //eCallnc,// ... if carry not set
            eRetn,  // pc = mem[sp++]
            //
            //Control
            eHalt,
            //
            //UNUSED
            eNotUsed
        };

        OpCode() {
            m_opCode = eNotUsed;
        }

        OpCode(const tAsBits &r) {
            m_opCode = (EOp)(int)r;
            ASSERT_TRUE(eNotUsed > m_opCode);
        }

        static bool isBranchOrCall(EOp opcode) {
            return (eBr==opcode || eCall==opcode);
        }

        bool isBranchOrCall() const {
            return isBranchOrCall(m_opCode);
        }

        static bool hasImmed(EOp opcode) {
            switch (opcode) {
                case eAddi: case eAsri: case eLoadi: case eLsli: case eLsri:
                case eSubi: case eAndi: case eOri: case eXori: case eCmpi:
                    return true;
                    break;
                default:
                    return false;
            }
        }

        bool hasImmed() const {
            return hasImmed(m_opCode);
        }

        EOp getOpcode() const {
            return m_opCode;
        }

    private:
        EOp m_opCode;
    };
};

#endif  //_miscpu_opcode_hxx_
