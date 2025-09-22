module multiplier_multiplicand_controler(op, shiftedMultiplicand,
		multiplierStart, multiplierIn, multiplierEnabler, multiplierOut,
		multiplicandStart, multiplicandIn, multiplicandEnabler, multiplicandOut);
	//main input and output
	input [1:0] op;
	output reg [127:0] shiftedMultiplicand;
	//for multiplier
	input [63:0] multiplierStart;
	input [64:0] multiplierIn;
	output [64:0] multiplierOut;
	output multiplierEnabler;
	//for multiplicand
	input [63:0] multiplicandStart;
	input [63:0] multiplicandIn;
	output [63:0] multiplicandOut;
	output multiplicandEnabler;
	//wires and buses for internal connection
	wire [63:0] multiplicandMinus;
	
	//state encoding
	parameter IDLE = 2'b00;
	parameter CALCULATING = 2'b01;
	parameter DONE = 2'b10;
	
	//assign main output
	assign multiplicandMinus = -multiplicandIn;
	always @(multiplierIn)
	case (multiplierIn[2:0])
		3'b000 : shiftedMultiplicand <= 128'b0;
		3'b001 : shiftedMultiplicand <= {multiplicandIn[63],multiplicandIn[63],multiplicandIn,62'b0};
		3'b010 : shiftedMultiplicand <= {multiplicandIn[63],multiplicandIn[63],multiplicandIn,62'b0};
		3'b011 : shiftedMultiplicand <= {multiplicandIn[63],multiplicandIn,63'b0};		
		3'b100 : shiftedMultiplicand <= {multiplicandMinus[63],multiplicandMinus,63'b0};		
		3'b101 : shiftedMultiplicand <= {multiplicandMinus[63],multiplicandMinus[63],multiplicandMinus,62'b0};		
		3'b110 : shiftedMultiplicand <= {multiplicandMinus[63],multiplicandMinus[63],multiplicandMinus,62'b0};	
		3'b111 : shiftedMultiplicand <= 128'b0;
	endcase
	
	//assign multiplier output
	assign multiplierOut = op == IDLE ? { multiplierStart, 1'b0 } :
								  op == CALCULATING ? { multiplierIn[64],multiplierIn[64],multiplierIn[64:2] } :	//multiplier = multiplier ASR 2
								  multiplierIn;
	assign multiplierEnabler = ~op[1];	//it's true when op doesn't equal DONE
	
	//assign multiplicand output
	assign multiplicandOut = op == IDLE ? multiplicandStart : multiplicandIn;
	assign multiplicandEnabler = ~(op[1] | op[0]);	//it's true when op equals CALCULATING
	
endmodule 