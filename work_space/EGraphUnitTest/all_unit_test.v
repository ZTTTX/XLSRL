module xls_test(
  input wire clk,
  input wire [31:0] x1,
  input wire [31:0] y1,
  input wire [31:0] z1,
  output wire [31:0] out
);
  // ===== Pipe stage 0:

  // Registers for pipe stage 0:
  reg [31:0] p0_x1;
  reg [31:0] p0_y1;
  reg [31:0] p0_z1;
  always_ff @ (posedge clk) begin
    p0_x1 <= x1;
    p0_y1 <= y1;
    p0_z1 <= z1;
  end

  // ===== Pipe stage 1:
  wire [31:0] p1_add_153_comb;
  assign p1_add_153_comb = p0_z1 + p0_x1;

  // Registers for pipe stage 1:
  reg [31:0] p1_y1;
  reg [31:0] p1_add_153;
  always_ff @ (posedge clk) begin
    p1_y1 <= p0_y1;
    p1_add_153 <= p1_add_153_comb;
  end

  // ===== Pipe stage 2:
  wire [31:0] p2_add_158_comb;
  assign p2_add_158_comb = p1_add_153 + p1_y1;

  // Registers for pipe stage 2:
  reg [31:0] p2_add_158;
  always_ff @ (posedge clk) begin
    p2_add_158 <= p2_add_158_comb;
  end
  assign out = p2_add_158;
endmodule
