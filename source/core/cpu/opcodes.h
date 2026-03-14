#pragma once

typedef enum {
  NOP_OPCOD = 0xEA,

  LDAI_OPCOD = 0xA9,   // move to register A an immediate value
  LDAZ_OPCOD = 0xA5,   // move to register A a value from zero page address
  LDAZX_OPCOD = 0xB5,  // move to register A a value from zero page address + X
  LDAA_OPCOD = 0xAD,   // move to register A a value from absolute address
  LDAAX_OPCOD = 0xBD,  // move to register A a value from absolute address + X
  LDAAY_OPCOD = 0xB9,  // move to register A a value from absolute address + X

  LDAIX_OPCOD = 0xA1,
  LDAIY_OPCOD = 0xB1,

  LDXI_OPCOD = 0xA2,   // move to register X an immediate value
  LDXZ_OPCOD = 0xA6,   // move to register X a value from zero page address
  LDXZY_OPCOD = 0xB6,  // move to register X a value from zero page address + Y
  LDXA_OPCOD = 0xAE,   // move to register X a value from absolute address
  LDXAY_OPCOD = 0xBE,  // move to register X a value from absolute address + Y

  LDYI_OPCOD = 0xA0,   // move to register Y an immediate value
  LDYZ_OPCOD = 0xA4,   // move to register Y a value from zero page address
  LDYZX_OPCOD = 0xB4,  // move to register Y a value from zero page address + X
  LDYA_OPCOD = 0xAC,   // move to register Y a value from absolute address
  LDYAX_OPCOD = 0xBC,  // move to register Y a value from absolute address + X

  STAZ_OPCOD = 0x85,   // store register A to address in zero page address
  STAZX_OPCOD = 0x95,  // store register A to address in zero page address + X
  STAA_OPCOD = 0x8D,   // store register A to address in absolute address
  STAAX_OPCOD = 0x9D,  // store register A to address in absolute address + X
  STAAY_OPCOD = 0x99,  // store register A to address in absolute address + Y

  STXZ_OPCOD = 0x86,   // store register X to address in zero page address
  STXZY_OPCOD = 0x96,  // store register X to address in zero page address + Y
  STXA_OPCOD = 0x8E,   // store register X to address in absolute address

  STYZ_OPCOD = 0x84,   // store register Y to address in zero page address
  STYZX_OPCOD = 0x94,  // store register Y to address in zero page address + X
  STYA_OPCOD = 0x8C,   // store register Y to address in absolute address

  STAIX_OPCOD = 0x81,
  STAIY_OPCOD = 0x91,

  ANDI_OPCOD = 0x29,   // logical and A with immediate
  ANDZ_OPCOD = 0x25,   // logical and A with a value from zero page address
  ANDZX_OPCOD = 0x35,  // logical and A with a value from zero page address + X
  ANDA_OPCOD = 0x2D,   // logical and A with a value from absolute address
  ANDAX_OPCOD = 0x3D,  // logical and A with a value from absolute address + X
  ANDAY_OPCOD = 0x39,  // logical and A with a value from absolute addres + Y

  JSR_OPCOD = 0x20,  // jump to subroutine (with saving return address)
  RTS_OPCOD = 0x60,  // return from subroutine

  ADDI_OPCOD = 0x69,  // add an immediate to A register
  ADDZ_OPCOD = 0x65,
  ADDZX_OPCOD = 0x75,
  ADDA_OPCOD = 0x6D,
  ADDAX_OPCOD = 0x7D,
  ADDAY_OPCOD = 0x79,

  JMPA_OPCOD = 0x4C,  // jump to an address

  BCC_OPCOD = 0x90,  // Branch if carry clear
  BCS_OPCOD = 0xB0,  // Branch if carry set
  BEQ_OPCOD = 0xF0,  // Branch if zero set
  BNE_OPCOD = 0xD0,  // Branch if zero clear
  BPL_OPCOD = 0x10,  // Branch if negative clear
  BMI_OPCOD = 0x30,  // Branch if negative set
  BVC_OPCOD = 0x50,  // Branch if overflow clear
  BVS_OPCOD = 0x70,  // Branch if overflow set

  CMPI_OPCOD = 0xC9,   // Compare accumulator with immediate
  CMPZ_OPCOD = 0xC5,   // Compare accumulator with zero page
  CMPZX_OPCOD = 0xD5,  // Compare accumulator with zero page
  CMPA_OPCOD = 0xCD,   // Compare accumulator with absolute
  CMPAY_OPCOD = 0xD9,
  CMPAX_OPCOD = 0xDD,
  CMPIX_OPCOD = 0xC1,
  CMPIY_OPCOD = 0xD1,

  CPXI_OPCOD = 0xE0,  // Compare X with immediate
  CPXZ_OPCOD = 0xE4,  // Compare X with zero page
  CPXA_OPCOD = 0xEC,  // Compare X with absolute

  CPYI_OPCOD = 0xC0,  // Compare Y with immediate
  CPYZ_OPCOD = 0xC4,  // Compare Y with zero page
  CPYA_OPCOD = 0xCC,  // Compare Y with absolute

  CLC_OPCOD = 0x18,  // clear carry
  CLD_OPCOD = 0xD8,  // clear decimal mode
  CLI_OPCOD = 0x58,  // clear interrupt disable
  CLV_OPCOD = 0xB8,  // clear overflow

  DECZ_OPCOD = 0xC6,
  DECZX_OPCOD = 0xD6,
  DECA_OPCOD = 0xCE,
  DECAX_OPCOD = 0xDE,

  DEX_OPCOD = 0xCA,
  DEY_OPCOD = 0x88,

  BITZ_OPCOD = 0x24,
  BITA_OPCOD = 0x2c,

  INCZ_OPCOD = 0xE6,
  INCZX_OPCOD = 0xF6,
  INCA_OPCOD = 0xEE,
  INCAX_OPCOD = 0xFE,

  INX_OPCOD = 0xE8,
  INY_OPCOD = 0xC8,

  SEC_OPCOD = 0x38,
  SED_OPDCOD = 0xF8,
  SEI_OPCOD = 0x78,

  TAX_OPCOD = 0xAA,
  TAY_OPCOD = 0xA8,
  TXA_OPCOD = 0x8A,
  TYA_OPCOD = 0x98,
  TSX_OPCOD = 0xBA,
  TXS_OPCOD = 0x9A,

  JMPI_OPCOD = 0x6C,

  EORI_OPCOD = 0x49,
  EORZ_OPCOD = 0x45,
  EORZX_OPCOD = 0x55,
  EORA_OPCOD = 0x4D,
  EORAX_OPCOD = 0x5D,
  EORAY_OPCOD = 0x59,
  EORIX_OPCOD = 0x41,
  EORIY_OPCOD = 0x51,

  LSRAC_OPCOD = 0x4A,
  LSRZ_OPCOD = 0x46,
  LSRZX_OPCOD = 0x56,
  LSRA_OPCOD = 0x4E,
  LSRAX_OPCOD = 0x5E,

  PHA_OPCOD = 0x48,
  PHP_OPCOD = 0x08,
  PLA_OPCOD = 0x68,
  PLP_OPCOD = 0x28,

  ORAI_OPCOD = 0x09,
  ORAZ_OPCOD = 0x05,
  ORAZX_OPCOD = 0x15,
  ORAA_OPCOD = 0x0D,
  ORAAX_OPCOD = 0x1D,
  ORAAY_OPCOD = 0x19,
  ORAIX_OPCOD = 0x01,
  ORAIY_OPCOD = 0x11,

  ASLAC_OPCOD = 0x0A,
  ASLZ_OPCOD = 0x06,
  ASLZX_OPCOD = 0x16,
  ASLA_OCCOD = 0x0E,
  ASLAX_OPCOD = 0x1E,

  // Undocumented opcodes:
  SLOZ_OPCOD = 0x07,
  SLOZX_OPCOD = 0x17,
  SLOA_OPCOD = 0x0F,
  SLOAX_OPCOD = 0x1F,
  SLOAY_OPCOD = 0x1B,
  SLOIX_OPCOD = 0x03,
  SLOIY_OPCOD = 0x13,

  SUBI_OPCOD = 0xE9,
  SUBZ_OPCOD = 0xE5,
  SUBZX_OPCOD = 0xF5,
  SUBA_OPCOD = 0xED,
  SUBAX_OPCOD = 0xFD,
  SUBAY_OPCOD = 0xF9,
  SUBIX_OPCOD = 0xE1,
  SUBIY_OPCOD = 0xF1,

  ROLAC_OPCOD = 0x2A,
  ROLZ_OPCOD = 0x26,
  ROLZX_OPCOD = 0x36,
  ROLA_OPCOD = 0x2E,
  ROLAX_OPCOD = 0x3E,

  RORAC_OPCOD = 0x6A,
  RORZ_OPCOD = 0x66,
  RORZX_OPCOD = 0x76,
  RORA_OPCOD = 0x6E,
  RORAX_OPCOD = 0x7E,
} opcode_e;
