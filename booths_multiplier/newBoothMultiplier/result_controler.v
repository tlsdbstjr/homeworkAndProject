module result_controler(op, shiftedNumber, resultIn, resultOut, resultEnabler);
	//input and output
	input [1:0] op;
	input [127:0] shiftedNumber, resultIn;
	output [127:0] resultOut;
	output resultEnabler;
	
	//state encoding
	parameter IDLE = 2'b00;
	parameter CALCULATING = 2'b01;
	parameter DONE = 2'b10;
	
	//assignments
	assign resultEnabler = ~op[1] & op[0];
	assign resultOut = shiftedNumber + { resultIn[127],resultIn[127],resultIn[127:2] };	//shifted number + (result ASR 2)
endmodule 