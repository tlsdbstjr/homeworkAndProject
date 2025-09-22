module state_count_output_controler(op_start, count, q, d, nextCount, countEnabler, op_done);
	//input and output
	input op_start;
	input [4:0] count;
	input [1:0] q;
	output [1:0] d;
	output [4:0]nextCount;
	output countEnabler, op_done;
	//state encoding
	parameter IDLE = 2'b00;
	parameter CALCULATING = 2'b01;
	parameter DONE = 2'b10;
	
	//regs and wires for casex
	wire finish;
	reg [1:0] d;
	
	//set outputs
	assign finish = count[0] & count[1] & count[2] & count[3] & count[4];
	always @(q, op_start, finish)
	casex({q, op_start, finish})
		4'b0000 : d <= IDLE;
		4'b0010 : d <= CALCULATING;
		4'b00x1 : d <= DONE;	//exception : non calculated, but finished => goto DONE state
		4'b01X0 : d <= CALCULATING;
		4'b01X1 : d <= DONE;
		4'b10XX : d <= DONE;
		default : d <= 2'bxx;	//exception : wrong inputs
	endcase
	assign op_done = q[1];
	assign nextCount = count + 1;
	assign countEnabler = ~q[1] & q[0];
endmodule 