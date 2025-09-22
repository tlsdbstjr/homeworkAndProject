`timescale 1ns/100ps
module tb_multiplier_internal;
		//input and output
	reg clk, reset_n, op_start, op_clear;
	reg [63:0] multiplier, multiplicand;
	wire op_done;
	wire [127:0] result;
	
	//wires and buses for internal connection
	wire compounded_reset_n, en_cntr, en_mltr, en_mltd, en_res;
	wire [1:0] op, nextOp;
	wire [4:0] counter, nextCounter;
	wire [63:0] mltd, nextMltd;
	wire [64:0] mltr, nextMltr;
	wire [127:0] shiftedNumber, nextResult;
	
	//assign wires and buses
	assign compounded_reset_n = ~op_clear & reset_n;
	
	//instantiate registers
	register_2bits R0_state (.clk(clk), .reset_n(compounded_reset_n), .d(nextOp), .q(op));
	register_5bits R1_count (.clk(clk), .reset_n(compounded_reset_n), .en(en_cntr), .d(nextCounter), .q(counter));
	register_64bits R2_multiplicand (.clk(clk), .reset_n(compounded_reset_n), .en(en_mltd), .d(nextMltd), .q(mltd));
	register_65bits R3_multiplier (.clk(clk), .reset_n(compounded_reset_n), .en(en_mltr), .d(nextMltr), .q(mltr));
	register_128bits R4_result(.clk(clk), .reset_n(compounded_reset_n), .en(en_res), .d(nextResult), .q(result));
	
	//instantiate submodules
	state_count_output_controler U0_state_count_output_controler(.op_start(op_start), .count(counter), .q(op), .d(nextOp), .nextCount(nextCounter), .countEnabler(en_cntr), .op_done(op_done));
	multiplier_multiplicand_controler U1_multiplier_multiplicand_controler (.op(op), .shiftedMultiplicand(shiftedNumber),
		.multiplierStart(multiplier), .multiplierIn(mltr), .multiplierEnabler(en_mltr), .multiplierOut(nextMltr),
		.multiplicandStart(multiplicand), .multiplicandIn(mltd), .multiplicandEnabler(en_mltd), .multiplicandOut(nextMltd));
	result_controler U2_result_controler(.op(op), .shiftedNumber(shiftedNumber), .resultIn(result), .resultOut(nextResult), .resultEnabler(en_res));
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
		op_start = 0; #30;
		//change values
		op_start = 1; #30;
		op_start = 0;
		multiplier = 0;
		multiplicand = 0; #270;
		
		//initializing
		op_start = 0;
		op_clear = 0;
		reset_n = 1; #1;
		reset_n = 0; #1;
		reset_n = 1; #1;
		//7*(-7)
		multiplier = 7;
		multiplicand = -7;
		op_start = 1; #30;
		//op_clear
		op_clear = 1; #1;
		
		//initializing
		op_start = 0;
		op_clear = 0;#10;

		//7*(-7)
		multiplier = 7;
		multiplicand = -7;
		op_start = 1; #10;
		op_start = 0; #30;
		//reset
		reset_n = 0; #1;
		
		//initializing
		op_start = 0;
		op_clear = 0;#10;
		//7*(-7)
		multiplier = 7;
		multiplicand = -7;
		op_start = 1; #10;
		op_start = 0; #30;
		//reset_n and op_clear
		reset_n = 0;
		op_clear = 1;#10;
		$stop;
	end
endmodule 