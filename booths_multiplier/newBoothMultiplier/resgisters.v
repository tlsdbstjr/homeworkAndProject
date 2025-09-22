module register_2bits(clk, reset_n, d, q);
	//inputs and output
	input clk, reset_n;
	input [1:0] d;
	output reg [1:0] q;
	
	//make register
	always @(posedge clk or negedge reset_n)
	begin
		if(reset_n == 0)
			q <= 2'b0;
		else
			q <= d;
	end
endmodule

module register_5bits(clk, reset_n, en, d, q);
	//inputs and output
	input clk, reset_n, en;
	input [4:0] d;
	output reg [4:0] q;
	
	//make register
	always @(posedge clk or negedge reset_n)
	begin
		if(reset_n == 0)
			q <= 4'b0;
		else if (en == 1)
			q <= d;
		else
			q <= q;
	end
endmodule

module register_64bits(clk, reset_n, en, d, q);
	//inputs and output
	input clk, reset_n, en;
	input [63:0] d;
	output reg [63:0] q;
	
	//make register
	always @(posedge clk or negedge reset_n)
	begin
		if(reset_n == 0)
			q <= 64'b0;
		else if (en == 1)
			q <= d;
		else
			q <= q;
	end
endmodule

module register_65bits(clk, reset_n, en, d, q);
	//inputs and output
	input clk, reset_n, en;
	input [64:0] d;
	output reg [64:0] q;
	
	//make register
	always @(posedge clk or negedge reset_n)
	begin
		if(reset_n == 0)
			q <= 65'b0;
		else if (en == 1)
			q <= d;
		else
			q <= q;
	end
endmodule

module register_128bits(clk, reset_n, en, d, q);
	//inputs and output
	input clk, reset_n, en;
	input [127:0] d;
	output reg [127:0] q;
	
	//make register
	always @(posedge clk or negedge reset_n)
	begin
		if(reset_n == 0)
			q <= 128'b0;
		else if (en == 1)
			q <= d;
		else
			q <= q;
	end
endmodule
