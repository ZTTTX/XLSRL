module xls_test(
  input wire clk,
  input wire [127:0] a,
  input wire [127:0] b,
  input wire [127:0] result,
  output wire [383:0] out
);
  // lint_off SIGNED_TYPE
  // lint_off MULTIPLY
  function automatic [31:0] smul32b_32b_x_32b (input reg [31:0] lhs, input reg [31:0] rhs);
    reg signed [31:0] signed_lhs;
    reg signed [31:0] signed_rhs;
    reg signed [31:0] signed_result;
    begin
      signed_lhs = $signed(lhs);
      signed_rhs = $signed(rhs);
      signed_result = signed_lhs * signed_rhs;
      smul32b_32b_x_32b = $unsigned(signed_result);
    end
  endfunction
  // lint_on MULTIPLY
  // lint_on SIGNED_TYPE
  wire [31:0] a_unflattened[2][2];
  assign a_unflattened[0][0] = a[31:0];
  assign a_unflattened[0][1] = a[63:32];
  assign a_unflattened[1][0] = a[95:64];
  assign a_unflattened[1][1] = a[127:96];
  wire [31:0] b_unflattened[2][2];
  assign b_unflattened[0][0] = b[31:0];
  assign b_unflattened[0][1] = b[63:32];
  assign b_unflattened[1][0] = b[95:64];
  assign b_unflattened[1][1] = b[127:96];
  wire [31:0] result_unflattened[2][2];
  assign result_unflattened[0][0] = result[31:0];
  assign result_unflattened[0][1] = result[63:32];
  assign result_unflattened[1][0] = result[95:64];
  assign result_unflattened[1][1] = result[127:96];

  // ===== Pipe stage 0:

  // Registers for pipe stage 0:
  reg [31:0] p0_a[2][2];
  reg [31:0] p0_b[2][2];
  always_ff @ (posedge clk) begin
    p0_a <= a_unflattened;
    p0_b <= b_unflattened;
  end

  // ===== Pipe stage 1:
  wire [31:0] p1_array_index_1465_comb;
  wire [31:0] p1_array_index_1466_comb;
  wire [31:0] p1_array_index_1467_comb;
  wire [31:0] p1_array_index_1468_comb;
  wire [31:0] p1_array_index_1469_comb;
  wire [31:0] p1_array_index_1470_comb;
  wire [31:0] p1_array_index_1471_comb;
  wire [31:0] p1_array_index_1472_comb;
  assign p1_array_index_1465_comb = p0_a[1'h0][1'h0];
  assign p1_array_index_1466_comb = p0_b[1'h0][1'h0];
  assign p1_array_index_1467_comb = p0_a[1'h0][1'h1];
  assign p1_array_index_1468_comb = p0_b[1'h1][1'h0];
  assign p1_array_index_1469_comb = p0_b[1'h0][1'h1];
  assign p1_array_index_1470_comb = p0_b[1'h1][1'h1];
  assign p1_array_index_1471_comb = p0_a[1'h1][1'h0];
  assign p1_array_index_1472_comb = p0_a[1'h1][1'h1];

  // Registers for pipe stage 1:
  reg [31:0] p1_a[2][2];
  reg [31:0] p1_b[2][2];
  reg [31:0] p1_array_index_1465;
  reg [31:0] p1_array_index_1466;
  reg [31:0] p1_array_index_1467;
  reg [31:0] p1_array_index_1468;
  reg [31:0] p1_array_index_1469;
  reg [31:0] p1_array_index_1470;
  reg [31:0] p1_array_index_1471;
  reg [31:0] p1_array_index_1472;
  always_ff @ (posedge clk) begin
    p1_a <= p0_a;
    p1_b <= p0_b;
    p1_array_index_1465 <= p1_array_index_1465_comb;
    p1_array_index_1466 <= p1_array_index_1466_comb;
    p1_array_index_1467 <= p1_array_index_1467_comb;
    p1_array_index_1468 <= p1_array_index_1468_comb;
    p1_array_index_1469 <= p1_array_index_1469_comb;
    p1_array_index_1470 <= p1_array_index_1470_comb;
    p1_array_index_1471 <= p1_array_index_1471_comb;
    p1_array_index_1472 <= p1_array_index_1472_comb;
  end

  // ===== Pipe stage 2:
  wire [31:0] p2_smul_1493_comb;
  wire [31:0] p2_smul_1494_comb;
  wire [31:0] p2_smul_1495_comb;
  wire [31:0] p2_smul_1496_comb;
  wire [31:0] p2_smul_1497_comb;
  wire [31:0] p2_smul_1498_comb;
  wire [31:0] p2_smul_1499_comb;
  wire [31:0] p2_smul_1500_comb;
  wire [31:0] p2_add_1501_comb;
  wire [31:0] p2_add_1502_comb;
  wire [31:0] p2_add_1503_comb;
  wire [31:0] p2_add_1504_comb;
  wire [31:0] p2_array_1505_comb[2];
  wire [31:0] p2_array_1506_comb[2];
  wire [31:0] p2_array_1507_comb[2][2];
  wire [383:0] p2_tuple_1508_comb;
  assign p2_smul_1493_comb = smul32b_32b_x_32b(p1_array_index_1465, p1_array_index_1466);
  assign p2_smul_1494_comb = smul32b_32b_x_32b(p1_array_index_1467, p1_array_index_1468);
  assign p2_smul_1495_comb = smul32b_32b_x_32b(p1_array_index_1465, p1_array_index_1469);
  assign p2_smul_1496_comb = smul32b_32b_x_32b(p1_array_index_1467, p1_array_index_1470);
  assign p2_smul_1497_comb = smul32b_32b_x_32b(p1_array_index_1471, p1_array_index_1466);
  assign p2_smul_1498_comb = smul32b_32b_x_32b(p1_array_index_1472, p1_array_index_1468);
  assign p2_smul_1499_comb = smul32b_32b_x_32b(p1_array_index_1471, p1_array_index_1469);
  assign p2_smul_1500_comb = smul32b_32b_x_32b(p1_array_index_1472, p1_array_index_1470);
  assign p2_add_1501_comb = p2_smul_1493_comb + p2_smul_1494_comb;
  assign p2_add_1502_comb = p2_smul_1495_comb + p2_smul_1496_comb;
  assign p2_add_1503_comb = p2_smul_1497_comb + p2_smul_1498_comb;
  assign p2_add_1504_comb = p2_smul_1499_comb + p2_smul_1500_comb;
  assign p2_array_1505_comb[0] = p2_add_1501_comb;
  assign p2_array_1505_comb[1] = p2_add_1502_comb;
  assign p2_array_1506_comb[0] = p2_add_1503_comb;
  assign p2_array_1506_comb[1] = p2_add_1504_comb;
  assign p2_array_1507_comb[0] = p2_array_1505_comb;
  assign p2_array_1507_comb[1] = p2_array_1506_comb;
  assign p2_tuple_1508_comb = {{{p1_a[1][1], p1_a[1][0]}, {p1_a[0][1], p1_a[0][0]}}, {{p1_b[1][1], p1_b[1][0]}, {p1_b[0][1], p1_b[0][0]}}, {{p2_array_1507_comb[1][1], p2_array_1507_comb[1][0]}, {p2_array_1507_comb[0][1], p2_array_1507_comb[0][0]}}};

  // Registers for pipe stage 2:
  reg [383:0] p2_tuple_1508;
  always_ff @ (posedge clk) begin
    p2_tuple_1508 <= p2_tuple_1508_comb;
  end
  assign out = p2_tuple_1508;
endmodule
