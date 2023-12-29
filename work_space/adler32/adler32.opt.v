module xls_test(
  input wire clk,
  input wire [7:0] message,
  output wire [31:0] out
);
  // ===== Pipe stage 0:

  // Registers for pipe stage 0:
  reg [7:0] p0_message;
  always_ff @ (posedge clk) begin
    p0_message <= message;
  end

  // ===== Pipe stage 1:
  wire [8:0] p1_add_130_comb;
  wire [31:0] p1_umod_133_comb;
  assign p1_add_130_comb = {1'h0, p0_message} + 9'h001;
  assign p1_umod_133_comb = {23'h00_0000, p1_add_130_comb} % 32'h0000_fff1;

  // Registers for pipe stage 1:
  reg [31:0] p1_umod_133;
  always_ff @ (posedge clk) begin
    p1_umod_133 <= p1_umod_133_comb;
  end

  // ===== Pipe stage 2:
  wire [31:0] p2_umod_137_comb;
  wire [31:0] p2_or_141_comb;
  assign p2_umod_137_comb = p1_umod_133 % 32'h0000_fff1;
  assign p2_or_141_comb = {p2_umod_137_comb[15:0], 16'h0000} | p1_umod_133;

  // Registers for pipe stage 2:
  reg [31:0] p2_or_141;
  always_ff @ (posedge clk) begin
    p2_or_141 <= p2_or_141_comb;
  end
  assign out = p2_or_141;
endmodule
