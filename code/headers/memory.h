
#ifndef __MEM__
#define __MEM__

#define RPM_SIZE 4096
#define EPM_SIZE 65536
#define RDM_SIZE 256
#define EDM_SIZE 65536

#include <stdint.h>

// TODO bit addressing features (page 19)

typedef struct Memory
{
	union Data_memory
	{
		// First 48 bytes support bit-addressing mode
		
		// Резидентная память и регистры общего назначения на ней
		struct Rdm_and_xfr
		{
			// 8 bit address
			// Addresses [00 - 7F]
			uint8_t MEM[128]; // 128 resident data memory
			uint8_t P0; //  [80] SFR port 0
			uint8_t SP; //  [81] Stack pointer
			
			union Dptr  // Data pointer [82-83]
			{
				uint16_t DPTR; // [82-83]
				struct Lh
				{
					uint8_t DPL; // [82] DPTR Data pointer low
					uint8_t DPH; // [83] DPTR Data pointer high
				} LH;
			} DPTR;
			
			int32_t: 24;  //     Skip [84-86] 3 bytes
			uint8_t PCON; // [87] Power control register
			uint8_t TCON; // [88] Timer/counter control register
			uint8_t TMOD; // [89] Timer/counter mode control register
			uint8_t TL0;  // [8A] Младший байт таймера 0
			uint8_t TL1;  // [8B] Младший байт таймера 1
			uint8_t TH0;  // [8C] Старший байт таймера 0
			uint8_t TH1;  // [8D] Старший байт таймера 1
			int16_t: 16;  //     Skip [8E-8F]
			uint8_t P1;   // [90] SFR port 1
			int64_t: 56;  //     Skip [91-97] 7 bytes
			uint8_t SCON; // [98] Serial port control register
			uint8_t SBUF; // [99] Serial data buffer
			uint64_t: 48; //     Skip [9A-9F] 6 bytes
			uint8_t P2;   // [A0] SFR port 2
			int64_t: 56;  //     Skip [A1-A7] 7 bytes
			uint8_t IE;   // [A8] Interrupt enable register
			int64_t: 56;  //     Skip [A9-AF] 7 bytes
			uint8_t P3;   // [B0] SFR port 3
			int64_t: 56;  //     Skip [B1-B7] 7 bytes
			uint8_t IP;   // [B8] Interrupt priority control register
			int64_t: 64;  //     Skip [B9-CF] 23 bytes = 184 bits
			int64_t: 64;  //     128
			//int64_t: 56;  //     184
			union Psw  // [D0] Program status word
			{
				uint8_t PSW;
				struct Psw_bits
				{
					unsigned C: 1; // Флаг переноса. Устанавливается и сбрасывается аппаратно или программно при выполнении арифметических и логических операций
					unsigned AC: 1; // Флаг вспомогательного переноса. Устанавливается и сбрасывается только аппаратно при выполнении команд сложения и вычитания и сигнализирует о переносе или займе в бите 3
					unsigned F0: 1; // Флаг 0. Может быть установлен, сброшен или проверен программой как флаг, специфицируемый пользователем
					unsigned RS1: 1; // Выбор банка регистров. Устанавливается и сбрасывается программно для выбора рабочего банка регистров (таблица 3)
					unsigned RS0: 1; // --//--
					unsigned OV: 1; // Флаг переполнения. Устанавливается и сбрасывается аппаратно при  выполнении арифметических операций
					unsigned: 1;
					unsigned P: 1; // Флаг паритета. Устанавливается и сбрасывается аппаратно в каждом цикле и фиксирует нечётное/чётное число единичных битов в аккумуляторе, т.е. выполняет контроль по четности
				} BITS;
			} PSW;
			int64_t: 64;  //     Skip [D1-DF] 15 bytes = 120 bits
			//int64_t: 56;  //     120
			uint8_t ACC;  // [E0] Accumulator
			int64_t: 64;  //     Skip [E1-EF] 15 bytes = 120 bits
			//int64_t: 56;  //     120
			uint8_t B;    // [F0] Multiplication register
		} RDM_REG;
		
		// Резидентная память в виде массива (с регистрами)
		uint8_t RDM[256];
		
		// Регистры общего назначения
		// Addresses 00 - 1F
		uint8_t bank_POH[4][8]; // 4 banks of 8-byte registers
		
		// 16 bit address
		// Addresses 0000 - FFFF
		uint8_t EDM[65536]; // 64K external data memory
		
		
	} DM;
	
	union Program_memory
	{
		// 16 bit address
		uint8_t RPM[4096];  // 4K resident program memory
		// 16 bit address
		uint8_t EPM[65536]; // 64K external program memory
	} PM;
	
	char **DM_str;
	char **PM_str;
	
	size_t PC;
} Memory; // Defined type

#endif
