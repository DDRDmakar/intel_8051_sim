

#ifndef __INSTRUCTIONS__
#define __INSTRUCTIONS__

#include <stdint.h>

#include "headers/memory.h"

/*
 * Из книжечки
 * 
Rn (n = 0, 1,..., 7) – регистр общего назначения в выбранном банке
регистров;
@Ri(i= 0, 1) – регистр общего назначения в выбранном банке
регистров, используемый в качестве регистра косвенного адреса;
ad – адрес прямоадресуемого байта;
ads – адрес прямо адресуемого байта-источника;
add – адрес прямо адресуемого байта-получателя;
ad11 – 11-разрядный абсолютный адрес перехода;
ad16 – 16-разрядный абсолютный адрес перехода;
rel – относительный адрес перехода;
#d – непосредственный операнд;
#d16 – непосредственный операнд (2 байта);
bit – адрес прямо адресуемого бита;
/bit – инверсия прямо адресуемого бита;
А - аккумулятор;
РС – счетчик команд;
DPTR – регистр указатель данных;
( ) – содержимое ячейки памяти или регистра.
 */

// Специальные регистры
#define FUNREGS mem->DM.RDM_REG
// Резидентная память программы
#define PROGMEM mem->PM.RPM
// Внешняя память программы
#define E_PROGMEM mem->PM.EPM
// Резидентная память данных
#define DATAMEM mem->DM.RDM
// Внешняя память данных
#define E_DATAMEM mem->DM.EDM
// Инструкция относительно PC
#define INSTR(x) PROGMEM[mem->PC + x]
// Текущий банк регистров
#define REGBANK mem->DM.bank_POH[(FUNREGS.PSW.PSW & 0x18) >> 3]
// Последние 3 бита инструкции
#define LAST_3(x) x & 0x07
// Последний 1 бит инструкции
#define LAST_1(x) x & 0x01

// Байт в текущем банке регистров, выбранный инструкцией (от 0 до 7)
#define Rn REGBANK[LAST_3(INSTR(0))]
// Байт в текущем банке регистров, выбранный инструкцией (от 0 до 1)
#define Ri REGBANK[LAST_1(INSTR(0))]
// Ещё один инкремент происходит в вызывающей функции

// Увеличить счётчик инструкций на 1
#define IncrPC_1 ++mem->PC
// Увеличить счётчик инструкций на 2
#define IncrPC_2 mem->PC+=2

// Поля PSW
#define PSWBITS FUNREGS.PSW.BITS

// Объявление функции-инструкции
#define I(x) void x(struct Memory *mem)

uint8_t is_odd(uint8_t a)
{
	uint8_t answer = 0;
	while (a)
	{
		answer += (a & 0x01);
		a >>= 1;
	}
	return answer & 0x01;
}

// MOV are RDM only

// ### Команды передачи данных

I(mov_a_rn) { FUNREGS.ACC = Rn; }

I(mov_a_ad) { FUNREGS.ACC = DATAMEM[INSTR(1)]; IncrPC_1; }

I(mov_a_rdm) { FUNREGS.ACC = DATAMEM[Ri]; }

I(mov_a_d) { FUNREGS.ACC = INSTR(1); IncrPC_1; }

I(mov_rn_a) { Rn = FUNREGS.ACC; }

I(mov_rn_ad) { Rn = DATAMEM[INSTR(1)]; IncrPC_1; }

I(mov_rn_d) { Rn = INSTR(1); IncrPC_1; }

I(mov_ad_a) { DATAMEM[INSTR(1)] = FUNREGS.ACC; IncrPC_1; }

I(mov_ad_rn) { Rn = DATAMEM[INSTR(1)]; }

I(mov_add_ads) { DATAMEM[INSTR(1)] = DATAMEM[INSTR(2)]; IncrPC_2; }

I(mov_ad_rdm) { DATAMEM[INSTR(1)] = DATAMEM[Ri]; IncrPC_1; }

I(mov_ad_d) { DATAMEM[INSTR(1)] = INSTR(2);  IncrPC_2; }

I(mov_rdm_a) { DATAMEM[Ri] = FUNREGS.ACC; }

I(mov_rdm_ad) { DATAMEM[Ri] = DATAMEM[INSTR(1)]; IncrPC_1; }

I(mov_rdm_d) { DATAMEM[Ri] = INSTR(1); IncrPC_1; }

I(mov_dptr_d16) // Загрузка указателя данных
{ 
	FUNREGS.DPTR.LH.DPH = INSTR(1);
	FUNREGS.DPTR.LH.DPL = INSTR(2);
	IncrPC_2;
}

I(movc_a_adptr) { FUNREGS.ACC = E_PROGMEM[FUNREGS.ACC + FUNREGS.DPTR.DPTR]; }

I(movc_a_apc) { IncrPC_1; FUNREGS.ACC = E_PROGMEM[FUNREGS.ACC + mem->PC]; }

I(movx_a_edm) { FUNREGS.ACC = E_DATAMEM[Ri]; }

I(movx_a_dptr) { FUNREGS.ACC = E_DATAMEM[FUNREGS.DPTR.DPTR]; }

I(movx_edm_a) { E_DATAMEM[Ri] = FUNREGS.ACC; }

I(movx_dptr_a) { E_DATAMEM[FUNREGS.DPTR.DPTR] = FUNREGS.ACC; }

// TODO посмотреть где стек
I(push_ad) { DATAMEM[++FUNREGS.SP] = DATAMEM[INSTR(1)]; IncrPC_1; }

I(pop_ad) { DATAMEM[INSTR(1)] = DATAMEM[FUNREGS.SP--]; IncrPC_1; }

I(xch_a_rn)
{
	uint8_t t = FUNREGS.ACC;
	FUNREGS.ACC = Rn;
	Rn = t;
}

I(xch_a_ad)
{
	uint8_t t = FUNREGS.ACC;
	FUNREGS.ACC = DATAMEM[INSTR(1)];
	DATAMEM[INSTR(1)] = t;
	IncrPC_1;
}

I(xch_a_rdm)
{
	uint8_t t = FUNREGS.ACC;
	FUNREGS.ACC = DATAMEM[Ri];
	DATAMEM[Ri] = t;
}

I(xchd_a_rdm)
{
	uint8_t t = FUNREGS.ACC & 0x0F;
	FUNREGS.ACC = DATAMEM[Ri] & 0x0F;
	DATAMEM[Ri] = t;
}



// ### Арифметические операции

/*
	По результату выполнения команд ADD, ADDC, SUBB, MUL и
	DIV устанавливаются флаги PSW, структура которых приведена в
	табл. 4.
	
	Флаг С устанавливается при переносе из разряда D7, т. е. в случае,
	если результат не помещается в восемь разрядов; флаг АС
	устанавливается при переносе из разряда D3 в командах сложения и
	вычитания и служит для реализации десятичной арифметики. Этот
	признак используется командой DAA.
	
	Флаг OV устанавливается при переносе из разряда D6, т. е. в
	случае, если результат не помещается в семь разрядов и восьмой не
	может быть интерпретирован как знаковый. Этот признак служит для
	организации обработки чисел со знаком.
	
	Наконец, флаг Р устанавливается и сбрасывается аппаратно. Если
	число единичных бит в аккумуляторе нечетно, то Р = 1, в противном
	случае Р = 0.

*/

I(add_a_rn)
{ 
	PSWBITS.C = ((uint16_t)FUNREGS.ACC + (uint16_t)Rn > 255) ? 1 : 0;
	PSWBITS.AC = ((FUNREGS.ACC & 0x0F) + (Rn & 0x0F) > 15) ? 1 : 0;
	PSWBITS.OV = ((FUNREGS.ACC & 0x7F) + (Rn & 0x7F) > 127) ? 1 : 0;
	FUNREGS.ACC += Rn;
	PSWBITS.P = is_odd(FUNREGS.ACC);
}

I(add_a_ad)
{
	PSWBITS.C = ((uint16_t)FUNREGS.ACC + (uint16_t)DATAMEM[INSTR(1)] > 255) ? 1 : 0;
	PSWBITS.AC = ((FUNREGS.ACC & 0x0F) + (DATAMEM[INSTR(1)] & 0x0F) > 15) ? 1 : 0;
	PSWBITS.OV = ((FUNREGS.ACC & 0x7F) + (DATAMEM[INSTR(1)] & 0x7F) > 127) ? 1 : 0;
	FUNREGS.ACC += DATAMEM[INSTR(1)];
	PSWBITS.P = is_odd(FUNREGS.ACC);
	IncrPC_1;
}





















I(nop) { (void)mem; }



// INSTRUCTIONS ARRAY

struct Instruction_storage
{
	void (*i)(struct Memory*);
	unsigned int n_bytes;
	unsigned int n_ticks;
};

struct Instruction_storage instr[256] = 
{
	// instruction n_bytes n_ticks
	
	{&nop, 1, 1}, // [0x00] [0b00000000]
	{NULL, 0, 0}, // [0x01] [0b00000001]
	{NULL, 0, 0}, // [0x02] [0b00000010]
	{NULL, 0, 0}, // [0x03] [0b00000011]
	{NULL, 0, 0}, // [0x04] [0b00000100]
	{NULL, 0, 0}, // [0x05] [0b00000101]
	{NULL, 0, 0}, // [0x06] [0b00000110]
	{NULL, 0, 0}, // [0x07] [0b00000111]
	{NULL, 0, 0}, // [0x08] [0b00001000]
	{NULL, 0, 0}, // [0x09] [0b00001001]
	{NULL, 0, 0}, // [0x0a] [0b00001010]
	{NULL, 0, 0}, // [0x0b] [0b00001011]
	{NULL, 0, 0}, // [0x0c] [0b00001100]
	{NULL, 0, 0}, // [0x0d] [0b00001101]
	{NULL, 0, 0}, // [0x0e] [0b00001110]
	{NULL, 0, 0}, // [0x0f] [0b00001111]
	{NULL, 0, 0}, // [0x10] [0b00010000]
	{NULL, 0, 0}, // [0x11] [0b00010001]
	{NULL, 0, 0}, // [0x12] [0b00010010]
	{NULL, 0, 0}, // [0x13] [0b00010011]
	{NULL, 0, 0}, // [0x14] [0b00010100]
	{NULL, 0, 0}, // [0x15] [0b00010101]
	{NULL, 0, 0}, // [0x16] [0b00010110]
	{NULL, 0, 0}, // [0x17] [0b00010111]
	{NULL, 0, 0}, // [0x18] [0b00011000]
	{NULL, 0, 0}, // [0x19] [0b00011001]
	{NULL, 0, 0}, // [0x1a] [0b00011010]
	{NULL, 0, 0}, // [0x1b] [0b00011011]
	{NULL, 0, 0}, // [0x1c] [0b00011100]
	{NULL, 0, 0}, // [0x1d] [0b00011101]
	{NULL, 0, 0}, // [0x1e] [0b00011110]
	{NULL, 0, 0}, // [0x1f] [0b00011111]
	{NULL, 0, 0}, // [0x20] [0b00100000]
	{NULL, 0, 0}, // [0x21] [0b00100001]
	{NULL, 0, 0}, // [0x22] [0b00100010]
	{NULL, 0, 0}, // [0x23] [0b00100011]
	{NULL, 0, 0}, // [0x24] [0b00100100]
	{&add_a_ad, 2, 1}, // [0x25] [0b00100101]
	{NULL, 0, 0}, // [0x26] [0b00100110]
	{NULL, 0, 0}, // [0x27] [0b00100111]
	{NULL, 0, 0}, // [0x28] [0b00101000]
	{NULL, 0, 0}, // [0x29] [0b00101001]
	{NULL, 0, 0}, // [0x2a] [0b00101010]
	{NULL, 0, 0}, // [0x2b] [0b00101011]
	{NULL, 0, 0}, // [0x2c] [0b00101100]
	{NULL, 0, 0}, // [0x2d] [0b00101101]
	{NULL, 0, 0}, // [0x2e] [0b00101110]
	{NULL, 0, 0}, // [0x2f] [0b00101111]
	{NULL, 0, 0}, // [0x30] [0b00110000]
	{NULL, 0, 0}, // [0x31] [0b00110001]
	{NULL, 0, 0}, // [0x32] [0b00110010]
	{NULL, 0, 0}, // [0x33] [0b00110011]
	{NULL, 0, 0}, // [0x34] [0b00110100]
	{NULL, 0, 0}, // [0x35] [0b00110101]
	{NULL, 0, 0}, // [0x36] [0b00110110]
	{NULL, 0, 0}, // [0x37] [0b00110111]
	{NULL, 0, 0}, // [0x38] [0b00111000]
	{NULL, 0, 0}, // [0x39] [0b00111001]
	{NULL, 0, 0}, // [0x3a] [0b00111010]
	{NULL, 0, 0}, // [0x3b] [0b00111011]
	{NULL, 0, 0}, // [0x3c] [0b00111100]
	{NULL, 0, 0}, // [0x3d] [0b00111101]
	{NULL, 0, 0}, // [0x3e] [0b00111110]
	{NULL, 0, 0}, // [0x3f] [0b00111111]
	{NULL, 0, 0}, // [0x40] [0b01000000]
	{NULL, 0, 0}, // [0x41] [0b01000001]
	{NULL, 0, 0}, // [0x42] [0b01000010]
	{NULL, 0, 0}, // [0x43] [0b01000011]
	{NULL, 0, 0}, // [0x44] [0b01000100]
	{NULL, 0, 0}, // [0x45] [0b01000101]
	{NULL, 0, 0}, // [0x46] [0b01000110]
	{NULL, 0, 0}, // [0x47] [0b01000111]
	{NULL, 0, 0}, // [0x48] [0b01001000]
	{NULL, 0, 0}, // [0x49] [0b01001001]
	{NULL, 0, 0}, // [0x4a] [0b01001010]
	{NULL, 0, 0}, // [0x4b] [0b01001011]
	{NULL, 0, 0}, // [0x4c] [0b01001100]
	{NULL, 0, 0}, // [0x4d] [0b01001101]
	{NULL, 0, 0}, // [0x4e] [0b01001110]
	{NULL, 0, 0}, // [0x4f] [0b01001111]
	{NULL, 0, 0}, // [0x50] [0b01010000]
	{NULL, 0, 0}, // [0x51] [0b01010001]
	{NULL, 0, 0}, // [0x52] [0b01010010]
	{NULL, 0, 0}, // [0x53] [0b01010011]
	{NULL, 0, 0}, // [0x54] [0b01010100]
	{NULL, 0, 0}, // [0x55] [0b01010101]
	{NULL, 0, 0}, // [0x56] [0b01010110]
	{NULL, 0, 0}, // [0x57] [0b01010111]
	{NULL, 0, 0}, // [0x58] [0b01011000]
	{NULL, 0, 0}, // [0x59] [0b01011001]
	{NULL, 0, 0}, // [0x5a] [0b01011010]
	{NULL, 0, 0}, // [0x5b] [0b01011011]
	{NULL, 0, 0}, // [0x5c] [0b01011100]
	{NULL, 0, 0}, // [0x5d] [0b01011101]
	{NULL, 0, 0}, // [0x5e] [0b01011110]
	{NULL, 0, 0}, // [0x5f] [0b01011111]
	{NULL, 0, 0}, // [0x60] [0b01100000]
	{NULL, 0, 0}, // [0x61] [0b01100001]
	{NULL, 0, 0}, // [0x62] [0b01100010]
	{NULL, 0, 0}, // [0x63] [0b01100011]
	{NULL, 0, 0}, // [0x64] [0b01100100]
	{NULL, 0, 0}, // [0x65] [0b01100101]
	{NULL, 0, 0}, // [0x66] [0b01100110]
	{NULL, 0, 0}, // [0x67] [0b01100111]
	{NULL, 0, 0}, // [0x68] [0b01101000]
	{NULL, 0, 0}, // [0x69] [0b01101001]
	{NULL, 0, 0}, // [0x6a] [0b01101010]
	{NULL, 0, 0}, // [0x6b] [0b01101011]
	{NULL, 0, 0}, // [0x6c] [0b01101100]
	{NULL, 0, 0}, // [0x6d] [0b01101101]
	{NULL, 0, 0}, // [0x6e] [0b01101110]
	{NULL, 0, 0}, // [0x6f] [0b01101111]
	{NULL, 0, 0}, // [0x70] [0b01110000]
	{NULL, 0, 0}, // [0x71] [0b01110001]
	{NULL, 0, 0}, // [0x72] [0b01110010]
	{NULL, 0, 0}, // [0x73] [0b01110011]
	{NULL, 0, 0}, // [0x74] [0b01110100]
	{&mov_ad_d, 3, 2}, // [0x75] [0b01110101]
	{NULL, 0, 0}, // [0x76] [0b01110110]
	{NULL, 0, 0}, // [0x77] [0b01110111]
	{&mov_rn_d, 2, 1}, // [0x78] [0b01111000]
	{&mov_rn_d, 2, 1}, // [0x79] [0b01111001]
	{&mov_rn_d, 2, 1}, // [0x7a] [0b01111010]
	{&mov_rn_d, 2, 1}, // [0x7b] [0b01111011]
	{&mov_rn_d, 2, 1}, // [0x7c] [0b01111100]
	{&mov_rn_d, 2, 1}, // [0x7d] [0b01111101]
	{&mov_rn_d, 2, 1}, // [0x7e] [0b01111110]
	{&mov_rn_d, 2, 1}, // [0x7f] [0b01111111]
	{NULL, 0, 0}, // [0x80] [0b10000000]
	{NULL, 0, 0}, // [0x81] [0b10000001]
	{NULL, 0, 0}, // [0x82] [0b10000010]
	{NULL, 0, 0}, // [0x83] [0b10000011]
	{NULL, 0, 0}, // [0x84] [0b10000100]
	{NULL, 0, 0}, // [0x85] [0b10000101]
	{NULL, 0, 0}, // [0x86] [0b10000110]
	{NULL, 0, 0}, // [0x87] [0b10000111]
	{NULL, 0, 0}, // [0x88] [0b10001000]
	{NULL, 0, 0}, // [0x89] [0b10001001]
	{NULL, 0, 0}, // [0x8a] [0b10001010]
	{NULL, 0, 0}, // [0x8b] [0b10001011]
	{NULL, 0, 0}, // [0x8c] [0b10001100]
	{NULL, 0, 0}, // [0x8d] [0b10001101]
	{NULL, 0, 0}, // [0x8e] [0b10001110]
	{NULL, 0, 0}, // [0x8f] [0b10001111]
	{NULL, 0, 0}, // [0x90] [0b10010000]
	{NULL, 0, 0}, // [0x91] [0b10010001]
	{NULL, 0, 0}, // [0x92] [0b10010010]
	{NULL, 0, 0}, // [0x93] [0b10010011]
	{NULL, 0, 0}, // [0x94] [0b10010100]
	{NULL, 0, 0}, // [0x95] [0b10010101]
	{NULL, 0, 0}, // [0x96] [0b10010110]
	{NULL, 0, 0}, // [0x97] [0b10010111]
	{NULL, 0, 0}, // [0x98] [0b10011000]
	{NULL, 0, 0}, // [0x99] [0b10011001]
	{NULL, 0, 0}, // [0x9a] [0b10011010]
	{NULL, 0, 0}, // [0x9b] [0b10011011]
	{NULL, 0, 0}, // [0x9c] [0b10011100]
	{NULL, 0, 0}, // [0x9d] [0b10011101]
	{NULL, 0, 0}, // [0x9e] [0b10011110]
	{NULL, 0, 0}, // [0x9f] [0b10011111]
	{NULL, 0, 0}, // [0xa0] [0b10100000]
	{NULL, 0, 0}, // [0xa1] [0b10100001]
	{NULL, 0, 0}, // [0xa2] [0b10100010]
	{NULL, 0, 0}, // [0xa3] [0b10100011]
	{NULL, 0, 0}, // [0xa4] [0b10100100]
	{NULL, 0, 0}, // [0xa5] [0b10100101]
	{NULL, 0, 0}, // [0xa6] [0b10100110]
	{NULL, 0, 0}, // [0xa7] [0b10100111]
	{NULL, 0, 0}, // [0xa8] [0b10101000]
	{NULL, 0, 0}, // [0xa9] [0b10101001]
	{NULL, 0, 0}, // [0xaa] [0b10101010]
	{NULL, 0, 0}, // [0xab] [0b10101011]
	{NULL, 0, 0}, // [0xac] [0b10101100]
	{NULL, 0, 0}, // [0xad] [0b10101101]
	{NULL, 0, 0}, // [0xae] [0b10101110]
	{NULL, 0, 0}, // [0xaf] [0b10101111]
	{NULL, 0, 0}, // [0xb0] [0b10110000]
	{NULL, 0, 0}, // [0xb1] [0b10110001]
	{NULL, 0, 0}, // [0xb2] [0b10110010]
	{NULL, 0, 0}, // [0xb3] [0b10110011]
	{NULL, 0, 0}, // [0xb4] [0b10110100]
	{NULL, 0, 0}, // [0xb5] [0b10110101]
	{NULL, 0, 0}, // [0xb6] [0b10110110]
	{NULL, 0, 0}, // [0xb7] [0b10110111]
	{NULL, 0, 0}, // [0xb8] [0b10111000]
	{NULL, 0, 0}, // [0xb9] [0b10111001]
	{NULL, 0, 0}, // [0xba] [0b10111010]
	{NULL, 0, 0}, // [0xbb] [0b10111011]
	{NULL, 0, 0}, // [0xbc] [0b10111100]
	{NULL, 0, 0}, // [0xbd] [0b10111101]
	{NULL, 0, 0}, // [0xbe] [0b10111110]
	{NULL, 0, 0}, // [0xbf] [0b10111111]
	{NULL, 0, 0}, // [0xc0] [0b11000000]
	{NULL, 0, 0}, // [0xc1] [0b11000001]
	{NULL, 0, 0}, // [0xc2] [0b11000010]
	{NULL, 0, 0}, // [0xc3] [0b11000011]
	{NULL, 0, 0}, // [0xc4] [0b11000100]
	{NULL, 0, 0}, // [0xc5] [0b11000101]
	{NULL, 0, 0}, // [0xc6] [0b11000110]
	{NULL, 0, 0}, // [0xc7] [0b11000111]
	{NULL, 0, 0}, // [0xc8] [0b11001000]
	{NULL, 0, 0}, // [0xc9] [0b11001001]
	{NULL, 0, 0}, // [0xca] [0b11001010]
	{NULL, 0, 0}, // [0xcb] [0b11001011]
	{NULL, 0, 0}, // [0xcc] [0b11001100]
	{NULL, 0, 0}, // [0xcd] [0b11001101]
	{NULL, 0, 0}, // [0xce] [0b11001110]
	{NULL, 0, 0}, // [0xcf] [0b11001111]
	{NULL, 0, 0}, // [0xd0] [0b11010000]
	{NULL, 0, 0}, // [0xd1] [0b11010001]
	{NULL, 0, 0}, // [0xd2] [0b11010010]
	{NULL, 0, 0}, // [0xd3] [0b11010011]
	{NULL, 0, 0}, // [0xd4] [0b11010100]
	{NULL, 0, 0}, // [0xd5] [0b11010101]
	{NULL, 0, 0}, // [0xd6] [0b11010110]
	{NULL, 0, 0}, // [0xd7] [0b11010111]
	{NULL, 0, 0}, // [0xd8] [0b11011000]
	{NULL, 0, 0}, // [0xd9] [0b11011001]
	{NULL, 0, 0}, // [0xda] [0b11011010]
	{NULL, 0, 0}, // [0xdb] [0b11011011]
	{NULL, 0, 0}, // [0xdc] [0b11011100]
	{NULL, 0, 0}, // [0xdd] [0b11011101]
	{NULL, 0, 0}, // [0xde] [0b11011110]
	{NULL, 0, 0}, // [0xdf] [0b11011111]
	{NULL, 0, 0}, // [0xe0] [0b11100000]
	{NULL, 0, 0}, // [0xe1] [0b11100001]
	{NULL, 0, 0}, // [0xe2] [0b11100010]
	{NULL, 0, 0}, // [0xe3] [0b11100011]
	{NULL, 0, 0}, // [0xe4] [0b11100100]
	{NULL, 0, 0}, // [0xe5] [0b11100101]
	{NULL, 0, 0}, // [0xe6] [0b11100110]
	{NULL, 0, 0}, // [0xe7] [0b11100111]
	{NULL, 0, 0}, // [0xe8] [0b11101000]
	{NULL, 0, 0}, // [0xe9] [0b11101001]
	{NULL, 0, 0}, // [0xea] [0b11101010]
	{NULL, 0, 0}, // [0xeb] [0b11101011]
	{NULL, 0, 0}, // [0xec] [0b11101100]
	{NULL, 0, 0}, // [0xed] [0b11101101]
	{NULL, 0, 0}, // [0xee] [0b11101110]
	{NULL, 0, 0}, // [0xef] [0b11101111]
	{NULL, 0, 0}, // [0xf0] [0b11110000]
	{NULL, 0, 0}, // [0xf1] [0b11110001]
	{NULL, 0, 0}, // [0xf2] [0b11110010]
	{NULL, 0, 0}, // [0xf3] [0b11110011]
	{NULL, 0, 0}, // [0xf4] [0b11110100]
	{NULL, 0, 0}, // [0xf5] [0b11110101]
	{NULL, 0, 0}, // [0xf6] [0b11110110]
	{NULL, 0, 0}, // [0xf7] [0b11110111]
	{NULL, 0, 0}, // [0xf8] [0b11111000]
	{NULL, 0, 0}, // [0xf9] [0b11111001]
	{NULL, 0, 0}, // [0xfa] [0b11111010]
	{NULL, 0, 0}, // [0xfb] [0b11111011]
	{NULL, 0, 0}, // [0xfc] [0b11111100]
	{NULL, 0, 0}, // [0xfd] [0b11111101]
	{NULL, 0, 0}, // [0xfe] [0b11111110]
	{NULL, 0, 0}  // [0xff] [0b11111111]
	
	
	/*
	&mov_rn_d, // [0x78]
	&mov_rn_d, // [0x79]
	&mov_rn_d, // [0x7a]
	&mov_rn_d, // [0x7b]
	&mov_rn_d, // [0x7c]
	&mov_rn_d, // [0x7d]
	&mov_rn_d, // [0x7e]
	&mov_rn_d, // [0x7f]
	NULL, // [0x80]
	NULL, // [0x81]
	NULL, // [0x82]
	NULL, // [0x83]
	NULL, // [0x84]
	NULL, // [0x85]
	NULL, // [0x86]
	NULL, // [0x87]
	NULL, // [0x88]
	NULL, // [0x89]
	NULL, // [0x8a]
	NULL, // [0x8b]
	NULL, // [0x8c]
	NULL, // [0x8d]
	NULL, // [0x8e]
	NULL, // [0x8f]
	NULL, // [0x90]
	NULL, // [0x91]
	NULL, // [0x92]
	NULL, // [0x93]
	NULL, // [0x94]
	NULL, // [0x95]
	NULL, // [0x96]
	NULL, // [0x97]
	NULL, // [0x98]
	NULL, // [0x99]
	NULL, // [0x9a]
	NULL, // [0x9b]
	NULL, // [0x9c]
	NULL, // [0x9d]
	NULL, // [0x9e]
	NULL, // [0x9f]
	NULL, // [0xa0]
	NULL, // [0xa1]
	NULL, // [0xa2]
	NULL, // [0xa3]
	NULL, // [0xa4]
	NULL, // [0xa5]
	NULL, // [0xa6]
	NULL, // [0xa7]
	NULL, // [0xa8]
	NULL, // [0xa9]
	NULL, // [0xaa]
	NULL, // [0xab]
	NULL, // [0xac]
	NULL, // [0xad]
	NULL, // [0xae]
	NULL, // [0xaf]
	NULL, // [0xb0]
	NULL, // [0xb1]
	NULL, // [0xb2]
	NULL, // [0xb3]
	NULL, // [0xb4]
	NULL, // [0xb5]
	NULL, // [0xb6]
	NULL, // [0xb7]
	NULL, // [0xb8]
	NULL, // [0xb9]
	NULL, // [0xba]
	NULL, // [0xbb]
	NULL, // [0xbc]
	NULL, // [0xbd]
	NULL, // [0xbe]
	NULL, // [0xbf]
	NULL, // [0xc0]
	NULL, // [0xc1]
	NULL, // [0xc2]
	NULL, // [0xc3]
	NULL, // [0xc4]
	NULL, // [0xc5]
	NULL, // [0xc6]
	NULL, // [0xc7]
	NULL, // [0xc8]
	NULL, // [0xc9]
	NULL, // [0xca]
	NULL, // [0xcb]
	NULL, // [0xcc]
	NULL, // [0xcd]
	NULL, // [0xce]
	NULL, // [0xcf]
	NULL, // [0xd0]
	NULL, // [0xd1]
	NULL, // [0xd2]
	NULL, // [0xd3]
	NULL, // [0xd4]
	NULL, // [0xd5]
	NULL, // [0xd6]
	NULL, // [0xd7]
	NULL, // [0xd8]
	NULL, // [0xd9]
	NULL, // [0xda]
	NULL, // [0xdb]
	NULL, // [0xdc]
	NULL, // [0xdd]
	NULL, // [0xde]
	NULL, // [0xdf]
	NULL, // [0xe0]
	NULL, // [0xe1]
	NULL, // [0xe2]
	NULL, // [0xe3]
	NULL, // [0xe4]
	NULL, // [0xe5]
	NULL, // [0xe6]
	NULL, // [0xe7]
	NULL, // [0xe8]
	NULL, // [0xe9]
	NULL, // [0xea]
	NULL, // [0xeb]
	NULL, // [0xec]
	NULL, // [0xed]
	NULL, // [0xee]
	NULL, // [0xef]
	NULL, // [0xf0]
	NULL, // [0xf1]
	NULL, // [0xf2]
	NULL, // [0xf3]
	NULL, // [0xf4]
	NULL, // [0xf5]
	NULL, // [0xf6]
	NULL, // [0xf7]
	NULL, // [0xf8]
	NULL, // [0xf9]
	NULL, // [0xfa]
	NULL, // [0xfb]
	NULL, // [0xfc]
	NULL, // [0xfd]
	NULL, // [0xfe]
	NULL  // [0xff]
	*/
};

#endif
