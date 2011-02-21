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

#include <fstream>
#include <cstdio>
#include "xyzzy/assert.hxx"
#include "miscpu.hxx"
#include "opcode.hxx"

using xyzzy::TBitVec;

namespace miscpu
{
    MiscCpu::MiscCpu(const char *memFname,
        unsigned numRegsN,
        unsigned memDepthN,
        bool useDfltPerfMon)
        :   cRegsN(numRegsN),
            m_regs(1 << numRegsN),
            m_mem(1 << memDepthN),
            cSpRegIx((1 << numRegsN) - 1) {
        ASSERT_TRUE((1 << OpCode::cOpCodeN) > OpCode::eNotUsed);
        initialize();
        if (useDfltPerfMon) {
            setPerfMon(DefaultPerfMon::create());
        }
        if (0 != memFname) {
            loadMemory(memFname);
        }
    }

    void MiscCpu::initialize() {
        unsigned totalBits = (3 * cRegsN) + OpCode::cOpCodeN;
        ASSERT_TRUE(cInstRegNbits >= totalBits);   //enough for all opcode bits.
        for (int i = 0; i < m_regs.length(); i++) {
            m_regs[i] = 0xDEADBEEF;
        }
        for (int i = 0; i < m_mem.length(); i++) {
            m_mem[i] = OpCode::eNotUsed;
        }
        reset();
    }

    void MiscCpu::reset() {
        m_pc = 0;
        m_ir = ~0;
        m_cy = m_zero = false;
    }

    void MiscCpu::loadMemory(string fname) {
        std::ifstream ifs(fname.c_str());
        ASSERT_TRUE(false == ifs.fail());
        TUint32 val;
        std::string s;
        unsigned i = 0;
        while (true) {
#ifdef OK
            ifs >> s;
            sscanf(s.c_str(),"%ud",&val);
#else
            ifs >> val;
            if (ifs.fail()) break;
#endif

            m_mem[i++] = val;
        }
        ifs.close();
        std::cout << "Info: " << fname << ": initialized " << i
                  << " location(s)" << std::endl;
    }

    void MiscCpu::loadMemory(const PTArray<TInt32> &instructs) {
        for (unsigned i = 0; i < instructs.length(); i++) {
            m_mem[i] = instructs[i];
        }
    }

    void MiscCpu::setPerfMon(TRcPerfMon pmon) {
        m_perfMon = pmon;
    }

    void MiscCpu::fetch() {
        static const unsigned cImmedLsb = cInstRegNbits - 1 -
                                          (OpCode::cOpCodeN + (2 * cRegsN));
        /**
         * instruction fields
         * |--opcode--|--ixJ--|--ixK--|--immed--|
         * |31......27|26...22|21...17|16......0|
         * NOTE: immed only used (as offset) for eLoad/eStore
         *
         * conditional
         * |--opcode--|--cond--|--immed--|
         * |31......27|26....24|23......0|
         *
         * xxxi (arith, load) immediate:  i.e., "m_opcode.hasImmed()==true"
         * |--opcode--|--ixJ--|--immed--|
         * |31......27|26...22|21......0|
         */
        m_ir = m_mem[m_pc++];
        //slice up instruction
        unsigned lb = cInstRegNbits - 1,
                 rb = cInstRegNbits - 1 - OpCode::cOpCodeN + 1;
        OpCode::tAsBits opc(m_ir(lb,rb));
        m_opCode = opc;
        lb = rb - 1;
        if (m_opCode.hasImmed()) {
            rb = lb - cRegsN + 1;
            m_ixJ = m_ir(lb,rb);
            lb = rb - 1;
            m_immed = m_ir(lb,0);
        } else if (m_opCode.isBranchOrCall()) {
            rb = lb - 2;
            m_cond = (ECond)((int)m_ir(lb,rb));
            ASSERT_TRUE(eNotUsed > m_cond);
            lb = rb - 1;
            m_immed = m_ir(lb,0);
        } else {
            m_cond = eNotUsed;
            rb = lb - cRegsN + 1;
            m_ixJ = m_ir(lb,rb);
            lb = rb - 1; rb = lb - cRegsN + 1;
            m_ixK = m_ir(lb,rb);
            m_immed = m_ir(lb=cImmedLsb,0);
        }
        if (0 != m_ir(lb)) { //sign extend
            TInt32 cSext = (~0) << lb;
            m_immed |= cSext;
        }
    }

	void MiscCpu::dumpRegs(unsigned lo, unsigned hi) const {
		std::ostream &os = std::cout;
		TInt32 val;
		for (; lo <= hi; lo++) {
			val = m_regs[lo];
			os << "m_regs[" << lo << "]=" << val << std::endl;
		}
	}

    void MiscCpu::decode() {
        OpCode::EOp opcode = m_opCode.getOpcode();
        switch (opcode) {
            case OpCode::eAdd:
            case OpCode::eSub:
            case OpCode::eCmp:
            case OpCode::eLoad: 
			case OpCode::eLoadr:
            case OpCode::eStore:
            case OpCode::eLsl:
            case OpCode::eLsr:
            case OpCode::eAsr:
            case OpCode::eAnd:
            case OpCode::eOr:
            case OpCode::eXor:
                m_rj = m_regs[m_ixJ];
                m_rk = m_regs[m_ixK];
                break;
            case OpCode::eAndi:
            case OpCode::eOri:
            case OpCode::eXori:
            case OpCode::eAddi:
            case OpCode::eSubi:
            case OpCode::eCmpi:
            case OpCode::eLoadi:
            case OpCode::eLsli:
            case OpCode::eLsri:
            case OpCode::eAsri:
            case OpCode::eNot:
            case OpCode::ePush:
                m_rj = m_regs[m_ixJ];
                break;
            case OpCode::eLoadil:
                //see execute();
				break;
            case OpCode::eNotUsed:
                ASSERT_NEVER;
                break;
            default:
                ;//do nothing
        }
    }
    
    void MiscCpu::execute() {
        OpCode::EOp opcode = m_opCode.getOpcode();
		switch(opcode) {
            case OpCode::eNop:  //fall through
            case OpCode::eHalt:
				break;
            //
            //Arithmetic
            case OpCode::eAdd:   // r[j] = r[j] + r[k]
                m_aluz = m_rj + m_rk;
                setFlagsUpdateRj(m_rk);
				break;
            case OpCode::eAddi:  // r[j] = r[j] + immed
                m_aluz = m_rj + m_immed;
                setFlagsUpdateRj(m_immed);
				break;
            case OpCode::eSub:   // r[j] = r[j] - r[k]
                m_aluz = m_rj - m_rk;
                setFlagsUpdateRj(m_rk);
				break;
            case OpCode::eCmp:   // r[j] - r[k]
                m_aluz = m_rj - m_rk;
                setFlags(m_rk);
				break;
            case OpCode::eSubi:  // r[j] = r[j] - immed
                m_aluz = m_rj - m_immed;
                setFlagsUpdateRj(m_immed);
				break;
            case OpCode::eCmpi:  // r[j] - immed
                m_aluz = m_rj - m_immed;
                setFlags(m_immed);
				break;
            //
            //Logical
            case OpCode::eLsl:   // r[j] = r[j] << r[k]  (0 fill LSBs)
                lsl(m_rk);
				break;
            case OpCode::eLsli:  // r[j] = r[j] << immed (0 fill LSBs)
                lsl(m_immed);
                break;
            case OpCode::eLsr:   // r[j] = r[j] >> r[k]  (0 fill MSBs)
                lsr(m_rk);
                break;
            case OpCode::eLsri:  // r[j] = r[j] >> immed (0 fill LSBs)
                lsl(m_immed);
                break;
            case OpCode::eAsr:   // r[j] = r[j] >> r[k]  (MSB fill MSBs)
                asr(m_rk);
                break;
            case OpCode::eAsri:  // r[j] = r[j] >> immed (MSB fill MSBs)
                asr(m_immed);
                break;
            case OpCode::eAnd:   // r[j] = r[j] & r[k]
                m_aluz = m_rj & m_rk;
                setFlagsUpdateRj();
				break;
            case OpCode::eOr:    // r[j] = r[j] | r[k]
                m_aluz = m_rj | m_rk;
                setFlagsUpdateRj();
				break;
            case OpCode::eXor:   // r[j] = r[j] ^ r[k]
                m_aluz = m_rj ^ m_rk;
                setFlagsUpdateRj();
				break;
            case OpCode::eAndi:   // r[j] = r[j] & immed
                m_aluz = m_rj & m_immed;
                setFlagsUpdateRj();
				break;
            case OpCode::eOri:    // r[j] = r[j] | immed
                m_aluz = m_rj | m_immed;
                setFlagsUpdateRj();
				break;
            case OpCode::eXori:   // r[j] = r[j] ^ immed
                m_aluz = m_rj ^ m_immed;
                setFlagsUpdateRj();
				break;
            case OpCode::eNot:   // r[j] = ~r[j]
                m_aluz = ~m_rj;
                setFlagsUpdateRj();
				break;
            //
            //Load/store (do not affect flags)
            case OpCode::eLoad:  // r[j] = mem[r[k]+immed]
                {   TInt32 addr = m_rk + m_immed;
                    m_regs[m_ixJ] = m_mem[addr];
                }
				break;
            case OpCode::eLoadr: // r[j] = r[k]
                m_regs[m_ixJ] = m_regs[m_ixK];
                break;
            case OpCode::eLoadi: // r[j] = immed
                m_regs[m_ixJ] = m_immed;
                break;
            case OpCode::eLoadil:// r[j] = [pc+1]   ([pc+1] is next full word)
                m_regs[m_ixJ] = m_mem[m_pc++];
                break;
            case OpCode::eStore: // mem[r[k]+immed] = r[j]
                {   TInt32 addr = m_rk + m_immed;
                    m_mem[addr] = m_rj;
                }
				break;
            case OpCode::ePush:
                push(m_rj);
                break;
            case OpCode::ePop:
                m_regs[m_ixJ] = pop();
                break;
            //
            //Branch
            case OpCode::eBr:    // pc = pc + immed  (immed is signed offset)
                if (checkCond()) {
                    m_pc += m_immed;
                }
				break;
            //
            //Call (push next pc onto stack)
            case OpCode::eCall:  // push pc+1 onto stack; pc = pc + immed
                call(checkCond());
                break;
            case OpCode::eRetn:
                m_pc = pop();
                break;
			default:
				ASSERT_NEVER;
		}
    }

    bool MiscCpu::checkCond() const {
        bool cond;
        ASSERT_TRUE(m_opCode.isBranchOrCall());
        switch(m_cond) {
            case eCy:
                cond = m_cy;
                break;
            case eZero:
                cond = m_zero;
                break;
            case eNotCy:
                cond = !m_cy;
                break;
            case eNotZero:
                cond = !m_zero;
                break;
            case eUncond:
                cond = true;
                break;
            default:
                ASSERT_NEVER;
        }
        return cond;
    }

    static bool sgn(TInt32 v) {
        return (0 > v);
    }

    void MiscCpu::push(TInt32 val) {
        TUint32 sp = m_regs[cSpRegIx] - 1;
        m_mem[sp] = val;
        m_regs[cSpRegIx] = sp;
    }

    TInt32 MiscCpu::pop() {
        TUint32 sp = m_regs[cSpRegIx];
        TInt32 rval = m_mem[sp++];
        m_regs[cSpRegIx] = sp;
        return rval;
    }

    void MiscCpu::call(bool cond) {
        if (cond) {
            push(m_pc);
            m_pc += m_immed;
        }
    }

    void MiscCpu::setFlags(TInt32 opb) {
        bool sgnA = sgn(m_rj), sgnB = sgn(opb), sgnZ = sgn(m_aluz);
        m_cy = (sgnA != sgnB) ? false : (sgnZ != sgnA);
        setFlags();
    }

    void MiscCpu::setFlags() {
        m_zero = (0 == m_aluz);
    }

    void MiscCpu::setFlagsUpdateRj(TInt32 opb) {
        setFlags(opb);
        m_regs[m_ixJ] = m_aluz;
    }

    void MiscCpu::setFlagsUpdateRj() {
        setFlags();
        m_regs[m_ixJ] = m_aluz;
    }

    void MiscCpu::setCy(unsigned pos) {
        TBitVec<32> bits(m_rj);
        m_cy = (0 != bits(pos));
    }

    /*
     * NOTE: Shift operations follow same as ARM Cortex M0.
     */
    void MiscCpu::lsl(TUint32 amt) {
        if (0 < amt) {
            if (32 >= amt) {
                setCy(32 - amt);
                m_aluz = m_rj << amt; 
            } else {
                m_aluz = 0;
                m_cy = false;
            }
        }
        setFlagsUpdateRj();
    }

    void MiscCpu::lsr(TUint32 amt, bool doUpdate) {
        if (0 < amt) {
            if (32 >= amt) {
                setCy(amt - 1);
                m_aluz = m_rj >> amt;
            } else {
                m_aluz = 0;
                m_cy = false;
            }
        }
        if (doUpdate) {
            setFlagsUpdateRj();
        }
    }

    void MiscCpu::asr(TUint32 amt) {
        bool isNeg = sgn(m_rj);
        lsr(amt, false);
        if (isNeg) {
            TInt32 mask = ~0 << (32 - amt);
            m_aluz |= mask;
        }
        setFlagsUpdateRj();
    }

    const MiscCpu& MiscCpu::run(TUint64 cnt) {
        bool loop = true, doCnt = (0 != cnt);
        while (loop) {
            fetch();
            decode();
            execute();
            if (false == m_perfMon.isNull()) {
                m_perfMon->process(*this);
            }
            if (OpCode::eHalt == m_opCode.getOpcode()) {
                loop = false;
            } else if (doCnt) {
                loop = (0 != --cnt);
            }
        }
        return *this;
    }

    TInt32 MiscCpu::instruction(OpCode::EOp opcode, unsigned j, unsigned k, int immed) {
        TBitVec<32> instrct;
        unsigned lb = 31, rb = 32 - OpCode::cOpCodeN;
        instrct(lb,rb) = opcode;
        lb = rb - 1; rb = lb - cRegsN + 1;
        instrct(lb,rb) = j;
        lb = rb - 1; rb = lb - cRegsN + 1;
        instrct(lb,rb) = k;
        instrct(rb-1,0) = immed;    //unused
        return instrct;
    }

    TInt32 MiscCpu::instructionb(OpCode::EOp opcode, ECond cond, int immed) {
        ASSERT_TRUE(eNotUsed != cond);
        TBitVec<32> instrct;
        unsigned lb = 31, rb = 32 - OpCode::cOpCodeN;
        instrct(lb,rb) = opcode;
        lb = rb - 1; rb = lb - 2;   //3-bits
        instrct(lb,rb) = (int)cond;
        instrct(rb-1,0) = immed;
        return instrct;
    }

    TInt32 MiscCpu::instructioni(OpCode::EOp opcode, unsigned j, int immed) {
        ASSERT_TRUE(OpCode::hasImmed(opcode) && !OpCode::isBranchOrCall(opcode));
        TBitVec<32> instrct;
        unsigned lb = 31, rb = 32 - OpCode::cOpCodeN;
        instrct(lb,rb) = opcode;
        lb = rb - 1; rb = lb - cRegsN + 1;
        instrct(lb,rb) = j;
        instrct(rb-1,0) = immed;
        return instrct;
    }
};

