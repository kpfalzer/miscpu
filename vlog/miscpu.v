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
module miscpu
	#(parameter REGNB = 5, N=32)
	(	//These outputs valid when "posedge ovld"
		output reg ovld,
		output reg [N-1:0] addr, output reg we,
		output reg [N-1:0] dout,

		//These inputs valid when "posedge ivld"
		input ivld,
		input [N-1:0] din
	);

	reg [N-1:0] m_regs[0:(1<<REGNB - 1)];
	reg [N-1:0] m_ir, m_pc;
	reg m_zero, m_cy;

	event ev_reset, ev_fetch, ev_decode, ev_execute;

	initial begin
		#0 ->ev_reset;
	end

	always @ev_reset begin
		m_pc = 0;
		ovld = 0;
		->ev_fetch;
	end

	always @ev_fetch begin
		addr = m_pc;
		we = 0;
		#1 ovld = 1;
		@(posedge ivld);
		#1 ovld = ~ovld;
		m_ir = din;
	end

	initial //always #1
		$monitor($time,,,"ovld=%d addr=%d we=%b dout=%d : ivld=%d din=%d ir=%d",
		          ovld,addr,we,dout,ivld,din,m_ir);
endmodule

module tb;
	parameter REGNB = 5, N=32;

	wire ovld;
	wire [N-1:0] addr; 
	wire we;
	wire [N-1:0] dout;

	reg ivld;
	reg [N-1:0] din;

	miscpu #(5,32) dut(ovld,addr,we,dout,ivld,din);

	initial begin
		#0 ivld = 0;
	end

	always @(posedge ovld) begin
		din = 1234;
		#1 ivld = ~ivld;
		@ovld;
		#1 ivld = ~ivld;
		#5;
		$finish;
	end

endmodule
