#include "arithmetic.h"
#include "branch.h"
#include "compare.h"
#include "cpu.h"
#include "flags.h"
#include "instructions_data.h"
#include "load_store.h"
#include "log.h"
#include "logical.h"
#include "opcodes.h"
#include "shifts.h"
#include "stack.h"
#include "transfer.h"

void jump_instr(const instr_context_t *context) {
  emu_log(INFO, "Jump;\n");

  const word_t address = cpu_resolve_first_operand(context->cpu, context->memory, context->mode, NULL);
  emu_log(INFO, "Jumping address: %x\n", address);

  context->cpu->reg_IP = address;
}

void nop_instr(const instr_context_t *context) {
  emu_log(INFO, "No operation;\n");
}

#define OPCODES_COUNT 256

const opcode_data_t *get_opcode_data(const opcode_e opcode) {
  if (opcode >= OPCODES_COUNT) {
    return NULL;
  }

  static const opcode_data_t opcodes_data_table[OPCODES_COUNT] = {
      /* ========== NOP ========== */
      [NOP_OPCOD] = {0, nop_instr, IMPLIED},

      /* ========== LDA ========== */
      [LDAI_OPCOD] = {1, load_a_instr, IMMEDIATE},
      [LDAZ_OPCOD] = {1, load_a_instr, ZERO_PAGE},
      [LDAZX_OPCOD] = {1, load_a_instr, ZERO_PAGE_X},
      [LDAA_OPCOD] = {2, load_a_instr, ABSOLUTE},
      [LDAAX_OPCOD] = {2, load_a_instr, ABSOLUTE_X},
      [LDAAY_OPCOD] = {2, load_a_instr, ABSOLUTE_Y},
      [LDAIX_OPCOD] = {1, load_a_instr, INDIRECT_X},
      [LDAIY_OPCOD] = {1, load_a_instr, INDIRECT_Y},

      /* ========== LDX ========== */
      [LDXI_OPCOD] = {1, load_x_instr, IMMEDIATE},
      [LDXZ_OPCOD] = {1, load_x_instr, ZERO_PAGE},
      [LDXZY_OPCOD] = {1, load_x_instr, ZERO_PAGE_Y},
      [LDXA_OPCOD] = {2, load_x_instr, ABSOLUTE},
      [LDXAY_OPCOD] = {2, load_x_instr, ABSOLUTE_Y},

      /* ========== LDY ========== */
      [LDYI_OPCOD] = {1, load_y_instr, IMMEDIATE},
      [LDYZ_OPCOD] = {1, load_y_instr, ZERO_PAGE},
      [LDYZX_OPCOD] = {1, load_y_instr, ZERO_PAGE_X},
      [LDYA_OPCOD] = {2, load_y_instr, ABSOLUTE},
      [LDYAX_OPCOD] = {2, load_y_instr, ABSOLUTE_X},

      /* ========== STA/STX/STY ========== */
      [STAZ_OPCOD] = {1, store_a_instr, ZERO_PAGE},
      [STAZX_OPCOD] = {1, store_a_instr, ZERO_PAGE_X},
      [STAA_OPCOD] = {2, store_a_instr, ABSOLUTE},
      [STAAX_OPCOD] = {2, store_a_instr, ABSOLUTE_X},
      [STAAY_OPCOD] = {2, store_a_instr, ABSOLUTE_Y},
      [STAIX_OPCOD] = {1, store_a_instr, INDIRECT_X},
      [STAIY_OPCOD] = {1, store_a_instr, INDIRECT_Y},

      [STXZ_OPCOD] = {1, store_x_instr, ZERO_PAGE},
      [STXZY_OPCOD] = {1, store_x_instr, ZERO_PAGE_Y},
      [STXA_OPCOD] = {2, store_x_instr, ABSOLUTE},

      [STYZ_OPCOD] = {1, store_y_instr, ZERO_PAGE},
      [STYZX_OPCOD] = {1, store_y_instr, ZERO_PAGE_X},
      [STYA_OPCOD] = {2, store_y_instr, ABSOLUTE},

      /* ========== AND ========== */
      [ANDI_OPCOD] = {1, and_with_accumulator_instr, IMMEDIATE},
      [ANDZ_OPCOD] = {1, and_with_accumulator_instr, ZERO_PAGE},
      [ANDZX_OPCOD] = {1, and_with_accumulator_instr, ZERO_PAGE_X},
      [ANDA_OPCOD] = {2, and_with_accumulator_instr, ABSOLUTE},
      [ANDAX_OPCOD] = {2, and_with_accumulator_instr, ABSOLUTE_X},
      [ANDAY_OPCOD] = {2, and_with_accumulator_instr, ABSOLUTE_Y},

      /* ========== ORA ========== */
      [ORAI_OPCOD] = {1, or_with_accumulator_instr, IMMEDIATE},
      [ORAZ_OPCOD] = {1, or_with_accumulator_instr, ZERO_PAGE},
      [ORAZX_OPCOD] = {1, or_with_accumulator_instr, ZERO_PAGE_X},
      [ORAA_OPCOD] = {2, or_with_accumulator_instr, ABSOLUTE},
      [ORAAX_OPCOD] = {2, or_with_accumulator_instr, ABSOLUTE_X},
      [ORAAY_OPCOD] = {2, or_with_accumulator_instr, ABSOLUTE_Y},
      [ORAIX_OPCOD] = {1, or_with_accumulator_instr, INDIRECT_X},
      [ORAIY_OPCOD] = {1, or_with_accumulator_instr, INDIRECT_Y},

      /* ========== EOR ========== */
      [EORI_OPCOD] = {1, exclusive_or_instr, IMMEDIATE},
      [EORZ_OPCOD] = {1, exclusive_or_instr, ZERO_PAGE},
      [EORZX_OPCOD] = {1, exclusive_or_instr, ZERO_PAGE_X},
      [EORA_OPCOD] = {2, exclusive_or_instr, ABSOLUTE},
      [EORAX_OPCOD] = {2, exclusive_or_instr, ABSOLUTE_X},
      [EORAY_OPCOD] = {2, exclusive_or_instr, ABSOLUTE_Y},
      [EORIX_OPCOD] = {1, exclusive_or_instr, INDIRECT_X},
      [EORIY_OPCOD] = {1, exclusive_or_instr, INDIRECT_Y},

      /* ========== ADC ========== */
      [ADDI_OPCOD] = {1, add_to_accumulator_instr, IMMEDIATE},
      [ADDZ_OPCOD] = {1, add_to_accumulator_instr, ZERO_PAGE},
      [ADDZX_OPCOD] = {1, add_to_accumulator_instr, ZERO_PAGE_X},
      [ADDA_OPCOD] = {2, add_to_accumulator_instr, ABSOLUTE},
      [ADDAX_OPCOD] = {2, add_to_accumulator_instr, ABSOLUTE_X},
      [ADDAY_OPCOD] = {2, add_to_accumulator_instr, ABSOLUTE_Y},

      /* ========== SBC ========== */
      [SUBI_OPCOD] = {1, sub_from_accumulator_instr, IMMEDIATE},
      [SUBZ_OPCOD] = {1, sub_from_accumulator_instr, ZERO_PAGE},
      [SUBZX_OPCOD] = {1, sub_from_accumulator_instr, ZERO_PAGE_X},
      [SUBA_OPCOD] = {2, sub_from_accumulator_instr, ABSOLUTE},
      [SUBAX_OPCOD] = {2, sub_from_accumulator_instr, ABSOLUTE_X},
      [SUBAY_OPCOD] = {2, sub_from_accumulator_instr, ABSOLUTE_Y},
      [SUBIX_OPCOD] = {1, sub_from_accumulator_instr, INDIRECT_X},
      [SUBIY_OPCOD] = {1, sub_from_accumulator_instr, INDIRECT_Y},

      /* ========== CMP/CPX/CPY ========== */
      [CMPI_OPCOD] = {1, cmp_instr, IMMEDIATE},
      [CMPZ_OPCOD] = {1, cmp_instr, ZERO_PAGE},
      [CMPZX_OPCOD] = {1, cmp_instr, ZERO_PAGE_X},
      [CMPA_OPCOD] = {2, cmp_instr, ABSOLUTE},
      [CMPAX_OPCOD] = {2, cmp_instr, ABSOLUTE_X},
      [CMPAY_OPCOD] = {2, cmp_instr, ABSOLUTE_Y},
      [CMPIX_OPCOD] = {1, cmp_instr, INDIRECT_X},
      [CMPIY_OPCOD] = {1, cmp_instr, INDIRECT_Y},

      [CPXI_OPCOD] = {1, cpx_instr, IMMEDIATE},
      [CPXZ_OPCOD] = {1, cpx_instr, ZERO_PAGE},
      [CPXA_OPCOD] = {2, cpx_instr, ABSOLUTE},

      [CPYI_OPCOD] = {1, cpy_instr, IMMEDIATE},
      [CPYZ_OPCOD] = {1, cpy_instr, ZERO_PAGE},
      [CPYA_OPCOD] = {2, cpy_instr, ABSOLUTE},

      /* ========== INC / DEC ========== */
      [INCZ_OPCOD] = {1, inc_memory_instr, ZERO_PAGE},
      [INCZX_OPCOD] = {1, inc_memory_instr, ZERO_PAGE_X},
      [INCA_OPCOD] = {2, inc_memory_instr, ABSOLUTE},
      [INCAX_OPCOD] = {2, inc_memory_instr, ABSOLUTE_X},

      [DECZ_OPCOD] = {1, dec_memory_instr, ZERO_PAGE},
      [DECZX_OPCOD] = {1, dec_memory_instr, ZERO_PAGE_X},
      [DECA_OPCOD] = {2, dec_memory_instr, ABSOLUTE},
      [DECAX_OPCOD] = {2, dec_memory_instr, ABSOLUTE_X},

      [INX_OPCOD] = {0, inc_x_instr, IMPLIED},
      [INY_OPCOD] = {0, inc_y_instr, IMPLIED},
      [DEX_OPCOD] = {0, dec_x_instr, IMPLIED},
      [DEY_OPCOD] = {0, dec_y_instr, IMPLIED},

      /* ========== BIT ========== */
      [BITZ_OPCOD] = {1, test_bit_instr, ZERO_PAGE},
      [BITA_OPCOD] = {2, test_bit_instr, ABSOLUTE},

      /* ========== SHIFT ========== */
      [ASLAC_OPCOD] = {0, arithmetic_shift_left, ACCUMULATOR},
      [ASLZ_OPCOD] = {1, arithmetic_shift_left, ZERO_PAGE},
      [ASLZX_OPCOD] = {1, arithmetic_shift_left, ZERO_PAGE_X},
      [ASLA_OCCOD] = {2, arithmetic_shift_left, ABSOLUTE},
      [ASLAX_OPCOD] = {2, arithmetic_shift_left, ABSOLUTE_X},

      [LSRAC_OPCOD] = {0, logical_shift_right_instr, ACCUMULATOR},
      [LSRZ_OPCOD] = {1, logical_shift_right_instr, ZERO_PAGE},
      [LSRZX_OPCOD] = {1, logical_shift_right_instr, ZERO_PAGE_X},
      [LSRA_OPCOD] = {2, logical_shift_right_instr, ABSOLUTE},
      [LSRAX_OPCOD] = {2, logical_shift_right_instr, ABSOLUTE_X},

      [ROLAC_OPCOD] = {0, rotate_left, ACCUMULATOR},
      [ROLZ_OPCOD] = {1, rotate_left, ZERO_PAGE},
      [ROLZX_OPCOD] = {1, rotate_left, ZERO_PAGE_X},
      [ROLA_OPCOD] = {2, rotate_left, ABSOLUTE},
      [ROLAX_OPCOD] = {2, rotate_left, ABSOLUTE_X},

      [RORAC_OPCOD] = {0, rotate_right, ACCUMULATOR},
      [RORZ_OPCOD] = {1, rotate_right, ZERO_PAGE},
      [RORZX_OPCOD] = {1, rotate_right, ZERO_PAGE_X},
      [RORA_OPCOD] = {2, rotate_right, ABSOLUTE},
      [RORAX_OPCOD] = {2, rotate_right, ABSOLUTE_X},

      /* ========== STACK ========== */
      [PHA_OPCOD] = {0, push_a_instr, IMPLIED},
      [PHP_OPCOD] = {0, push_processor_status_instr, IMPLIED},
      [PLA_OPCOD] = {0, pull_a_instr, IMPLIED},
      [PLP_OPCOD] = {0, pull_processor_status_instr, IMPLIED},

      /* ========== JUMP ========== */
      [JMPA_OPCOD] = {2, jump_instr, ABSOLUTE},
      [JMPI_OPCOD] = {2, jump_instr, INDIRECT},
      [JSR_OPCOD] = {2, jump_subroutine_instr, ABSOLUTE},
      [RTS_OPCOD] = {0, return_from_subroutine_instr, IMPLIED},

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
      [TAX_OPCOD] = {0, transfer_a_to_x_instr, IMPLIED},
      [TAY_OPCOD] = {0, transfer_a_to_y_instr, IMPLIED},
      [TXA_OPCOD] = {0, transfer_x_to_a_instr, IMPLIED},
      [TYA_OPCOD] = {0, transfer_y_to_a_instr, IMPLIED},
      [TSX_OPCOD] = {0, transfer_sp_to_x_instr, IMPLIED},
      [TXS_OPCOD] = {0, transfer_x_to_sp_instr, IMPLIED},
  };

  const opcode_data_t data = opcodes_data_table[opcode];

  if (data.addressing_mode == 0 && data.bytes_for_operands == 0 && data.impl == NULL) {
    return NULL;
  }

  return &opcodes_data_table[opcode];
}
