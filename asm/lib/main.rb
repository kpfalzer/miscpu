#/**
# * The MIT License
# * 
# * Copyright (c) 2010  Karl W. Pfalzer
# * 
# * Permission is hereby granted, free of charge, to any person obtaining a copy
# * of this software and associated documentation files (the "Software"), to deal
# * in the Software without restriction, including without limitation the rights
# * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# * copies of the Software, and to permit persons to whom the Software is
# * furnished to do so, subject to the following conditions:
# * 
# * The above copyright notice and this permission notice shall be included in
# * all copies or substantial portions of the Software.
# * 
# * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# * THE SOFTWARE.
#**/

REG_N_BITS = 5
REG_IX_HI = (1 << REG_N_BITS)-1

def abs(n)
  n = -n if n<0
  return n
end

class OpCodes
  class Op
    def initialize(enum,code)
      @enum = enum
      @code = code
    end
    attr_reader :enum, :code
  end
  def initialize(opcode_hxx)
    @op_by_name = Hash.new
    lnum = 0
    enum_val = 0
    File.open(opcode_hxx) do |file|
      while line = file.gets
        lnum += 1
        if line =~ /^\s*(e([A-Z][a-z][a-zA-Z]*))(.\s*=\s*([0-9]+))?/
          enum = $1
          instruction = $2.downcase
          enum_val = $4.to_i if $4
          #puts "#{instruction} : #{enum_val}"
          @op_by_name[instruction] = Op.new(enum, enum_val)
          enum_val += 1
        end
      end
    end
    map_op = {"+"=>"add","+i"=>"addi","-"=>"sub","-i"=>"subi"}
    map_op.merge!({"==?"=>"cmp","==?i"=>"cmpi"})
    map_op.merge!({"<<"=>"lsl","<<i"=>"lsli"})
    map_op.merge!({">>"=>"lsr",">>i"=>"lsri"})
    map_op.merge!({">a>"=>"asr",">a>i"=>"asri"})
    map_op.merge!({"&"=>"and","|"=>"or","^"=>"xor","~"=>"not"})
    map_op.merge!({"&i"=>"andi","|i"=>"ori","^i"=>"xori"})
    map_op.merge!({"push"=>"push","pop"=>"pop"})
    map_op.merge!({"->"=>"br","+>"=>"call"})
    map_op.merge!({"ret"=>"retn","halt"=>"halt","nop"=>"nop"})
    map_op.merge!({"load"=>"load","store"=>"store"})
    map_op.merge!({"loadr"=>"loadr"})
    map_op.merge!({"loadi"=>"loadi","loadil"=>"loadil"})
    @opcode_by_operator = Hash.new
    map_op.each do |k,v|
      @opcode_by_operator[k] = @op_by_name[v]
    end
  end
  def get_last_opcode
    return @op_by_name['notused'].code
  end
  def get_instruction(opcode, cond, j, k, immed)
    opcode += 'i' if k and k<0
    opc = @opcode_by_operator[opcode]
    return Instruction.new(opc, cond, j, k, immed)
  end
  class Instruction
    IMMED_N_BITS_MEM = 17
    IMMED_N_BITS_COND = 24
    IMMED_N_BIT = 22
    MAX_IMMED = (1<<(IMMED_N_BIT-1)) - 1
    OPCODE_N_LSL = (2 * REG_N_BITS) + IMMED_N_BITS_MEM
    MAP_COND = {"cy"=>1,"!cy"=>2,"zero"=>3,"!zero"=>4} #see MiscCpu::ECond
    def initialize(opcode, cond, j, k, immed)
      @opcode = opcode
      @cond = cond ? MAP_COND[cond] : 0
      @j = j          #can be nil
      @k = k          #can be nil
      @immed = immed  #can be nil
    end
    def gen(ofid)
      op = @opcode.code << OPCODE_N_LSL
      longi = nil
      case @opcode.enum
      when /^(eHalt|eNop|eRetn)$/
        ins = op
      when /^(eAdd|eSub|eLsl|eLsr|eAsr|eAnd|eOr|eXor|eLoad|eStore|eLoadr|eCmp)$/
        @j <<= IMMED_N_BITS_MEM + REG_N_BITS
        @k <<= IMMED_N_BITS_MEM
        ins = op + @j + @k
        ins += mask_immed(IMMED_N_BITS_MEM) if @opcode.enum =~ /^(eLoad|eStore)$/
      when /^(eLoadi|eAddi|eSubi|eLsli|eLsri|eAsri|eAndi|eOri|eXori|eCmpi)$/
        @j <<= IMMED_N_BIT
        ins = op + @j + mask_immed(IMMED_N_BIT)
      when /^(eNot|ePush|ePop)$/
        @j <<= IMMED_N_BITS_MEM + REG_N_BITS
        ins = op + @j
      when /^(eBr|eCall)$/
        @cond <<= IMMED_N_BITS_COND
        ins = op + @cond + mask_immed(IMMED_N_BITS_COND)
      when /^eLoadil$/
        ins = op
        loadil = immed
      else
        STDERR.puts "Error: #{@opcode.enum}: Unsupported instruction"
      end
      ofid.puts ins
      ofid.puts loadil if loadil
    end
    private
    def mask_immed(nbits)
      mask = ~(~0 << nbits)
      @immed & mask
    end
  end
end

class Asm
  def initialize(opcode_hxx)
    @opcodes = OpCodes.new(opcode_hxx)
    @var_values = VarVals.new
    @instructions = []
  end
  def asm(infile, outfile = nil)
    @ifname = infile
    for @pass in 1..2
      @lnum = 0
      @org = 0
      File.open(infile) do |file|
        while @orig_line = file.gets
          @lnum += 1
          @line = @orig_line.sub(/\/\/.*/,'').strip
          do_line unless @line.empty?
        end
      end
    end
    if outfile
      ofid = File.new(outfile,'w')
    else
      ofid = STDOUT
    end
    @instructions.each do |ins|
      ins.gen(ofid)
    end
  end
  #
  #regexp constants
  LHS = '(r(?<lhs_ix>\\d+))'
  IMMED = '(?<immed>#\\-?(((0x)?[0-9a-fA-F_]+)|([a-zA-Z_][a-zA-Z0-9_]*)))'
  OP = '((?<op>\\+|\\-|<<|>>|>a>|\\&|\\||\\^)=)'
  CMP_OP = '(?<op>==\?)'
  RHS = "((r(?<rhs_ix>\\d+))|#{IMMED})"
  LABEL = '((?<label>[a-zA-z_][a-zA-z_0-9]+)\\s*:)'
  PROD2 = "#{LABEL}?\\s*#{LHS}\\s*#{OP}\\s*#{RHS}"
  PROD2_REX = Regexp.new("^#{PROD2}$")
  CMP_REX = Regexp.new("^#{LABEL}?\\s*#{LHS}\\s*#{CMP_OP}\\s*#{RHS}$")
  #mem[rk (+|- immed)?]
  MEM_IX = "#{LHS}\\s*((?<op>\\+|\\-)\\s*#{IMMED})?"
  MEM_ALT1 = "(?<mem_lhs>m(em)?\\[(?<mem_ix>#{MEM_IX})\\])"
  PROD3_ALT = "(r(?<lhs_ix>\\d+)|#{MEM_ALT1})"
  PROD3 = "#{LABEL}?\\s*#{PROD3_ALT}\\s*=\\s*#{PROD3_ALT.gsub('lhs','rhs')}"
  PROD3_REX = Regexp.new("^#{PROD3}$")
  PROD4 = "#{LABEL}?\\s*r(?<lhs_ix>\\d+)\\s*=\\s*(?<op>\\~)\\s*r(?<rhs_ix>\\d+)"
  PROD4_REX = Regexp.new("#{PROD4}$")
  PROD5 = "#{LABEL}?\\s*\\s*(?<op>push|pop)\\s*r(?<lhs_ix>\\d+)"
  PROD5_REX = Regexp.new("^#{PROD5}$")
  PROD6 = "#{LABEL}?\\s*((?<cond>\\!?(cy|zero))\\?)?\\s*(?<op>(\\-|\\+)>)\\s*(?<goto>[a-zA-Z_][a-zA-Z0-9_]*)"
  PROD6_REX = Regexp.new("^#{PROD6}$")
  #   8)    ret|rtn|return
  #   9)    halt|nop
  PROD8_REX = Regexp.new("^#{LABEL}?\\s*(?<op>ret|halt|nop)$")
  LDI_REX = Regexp.new("^#{LABEL}?\\s*#{LHS}\\s*=\\s*#{IMMED}$")
  def do_line
    case @line
    when /\.ORG\s+(\S+)/
      @org = eval($1)
      #TODO: add_instruction META for @location so
      #generate spits out locator
    when /\.DEF\s+(\S+)\s*=\s*(.+)/
      define($1,$2) if @pass==1
    else
      case
      when match = PROD2_REX.match(@line)
        prod2(match)
      when match = PROD3_REX.match(@line)
        prod3(match)
      when match = PROD4_REX.match(@line)
        prod4(match)
      when match = PROD5_REX.match(@line)
        prod5(match)
      when match = PROD6_REX.match(@line)
        prod6(match)
      when match = PROD8_REX.match(@line)
        prod8(match)
      when match = CMP_REX.match(@line)
        prod_cmp(match)
      when match = LDI_REX.match(@line)
        prod_ldi(match)
      else
        error("syntax error: #{@line}")
      end
      @org += 1
    end
  end
  def eval(expr)
    return @var_values.eval(expr)
  end
  def define(var,expr)
    return @var_values.set(var, expr)
  end
  def error(msg)
    STDERR.puts "Error: #{@ifname}:#{@lnum}: #{msg}"
    exit 1
  end
  def add_instruction(op, cond, j, k, immed)
    return if @pass==1
    ins = @opcodes.get_instruction(op, cond, j, k, immed)
    @instructions << ins
  end
  def prod_ldi(match)
    set_label(match[:label])
    lhs_ix = match[:lhs_ix].to_i
    check_valid_reg_ix(lhs_ix)
    immed = get_immed(match)
    op = 'loadi'
    if (abs(immed) > OpCodes::Instruction::MAX_IMMED)
      op = 'loadil'
      @org += 1  #account for next word used by immed
    end
    add_instruction(op, nil, lhs_ix, 0, immed)
  end
  # 2)    lhs op= rhs
  def prod2(match)
    set_label(match[:label])
    lhs_ix = match[:lhs_ix].to_i
    check_valid_reg_ix(lhs_ix)
    if match[:rhs_ix]
      rhs_ix = match[:rhs_ix].to_i
      check_valid_reg_ix(rhs_ix)
    else
      rhs_ix = -1
    end
    immed = get_immed(match)
    add_instruction(match[:op], nil, lhs_ix, rhs_ix, immed)
  end
  # lhs ==? rhs
  def prod_cmp(match)
    set_label(match[:label])
    lhs_ix = match[:lhs_ix].to_i
    check_valid_reg_ix(lhs_ix)
    if match[:rhs_ix]
      rhs_ix = match[:rhs_ix].to_i
      check_valid_reg_ix(rhs_ix)
    else
      rhs_ix = -1
    end
    immed = get_immed(match)
    add_instruction(match[:op], nil, lhs_ix, rhs_ix, immed)
  end
  # 3)    (rn | mem[...]) = (rn | mem[...])
  def prod3(match)  #also matches "rn = rm"  :loadr
    set_label(match[:label])
    if (match[:mem_lhs] && match[:mem_rhs])
      error("mem[...]=mem[...] style not supported")
    end
    lhs_ix = match[:lhs_ix].to_i
    rhs_ix = match[:rhs_ix].to_i
    check_valid_reg_ix(lhs_ix)
    check_valid_reg_ix(rhs_ix)
    immed = match[:immed] ? get_immed(match) : nil
    op = match[:mem_rhs] ? 'load' : (match[:immed] ? 'store' : 'loadr')
    add_instruction(op, nil, lhs_ix, rhs_ix, immed)
  end
  #   4)    rj = ~rk
  def prod4(match)
    set_label(match[:label])
    lhs_ix = match[:lhs_ix].to_i
    rhs_ix = match[:rhs_ix].to_i
    check_valid_reg_ix(lhs_ix)
    check_valid_reg_ix(rhs_ix)
    add_instruction(match[:op], nil, lhs_ix, rhs_ix, nil)
  end
  #   5)    push|pop rk
  def prod5(match)
    set_label(match[:label])
    lhs_ix = match[:lhs_ix].to_i
    check_valid_reg_ix(lhs_ix)
    add_instruction(match[:op], nil, lhs_ix, nil, nil)
  end
  #   6,7)    cond? [-+]> label  #a branch or call
  #   6.1)    where cond is one of: cy zero !cy !zero
  #   6.2)    where label is ident (line starts with "ident:"
  def prod6(match)
    return if @pass==1
    set_label(match[:label])
    goto = @var_values.get(match[:goto])
    if goto
      goto = goto - @org - 1 #TODO: correct calc?
    else
      error("'"+match[:goto]+"': target of goto undefined")
    end
    add_instruction(match[:op], match[:cond], nil, nil, goto)
  end
  def prod8(match)
    set_label(match[:label])
    add_instruction(match[:op], nil, nil, nil, nil)
  end
  def set_label(lbl)
    @var_values.set(lbl, @org.to_s) if (lbl && @pass==1)
  end
  def check_valid_reg_ix(ix)
    if (ix < 0) || (ix > REG_IX_HI)
      error("r\'#{ix}\': index out of range \[0..#{REG_IX_HI}\]")
    end
  end

  private

  def get_immed(match)
    return 0 unless match[:immed]
    immed = eval(match[:immed].delete('#'))
    return immed
  end

  #Use this class to encapsulate Hash and also binding
  #to eval expressions
  class VarVals
    def initialize
      @value_by_name = Hash.new
      @namespace = Namespace.new
    end
    def set(name, expr)
      val = eval(expr)
      @value_by_name[name] = val
      return eval(name+"="+val.to_s)
    end
    def get(name)
      return @value_by_name[name]
    end
    def eval(expr)
      return @namespace.evalx(expr)
    end
    class Namespace
      def initialize
        @binding = binding()
      end
      def evalx(expr)
        eval(expr,@binding)
      end
    end
  end
end

asm = Asm.new(ARGV[0])
asm.asm(ARGV[1])


#Format of .s files
#
#   1)  // comment line
#
#   2)    lhs op= rhs
#   2.1)    where op is one of: + - << >> >a> & | ^
#   2.2)    lhs is one of: rj  where j is [0-9]+
#   2.3)    rhs is one of: rk  where k is [0-9]+
#                          #immed  where immed is number or ident
#
#   3)    lhs2 = rhs2
#   3.1)    {l,r}hs2: mem[rk (+ immed)?]   #NOTE: lhs2 cannot be same as rhs2 since cant do mem to mem
#                   | rk
#
#   4)    rj = ~rk
#   5)    push|pop rk
#   6)    cond? -> label  #a branch
#   6.1)    where cond is one of: cy zero !cy !zero
#   6.2)    where label is ident (line starts with "ident:"
#   7)    cond? +> label  #a call/subroutine
#   8)    ret|rtn|return
#   9)    halt|nop
#
#   10)  any instruction line can start with a label "ident:"
#
#  
#   11) The following set variables which can be reference later
#         .DEF ident = expr
#
#       expr: (primary op2)* primary
#        op2: +|-|*|/|%|<<|>>  (integer operators; no parentheses; ruby operator precedence)
#
#       primary: number
#              | ident
#
#       number: (+|-)?[0-9]+
#             | 0x[0-9_a-fA-F]+
#
#   12) The following sets origin of code
#         .ORG number

