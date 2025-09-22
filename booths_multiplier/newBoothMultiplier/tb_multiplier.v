`timescale 1ns/100ps
module tb_multiplier;
		//input and output
	reg clk, reset_n, op_start, op_clear;
	reg [63:0] multiplier, multiplicand;
	wire op_done;
	wire [127:0] result;
	
	multiplier testmodule (clk, reset_n, multiplier, multiplicand, op_start, op_clear, result, op_done);
	
	
	always begin clk = 1; #5; clk = 0; #5; end
	//start test bench
	initial
	begin
		//initializing
		op_start = 0;
		op_clear = 0;
		reset_n = 1; #1;
		reset_n = 0; #1;
		reset_n = 1; #1;
		//7*(-7)
		multiplier = 7;
		multiplicand = -7;
		op_start = 1; #10;
		op_start = 0; #350;
		//finish, initializing
		op_clear = 1; #1;
		op_clear = 0;
		//945 * 1234
		multiplier = 945;
		multiplicand = 1234;
		op_start = 1; #350;
		//finish, initializing
		op_clear = 1; #1;
		op_clear = 0;
		//0 * 0
		multiplier = 0;
		multiplicand = 0; #350;
		//finish, initializing
		op_clear = 1; #1;
		op_clear = 0;
		//-7*7
		multiplier = -7;
		multiplicand = 7; #350;
		//finish, initializing
		op_clear = 1; #1;
		op_clear = 0;
		//-159*-753
		multiplier = -159;
		multiplicand = -753; #350;
		//finish
		$stop;
	end
endmodule 