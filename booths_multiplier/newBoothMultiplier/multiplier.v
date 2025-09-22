module multiplier(clk, reset_n, multiplier, multiplicand, op_start, op_clear, result, op_done);
	//input and output
	input clk, reset_n, op_start, op_clear;
	input [63:0] multiplier, multiplicand;
	output op_done;
	output [127:0] result;
	
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
	
endmodule 