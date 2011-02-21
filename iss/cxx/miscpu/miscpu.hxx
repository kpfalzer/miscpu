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

#if !defined(_miscpu_miscpu_hxx_)
#    define  _miscpu_miscpu_hxx_

#include <string>
#include "xyzzy/portable.hxx"
#include "xyzzy/array.hxx"
#include "xyzzy/refcnt.hxx"
#include "xyzzy/bitvec.hxx"
#include "opcode.hxx"
#include "miscpu.hxx"

using std::string;
using xyzzy::TUint32;
using xyzzy::TUint64;
using xyzzy::TInt32;
using xyzzy::PTArray;
using xyzzy::TRcObj;
using xyzzy::PTRcObjPtr;
using xyzzy::TBitVec;


namespace miscpu
{
    class MiscCpu;	//forward reference

    class PerfMon : public TRcObj {
    public:
        virtual ~PerfMon() = 0;

        void incrInstructionCnt(unsigned incr = 1) {
            m_instructionCnt += incr;
        }

        TUint64 getInstructionCnt() const {
            return m_instructionCnt;
        }

        //Called during each instruction eval.
        virtual void process(const MiscCpu &cpu) = 0;

    protected:
        explicit PerfMon()
            :   m_instructionCnt(0) {
        }

    private:
        TUint64 m_instructionCnt;
    };

    typedef PTRcObjPtr<PerfMon> TRcPerfMon;

    class DefaultPerfMon : public PerfMon {
    public:
        explicit DefaultPerfMon();

        void process(const MiscCpu &cpu);
        
        static TRcPerfMon create();
        
        ~DefaultPerfMon();

    private:
        //TODO: add cycle count, etc
    };

    class MiscCpu {
    public:
        enum ECond {
            eUncond = 0,
            eCy=1, eNotCy=2,
            eZero=3, eNotZero=4,
            eNotUsed
        };

        explicit MiscCpu(const char *memFname = 0,
                unsigned numRegsN = 5,
                unsigned memDepthN = 20,
                bool useDfltPerfMon = true);

        void loadMemory(string fname);
        void loadMemory(const PTArray<TInt32> &instructs);

        //Generate 32-bit opcode
        TInt32 instruction(OpCode::EOp opcode, unsigned j, unsigned k=0, int immed=0);
        TInt32 instructionb(OpCode::EOp opcode, ECond cond, int immed);
        TInt32 instructioni(OpCode::EOp opcode, unsigned j, int immed);
        TInt32 instruction(OpCode::EOp opcode) {
            return instruction(opcode, 0u, 0u);
        }

        void setPerfMon(TRcPerfMon pmon);

        TRcPerfMon getPerfMon() const {
            return m_perfMon;
        }

        virtual const MiscCpu& run(TUint64 cnt = 0);

        unsigned getMemDepth() const {
            return m_mem.length();
        }

        unsigned getNumRegs() const {
            return m_regs.length();
        }

		void dumpRegs(unsigned lo, unsigned hi) const;

		void dumpRegs(unsigned ix) const {
			dumpRegs(ix, ix);
		}

        const unsigned cRegsN;
        const unsigned cSpRegIx;    //m_regs[cSpRegIx] is stack pointer
        
    protected:
        TUint32     m_pc;
        bool        m_zero, m_cy;
        TBitVec<32> m_ir;

        PTArray<TInt32> m_regs;
        PTArray<TInt32> m_mem;

        //The following initialized by decode()
        TInt32      m_rj, m_rk;         //r[j], r[k]
        TInt32      m_aluz;             //alu output
        OpCode      m_opCode;
        unsigned    m_ixJ, m_ixK;       //m_ixJ is [j] of "r[j]", ...
        TInt32      m_immed;            //immediate field in opcode (signed)

        ECond       m_cond; //from conditional field for branch/call

        TRcPerfMon  m_perfMon;

    private:
        void initialize();
        void reset();

        void fetch();
        void decode();
        void execute();

        void setFlags(TInt32 opb);  //{cy,zero}
        void setFlags();            //{zero}

        void setFlagsUpdateRj(TInt32 opb);    //{cy,zero} then update r[j]
        void setFlagsUpdateRj();              //{zero} ...

        void lsl(TUint32 amt);
        void lsr(TUint32 amt, bool doUpdate = true);
        void asr(TUint32 amt);

        void setCy(unsigned pos);

        bool checkCond() const;

        void call(bool cond = true);    //save pc+1 on stack, ...
        void push(TInt32 val);
        TInt32 pop();

        static const unsigned cInstRegNbits = 32;
    };
};

#endif	//_miscpu_miscpu_hxx_

