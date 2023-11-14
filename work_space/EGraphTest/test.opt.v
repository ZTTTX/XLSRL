module xls_test(
  input wire clk,
  input wire [31:0] x,
  input wire [31:0] y,
  input wire [31:0] z,
  input wire [31:0] a,
  input wire [31:0] b,
  input wire [31:0] c,
  output wire [31:0] out
);
  // ===== Pipe stage 0:

  // Registers for pipe stage 0:
  reg [31:0] p0_x;
  reg [31:0] p0_y;
  reg [31:0] p0_z;
  reg [31:0] p0_a;
  reg [31:0] p0_c;
  always_ff @ (posedge clk) begin
    p0_x <= x;
    p0_y <= y;
    p0_z <= z;
    p0_a <= a;
    p0_c <= c;
  end

  // ===== Pipe stage 1:
  wire [31:0] p1_add_159_comb;
  wire [30:0] p1_add_157_comb;
  wire [30:0] p1_bit_slice_156_comb;
  wire p1_bit_slice_158_comb;
  assign p1_add_159_comb = p0_a + p0_c;
  assign p1_add_157_comb = p0_x[30:0] + p0_y[30:0];
  assign p1_bit_slice_156_comb = p0_z[31:1];
  assign p1_bit_slice_158_comb = p0_z[0];

  // Registers for pipe stage 1:
  reg [31:0] p1_add_159;
  reg [30:0] p1_add_157;
  reg [30:0] p1_bit_slice_156;
  reg p1_bit_slice_158;
  always_ff @ (posedge clk) begin
    p1_add_159 <= p1_add_159_comb;
    p1_add_157 <= p1_add_157_comb;
    p1_bit_slice_156 <= p1_bit_slice_156_comb;
    p1_bit_slice_158 <= p1_bit_slice_158_comb;
  end

  // ===== Pipe stage 2:
  wire [30:0] p2_add_168_comb;
  wire [31:0] p2_concat_169_comb;
  assign p2_add_168_comb = p1_bit_slice_156 + p1_add_157;
  assign p2_concat_169_comb = {p2_add_168_comb, p1_bit_slice_158};

  // Registers for pipe stage 2:
  reg [31:0] p2_add_159;
  reg [31:0] p2_concat_169;
  always_ff @ (posedge clk) begin
    p2_add_159 <= p1_add_159;
    p2_concat_169 <= p2_concat_169_comb;
  end

  // ===== Pipe stage 3:
  wire [31:0] p3_add_174_comb;
  assign p3_add_174_comb = p2_concat_169 + p2_add_159;

  // Registers for pipe stage 3:
  reg [31:0] p3_add_174;
  always_ff @ (posedge clk) begin
    p3_add_174 <= p3_add_174_comb;
  end
  assign out = p3_add_174;
endmodule
