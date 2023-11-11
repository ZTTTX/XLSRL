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

  // Registers for pipe stage 1:
  reg [31:0] p1_a[2][2];
  reg [31:0] p1_b[2][2];
  always_ff @ (posedge clk) begin
    p1_a <= p0_a;
    p1_b <= p0_b;
  end

  // ===== Pipe stage 2:

  // Registers for pipe stage 2:
  reg [31:0] p2_a[2][2];
  reg [31:0] p2_b[2][2];
  always_ff @ (posedge clk) begin
    p2_a <= p1_a;
    p2_b <= p1_b;
  end

  // ===== Pipe stage 3:
  wire [31:0] p3_array_index_1473_comb;
  wire [31:0] p3_array_index_1474_comb;
  wire [31:0] p3_array_index_1475_comb;
  wire [31:0] p3_array_index_1476_comb;
  wire [31:0] p3_array_index_1477_comb;
  wire [31:0] p3_array_index_1478_comb;
  wire [31:0] p3_array_index_1479_comb;
  wire [31:0] p3_array_index_1480_comb;
  assign p3_array_index_1473_comb = p2_a[1'h0][1'h0];
  assign p3_array_index_1474_comb = p2_b[1'h0][1'h0];
  assign p3_array_index_1475_comb = p2_a[1'h0][1'h1];
  assign p3_array_index_1476_comb = p2_b[1'h1][1'h0];
  assign p3_array_index_1477_comb = p2_b[1'h0][1'h1];
  assign p3_array_index_1478_comb = p2_b[1'h1][1'h1];
  assign p3_array_index_1479_comb = p2_a[1'h1][1'h0];
  assign p3_array_index_1480_comb = p2_a[1'h1][1'h1];

  // Registers for pipe stage 3:
  reg [31:0] p3_a[2][2];
  reg [31:0] p3_b[2][2];
  reg [31:0] p3_array_index_1473;
  reg [31:0] p3_array_index_1474;
  reg [31:0] p3_array_index_1475;
  reg [31:0] p3_array_index_1476;
  reg [31:0] p3_array_index_1477;
  reg [31:0] p3_array_index_1478;
  reg [31:0] p3_array_index_1479;
  reg [31:0] p3_array_index_1480;
  always_ff @ (posedge clk) begin
    p3_a <= p2_a;
    p3_b <= p2_b;
    p3_array_index_1473 <= p3_array_index_1473_comb;
    p3_array_index_1474 <= p3_array_index_1474_comb;
    p3_array_index_1475 <= p3_array_index_1475_comb;
    p3_array_index_1476 <= p3_array_index_1476_comb;
    p3_array_index_1477 <= p3_array_index_1477_comb;
    p3_array_index_1478 <= p3_array_index_1478_comb;
    p3_array_index_1479 <= p3_array_index_1479_comb;
    p3_array_index_1480 <= p3_array_index_1480_comb;
  end

  // ===== Pipe stage 4:
  wire [31:0] p4_smul_1501_comb;
  wire [31:0] p4_smul_1502_comb;
  wire [31:0] p4_smul_1503_comb;
  wire [31:0] p4_smul_1504_comb;
  wire [31:0] p4_smul_1505_comb;
  wire [31:0] p4_smul_1506_comb;
  wire [31:0] p4_smul_1507_comb;
  wire [31:0] p4_smul_1508_comb;
  assign p4_smul_1501_comb = smul32b_32b_x_32b(p3_array_index_1473, p3_array_index_1474);
  assign p4_smul_1502_comb = smul32b_32b_x_32b(p3_array_index_1475, p3_array_index_1476);
  assign p4_smul_1503_comb = smul32b_32b_x_32b(p3_array_index_1473, p3_array_index_1477);
  assign p4_smul_1504_comb = smul32b_32b_x_32b(p3_array_index_1475, p3_array_index_1478);
  assign p4_smul_1505_comb = smul32b_32b_x_32b(p3_array_index_1479, p3_array_index_1474);
  assign p4_smul_1506_comb = smul32b_32b_x_32b(p3_array_index_1480, p3_array_index_1476);
  assign p4_smul_1507_comb = smul32b_32b_x_32b(p3_array_index_1479, p3_array_index_1477);
  assign p4_smul_1508_comb = smul32b_32b_x_32b(p3_array_index_1480, p3_array_index_1478);

  // Registers for pipe stage 4:
  reg [31:0] p4_a[2][2];
  reg [31:0] p4_b[2][2];
  reg [31:0] p4_smul_1501;
  reg [31:0] p4_smul_1502;
  reg [31:0] p4_smul_1503;
  reg [31:0] p4_smul_1504;
  reg [31:0] p4_smul_1505;
  reg [31:0] p4_smul_1506;
  reg [31:0] p4_smul_1507;
  reg [31:0] p4_smul_1508;
  always_ff @ (posedge clk) begin
    p4_a <= p3_a;
    p4_b <= p3_b;
    p4_smul_1501 <= p4_smul_1501_comb;
    p4_smul_1502 <= p4_smul_1502_comb;
    p4_smul_1503 <= p4_smul_1503_comb;
    p4_smul_1504 <= p4_smul_1504_comb;
    p4_smul_1505 <= p4_smul_1505_comb;
    p4_smul_1506 <= p4_smul_1506_comb;
    p4_smul_1507 <= p4_smul_1507_comb;
    p4_smul_1508 <= p4_smul_1508_comb;
  end

  // ===== Pipe stage 5:
  wire [31:0] p5_add_1529_comb;
  wire [31:0] p5_add_1530_comb;
  wire [31:0] p5_add_1531_comb;
  wire [31:0] p5_add_1532_comb;
  wire [31:0] p5_array_1533_comb[2];
  wire [31:0] p5_array_1534_comb[2];
  wire [31:0] p5_array_1535_comb[2][2];
  wire [383:0] p5_tuple_1536_comb;
  assign p5_add_1529_comb = p4_smul_1501 + p4_smul_1502;
  assign p5_add_1530_comb = p4_smul_1503 + p4_smul_1504;
  assign p5_add_1531_comb = p4_smul_1505 + p4_smul_1506;
  assign p5_add_1532_comb = p4_smul_1507 + p4_smul_1508;
  assign p5_array_1533_comb[0] = p5_add_1529_comb;
  assign p5_array_1533_comb[1] = p5_add_1530_comb;
  assign p5_array_1534_comb[0] = p5_add_1531_comb;
  assign p5_array_1534_comb[1] = p5_add_1532_comb;
  assign p5_array_1535_comb[0] = p5_array_1533_comb;
  assign p5_array_1535_comb[1] = p5_array_1534_comb;
  assign p5_tuple_1536_comb = {{{p4_a[1][1], p4_a[1][0]}, {p4_a[0][1], p4_a[0][0]}}, {{p4_b[1][1], p4_b[1][0]}, {p4_b[0][1], p4_b[0][0]}}, {{p5_array_1535_comb[1][1], p5_array_1535_comb[1][0]}, {p5_array_1535_comb[0][1], p5_array_1535_comb[0][0]}}};

  // Registers for pipe stage 5:
  reg [383:0] p5_tuple_1536;
  always_ff @ (posedge clk) begin
    p5_tuple_1536 <= p5_tuple_1536_comb;
  end
  assign out = p5_tuple_1536;
endmodule
