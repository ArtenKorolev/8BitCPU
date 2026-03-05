#include "instructions.h"

#include "cpu.h"
#include "log.h"
#include "memory.h"

void nop_instr(const instr_context_t *ctx);
void jump_instr(cpu_t *self, addressing_mode_e mode, const memory_t *memory);
void load_register_instr(cpu_t *self, byte_t *register_ptr, addressing_mode_e mode, const memory_t *memory);
void store_register_instr(const cpu_t *self, byte_t register_value, memory_t *memory, addressing_mode_e mode);
void add_to_accumulator_instr(cpu_t *self, const memory_t *memory, addressing_mode_e mode);
void and_with_accumulator_instr(cpu_t *self, const memory_t *memory, addressing_mode_e mode);
void jump_subroutine_instr(cpu_t *self, memory_t *memory);
void return_from_subroutine_instr(cpu_t *self, const memory_t *memory);
void branch_on_flag_instr(cpu_t *self, byte_t mask, bool branch_if_set);
void compare_instr(cpu_t *self, const memory_t *memory, byte_t register_value, addressing_mode_e mode);
void dec_memory_instr(cpu_t *self, memory_t *memory, addressing_mode_e mode);
void inc_memory_instr(cpu_t *self, memory_t *memory, addressing_mode_e mode);
void dec_register_instr(cpu_t *self, byte_t *register_ptr);
void inc_register_instr(cpu_t *self, byte_t *register_ptr);
void test_bit_instr(cpu_t *self, const memory_t *memory, addressing_mode_e mode);
void transfer_regs_instr(cpu_t *self, const byte_t *from, byte_t *to);
void exclusive_or_instr(cpu_t *self, const memory_t *memory, addressing_mode_e mode);
void logical_shift_right_instr(cpu_t *self, memory_t *memory, addressing_mode_e);
void arithmetic_shift_left(cpu_t *self, memory_t *memory, addressing_mode_e);
byte_t pull_from_stack_instr(cpu_t *self, const memory_t *memory);
void push_onto_stack_instr(cpu_t *self, memory_t *memory, const byte_t value);
void or_with_accumulator_instr(cpu_t *self, const memory_t *memory, addressing_mode_e mode);

void lda_instr(const instr_context_t *context) {
  load_register_instr(context->cpu, &context->cpu->reg_A, context->mode, context->memory);
}

void ldx_instr(const instr_context_t *context) {
  load_register_instr(context->cpu, &context->cpu->reg_X, context->mode, context->memory);
}

void ldy_instr(const instr_context_t *context) {
  load_register_instr(context->cpu, &context->cpu->reg_Y, context->mode, context->memory);
}

void sta_instr(const instr_context_t *context) {
  store_register_instr(context->cpu, context->cpu->reg_A, context->memory, context->mode);
}

void stx_instr(const instr_context_t *context) {
  store_register_instr(context->cpu, context->cpu->reg_X, context->memory, context->mode);
}

void sty_instr(const instr_context_t *context) {
  store_register_instr(context->cpu, context->cpu->reg_Y, context->memory, context->mode);
}

void and_instr(const instr_context_t *context) {
  and_with_accumulator_instr(context->cpu, context->memory, context->mode);
}

void ora_instr(const instr_context_t *context) {
  or_with_accumulator_instr(context->cpu, context->memory, context->mode);
}

void eor_instr(const instr_context_t *context) {
  exclusive_or_instr(context->cpu, context->memory, context->mode);
}

void adc_instr(const instr_context_t *context) {
  add_to_accumulator_instr(context->cpu, context->memory, context->mode);
}

void cmp_instr(const instr_context_t *context) {
  compare_instr(context->cpu, context->memory, context->cpu->reg_A, context->mode);
}

void cpx_instr(const instr_context_t *context) {
  compare_instr(context->cpu, context->memory, context->cpu->reg_X, context->mode);
}

void cpy_instr(const instr_context_t *context) {
  compare_instr(context->cpu, context->memory, context->cpu->reg_Y, context->mode);
}

void inc_instr(const instr_context_t *context) {
  inc_memory_instr(context->cpu, context->memory, context->mode);
}

void dec_instr(const instr_context_t *context) {
  dec_memory_instr(context->cpu, context->memory, context->mode);
}

void inx_instr(const instr_context_t *context) {
  inc_register_instr(context->cpu, &context->cpu->reg_X);
}

void iny_instr(const instr_context_t *context) {
  inc_register_instr(context->cpu, &context->cpu->reg_Y);
}

void dex_instr(const instr_context_t *context) {
  dec_register_instr(context->cpu, &context->cpu->reg_X);
}

void dey_instr(const instr_context_t *context) {
  dec_register_instr(context->cpu, &context->cpu->reg_Y);
}

void bit_instr(const instr_context_t *context) {
  test_bit_instr(context->cpu, context->memory, context->mode);
}

void asl_instr(const instr_context_t *context) {
  arithmetic_shift_left(context->cpu, context->memory, context->mode);
}

void lsr_instr(const instr_context_t *context) {
  logical_shift_right_instr(context->cpu, context->memory, context->mode);
}

void pha_instr(const instr_context_t *context) {
  push_onto_stack_instr(context->cpu, context->memory, context->cpu->reg_A);
}

void php_instr(const instr_context_t *context) {
  push_onto_stack_instr(context->cpu, context->memory, context->cpu->reg_P);
}

void pla_instr(const instr_context_t *context) {
  context->cpu->reg_A = pull_from_stack_instr(context->cpu, context->memory);
  cpu_update_zero_and_negative_flags(context->cpu, context->cpu->reg_A);
}

void plp_instr(const instr_context_t *context) {
  context->cpu->reg_P = pull_from_stack_instr(context->cpu, context->memory);
}

void jmp_instr(const instr_context_t *context) {
  jump_instr(context->cpu, context->mode, context->memory);
}

void jsr_instr(const instr_context_t *context) {
  jump_subroutine_instr(context->cpu, context->memory);
}

void rts_instr(const instr_context_t *context) {
  return_from_subroutine_instr(context->cpu, context->memory);
}

void bcc_instr(const instr_context_t *context) {
  branch_on_flag_instr(context->cpu, CARRY, false);
}

void bcs_instr(const instr_context_t *context) {
  branch_on_flag_instr(context->cpu, CARRY, true);
}

void beq_instr(const instr_context_t *context) {
  branch_on_flag_instr(context->cpu, ZERO, true);
}

void bne_instr(const instr_context_t *context) {
  branch_on_flag_instr(context->cpu, ZERO, false);
}

void bmi_instr(const instr_context_t *context) {
  branch_on_flag_instr(context->cpu, NEGATIVE, true);
}

void bpl_instr(const instr_context_t *context) {
  branch_on_flag_instr(context->cpu, NEGATIVE, false);
}

void bvs_instr(const instr_context_t *context) {
  branch_on_flag_instr(context->cpu, OVERFLOW_, true);
}

void bvc_instr(const instr_context_t *context) {
  branch_on_flag_instr(context->cpu, OVERFLOW_, false);
}

void clc_instr(const instr_context_t *context) {
  cpu_status_flag_clear(context->cpu, CARRY);
}

void sec_instr(const instr_context_t *context) {
  cpu_status_flag_set(context->cpu, CARRY);
}

void cli_instr(const instr_context_t *context) {
  cpu_status_flag_clear(context->cpu, INTERRUPT);
}

void sei_instr(const instr_context_t *context) {
  cpu_status_flag_set(context->cpu, INTERRUPT);
}

void clv_instr(const instr_context_t *context) {
  cpu_status_flag_clear(context->cpu, OVERFLOW_);
}

void cld_instr(const instr_context_t *context) {
  cpu_status_flag_clear(context->cpu, DECIMAL);
}

void sed_instr(const instr_context_t *context) {
  cpu_status_flag_set(context->cpu, DECIMAL);
}

void tax_instr(const instr_context_t *context) {
  transfer_regs_instr(context->cpu, &context->cpu->reg_A, &context->cpu->reg_X);
}

void tay_instr(const instr_context_t *context) {
  transfer_regs_instr(context->cpu, &context->cpu->reg_A, &context->cpu->reg_Y);
}

void txa_instr(const instr_context_t *context) {
  transfer_regs_instr(context->cpu, &context->cpu->reg_X, &context->cpu->reg_A);
}

void tya_instr(const instr_context_t *context) {
  transfer_regs_instr(context->cpu, &context->cpu->reg_Y, &context->cpu->reg_A);
}

void tsx_instr(const instr_context_t *context) {
  transfer_regs_instr(context->cpu, &context->cpu->reg_SP, &context->cpu->reg_X);
}

void txs_instr(const instr_context_t *context) {
  context->cpu->reg_SP = context->cpu->reg_X;
}

void nop_instr(const instr_context_t *ctx) {
  emu_log(INFO, "No operation;\n");
}

const opcode_data_t *get_opcode_data(const opcode_e opcode) {
  if (opcode > OPCODES_COUNT) {
    return NULL;
  }

  static const opcode_data_t opcodes_data_table[OPCODES_COUNT] = {
      /* ========== NOP ========== */
      [NOP_OPCOD] = {0, nop_instr, IMPLIED},

      /* ========== LDA ========== */
      [LDAI_OPCOD] = {1, lda_instr, IMMEDIATE},
      [LDAZ_OPCOD] = {1, lda_instr, ZERO_PAGE},
      [LDAZX_OPCOD] = {1, lda_instr, ZERO_PAGE_X},
      [LDAA_OPCOD] = {2, lda_instr, ABSOLUTE},
      [LDAAX_OPCOD] = {2, lda_instr, ABSOLUTE_X},
      [LDAAY_OPCOD] = {2, lda_instr, ABSOLUTE_Y},
      [LDAIX_OPCOD] = {1, lda_instr, INDIRECT_X},
      [LDAIY_OPCOD] = {1, lda_instr, INDERECT_Y},

      /* ========== LDX ========== */
      [LDXI_OPCOD] = {1, ldx_instr, IMMEDIATE},
      [LDXZ_OPCOD] = {1, ldx_instr, ZERO_PAGE},
      [LDXZY_OPCOD] = {1, ldx_instr, ZERO_PAGE_Y},
      [LDXA_OPCOD] = {2, ldx_instr, ABSOLUTE},
      [LDXAY_OPCOD] = {2, ldx_instr, ABSOLUTE_Y},

      /* ========== LDY ========== */
      [LDYI_OPCOD] = {1, ldy_instr, IMMEDIATE},
      [LDYZ_OPCOD] = {1, ldy_instr, ZERO_PAGE},
      [LDYZX_OPCOD] = {1, ldy_instr, ZERO_PAGE_X},
      [LDYA_OPCOD] = {2, ldy_instr, ABSOLUTE},
      [LDYAX_OPCOD] = {2, ldy_instr, ABSOLUTE_X},

      /* ========== STA/STX/STY ========== */
      [STAZ_OPCOD] = {1, sta_instr, ZERO_PAGE},
      [STAZX_OPCOD] = {1, sta_instr, ZERO_PAGE_X},
      [STAA_OPCOD] = {2, sta_instr, ABSOLUTE},
      [STAAX_OPCOD] = {2, sta_instr, ABSOLUTE_X},
      [STAAY_OPCOD] = {2, sta_instr, ABSOLUTE_Y},
      [STAIX_OPCOD] = {1, sta_instr, INDIRECT_X},
      [STAIY_OPCOD] = {1, sta_instr, INDERECT_Y},

      [STXZ_OPCOD] = {1, stx_instr, ZERO_PAGE},
      [STXZY_OPCOD] = {1, stx_instr, ZERO_PAGE_Y},
      [STXA_OPCOD] = {2, stx_instr, ABSOLUTE},

      [STYZ_OPCOD] = {1, sty_instr, ZERO_PAGE},
      [STYZX_OPCOD] = {1, sty_instr, ZERO_PAGE_X},
      [STYA_OPCOD] = {2, sty_instr, ABSOLUTE},

      /* ========== AND ========== */
      [ANDI_OPCOD] = {1, and_instr, IMMEDIATE},
      [ANDZ_OPCOD] = {1, and_instr, ZERO_PAGE},
      [ANDZX_OPCOD] = {1, and_instr, ZERO_PAGE_X},
      [ANDA_OPCOD] = {2, and_instr, ABSOLUTE},
      [ANDAX_OPCOD] = {2, and_instr, ABSOLUTE_X},
      [ANDAY_OPCOD] = {2, and_instr, ABSOLUTE_Y},

      /* ========== ORA ========== */
      [ORAI_OPCOD] = {1, ora_instr, IMMEDIATE},
      [ORAZ_OPCOD] = {1, ora_instr, ZERO_PAGE},
      [ORAZX_OPCOD] = {1, ora_instr, ZERO_PAGE_X},
      [ORAA_OPCOD] = {2, ora_instr, ABSOLUTE},
      [ORAAX_OPCOD] = {2, ora_instr, ABSOLUTE_X},
      [ORAAY_OPCOD] = {2, ora_instr, ABSOLUTE_Y},
      [ORAIX_OPCOD] = {1, ora_instr, INDIRECT_X},
      [ORAIY_OPCOD] = {1, ora_instr, INDERECT_Y},

      /* ========== EOR ========== */
      [EORI_OPCOD] = {1, eor_instr, IMMEDIATE},
      [EORZ_OPCOD] = {1, eor_instr, ZERO_PAGE},
      [EORZX_OPCOD] = {1, eor_instr, ZERO_PAGE_X},
      [EORA_OPCOD] = {2, eor_instr, ABSOLUTE},
      [EORAX_OPCOD] = {2, eor_instr, ABSOLUTE_X},
      [EORAY_OPCOD] = {2, eor_instr, ABSOLUTE_Y},
      [EORIX_OPCOD] = {1, eor_instr, INDIRECT_X},
      [EORIY_OPCOD] = {1, eor_instr, INDERECT_Y},

      /* ========== ADC ========== */
      [ADDI_OPCOD] = {1, adc_instr, IMMEDIATE},
      [ADDZ_OPCOD] = {1, adc_instr, ZERO_PAGE},
      [ADDZX_OPCOD] = {1, adc_instr, ZERO_PAGE_X},
      [ADDA_OPCOD] = {2, adc_instr, ABSOLUTE},
      [ADDAX_OPCOD] = {2, adc_instr, ABSOLUTE_X},
      [ADDAY_OPCOD] = {2, adc_instr, ABSOLUTE_Y},

      /* ========== CMP/CPX/CPY ========== */
      [CMPI_OPCOD] = {1, cmp_instr, IMMEDIATE},
      [CMPZ_OPCOD] = {1, cmp_instr, ZERO_PAGE},
      [CMPA_OPCOD] = {2, cmp_instr, ABSOLUTE},
      [CMPAX_OPCOD] = {2, cmp_instr, ABSOLUTE_X},
      [CMPAY_OPCOD] = {2, cmp_instr, ABSOLUTE_Y},
      [CMPIX_OPCOD] = {1, cmp_instr, INDIRECT_X},
      [CMPIY_OPCOD] = {1, cmp_instr, INDERECT_Y},

      [CPXI_OPCOD] = {1, cpx_instr, IMMEDIATE},
      [CPXZ_OPCOD] = {1, cpx_instr, ZERO_PAGE},
      [CPXA_OPCOD] = {2, cpx_instr, ABSOLUTE},

      [CPYI_OPCOD] = {1, cpy_instr, IMMEDIATE},
      [CPYZ_OPCOD] = {1, cpy_instr, ZERO_PAGE},
      [CPYA_OPCOD] = {2, cpy_instr, ABSOLUTE},

      /* ========== INC / DEC ========== */
      [INCZ_OPCOD] = {1, inc_instr, ZERO_PAGE},
      [INCZX_OPCOD] = {1, inc_instr, ZERO_PAGE_X},
      [INCA_OPCOD] = {2, inc_instr, ABSOLUTE},
      [INCAX_OPCOD] = {2, inc_instr, ABSOLUTE_X},

      [DECZ_OPCOD] = {1, dec_instr, ZERO_PAGE},
      [DECZX_OPCOD] = {1, dec_instr, ZERO_PAGE_X},
      [DECA_OPCOD] = {2, dec_instr, ABSOLUTE},
      [DECAX_OPCOD] = {2, dec_instr, ABSOLUTE_X},

      [INX_OPCOD] = {0, inx_instr, IMPLIED},
      [INY_OPCOD] = {0, iny_instr, IMPLIED},
      [DEX_OPCOD] = {0, dex_instr, IMPLIED},
      [DEY_OPCOD] = {0, dey_instr, IMPLIED},

      /* ========== BIT ========== */
      [BITZ_OPCOD] = {1, bit_instr, ZERO_PAGE},
      [BITA_OPCOD] = {2, bit_instr, ABSOLUTE},

      /* ========== SHIFT ========== */
      [ASLAC_OPCOD] = {0, asl_instr, ACCUMULATOR},
      [ASLZ_OPCOD] = {1, asl_instr, ZERO_PAGE},
      [ASLZX_OPCOD] = {1, asl_instr, ZERO_PAGE_X},
      [ASLA_OCCOD] = {2, asl_instr, ABSOLUTE},
      [ASLAX_OPCOD] = {2, asl_instr, ABSOLUTE_X},

      [LSRAC_OPCOD] = {0, lsr_instr, ACCUMULATOR},
      [LSRZ_OPCOD] = {1, lsr_instr, ZERO_PAGE},
      [LSRZX_OPCOD] = {1, lsr_instr, ZERO_PAGE_X},
      [LSRA_OPCOD] = {2, lsr_instr, ABSOLUTE},
      [LSRAX_OPCOD] = {2, lsr_instr, ABSOLUTE_X},

      /* ========== STACK ========== */
      [PHA_OPCOD] = {0, pha_instr, IMPLIED},
      [PHP_OPCOD] = {0, php_instr, IMPLIED},
      [PLA_OPCOD] = {0, pla_instr, IMPLIED},
      [PLP_OPCOD] = {0, plp_instr, IMPLIED},

      /* ========== JUMP ========== */
      [JMPA_OPCOD] = {2, jmp_instr, ABSOLUTE},
      [JMPI_OPCOD] = {2, jmp_instr, INDIRECT},
      [JSR_OPCOD] = {2, jsr_instr, ABSOLUTE},
      [RTS_OPCOD] = {0, rts_instr, IMPLIED},

      /* ========== BRANCH ========== */
      [BCC_OPCOD] = {1, bcc_instr, RELATIVE},
      [BCS_OPCOD] = {1, bcs_instr, RELATIVE},
      [BEQ_OPCOD] = {1, beq_instr, RELATIVE},
      [BNE_OPCOD] = {1, bne_instr, RELATIVE},
      [BPL_OPCOD] = {1, bpl_instr, RELATIVE},
      [BMI_OPCOD] = {1, bmi_instr, RELATIVE},
      [BVC_OPCOD] = {1, bvc_instr, RELATIVE},
      [BVS_OPCOD] = {1, bvs_instr, RELATIVE},

      /* ========== FLAGS ========== */
      [CLC_OPCOD] = {0, clc_instr, IMPLIED},
      [CLD_OPCOD] = {0, cld_instr, IMPLIED},
      [CLI_OPCOD] = {0, cli_instr, IMPLIED},
      [CLV_OPCOD] = {0, clv_instr, IMPLIED},
      [SEC_OPCOD] = {0, sec_instr, IMPLIED},
      [SED_OPDCOD] = {0, sed_instr, IMPLIED},
      [SEI_OPCOD] = {0, sei_instr, IMPLIED},

      /* ========== TRANSFER ========== */
      [TAX_OPCOD] = {0, tax_instr, IMPLIED},
      [TAY_OPCOD] = {0, tay_instr, IMPLIED},
      [TXA_OPCOD] = {0, txa_instr, IMPLIED},
      [TYA_OPCOD] = {0, tya_instr, IMPLIED},
      [TSX_OPCOD] = {0, tsx_instr, IMPLIED},
      [TXS_OPCOD] = {0, txs_instr, IMPLIED},
  };

  return &opcodes_data_table[opcode];
}

void load_register_instr(cpu_t *self, byte_t *register_ptr, const addressing_mode_e mode, const memory_t *memory) {
  emu_log(INFO, "Load to register;\n");
  bool suc = true;
  bool is_address = true;

  const word_t first_operand = cpu_resolve_first_operand(self, memory, mode, &is_address);

  byte_t value = 0;

  if (is_address) {
    value = memory_read(memory, first_operand, &suc);
  } else {
    value = first_operand;
  }

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  cpu_update_zero_and_negative_flags(self, value);
  *register_ptr = value;
}

void and_with_accumulator_instr(cpu_t *self, const memory_t *memory, const addressing_mode_e mode) {
  emu_log(INFO, "Logical AND with accumulator;\n");
  bool suc = true;
  bool is_address = true;

  const word_t operand = cpu_resolve_first_operand(self, memory, mode, &is_address);

  byte_t value = 0;

  if (is_address) {
    value = memory_read(memory, operand, &suc);
  } else {
    value = operand;
  }

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  self->reg_A &= value;

  cpu_update_zero_and_negative_flags(self, self->reg_A);
}

void or_with_accumulator_instr(cpu_t *self, const memory_t *memory, addressing_mode_e mode) {
  emu_log(INFO, "Logical OR with accumulator;\n");
  bool suc = true;
  bool is_address = true;

  const word_t operand = cpu_resolve_first_operand(self, memory, mode, &is_address);

  byte_t value = 0;

  if (is_address) {
    value = memory_read(memory, operand, &suc);
  } else {
    value = operand;
  }

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  self->reg_A |= value;

  cpu_update_zero_and_negative_flags(self, self->reg_A);
}

void store_register_instr(const cpu_t *self, const byte_t register_value, memory_t *memory,
                          const addressing_mode_e mode) {
  emu_log(INFO, "Store register;\n");

  const word_t address = cpu_resolve_first_operand(self, memory, mode, NULL);

  memory_write(memory, address, register_value);
}

void add_to_accumulator_instr(cpu_t *self, const memory_t *memory, const addressing_mode_e mode) {
  emu_log(INFO, "Add to accumulator;\n");

  bool suc = true;
  bool is_address = true;
  word_t value = cpu_resolve_first_operand(self, memory, mode, &is_address);

  if (is_address) {
    value = memory_read(memory, value, &suc);
  }

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  const byte_t carry = cpu_status_flag_is_set(self, CARRY) ? 1 : 0;
  const word_t result = value + self->reg_A + carry;

  if (result > 0xFF) {
    cpu_status_flag_set(self, CARRY);
  } else {
    cpu_status_flag_clear(self, CARRY);
  }

  self->reg_A = (byte_t)result;
  cpu_dump(self, stdout);
  cpu_update_zero_and_negative_flags(self, self->reg_A);
}

#define STACK_LOWEST_ADDRESS 0x0100

void push_onto_stack_instr(cpu_t *self, memory_t *memory, const byte_t value) {
  emu_log(INFO, "Pushing onto the stack;\n");

  if (self->reg_SP == 0) {
    emu_log(ERROR, "Stack underflow;\n");
    self->last_trap = STACK_OVERFLOW;
    return;
  }

  memory_write(memory, STACK_LOWEST_ADDRESS + self->reg_SP--, value);
}

byte_t pull_from_stack_instr(cpu_t *self, const memory_t *memory) {
  emu_log(INFO, "Pulling from the stack;\n");

  bool suc = true;

  if (self->reg_SP == 0xFF) {
    emu_log(ERROR, "Stack underflow;\n");
    self->last_trap = STACK_UNDERFLOW;
    return 0;
  }

  const byte_t value = memory_read(memory, STACK_LOWEST_ADDRESS + ++self->reg_SP, &suc);

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return 0;
  }

  return value;
}

void jump_subroutine_instr(cpu_t *self, memory_t *memory) {
  emu_log(INFO, "Jumping to subroutine;\n");

  const word_t jumping_address = cpu_resolve_first_operand(self, memory, ABSOLUTE, NULL);

  emu_log(INFO, "Jumping address: %x;\n", jumping_address);

  const word_t pushing_address =
      self->reg_IP - 1;  // address of the next instruction to execute after the subroutine call

  push_onto_stack_instr(self, memory, (pushing_address >> 8) & 0xFF);  // high
  push_onto_stack_instr(self, memory, pushing_address & 0xFF);         // low

  self->reg_IP = jumping_address;
}

void return_from_subroutine_instr(cpu_t *self, const memory_t *memory) {
  emu_log(INFO, "Return from subroutine;\n");

  word_t return_address = pull_from_stack_instr(self, memory);
  return_address += (pull_from_stack_instr(self, memory) << 8);
  ++return_address;

  emu_log(INFO, "Return address: %x;\n", return_address);

  self->reg_IP = return_address;
}

void jump_instr(cpu_t *self, const addressing_mode_e mode, const memory_t *memory) {
  emu_log(INFO, "Jump;\n");

  const word_t address = cpu_resolve_first_operand(self, memory, mode, NULL);

  self->reg_IP = address;
}

byte_t cpu_fetch(cpu_t *self, const memory_t *memory, bool *success) {
  return memory_read(memory, self->reg_IP++, success);
}

void branch_on_flag_instr(cpu_t *self, const byte_t mask, const bool branch_if_set) {
  emu_log(INFO, "Branching;\n");

  const byte_t offset = cpu_resolve_first_operand(self, NULL, RELATIVE, NULL);

  const bool flag_is_set = cpu_status_flag_is_set(self, mask);

  if ((branch_if_set && flag_is_set) || (!branch_if_set && !flag_is_set)) {
    emu_log(INFO, "Condition is satisfied, jump;\n");
    self->reg_IP += (int8_t)offset;
  } else {
    emu_log(INFO, "Condition is not satisfied;\n");
  }
}

void compare_instr(cpu_t *self, const memory_t *memory, const byte_t register_value, const addressing_mode_e mode) {
  emu_log(INFO, "Comparing;\n");

  bool suc = true;
  bool is_address = true;
  word_t value = cpu_resolve_first_operand(self, memory, mode, &is_address);

  if (is_address) {
    value = memory_read(memory, value, &suc);
  }

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  const byte_t result = (byte_t)(register_value - (byte_t)value);
  cpu_update_zero_and_negative_flags(self, result);

  if (register_value >= value) {
    cpu_status_flag_set(self, CARRY);
  } else {
    cpu_status_flag_clear(self, CARRY);
  }
}

void dec_memory_instr(cpu_t *self, memory_t *memory, const addressing_mode_e mode) {
  emu_log(INFO, "Decrement memory;\n");

  bool suc = true;

  const word_t address = cpu_resolve_first_operand(self, memory, mode, NULL);

  emu_log(INFO, "Decrementing address: %x\n", address);

  const byte_t value = memory_read(memory, address, &suc) - 1;

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  cpu_update_zero_and_negative_flags(self, value);

  memory_write(memory, address, value);
}

void dec_register_instr(cpu_t *self, byte_t *register_ptr) {
  emu_log(INFO, "Decrement register;\n");
  --(*register_ptr);
  cpu_update_zero_and_negative_flags(self, *register_ptr);
}

void inc_register_instr(cpu_t *self, byte_t *register_ptr) {
  emu_log(INFO, "Increment register;\n");
  ++(*register_ptr);
  cpu_update_zero_and_negative_flags(self, *register_ptr);
}

void inc_memory_instr(cpu_t *self, memory_t *memory, const addressing_mode_e mode) {
  emu_log(INFO, "Increment memory;\n");

  const word_t address = cpu_resolve_first_operand(self, memory, mode, NULL);

  emu_log(INFO, "Incrementing address: %x\n", address);

  bool suc = true;
  const byte_t value = memory_read(memory, address, &suc) + 1;

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  cpu_update_zero_and_negative_flags(self, value + 1);

  memory_write(memory, address, value);
}

void test_bit_instr(cpu_t *self, const memory_t *memory, const addressing_mode_e mode) {
  emu_log(INFO, "Test bit;\n");

  bool suc = true;

  const word_t address = cpu_resolve_first_operand(self, memory, mode, NULL);

  const byte_t value = memory_read(memory, address, &suc);

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  const byte_t result = self->reg_A & value;

  if (result == 0) {
    cpu_status_flag_set(self, ZERO);
  } else {
    cpu_status_flag_clear(self, ZERO);
  }

#define NEGATIVE_MASK 0x80

  if (value & NEGATIVE_MASK) {
    cpu_status_flag_set(self, ZERO);
  } else {
    cpu_status_flag_clear(self, ZERO);
  }

#define OVERFLOW_MASK 0x40

  if (value & OVERFLOW_MASK) {
    cpu_status_flag_set(self, OVERFLOW_);
  } else {
    cpu_status_flag_clear(self, OVERFLOW_);
  }
}

void exclusive_or_instr(cpu_t *self, const memory_t *memory, addressing_mode_e mode) {
  emu_log(INFO, "Exclusive OR;\n");
  bool is_address = true, suc = true;

  byte_t value = cpu_resolve_first_operand(self, memory, mode, &is_address);

  if (is_address) {
    value = memory_read(memory, value, &suc);
  }

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  self->reg_A ^= value;
  cpu_update_zero_and_negative_flags(self, self->reg_A);
}

void transfer_regs_instr(cpu_t *self, const byte_t *from, byte_t *to) {
  emu_log(INFO, "Transferring registers;\n");

  if (from == NULL || to == NULL) {
    return;
  }

  *to = *from;
  cpu_update_zero_and_negative_flags(self, *to);
}

void logical_shift_right_instr(cpu_t *self, memory_t *memory, const addressing_mode_e mode) {
  emu_log(INFO, "Logical shift right;\n");
  bool is_address = true, suc = true;

  word_t first_operand = cpu_resolve_first_operand(self, memory, mode, &is_address);
  byte_t value = 0;

  if (is_address) {
    value = memory_read(memory, first_operand, &suc);
  } else {
    value = self->reg_A;
  }

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  if (value & 0x01) {
    cpu_status_flag_set(self, CARRY);
  } else {
    cpu_status_flag_clear(self, CARRY);
  }

  const byte_t result = value >> 1;
  cpu_update_zero_and_negative_flags(self, result);

  if (is_address) {
    memory_write(memory, first_operand, result);
  } else {
    self->reg_A = result;
  }
}

void arithmetic_shift_left(cpu_t *self, memory_t *memory, const addressing_mode_e mode) {
  emu_log(INFO, "Arithmetci shift left;\n");
  bool is_address = true, suc = true;

  word_t first_operand = cpu_resolve_first_operand(self, memory, mode, &is_address);
  byte_t value = 0;

  if (is_address) {
    value = memory_read(memory, first_operand, &suc);
  } else {
    value = self->reg_A;
  }

  if (!suc) {
    self->last_trap = SEGMENTATION_FAULT;
    return;
  }

  if (value & NEGATIVE_MASK) {
    cpu_status_flag_set(self, CARRY);
  } else {
    cpu_status_flag_clear(self, CARRY);
  }

  const byte_t result = value << 1;
  cpu_update_zero_and_negative_flags(self, result);

  if (is_address) {
    memory_write(memory, first_operand, result);
  } else {
    self->reg_A = result;
  }
}

void cpu_dump_one_flag(const cpu_t *self, const char *flag_name, const flag_e flag, FILE *stream) {
  if (cpu_status_flag_is_set(self, flag)) {
    fprintf(stream, "\t%s flag is set\n", flag_name);
  } else {
    fprintf(stream, "\t%s flag is clear\n", flag_name);
  }
}

void cpu_dump_processor_status(const cpu_t *self, FILE *stream) {
  switch (g_log_level) {
    case NO_LOG:
    case ERROR:
    case WARN:
    default:
      return;
    case INFO:
      break;
  }

  cpu_dump_one_flag(self, "Carry", CARRY, stream);
  cpu_dump_one_flag(self, "Zero", ZERO, stream);
  cpu_dump_one_flag(self, "Interrupt", INTERRUPT, stream);
  cpu_dump_one_flag(self, "Decimal", DECIMAL, stream);
  cpu_dump_one_flag(self, "Break", BREAK, stream);
  cpu_dump_one_flag(self, "Overflow", OVERFLOW_, stream);
  cpu_dump_one_flag(self, "Negative", NEGATIVE, stream);
}

void cpu_dump(const cpu_t *self, FILE *stream) {
  switch (g_log_level) {
    case NO_LOG:
    case ERROR:
    case WARN:
    default:
      return;
    case INFO:
      break;
  }

  fputs("======= Dumping CPU =======\n", stream);

  fprintf(stream, "\tProgram counter (instruction pointer): %x\n", self->reg_IP);
  fprintf(stream, "\tStack pointer: %x\n", self->reg_SP);
  fprintf(stream, "\tInstruction register: %x\n", self->reg_IR);
  fprintf(stream, "\tAccumulator: %x\n", self->reg_A);
  fprintf(stream, "\tIndex register X: %x\n", self->reg_X);
  fprintf(stream, "\tIndex register Y: %x\n", self->reg_Y);
  cpu_dump_processor_status(self, stream);

  fputs("===== End dumping CPU =====\n", stream);
}
