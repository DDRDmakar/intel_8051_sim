

#ifndef __INSTRUCTIONS__
#define __INSTRUCTIONS__

#include <stdint.h>

#include "headers/memory.h"

/*
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
}

#endif
