

#ifndef __INSTRUCTIONS
#define __INSTRUCTIONS

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
// Поля PSW
#define PSWBITS FUNREGS.PSW.BITS
// Флаг переноса
#define Cout PSWBITS.C
// Аккумулятор
#define ACCUM FUNREGS.ACC
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
#define LAST_3 INSTR(0) & 0x07
// Последний 1 бит инструкции
#define LAST_1 INSTR(0) & 0x01

// Байт в текущем банке регистров, выбранный инструкцией (от 0 до 7)
#define Rn REGBANK[LAST_3]
// Байт в текущем банке регистров, выбранный инструкцией (от 0 до 1)
#define Ri REGBANK[LAST_1]
// Ещё один инкремент происходит в вызывающей функции

// Увеличить счётчик инструкций на 1
#define IncrPC_1 ++mem->PC
// Увеличить счётчик инструкций на 2
#define IncrPC_2 mem->PC+=2
// Уменьшить счётчик инструкций на 1
#define DecPC_1 --mem->PC

// Количество единичных битов в аккамуляторе нечётное ? 1 : 0;
#define ODD_ACCUM PSWBITS.P = is_odd_number_of_bits(ACCUM)

// Обмен 2 8-битных чисел
#define XCH(a, b) \
	uint8_t t = a; \
	a = b; \
	b = t; \

// Объявление функции-инструкции
#define I(x) void x(Memory *mem)


uint8_t is_odd_number_of_bits(uint8_t a)
{
	uint8_t answer = 0;
	while (a)
	{
		answer += (a & 0x01);
		a >>= 1;
	}
	return answer & 0x01;
}

// Бит, адресуемый байтом после инструкции
// Адреса битов на стр. 19
uint8_t *bitbyteptr(Memory *mem, uint8_t addr)
{
	if (addr < 0x80)
		return &FUNREGS.MEM[0x20 + (addr/8)];
	else if (0x80 <= addr && addr <= 0x87)
		return &FUNREGS.P0;
	else if (0x88 <= addr && addr <= 0x8F)
		return &FUNREGS.TCON;
	else if (0x90 <= addr && addr <= 0x97)
		return &FUNREGS.P1;
	else if (0x98 <= addr && addr <= 0x9F)
		return &FUNREGS.SCON;
	else if (0xA0 <= addr && addr <= 0xA7)
		return &FUNREGS.P2;
	else if ((0xA8 <= addr && addr <= 0xAC) || addr == 0xAF)
		return &FUNREGS.IE;
	else if (0xB0 <= addr && addr <= 0xB7)
		return &FUNREGS.P3;
	else if (0xB8 <= addr && addr <= 0xBC)
		return &FUNREGS.IP;
	else if (0xD0 <= addr && addr <= 0xD7)
		return &FUNREGS.PSW.PSW;
	else if (0xE0 <= addr && addr <= 0xE7)
		return &FUNREGS.ACC;
	else if (0xF0 <= addr && addr <= 0xF7)
		return &FUNREGS.B;
	
	else return NULL; /*ERR*/
}

uint8_t getbit(Memory *mem, uint8_t addr)
{
	uint8_t *byteptr = bitbyteptr(mem, addr);
	if (byteptr) return (*byteptr) & (1 << addr%8);
	else return 0; /*ERR*/
}

void setbit(Memory *mem, uint8_t addr, uint8_t value)
{
	uint8_t *byteptr = bitbyteptr(mem, addr);
	if (byteptr)
	{
		if (value) *byteptr |=  (1 << addr%8);
		else       *byteptr &= ~(1 << addr%8);
	}
}


// MOV are RDM only

// ### Команды передачи данных

I(mov_a_rn) { ACCUM = Rn; }

I(mov_a_ad) { ACCUM = DATAMEM[INSTR(1)]; IncrPC_1; }

I(mov_a_rdm) { ACCUM = DATAMEM[Ri]; }

I(mov_a_d) { ACCUM = INSTR(1); IncrPC_1; }

I(mov_rn_a) { Rn = ACCUM; }

I(mov_rn_ad) { Rn = DATAMEM[INSTR(1)]; IncrPC_1; }

I(mov_rn_d) { Rn = INSTR(1); IncrPC_1; }

I(mov_ad_a) { DATAMEM[INSTR(1)] = ACCUM; IncrPC_1; }

I(mov_ad_rn) { DATAMEM[INSTR(1)] = Rn; IncrPC_1; }

I(mov_add_ads) { DATAMEM[INSTR(1)] = DATAMEM[INSTR(2)]; IncrPC_2; }

I(mov_ad_rdm) { DATAMEM[INSTR(1)] = DATAMEM[Ri]; IncrPC_1; }

I(mov_ad_d) { DATAMEM[INSTR(1)] = INSTR(2);  IncrPC_2; }

I(mov_rdm_a) { DATAMEM[Ri] = ACCUM; }

I(mov_rdm_ad) { DATAMEM[Ri] = DATAMEM[INSTR(1)]; IncrPC_1; }

I(mov_rdm_d) { DATAMEM[Ri] = INSTR(1); IncrPC_1; }

I(mov_dptr_d16) // Загрузка указателя данных
{
	// см стр. 59
	FUNREGS.DPTR.LH.DPH = INSTR(1);
	FUNREGS.DPTR.LH.DPL = INSTR(2);
	IncrPC_2;
}

I(movc_a_adptr) { ACCUM = E_PROGMEM[ACCUM + FUNREGS.DPTR.DPTR]; }

I(movc_a_apc) { ACCUM = E_PROGMEM[ACCUM + mem->PC + 1]; }

I(movx_a_edm) { ACCUM = E_DATAMEM[Ri]; }

I(movx_a_dptr) { ACCUM = E_DATAMEM[FUNREGS.DPTR.DPTR]; }

I(movx_edm_a) { E_DATAMEM[Ri] = ACCUM; }

I(movx_dptr_a) { E_DATAMEM[FUNREGS.DPTR.DPTR] = ACCUM; }

I(push_ad) { DATAMEM[++FUNREGS.SP] = DATAMEM[INSTR(1)]; IncrPC_1; }

I(pop_ad) { DATAMEM[INSTR(1)] = DATAMEM[FUNREGS.SP--]; IncrPC_1; }

I(xch_a_rn) { XCH(ACCUM, Rn); }

I(xch_a_ad) { XCH(ACCUM, DATAMEM[INSTR(1)]); IncrPC_1; }

I(xch_a_rdm) { XCH(ACCUM, DATAMEM[Ri]); }

I(xchd_a_rdm)
{
	uint8_t t = ACCUM;
	ACCUM = (ACCUM & 0xF0) | (DATAMEM[Ri] & 0x0F);
	DATAMEM[Ri] = (DATAMEM[Ri] & 0xF0) | (t & 0x0F);
}



// ### Арифметические операции

/*
	По результату выполнения команд ADD, ADDC, SUBB, MUL и
	DIV устанавливаются флаги PSW, структура которых приведена в
	табл. 4.
	
	Флаг С устанавливается при переносе из разряда D7, т. е. в случае,
	если результат не помещается в восемь разрядов; 
	
	Флаг АС устанавливается при переносе из разряда D3 в командах сложения и
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

#define ADD(a, b) \
	Cout = ((uint16_t)(a) + (b) + Cout > 0x00FF) ? 1 : 0; \
	PSWBITS.AC = (((a) & 0x0F) + ((b) & 0x0F)) > 0x0F ? 1 : 0; \
	PSWBITS.OV = \
	(                                                     \
		(((a) & 0x7F) + ((b) & 0x7F) > 0x7F && !(Cout)) || \
		(((a) & 0x7F) + ((b) & 0x7F) <= 0x7F && (Cout))    \
	) ? 1 : 0;                                            \
	(a) += (b);

#define ADDC(a, b, c) \
	unsigned new_c = ((uint16_t)(a) + (b) + (c) > 0x00FF) ? 1 : 0; \
	PSWBITS.AC = (((a) & 0x0F) + ((b) & 0x0F) + ((c) & 0x0F) > 0x0F) ? 1 : 0; \
	PSWBITS.OV = \
	(                                                                    \
		(((a) & 0x7F) + ((b) & 0x7F) + ((c) & 0x7F) > 0x7F && !new_c) || \
		(((a) & 0x7F) + ((b) & 0x7F) + ((c) & 0x7F) <= 0x7F && new_c)    \
	) ? 1 : 0;                                                           \
	(a) += (b + c); \
	(c) = new_c;

I(add_a_rn) { ADD(ACCUM, Rn); }

I(add_a_ad) { ADD(ACCUM, DATAMEM[INSTR(1)]); IncrPC_1; }

I(add_a_rdm) { ADD(ACCUM, DATAMEM[Ri]); }

I(add_a_d) { ADD(ACCUM, INSTR(1)); IncrPC_1; }

I(addc_a_rn) { ADDC(ACCUM, Rn, Cout); }

I(addc_a_ad) { ADDC(ACCUM, DATAMEM[INSTR(1)], Cout); IncrPC_1; }

I(addc_a_rdm) { ADDC(ACCUM, DATAMEM[Ri], Cout); }

I(addc_a_d) { ADDC(ACCUM, INSTR(1), Cout); IncrPC_1; }

I(da_a) // Десятичная коррекция аккумулятора
{
	if ((ACCUM & 0x0F) > 0x09 || PSWBITS.AC)
	{
		if ((uint16_t)ACCUM + 0x06 > 0xFF) Cout = 1;
		ACCUM = (ACCUM & 0xF0) | ((ACCUM + 0x06) & 0x0F);
	}
	if ((ACCUM & 0xF0) > 0x90 || Cout)
	{
		if ((uint16_t)ACCUM + 0x60 > 0xFF) Cout = 1;
		ACCUM = (ACCUM & 0x0F) | ((ACCUM + 0x60) & 0xF0);
	}
}


#define SUBB(a, b, c) \
	unsigned new_c = (a) < ((uint16_t)(b) + (c)) ? 1 : 0; \
	PSWBITS.AC = ((a) & 0x0F) < ((b) & 0x0F) + ((c) & 0x0F) ? 1 : 0; \
	PSWBITS.OV = \
	(                                                             \
		(((a) & 0x7F) < ((b) & 0x7F) + ((c) & 0x7F) && !new_c) || \
		(((a) & 0x7F) >= ((b) & 0x7F) + ((c) & 0x7F) && new_c)    \
	) ? 1 : 0;                                                    \
	(a) -= (b + c); \
	(c) = new_c;

I(subb_a_rn) { SUBB(ACCUM, Rn, Cout); }

I(subb_a_ad) { SUBB(ACCUM, DATAMEM[INSTR(1)], Cout); IncrPC_1; }

I(subb_a_rdm) { SUBB(ACCUM, DATAMEM[Ri], Cout); }

I(subb_a_d) { SUBB(ACCUM, INSTR(1), Cout); IncrPC_1; }


I(inc_a) { ++ACCUM; }

I(inc_rn) { ++Rn; }

I(inc_ad) { ++DATAMEM[INSTR(1)]; IncrPC_1; }

I(inc_rdm) { ++DATAMEM[Ri]; }

I(inc_dptr) { ++FUNREGS.DPTR.DPTR; }

I(dec_a) { --ACCUM; }

I(dec_rn) { --Rn; }

I(dec_ad) { --DATAMEM[INSTR(1)]; IncrPC_1; }

I(dec_rdm) { --DATAMEM[Ri]; }

I(mul_a_b)
{
	uint16_t result = (uint16_t)ACCUM * (uint16_t)FUNREGS.B;
	Cout = 0;
	PSWBITS.OV = result > 0xFF ? 1 : 0;
	
	ACCUM = result & 0x00FF;
	FUNREGS.B = (result & 0xFF00) >> 8;
}

I(div_a_b)
{
	Cout = 0;
	PSWBITS.OV = !FUNREGS.B;
	
	ACCUM = ACCUM / FUNREGS.B;
	FUNREGS.B = ACCUM % FUNREGS.B;
}




// ### Логические операции

I(anl_a_rn) { ACCUM &= Rn; }

I(anl_a_ad) { ACCUM &= DATAMEM[INSTR(1)]; IncrPC_1; }

I(anl_a_rdm) { ACCUM &= DATAMEM[Ri]; }

I(anl_a_d) { ACCUM &= INSTR(1); IncrPC_1; }

I(anl_ad_a) { DATAMEM[INSTR(1)] &= ACCUM; IncrPC_1; }

I(anl_ad_d) { DATAMEM[INSTR(1)] &= INSTR(2); IncrPC_2; }

I(orl_a_rn) { ACCUM |= Rn; }

I(orl_a_ad) { ACCUM |= DATAMEM[INSTR(1)]; IncrPC_1; }

I(orl_a_rdm) { ACCUM |= DATAMEM[Ri]; }

I(orl_a_d) { ACCUM |= INSTR(1); IncrPC_1; }

I(orl_ad_a) { DATAMEM[INSTR(1)] |= ACCUM; IncrPC_1; }

I(orl_ad_d) { DATAMEM[INSTR(1)] |= INSTR(2); IncrPC_2; }

I(xrl_a_rn) { ACCUM ^= Rn; }

I(xrl_a_ad) { ACCUM ^= DATAMEM[INSTR(1)]; IncrPC_1; }

I(xrl_a_rdm) { ACCUM ^= DATAMEM[Ri]; }

I(xrl_a_d) { ACCUM ^= INSTR(1); IncrPC_1; }

I(xrl_ad_a) { DATAMEM[INSTR(1)] ^= ACCUM; IncrPC_1; }

I(xrl_ad_d) { DATAMEM[INSTR(1)] ^= INSTR(2); IncrPC_2; }

// Accum is uint8_t

I(clr_a) { ACCUM = 0; }

I(cpl_a) { ACCUM = ~ACCUM; }

I(rl_a) { ACCUM = (ACCUM << 1) | (ACCUM >> 7); }

I(rlc_a)
{
	unsigned t = ACCUM >> 7;
	ACCUM = (ACCUM << 1) | Cout;
	Cout = t;
}

I(rr_a) { ACCUM = (ACCUM >> 1) | (ACCUM << 7); }

I(rrc_a)
{
	unsigned t = ACCUM & 0x01;
	ACCUM = (ACCUM >> 1) | (Cout << 7);
	Cout = t;
}

I(swap_a) { ACCUM = ((ACCUM & 0x0F) << 4) | ((ACCUM & 0xF0) >> 4); }



// ### Команды передачи управления
/*
 * Команда CJNE устанавливает флаг C,
 * если первый операнд оказывается меньше второго. 
 * Команда JBC сбрасывает флаг C в случае перехода
 */

// 1, а не 2, потому что после исполнения инструкции PC инкрементится на 1
// 2, а не 3

I(ljmp_ad16) { mem->PC = (uint16_t)INSTR(1) << 8 | INSTR(2); DecPC_1; }

I(ajmp_ad11) { mem->PC = ((mem->PC + 2) & 0xF800) | (uint16_t)INSTR(1) | (uint16_t)(INSTR(0) & 0xE0) << 3; DecPC_1; }

I(sjmp_rel) { mem->PC += ((int8_t)INSTR(1) + 1/*2*/); }

I(jmp_a_dptr) { mem->PC = ACCUM + FUNREGS.DPTR.DPTR; DecPC_1; }


// def_len - длина инструкции в байтах. Я указываю на 1 меньше, потому что в функции execute к PC прибавляется ещё 1
// def_cond - условие перехода
// def_offset - относительный адрес
#define JMP_IF(def_len, def_cond, def_rel) mem->PC += def_len + (int8_t)((def_cond) ? (int8_t)(def_rel) : 0);

                 /*2*/
I(jz_rel) { JMP_IF(1, !ACCUM, INSTR(1)); }

I(jnz_rel) { JMP_IF(1, ACCUM, INSTR(1)); }

I(jc_rel) { JMP_IF(1, Cout, INSTR(1)); }

I(jnc_rel) { JMP_IF(1, !Cout, INSTR(1)); }

I(jb_bit_rel) { JMP_IF(2, getbit(mem, INSTR(1)), INSTR(2)) }

I(jnb_bit_rel) { JMP_IF(2, !getbit(mem, INSTR(1)), INSTR(2)) }

I(jbc_bit_rel)
{
	if (getbit(mem, INSTR(1)))
	{
		setbit(mem, INSTR(1), 0);
		mem->PC += (int8_t)INSTR(2);
		Cout = 0;
	}
	IncrPC_2;
}

I(djnz_rn_rel) { --Rn; JMP_IF(1, Rn, INSTR(1)); }

I(djnz_ad_rel) { --DATAMEM[INSTR(1)]; JMP_IF(2, DATAMEM[INSTR(1)], INSTR(2)); }

I(cjne_a_ad_rel) { Cout = ACCUM < DATAMEM[INSTR(1)] ? 1 : 0; JMP_IF(2, ACCUM != DATAMEM[INSTR(1)], INSTR(2)); }

I(cjne_a_d_rel) { Cout = ACCUM < INSTR(1) ? 1 : 0; JMP_IF(2, ACCUM != INSTR(1), INSTR(2)); }

I(cjne_rn_d_rel) { Cout = Rn < INSTR(1) ? 1 : 0; JMP_IF(2, Rn != INSTR(1), INSTR(2)); }

I(cjne_rdm_d_rel) { Cout = DATAMEM[Ri] < INSTR(1) ? 1 : 0; JMP_IF(2, DATAMEM[Ri] != INSTR(1), INSTR(2)); }

I(lcall_ad16)
{
	DATAMEM[++FUNREGS.SP] = (uint8_t)((mem->PC+3) & 0x00FF); 
	DATAMEM[++FUNREGS.SP] = (uint8_t)(((mem->PC+3) & 0xFF00) >> 8); 
	mem->PC = ((uint16_t)INSTR(1) << 8) | (INSTR(2));
	DecPC_1;
}

I(acall_ad11)
{
	DATAMEM[++FUNREGS.SP] = (uint8_t)((mem->PC+2) & 0x00FF); 
	DATAMEM[++FUNREGS.SP] = (uint8_t)(((mem->PC+2) & 0xFF00) >> 8); 
	mem->PC = ((mem->PC+2) & 0xF800) | INSTR(1) | (uint16_t)(INSTR(0) & 0xE0) << 3;
	DecPC_1;
}

I(ret) 
{
	mem->PC = ((uint16_t)(DATAMEM[FUNREGS.SP]) << 8) | (uint16_t)(DATAMEM[FUNREGS.SP - 1]); 
	FUNREGS.SP -= 2;
	DecPC_1;
}

I(reti) { ret(mem); }

I(nop) { (void)mem; } // Чтобы компилятор не ругался



// ### Операции с битами

I(clr_c)  { Cout = 0; }

I(clr_bit)  { setbit(mem, INSTR(1), 0); IncrPC_1; }

I(setb_c) { Cout = 1; }

I(setb_bit) { setbit(mem, INSTR(1), 1); IncrPC_1; }

I(cpl_c) { Cout = Cout ? 0 : 1; }

I(cpl_bit) { setbit(mem, INSTR(1), getbit(mem, INSTR(1)) ? 0 : 1); IncrPC_1; }

I(anl_c_bit) { Cout = (Cout && getbit(mem, INSTR(1))) ? 1 : 0; IncrPC_1; }

I(anl_c_nbit) { Cout = (Cout && !getbit(mem, INSTR(1))) ? 1 : 0; IncrPC_1; }

I(orl_c_bit) { Cout = (Cout || getbit(mem, INSTR(1))) ? 1 : 0; IncrPC_1; }

I(orl_c_nbit) { Cout = (Cout || !getbit(mem, INSTR(1))) ? 1 : 0; IncrPC_1; }

I(mov_c_bit) { Cout = getbit(mem, INSTR(1)) ? 1 : 0; IncrPC_1; }

I(mov_bit_c) { setbit(mem, INSTR(1), Cout); IncrPC_1; }



// INSTRUCTIONS ARRAY

typedef struct Instruction_storage
{
	void (*i)(Memory*);
	unsigned int n_bytes;
	unsigned int n_ticks;
	char *mnemonic_str;
} Instruction_storage;

Instruction_storage instr[256] = 
{
	// [instruction]    [n_bytes]    [n_ticks]
	
	{nop,            1, 1, NULL}, // [0x00] [0b00000000]
	{ajmp_ad11,      2, 2, NULL}, // [0x01] [0b00000001]
	{ljmp_ad16,      3, 2, NULL}, // [0x02] [0b00000010]
	{rr_a,           1, 1, NULL}, // [0x03] [0b00000011]
	{inc_a,          1, 1, NULL}, // [0x04] [0b00000100]
	{inc_ad,         2, 1, NULL}, // [0x05] [0b00000101]
	{inc_rdm,        1, 1, NULL}, // [0x06] [0b00000110]
	{inc_rdm,        1, 1, NULL}, // [0x07] [0b00000111]
	{inc_rn,         1, 1, NULL}, // [0x08] [0b00001000]
	{inc_rn,         1, 1, NULL}, // [0x09] [0b00001001]
	{inc_rn,         1, 1, NULL}, // [0x0a] [0b00001010]
	{inc_rn,         1, 1, NULL}, // [0x0b] [0b00001011]
	{inc_rn,         1, 1, NULL}, // [0x0c] [0b00001100]
	{inc_rn,         1, 1, NULL}, // [0x0d] [0b00001101]
	{inc_rn,         1, 1, NULL}, // [0x0e] [0b00001110]
	{inc_rn,         1, 1, NULL}, // [0x0f] [0b00001111]
	{jbc_bit_rel,    3, 2, NULL}, // [0x10] [0b00010000]
	{acall_ad11,     2, 2, NULL}, // [0x11] [0b00010001]
	{lcall_ad16,     3, 2, NULL}, // [0x12] [0b00010010]
	{rrc_a,          1, 1, NULL}, // [0x13] [0b00010011]
	{dec_a,          1, 1, NULL}, // [0x14] [0b00010100]
	{dec_ad,         2, 1, NULL}, // [0x15] [0b00010101]
	{dec_rdm,        1, 1, NULL}, // [0x16] [0b00010110]
	{dec_rdm,        1, 1, NULL}, // [0x17] [0b00010111]
	{dec_rn,         1, 1, NULL}, // [0x18] [0b00011000]
	{dec_rn,         1, 1, NULL}, // [0x19] [0b00011001]
	{dec_rn,         1, 1, NULL}, // [0x1a] [0b00011010]
	{dec_rn,         1, 1, NULL}, // [0x1b] [0b00011011]
	{dec_rn,         1, 1, NULL}, // [0x1c] [0b00011100]
	{dec_rn,         1, 1, NULL}, // [0x1d] [0b00011101]
	{dec_rn,         1, 1, NULL}, // [0x1e] [0b00011110]
	{dec_rn,         1, 1, NULL}, // [0x1f] [0b00011111]
	{jb_bit_rel,     3, 2, NULL}, // [0x20] [0b00100000]
	{ajmp_ad11,      2, 2, NULL}, // [0x21] [0b00100001]
	{ret,            1, 2, NULL}, // [0x22] [0b00100010]
	{rl_a,           1, 1, NULL}, // [0x23] [0b00100011]
	{add_a_d,        2, 1, NULL}, // [0x24] [0b00100100]
	{add_a_ad,       2, 1, NULL}, // [0x25] [0b00100101]
	{add_a_rdm,      1, 1, NULL}, // [0x26] [0b00100110]
	{add_a_rdm,      1, 1, NULL}, // [0x27] [0b00100111]
	{add_a_rn,       1, 1, NULL}, // [0x28] [0b00101000]
	{add_a_rn,       1, 1, NULL}, // [0x29] [0b00101001]
	{add_a_rn,       1, 1, NULL}, // [0x2a] [0b00101010]
	{add_a_rn,       1, 1, NULL}, // [0x2b] [0b00101011]
	{add_a_rn,       1, 1, NULL}, // [0x2c] [0b00101100]
	{add_a_rn,       1, 1, NULL}, // [0x2d] [0b00101101]
	{add_a_rn,       1, 1, NULL}, // [0x2e] [0b00101110]
	{add_a_rn,       1, 1, NULL}, // [0x2f] [0b00101111]
	{jnb_bit_rel,    3, 2, NULL}, // [0x30] [0b00110000]
	{acall_ad11,     2, 2, NULL}, // [0x31] [0b00110001]
	{reti,           1, 2, NULL}, // [0x32] [0b00110010]
	{rlc_a,          1, 1, NULL}, // [0x33] [0b00110011]
	{addc_a_d,       2, 1, NULL}, // [0x34] [0b00110100]
	{addc_a_ad,      2, 1, NULL}, // [0x35] [0b00110101]
	{addc_a_rdm,     1, 1, NULL}, // [0x36] [0b00110110]
	{addc_a_rdm,     1, 1, NULL}, // [0x37] [0b00110111]
	{addc_a_rn,      1, 1, NULL}, // [0x38] [0b00111000]
	{addc_a_rn,      1, 1, NULL}, // [0x39] [0b00111001]
	{addc_a_rn,      1, 1, NULL}, // [0x3a] [0b00111010]
	{addc_a_rn,      1, 1, NULL}, // [0x3b] [0b00111011]
	{addc_a_rn,      1, 1, NULL}, // [0x3c] [0b00111100]
	{addc_a_rn,      1, 1, NULL}, // [0x3d] [0b00111101]
	{addc_a_rn,      1, 1, NULL}, // [0x3e] [0b00111110]
	{addc_a_rn,      1, 1, NULL}, // [0x3f] [0b00111111]
	{jc_rel,         2, 2, NULL}, // [0x40] [0b01000000]
	{ajmp_ad11,      2, 2, NULL}, // [0x41] [0b01000001]
	{orl_ad_a,       2, 1, NULL}, // [0x42] [0b01000010]
	{orl_ad_d,       3, 2, NULL}, // [0x43] [0b01000011]
	{orl_a_d,        2, 1, NULL}, // [0x44] [0b01000100]
	{orl_a_ad,       2, 1, NULL}, // [0x45] [0b01000101]
	{orl_a_rdm,      1, 1, NULL}, // [0x46] [0b01000110]
	{orl_a_rdm,      1, 1, NULL}, // [0x47] [0b01000111]
	{orl_a_rn,       1, 1, NULL}, // [0x48] [0b01001000]
	{orl_a_rn,       1, 1, NULL}, // [0x49] [0b01001001]
	{orl_a_rn,       1, 1, NULL}, // [0x4a] [0b01001010]
	{orl_a_rn,       1, 1, NULL}, // [0x4b] [0b01001011]
	{orl_a_rn,       1, 1, NULL}, // [0x4c] [0b01001100]
	{orl_a_rn,       1, 1, NULL}, // [0x4d] [0b01001101]
	{orl_a_rn,       1, 1, NULL}, // [0x4e] [0b01001110]
	{orl_a_rn,       1, 1, NULL}, // [0x4f] [0b01001111]
	{jnc_rel,        2, 2, NULL}, // [0x50] [0b01010000]
	{acall_ad11,     2, 2, NULL}, // [0x51] [0b01010001]
	{anl_ad_a,       2, 1, NULL}, // [0x52] [0b01010010]
	{anl_ad_d,       3, 2, NULL}, // [0x53] [0b01010011]
	{anl_a_d,        2, 1, NULL}, // [0x54] [0b01010100]
	{anl_a_ad,       2, 1, NULL}, // [0x55] [0b01010101]
	{anl_a_rdm,      1, 1, NULL}, // [0x56] [0b01010110]
	{anl_a_rdm,      1, 1, NULL}, // [0x57] [0b01010111]
	{anl_a_rn,       1, 1, NULL}, // [0x58] [0b01011000]
	{anl_a_rn,       1, 1, NULL}, // [0x59] [0b01011001]
	{anl_a_rn,       1, 1, NULL}, // [0x5a] [0b01011010]
	{anl_a_rn,       1, 1, NULL}, // [0x5b] [0b01011011]
	{anl_a_rn,       1, 1, NULL}, // [0x5c] [0b01011100]
	{anl_a_rn,       1, 1, NULL}, // [0x5d] [0b01011101]
	{anl_a_rn,       1, 1, NULL}, // [0x5e] [0b01011110]
	{anl_a_rn,       1, 1, NULL}, // [0x5f] [0b01011111]
	{jz_rel,         2, 2, NULL}, // [0x60] [0b01100000]
	{ajmp_ad11,      2, 2, NULL}, // [0x61] [0b01100001]
	{xrl_ad_a,       2, 1, NULL}, // [0x62] [0b01100010]
	{xrl_ad_d,       3, 2, NULL}, // [0x63] [0b01100011]
	{xrl_a_d,        2, 1, NULL}, // [0x64] [0b01100100]
	{xrl_a_ad,       2, 1, NULL}, // [0x65] [0b01100101]
	{xrl_a_rdm,      1, 1, NULL}, // [0x66] [0b01100110]
	{xrl_a_rdm,      1, 1, NULL}, // [0x67] [0b01100111]
	{xrl_a_rn,       1, 1, NULL}, // [0x68] [0b01101000]
	{xrl_a_rn,       1, 1, NULL}, // [0x69] [0b01101001]
	{xrl_a_rn,       1, 1, NULL}, // [0x6a] [0b01101010]
	{xrl_a_rn,       1, 1, NULL}, // [0x6b] [0b01101011]
	{xrl_a_rn,       1, 1, NULL}, // [0x6c] [0b01101100]
	{xrl_a_rn,       1, 1, NULL}, // [0x6d] [0b01101101]
	{xrl_a_rn,       1, 1, NULL}, // [0x6e] [0b01101110]
	{xrl_a_rn,       1, 1, NULL}, // [0x6f] [0b01101111]
	{jnz_rel,        2, 2, NULL}, // [0x70] [0b01110000]
	{acall_ad11,     2, 2, NULL}, // [0x71] [0b01110001]
	{orl_c_bit,      2, 2, NULL}, // [0x72] [0b01110010]
	{jmp_a_dptr,     1, 2, NULL}, // [0x73] [0b01110011]
	{mov_a_d,        2, 1, NULL}, // [0x74] [0b01110100]
	{mov_ad_d,       3, 2, NULL}, // [0x75] [0b01110101]
	{mov_rdm_d,      2, 1, NULL}, // [0x76] [0b01110110]
	{mov_rdm_d,      2, 1, NULL}, // [0x77] [0b01110111]
	{mov_rn_d,       2, 1, NULL}, // [0x78] [0b01111000]
	{mov_rn_d,       2, 1, NULL}, // [0x79] [0b01111001]
	{mov_rn_d,       2, 1, NULL}, // [0x7a] [0b01111010]
	{mov_rn_d,       2, 1, NULL}, // [0x7b] [0b01111011]
	{mov_rn_d,       2, 1, NULL}, // [0x7c] [0b01111100]
	{mov_rn_d,       2, 1, NULL}, // [0x7d] [0b01111101]
	{mov_rn_d,       2, 1, NULL}, // [0x7e] [0b01111110]
	{mov_rn_d,       2, 1, NULL}, // [0x7f] [0b01111111]
	{sjmp_rel,       2, 2, NULL}, // [0x80] [0b10000000]
	{ajmp_ad11,      2, 2, NULL}, // [0x81] [0b10000001]
	{anl_c_bit,      2, 2, NULL}, // [0x82] [0b10000010]
	{movc_a_apc,     1, 2, NULL}, // [0x83] [0b10000011]
	{div_a_b,        1, 4, NULL}, // [0x84] [0b10000100]
	{mov_add_ads,    3, 2, NULL}, // [0x85] [0b10000101]
	{mov_ad_rdm,     2, 2, NULL}, // [0x86] [0b10000110]
	{mov_ad_rdm,     2, 2, NULL}, // [0x87] [0b10000111]
	{mov_ad_rn,      2, 2, NULL}, // [0x88] [0b10001000]
	{mov_ad_rn,      2, 2, NULL}, // [0x89] [0b10001001]
	{mov_ad_rn,      2, 2, NULL}, // [0x8a] [0b10001010]
	{mov_ad_rn,      2, 2, NULL}, // [0x8b] [0b10001011]
	{mov_ad_rn,      2, 2, NULL}, // [0x8c] [0b10001100]
	{mov_ad_rn,      2, 2, NULL}, // [0x8d] [0b10001101]
	{mov_ad_rn,      2, 2, NULL}, // [0x8e] [0b10001110]
	{mov_ad_rn,      2, 2, NULL}, // [0x8f] [0b10001111]
	{mov_dptr_d16,   3, 2, NULL}, // [0x90] [0b10010000]
	{acall_ad11,     2, 2, NULL}, // [0x91] [0b10010001]
	{mov_bit_c,      2, 2, NULL}, // [0x92] [0b10010010]
	{movc_a_adptr,   1, 2, NULL}, // [0x93] [0b10010011]
	{subb_a_d,       2, 1, NULL}, // [0x94] [0b10010100]
	{subb_a_ad,      2, 1, NULL}, // [0x95] [0b10010101]
	{subb_a_rdm,     1, 1, NULL}, // [0x96] [0b10010110]
	{subb_a_rdm,     1, 1, NULL}, // [0x97] [0b10010111]
	{subb_a_rn,      1, 1, NULL}, // [0x98] [0b10011000]
	{subb_a_rn,      1, 1, NULL}, // [0x99] [0b10011001]
	{subb_a_rn,      1, 1, NULL}, // [0x9a] [0b10011010]
	{subb_a_rn,      1, 1, NULL}, // [0x9b] [0b10011011]
	{subb_a_rn,      1, 1, NULL}, // [0x9c] [0b10011100]
	{subb_a_rn,      1, 1, NULL}, // [0x9d] [0b10011101]
	{subb_a_rn,      1, 1, NULL}, // [0x9e] [0b10011110]
	{subb_a_rn,      1, 1, NULL}, // [0x9f] [0b10011111]
	{orl_c_nbit,     2, 2, NULL}, // [0xa0] [0b10100000]
	{ajmp_ad11,      2, 2, NULL}, // [0xa1] [0b10100001]
	{mov_c_bit,      2, 1, NULL}, // [0xa2] [0b10100010]
	{inc_dptr,       1, 2, NULL}, // [0xa3] [0b10100011]
	{mul_a_b,        1, 4, NULL}, // [0xa4] [0b10100100]
	{NULL,           0, 0, NULL}, // [0xa5] [0b10100101]
	{mov_rdm_ad,     2, 2, NULL}, // [0xa6] [0b10100110]
	{mov_rdm_ad,     2, 2, NULL}, // [0xa7] [0b10100111]
	{mov_rn_ad,      2, 2, NULL}, // [0xa8] [0b10101000]
	{mov_rn_ad,      2, 2, NULL}, // [0xa9] [0b10101001]
	{mov_rn_ad,      2, 2, NULL}, // [0xaa] [0b10101010]
	{mov_rn_ad,      2, 2, NULL}, // [0xab] [0b10101011]
	{mov_rn_ad,      2, 2, NULL}, // [0xac] [0b10101100]
	{mov_rn_ad,      2, 2, NULL}, // [0xad] [0b10101101]
	{mov_rn_ad,      2, 2, NULL}, // [0xae] [0b10101110]
	{mov_rn_ad,      2, 2, NULL}, // [0xaf] [0b10101111]
	{anl_c_nbit,     2, 2, NULL}, // [0xb0] [0b10110000]
	{acall_ad11,     2, 2, NULL}, // [0xb1] [0b10110001]
	{cpl_bit,        2, 1, NULL}, // [0xb2] [0b10110010]
	{cpl_c,          1, 1, NULL}, // [0xb3] [0b10110011]
	{cjne_a_d_rel,   3, 2, NULL}, // [0xb4] [0b10110100]
	{cjne_a_ad_rel,  3, 2, NULL}, // [0xb5] [0b10110101]
	{cjne_rdm_d_rel, 3, 2, NULL}, // [0xb6] [0b10110110]
	{cjne_rdm_d_rel, 3, 2, NULL}, // [0xb7] [0b10110111]
	{cjne_rn_d_rel,  3, 2, NULL}, // [0xb8] [0b10111000]
	{cjne_rn_d_rel,  3, 2, NULL}, // [0xb9] [0b10111001]
	{cjne_rn_d_rel,  3, 2, NULL}, // [0xba] [0b10111010]
	{cjne_rn_d_rel,  3, 2, NULL}, // [0xbb] [0b10111011]
	{cjne_rn_d_rel,  3, 2, NULL}, // [0xbc] [0b10111100]
	{cjne_rn_d_rel,  3, 2, NULL}, // [0xbd] [0b10111101]
	{cjne_rn_d_rel,  3, 2, NULL}, // [0xbe] [0b10111110]
	{cjne_rn_d_rel,  3, 2, NULL}, // [0xbf] [0b10111111]
	{push_ad,        2, 2, NULL}, // [0xc0] [0b11000000]
	{ajmp_ad11,      2, 2, NULL}, // [0xc1] [0b11000001]
	{clr_bit,        2, 1, NULL}, // [0xc2] [0b11000010]
	{clr_c,          1, 1, NULL}, // [0xc3] [0b11000011]
	{swap_a,         1, 1, NULL}, // [0xc4] [0b11000100]
	{xch_a_ad,       2, 1, NULL}, // [0xc5] [0b11000101]
	{xch_a_rdm,      1, 1, NULL}, // [0xc6] [0b11000110]
	{xch_a_rdm,      1, 1, NULL}, // [0xc7] [0b11000111]
	{xch_a_rn,       1, 1, NULL}, // [0xc8] [0b11001000]
	{xch_a_rn,       1, 1, NULL}, // [0xc9] [0b11001001]
	{xch_a_rn,       1, 1, NULL}, // [0xca] [0b11001010]
	{xch_a_rn,       1, 1, NULL}, // [0xcb] [0b11001011]
	{xch_a_rn,       1, 1, NULL}, // [0xcc] [0b11001100]
	{xch_a_rn,       1, 1, NULL}, // [0xcd] [0b11001101]
	{xch_a_rn,       1, 1, NULL}, // [0xce] [0b11001110]
	{xch_a_rn,       1, 1, NULL}, // [0xcf] [0b11001111]
	{pop_ad,         2, 2, NULL}, // [0xd0] [0b11010000]
	{acall_ad11,     2, 2, NULL}, // [0xd1] [0b11010001]
	{setb_bit,       2, 1, NULL}, // [0xd2] [0b11010010]
	{setb_c,         1, 1, NULL}, // [0xd3] [0b11010011]
	{da_a,           1, 1, NULL}, // [0xd4] [0b11010100]
	{djnz_ad_rel,    3, 2, NULL}, // [0xd5] [0b11010101]
	{xchd_a_rdm,     1, 1, NULL}, // [0xd6] [0b11010110]
	{xchd_a_rdm,     1, 1, NULL}, // [0xd7] [0b11010111]
	{djnz_rn_rel,    2, 2, NULL}, // [0xd8] [0b11011000]
	{djnz_rn_rel,    2, 2, NULL}, // [0xd9] [0b11011001]
	{djnz_rn_rel,    2, 2, NULL}, // [0xda] [0b11011010]
	{djnz_rn_rel,    2, 2, NULL}, // [0xdb] [0b11011011]
	{djnz_rn_rel,    2, 2, NULL}, // [0xdc] [0b11011100]
	{djnz_rn_rel,    2, 2, NULL}, // [0xdd] [0b11011101]
	{djnz_rn_rel,    2, 2, NULL}, // [0xde] [0b11011110]
	{djnz_rn_rel,    2, 2, NULL}, // [0xdf] [0b11011111]
	{movx_a_dptr,    1, 2, NULL}, // [0xe0] [0b11100000]
	{ajmp_ad11,      2, 2, NULL}, // [0xe1] [0b11100001]
	{movx_a_edm,     1, 2, NULL}, // [0xe2] [0b11100010]
	{movx_a_edm,     1, 2, NULL}, // [0xe3] [0b11100011]
	{clr_a,          1, 1, NULL}, // [0xe4] [0b11100100]
	{mov_a_ad,       2, 1, NULL}, // [0xe5] [0b11100101]
	{mov_a_rdm,      1, 1, NULL}, // [0xe6] [0b11100110]
	{mov_a_rdm,      1, 1, NULL}, // [0xe7] [0b11100111]
	{mov_a_rn,       1, 1, NULL}, // [0xe8] [0b11101000]
	{mov_a_rn,       1, 1, NULL}, // [0xe9] [0b11101001]
	{mov_a_rn,       1, 1, NULL}, // [0xea] [0b11101010]
	{mov_a_rn,       1, 1, NULL}, // [0xeb] [0b11101011]
	{mov_a_rn,       1, 1, NULL}, // [0xec] [0b11101100]
	{mov_a_rn,       1, 1, NULL}, // [0xed] [0b11101101]
	{mov_a_rn,       1, 1, NULL}, // [0xee] [0b11101110]
	{mov_a_rn,       1, 1, NULL}, // [0xef] [0b11101111]
	{movx_dptr_a,    1, 2, NULL}, // [0xf0] [0b11110000]
	{acall_ad11,     2, 2, NULL}, // [0xf1] [0b11110001]
	{movx_edm_a,     1, 2, NULL}, // [0xf2] [0b11110010]
	{movx_edm_a,     1, 2, NULL}, // [0xf3] [0b11110011]
	{cpl_a,          1, 1, NULL}, // [0xf4] [0b11110100]
	{mov_ad_a,       2, 1, NULL}, // [0xf5] [0b11110101]
	{mov_rdm_a,      1, 1, NULL}, // [0xf6] [0b11110110]
	{mov_rdm_a,      1, 1, NULL}, // [0xf7] [0b11110111]
	{mov_rn_a,       1, 1, NULL}, // [0xf8] [0b11111000]
	{mov_rn_a,       1, 1, NULL}, // [0xf9] [0b11111001]
	{mov_rn_a,       1, 1, NULL}, // [0xfa] [0b11111010]
	{mov_rn_a,       1, 1, NULL}, // [0xfb] [0b11111011]
	{mov_rn_a,       1, 1, NULL}, // [0xfc] [0b11111100]
	{mov_rn_a,       1, 1, NULL}, // [0xfd] [0b11111101]
	{mov_rn_a,       1, 1, NULL}, // [0xfe] [0b11111110]
	{mov_rn_a,       1, 1, NULL}  // [0xff] [0b11111111]
};

#endif
