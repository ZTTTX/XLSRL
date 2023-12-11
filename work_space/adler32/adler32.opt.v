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
  wire [8:0] p1_add_126_comb;
  assign p1_add_126_comb = {1'h0, p0_message} + 9'h001;

  // Registers for pipe stage 1:
  reg [8:0] p1_add_126;
  always_ff @ (posedge clk) begin
    p1_add_126 <= p1_add_126_comb;
  end

  // ===== Pipe stage 2:
  wire [31:0] p2_umod_132_comb;
  assign p2_umod_132_comb = {23'h00_0000, p1_add_126} % 32'h0000_fff1;

  // Registers for pipe stage 2:
  reg [31:0] p2_umod_132;
  always_ff @ (posedge clk) begin
    p2_umod_132 <= p2_umod_132_comb;
  end

  // ===== Pipe stage 3:
  wire [31:0] p3_umod_136_comb;
  wire [15:0] p3_bit_slice_137_comb;
  assign p3_umod_136_comb = p2_umod_132 % 32'h0000_fff1;
  assign p3_bit_slice_137_comb = p3_umod_136_comb[15:0];

  // Registers for pipe stage 3:
  reg [31:0] p3_umod_132;
  reg [15:0] p3_bit_slice_137;
  always_ff @ (posedge clk) begin
    p3_umod_132 <= p2_umod_132;
    p3_bit_slice_137 <= p3_bit_slice_137_comb;
  end

  // ===== Pipe stage 4:
  wire [31:0] p4_or_144_comb;
  assign p4_or_144_comb = {p3_bit_slice_137, 16'h0000} | p3_umod_132;

  // Registers for pipe stage 4:
  reg [31:0] p4_or_144;
  always_ff @ (posedge clk) begin
    p4_or_144 <= p4_or_144_comb;
  end
  assign out = p4_or_144;
endmodule
