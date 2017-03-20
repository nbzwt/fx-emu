//ePS6800 Emulation Core
#include "cpu.hpp"
#include "mmio.hpp"
#include "rom.hpp"

enum cpu_mode { MODE_SLOW, MODE_FAST, MODE_IDLE, MODE_SLEEP};

uint8_t status; //r0fh
uint32_t pc; //program counter
uint32_t stack[32];//very niubility stack
uint8_t rpt_counter; //rpt counter
enum cpu_mode mode;

void status_zero(uint8_t x) { if (((x)) == 0) status |= 0x04; else status &= (~0x04); }
void status_carry(uint8_t x) { if ((x)) status |= 0x01; else status &= (~0x01); }
void status_ov(uint8_t x)  { if ((x)) status |= 0x08; else status &= (~0x08); }
void status_sge(uint8_t x)  { if ((x)) status |= 0x20; else status &= (~0x20); }
void status_sle(uint8_t x)  { if ((x)) status |= 0x10; else status &= (~0x10); }

#define status_ov_add(op1, op2, result) status_ov(((op1) ^ (result)) & ((op2) ^ (result)) & 0x80)
#define status_ov_sub(op1, op2, reuslt) status_ov(((op1) ^ (op2)) & ((op1) ^ (result)) & 0x80)

void cpu_push(uint32_t dat) {
    uint8_t stkptr;
    stkptr = mmio_read_byte_internal(REG_STKPTR);
    stack[stkptr] = dat;
    if (stkptr < 32)
        stkptr++;
    else 
        stkptr=0;
    mmio_write_byte_internal(REG_STKPTR, stkptr);
}

uint32_t cpu_pop() {
    uint8_t stkptr;
    stkptr = mmio_read_byte_internal(REG_STKPTR);
    if (stkptr > 0)
        stkptr--;
    return stack[stkptr];
}

void cpu_loop(uint32_t count) {
    int32_t cnt;
    uint32_t instr;
    
    cnt = count;
    while ((cnt!=0)&&((mode==MODE_SLOW)||(mode==MODE_FAST))) {
        instr = ((rom_read_word(pc) << 16) | rom_read_word(pc+1));
        printf("PC=%04x, INSTR=%08x\n", pc, instr);
        cpu_interpret_instruction(instr);
        cnt --;
    }
}

void cpu_reset() {
    pc = 0;
    status = 0;
    rpt_counter = 0;
    mode = MODE_FAST;
    mmio_reset();
}

uint8_t alu_add(uint8_t a, uint8_t b) {
    uint8_t result;
    
    result = a + b;
    status_zero(result);
    status_carry(((uint16_t)a+(uint16_t)b)&0x100);
    status_ov_add(a, b, result);
    status_sle(!(result&0x80));//Working weird
    status_sge(result&0x80);
    
    return result;
}

uint8_t alu_adc(uint8_t a, uint8_t b, uint8_t c) {
    uint8_t result;
    
    result = a + b + c;
    status_zero(result);
    status_carry(((uint16_t)a+(uint16_t)b+(uint16_t)c)&0x100);
    status_ov(((uint16_t)a+(uint16_t)b+(uint16_t)c)>0x7F);
    status_sle(!(result&0x80));//Working weird
    status_sge(result&0x80);
    
    return result;
}

uint8_t alu_sub(uint8_t a, uint8_t b) {
    uint8_t result;

    result = a - b;
    status_zero(result);
    status_carry(!(a<b));
    status_ov_sub(a, b, result);
    status_sle(!(result&0x80));//Working weird
    status_sge(result&0x80);

    return result;
}

uint8_t alu_subb(uint8_t a, uint8_t b, uint8_t c) {
    uint8_t result;

    result = a - b - c;
    status_zero(result);
    status_carry(!(a<(b+c)));
    status_ov(((int16_t)a-(int16_t)b-(int16_t)c)<(0-0x7F));
    status_sle(!(result&0x80));//Working weird
    status_sge(result&0x80);

    return result;
}

//BCD Addition
uint8_t alu_adddc(uint8_t a, uint8_t b, uint8_t c) {
    uint8_t a1, b1, s1;

    a1 = ( a >> 4 ) * 10 + ( a & 0xFF );
    b1 = ( b >> 4 ) * 10 + ( b & 0xFF );
    
    s1 = a1 + b1 + c;
    status_carry((s1 > 99));

    return (s1/10) << 4 | (s1%10);
}

uint8_t alu_subdb(uint8_t a, uint8_t b, uint8_t c) {
    uint8_t a1, b1;
    uint8_t s1;

    a1 = ( a >> 4 ) * 10 + ( a & 0xFF );
    b1 = ( b >> 4 ) * 10 + ( b & 0xFF );
    
    s1 = a1 - b1 - c;
    status_carry(!(a1<(b1+c)));

    return (s1/10) << 4 | (s1%10);
}

void cpu_interpret_instruction(uint32_t instr) {
    uint16_t instr1;
    uint16_t imm1, imm2, imm3;
    uint32_t newpc;
    uint8_t  temp8;  //instruction related temp
    uint16_t temp16; //instruction related temp
    uint32_t temp32; //instruction related temp
    

    instr1 = instr >> 16;
    newpc = pc + 1;
    status = mmio_read_byte_internal(REG_STATUS);

    if (rpt_counter != 0) {
        newpc = pc;
        rpt_counter --;
    }

    switch (instr1 & 0xFFFF) {
        case 0x0000: break;  //nop
        case 0x0001: break;  //not implemented
        case 0x0002: temp8 = mmio_read_byte_internal(REG_CPUCON);
                     if (temp8 & 0x02) mode = MODE_IDLE;
                     else mode = MODE_SLEEP;
                     break;  //slep
        case 0x2BFE: temp8 = mmio_read_byte_internal(REG_CPUCON);
                     temp8 |= 0x04; 
                     mmio_write_byte_internal(REG_CPUCON, temp8);
                     //reti, enable interrupt and run through
        case 0x2BFF: newpc = cpu_pop();
                     break;//ret
        default: //no match
        switch (instr1 & 0xFFFE) {
            case 0x0030: cpu_push(newpc); //run through
            case 0x0020: imm1 = instr & 0x0001FFFF;
                         newpc = imm1;
                         break;
            default: //no match again
            imm1 = instr1 & 0xFF; //just assume
            switch (instr1 & 0xFF00) {
                case 0x2000: mmio_write_byte_internal(REG_ACC, mmio_read_byte(imm1));
                             break;//mov a, r
                case 0x2100: mmio_write_byte(imm1, mmio_read_byte_internal(REG_ACC));
                             break;//mov r, a
                case 0x4E00: mmio_write_byte_internal(REG_ACC, imm1);
                             break;//mov r, #
                case 0x2500: status_zero(mmio_read_byte(imm1));
                             break;//test r
                case 0x2700: rpt_counter = mmio_read_byte(imm1);
                             break;//rpt r
                case 0x8300: mmio_write_byte_internal(REG_BSR, imm1);
                             break;//bank #
                case 0x2400: mmio_write_byte(imm1, 0);
                             break;//clr r
                case 0x4000: mmio_write_byte_internal(REG_TABPTRL, imm1);
                             break;//tbptl #
                case 0x4100: mmio_write_byte_internal(REG_TABPTRM, imm1);
                             break;//tbptm #
                case 0x4200: mmio_write_byte_internal(REG_TABPTRH, imm1);
                             break;//tbpth #
                case 0x2f00: temp32 = (mmio_read_byte_internal(REG_TABPTRH) << 16) |
                                      (mmio_read_byte_internal(REG_TABPTRM) << 8)  |
                                       mmio_read_byte_internal(REG_TABPTRL);
                             temp32 += mmio_read_byte_internal(REG_ACC);
                             temp16 = rom_read_word(temp32 >> 1);
                             if (temp32&0x01) temp16 >>= 8;
                             mmio_write_byte(imm1, temp16&0xff);
                             break;//tbrd a, r
                case 0x0200: temp8 = mmio_read_byte_internal(REG_ACC);
                             temp8 |= mmio_read_byte(imm1);
                             status_zero(temp8);
                             mmio_write_byte_internal(REG_ACC, temp8);
                             break;//or a, r
                case 0x0300: temp8 = mmio_read_byte_internal(REG_ACC);
                             temp8 |= mmio_read_byte(imm1);
                             status_zero(temp8);
                             mmio_write_byte(imm1, temp8);
                             break;//or r, a
                case 0x4400: temp8 = mmio_read_byte_internal(REG_ACC);
                             temp8 |= imm1;
                             status_zero(temp8);
                             mmio_write_byte_internal(REG_ACC, temp8);
                             break;//or a, #
                case 0x0400: temp8 = mmio_read_byte_internal(REG_ACC);
                             temp8 &= mmio_read_byte(imm1);
                             status_zero(temp8);
                             mmio_write_byte_internal(REG_ACC, temp8);
                             break;//and a, r
                case 0x0500: temp8 = mmio_read_byte_internal(REG_ACC);
                             temp8 &= mmio_read_byte(imm1);
                             status_zero(temp8);
                             mmio_write_byte(imm1, temp8);
                             break;//and r, a
                case 0x4500: temp8 = mmio_read_byte_internal(REG_ACC);
                             temp8 &= imm1;
                             status_zero(temp8);
                             mmio_write_byte_internal(REG_ACC, temp8);
                             break;//and a, #
                case 0x0600: temp8 = mmio_read_byte_internal(REG_ACC);
                             temp8 ^= mmio_read_byte(imm1);
                             status_zero(temp8);
                             mmio_write_byte_internal(REG_ACC, temp8);
                             break;//xor a, r
                case 0x0700: temp8 = mmio_read_byte_internal(REG_ACC);
                             temp8 ^= mmio_read_byte(imm1);
                             status_zero(temp8);
                             mmio_write_byte(imm1, temp8);
                             break;//xor r, a
                case 0x4600: temp8 = mmio_read_byte_internal(REG_ACC);
                             temp8 ^= imm1;
                             status_zero(temp8);
                             mmio_write_byte_internal(REG_ACC, temp8);
                             break;//xor a, #
                case 0x0800: temp8 = mmio_read_byte(imm1);
                             temp8 = ~temp8; 
                             status_zero(temp8);
                             mmio_write_byte_internal(REG_ACC, temp8);
                             break;//coma r
                case 0x0900: temp8 = mmio_read_byte(imm1);
                             temp8 = ~temp8; 
                             status_zero(temp8);
                             mmio_write_byte(imm1, temp8);
                             break;//com r        
                case 0x1C00: temp8 = mmio_read_byte(imm1);
                             status_carry(((uint16_t)temp8+1)&0x100);
                             temp8 += 1; 
                             status_zero(temp8);
                             mmio_write_byte_internal(REG_ACC, temp8);
                             break;//inca r
                case 0x1D00: temp8 = mmio_read_byte(imm1);
                             status_carry(((uint16_t)temp8+1)&0x100);
                             temp8 += 1; 
                             status_zero(temp8);
                             mmio_write_byte(imm1, temp8);
                             break;//inc r
                case 0x1000: mmio_write_byte_internal(REG_ACC, 
                                alu_add(mmio_read_byte(imm1), mmio_read_byte_internal(REG_ACC)));
                             break;//add a, r
                case 0x1100: mmio_write_byte(imm1, 
                                alu_add(mmio_read_byte(imm1), mmio_read_byte_internal(REG_ACC)));
                             break;//add r, a
                case 0x4A00: mmio_write_byte_internal(REG_ACC, 
                                alu_add(imm1, mmio_read_byte_internal(REG_ACC)));
                             break;//add a, #
                case 0x1200: mmio_write_byte_internal(REG_ACC, 
                                alu_adc(mmio_read_byte(imm1), mmio_read_byte_internal(REG_ACC),
                                status&0x01));
                             break;//adc a, r
                case 0x1300: mmio_write_byte_internal(imm1, 
                                alu_adc(mmio_read_byte(imm1), mmio_read_byte_internal(REG_ACC),
                                status&0x01));
                             break;//adc r, a
                case 0x4B00: mmio_write_byte_internal(REG_ACC, 
                                alu_adc(imm1, mmio_read_byte_internal(REG_ACC),
                                status&0x01));
                             break;//adc a, #
                case 0x1E00: temp8 = mmio_read_byte(imm1);
                             status_carry(!(imm1<1));
                             temp8 -= 1; 
                             status_zero(temp8);
                             mmio_write_byte_internal(REG_ACC, temp8);
                             break;//deca r
                case 0x1F00: temp8 = mmio_read_byte(imm1);
                             status_carry(!(imm1<1));
                             temp8 -= 1; 
                             status_zero(temp8);
                             mmio_write_byte(imm1, temp8);
                             break;//dec r
                case 0x1600: mmio_write_byte_internal(REG_ACC,
                                alu_sub(mmio_read_byte(imm1), mmio_read_byte_internal(REG_ACC)));
                             break;//sub a, r            
                case 0x1700: mmio_write_byte(imm1,
                                alu_sub(mmio_read_byte(imm1), mmio_read_byte_internal(REG_ACC)));
                             break;//sub r, a
                case 0x4C00: mmio_write_byte_internal(REG_ACC,
                                alu_sub(imm1, mmio_read_byte_internal(REG_ACC)));
                             break;//sub a, #         
                case 0x1800: mmio_write_byte_internal(REG_ACC,
                                alu_subb(mmio_read_byte(imm1), mmio_read_byte_internal(REG_ACC),
                                ((status&0x01)?0:1)));
                             break;//subb a, r            
                case 0x1900: mmio_write_byte(imm1,
                                alu_subb(mmio_read_byte(imm1), mmio_read_byte_internal(REG_ACC),
                                ((status&0x01)?0:1)));
                             break;//subb r, a
                case 0x4D00: mmio_write_byte_internal(REG_ACC,
                                alu_subb(imm1, mmio_read_byte_internal(REG_ACC),
                                ((status&0x01)?0:1)));
                             break;//subb a, #         
                case 0x1400: mmio_write_byte_internal(REG_ACC,
                                alu_adddc(mmio_read_byte(imm1), mmio_read_byte_internal(REG_ACC),
                                status&0x01));
                             break;//adddc a, r
                case 0x1500: mmio_write_byte(imm1,
                                alu_adddc(mmio_read_byte(imm1), mmio_read_byte_internal(REG_ACC),
                                status&0x01));
                             break;//adddc r, a
                case 0x1A00: mmio_write_byte_internal(REG_ACC,
                                alu_subdb(mmio_read_byte(imm1), mmio_read_byte_internal(REG_ACC),
                                ((status&0x01)?0:1)));
                             break;//subdb a, r
                case 0x1B00: mmio_write_byte(imm1,
                                alu_subdb(mmio_read_byte(imm1), mmio_read_byte_internal(REG_ACC),
                                ((status&0x01)?0:1)));
                             break;//subdb r,a
                case 0x0A00: temp8 = mmio_read_byte(imm1);
                             temp16 = temp8 & 0x01;
                             temp8 >>= 1;
                             temp8 |= (status&0x01) << 7;
                             status_carry(temp16);
                             mmio_write_byte_internal(REG_ACC, temp8);
                             break;//rrca r
                case 0x0B00: temp8 = mmio_read_byte(imm1);
                             temp16 = temp8 & 0x01;
                             temp8 >>= 1;
                             temp8 |= (status&0x01) << 7;
                             status_carry(temp16);
                             mmio_write_byte(imm1, temp8);
                             break;//rrc r
                case 0x0C00: temp8 = mmio_read_byte(imm1);
                             temp16 = temp8 & 0x80;
                             temp8 <<= 1;
                             temp8 |= status&0x01;
                             status_carry(temp16);
                             mmio_write_byte_internal(REG_ACC, temp8);
                             break;//rlca r
                case 0x0D00: temp8 = mmio_read_byte(imm1);
                             temp16 = temp8 & 0x80;
                             temp8 <<= 1;
                             temp8 |= status&0x01;
                             status_carry(temp16);
                             mmio_write_byte(imm1, temp8);
                             break;//rlc r
                case 0x2200: temp8 = mmio_read_byte(imm1);
                             temp8 >>= 1;
                             temp8 |= (status&0x01) << 7;
                             mmio_write_byte_internal(REG_ACC, temp8);
                             break;//shra r
                case 0x2300: temp8 = mmio_read_byte(imm1);
                             temp8 <<= 1;
                             temp8 |= status&0x01;
                             mmio_write_byte_internal(REG_ACC, temp8);
                             break;//shla r
                case 0x5400: temp8 = mmio_read_byte_internal(REG_ACC);
                             mmio_write_byte_internal(REG_ACC, mmio_read_byte(imm1));
                             mmio_write_byte(imm1, temp8);
                             break;//ex r
                case 0x5200: temp8 = mmio_read_byte_internal(REG_ACC);
                             temp16 = mmio_read_byte(imm1);
                             temp32 = temp8;
                             temp8 &= 0xF0;
                             temp8 |= temp16 & 0x0F;
                             temp16 &= 0xF0;
                             temp16 |= temp32 & 0x0F;
                             mmio_write_byte(imm1, temp16);
                             mmio_write_byte_internal(REG_ACC, temp8);
                             break;//exl r
                case 0x5300: temp8 = mmio_read_byte_internal(REG_ACC);
                             temp16 = mmio_read_byte(imm1);
                             temp32 = temp8;
                             temp8 &= 0x0F;
                             temp8 |= temp16 & 0xF0;
                             temp16 &= 0x0F;
                             temp16 |= temp32 & 0xF0;
                             mmio_write_byte(imm1, temp16);
                             mmio_write_byte_internal(REG_ACC, temp8);
                             break;//exh r
                case 0x2600: temp8 = mmio_read_byte(imm1);
                             temp8 &= 0xF0;
                             temp8 |= mmio_read_byte_internal(REG_ACC) & 0xF;
                             mmio_write_byte(imm1, temp8);
                             break;//movl r, a
                case 0x2800: temp8 = mmio_read_byte(imm1);
                             temp8 &= 0x0F;
                             temp8 |= mmio_read_byte_internal(REG_ACC) << 4;
                             mmio_write_byte(imm1, temp8);
                             break;//movh r, a
                case 0x2900: mmio_write_byte_internal(REG_ACC, mmio_read_byte(imm1)&0x0F);
                             break;//movl a, r
                case 0x2A00: mmio_write_byte_internal(REG_ACC, mmio_read_byte(imm1)>>4);
                             break;//movh a, r
            }
            break;
        }
        break;
    }
    pc = newpc;
    mmio_write_byte_internal(REG_PCL, pc&0xFF);
    mmio_write_byte_internal(REG_PCM, (pc>>8)&0xFF);
    mmio_write_byte_internal(REG_PCH, (pc>>16)&0xFF);
    mmio_write_byte_internal(REG_STATUS, status);
    
}

void *cpu_save_state(size_t *size) {

}

void cpu_load_state(void *state) {

}