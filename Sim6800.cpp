// Sim6800.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock2.h>

#pragma comment(lib, "wsock32.lib")


#define STUDENT_NUMBER    "14018789"

#define IP_ADDRESS_SERVER "127.0.0.1"

#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.


#define WORD  unsigned short
#define DWORD unsigned long
#define BYTE  unsigned char

#define MAX_FILENAME_SIZE 500

#define MAX_BUFFER_SIZE   500



SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;




char InputBuffer [MAX_BUFFER_SIZE];

char hex_file [MAX_BUFFER_SIZE];
char trc_file [MAX_BUFFER_SIZE];



//////////////////////////
// Intel 6800 Registers //
//////////////////////////

#define REGISTER_A	0
#define REGISTER_B	1

#define FLAG_N	0x08
#define FLAG_Z	0x04
#define FLAG_H	0x20
#define FLAG_I	0x10
#define FLAG_V	0x02
#define FLAG_C	0x01

BYTE Registers[2];
BYTE Flags;
WORD IndexRegister;
WORD ProgramCounter;
WORD StackPointer;

////////////
// Memory //
////////////

#define K_1			1024
#define MEMORY_SIZE	K_1

BYTE Memory[MEMORY_SIZE];

#define TEST_ADDRESS_1  0x00FD
#define TEST_ADDRESS_2  0x00FE
#define TEST_ADDRESS_3  0x00FF
#define TEST_ADDRESS_4  0x0100
#define TEST_ADDRESS_5  0x0101


///////////////////////
// Control variables //
///////////////////////

bool memory_in_range = true;
bool halt = false;

///////////////////////
// Disassembly table //
///////////////////////

char opcode_mneumonics[][12] =
{
	"Illegal    ", 
	"NOP        ", 
	"Illegal    ", 
	"Illegal    ", 
	"Illegal    ", 
	"Illegal    ", 
	"TAP        ", 
	"TPA        ", 
	"INX        ", 
	"DEX        ", 
	"CLV        ", 
	"SEV        ", 
	"CLC        ", 
	"SEC        ", 
	"CLI        ", 
	"SEI        ", 

	"SBA        ", 
	"CBA        ", 
	"Illegal    ", 
	"Illegal    ", 
	"Illegal    ", 
	"Illegal    ", 
	"TAB        ", 
	"TBA        ", 
	"Illegal    ", 
	"DAA        ", 
	"Illegal    ", 
	"ABA        ", 
	"Illegal    ", 
	"Illegal    ", 
	"Illegal    ", 
	"Illegal    ", 

	"BRA        ", 
	"Illegal    ", 
	"BHI        ", 
	"BLS        ", 
	"BCC        ", 
	"BCS        ", 
	"BNE        ", 
	"BEQ        ", 
	"BVC        ", 
	"BVS        ", 
	"BPL        ", 
	"BMI        ", 
	"BGE        ", 
	"BLT        ", 
	"BGT        ", 
	"BLE        ", 

	"TSX        ", 
	"INS        ", 
	"PULA       ", 
	"PULB       ", 
	"DES        ", 
	"TXS        ", 
	"PSHA       ", 
	"PSHB       ", 
	"Illegal    ", 
	"RTS        ", 
	"Illegal    ", 
	"RTI        ", 
	"Illegal    ", 
	"Illegal    ", 
	"WAI        ", 
	"SWI        ", 

	"NEGA       ", 
	"Illegal    ", 
	"Illegal    ", 
	"COMA       ", 
	"LSRA       ", 
	"Illegal    ", 
	"RORA       ", 
	"ASRA       ", 
	"ASLA       ", 
	"ROLA       ", 
	"DECA       ", 
	"Illegal    ", 
	"INCA       ", 
	"TSTA       ", 
	"Illegal    ", 
	"CLRA       ", 

	"NEGB       ", 
	"Illegal    ", 
	"Illegal    ", 
	"COMB       ", 
	"LSRB       ", 
	"Illegal    ", 
	"RORB       ", 
	"ASRB       ", 
	"ASLB       ", 
	"ROLB       ", 
	"DECB       ", 
	"Illegal    ", 
	"INCB       ", 
	"TSTB       ", 
	"Illegal    ", 
	"CLRB       ", 

	"NEG d,X    ", 
	"Illegal    ", 
	"Illegal    ", 
	"COM d,X    ", 
	"LSR d,X    ", 
	"Illegal    ", 
	"ROR d,X    ", 
	"ASR d,X    ", 
	"ASL d,X    ", 
	"ROL d,X    ", 
	"DEC d,X    ", 
	"Illegal    ", 
	"INC d,X    ", 
	"TST d,X    ", 
	"JMP d,X    ", 
	"CLR d,X    ", 

	"NEG w      ", 
	"Illegal    ", 
	"Illegal    ", 
	"COM w      ", 
	"LSR w      ", 
	"Illegal    ", 
	"ROR w      ", 
	"ASR w      ", 
	"ASL w      ", 
	"ROL w      ", 
	"DEC w      ", 
	"Illegal    ", 
	"INC w      ", 
	"TST w      ", 
	"JMP w      ", 
	"CLR w      ", 

	"SUBA #d    ", 
	"CMPA #d    ", 
	"SBCA #d    ", 
	"Illegal    ", 
	"ANDA #d    ", 
	"BITA #d    ", 
	"LDAA #d    ", 
	"Illegal    ", 
	"EORA #d    ", 
	"ADCA #d    ", 
	"ORAA #d    ", 
	"ADDA #d    ", 
	"CPX #w     ", 
	"BSR d      ", 
	"LDS #w     ", 
	"Illegal    ", 

	"SUBA d     ", 
	"CMPA d     ", 
	"SBCA d     ", 
	"Illegal    ", 
	"ANDA d     ", 
	"BITA d     ", 
	"LDAA d     ", 
	"STAA d     ", 
	"EORA d     ", 
	"ADCA d     ", 
	"ORAA d     ", 
	"ADDA d     ", 
	"CPX d      ", 
	"Illegal    ", 
	"LDS d      ", 
	"STS d      ", 

	"SUBA d,X   ", 
	"CMPA d,X   ", 
	"SBCA d,X   ", 
	"Illegal    ", 
	"ANDA d,X   ", 
	"BITA d,X   ", 
	"LDAA d,X   ", 
	"STAA d,X   ", 
	"EORA d,X   ", 
	"ADCA d,X   ", 
	"ORAA d,X   ", 
	"ADDA d,X   ", 
	"CPX d,X    ", 
	"JSR d,X    ", 
	"LDS d,X    ", 
	"STS d,X    ", 

	"SUBA w     ", 
	"CMPA w     ", 
	"SBCA w     ", 
	"Illegal    ", 
	"ANDA w     ", 
	"BITA w     ", 
	"LDAA w     ", 
	"STAA w     ", 
	"EORA w     ", 
	"ADCA w     ", 
	"ORAA w     ", 
	"ADDA w     ", 
	"CPX w      ", 
	"JSR w      ", 
	"LDS w      ", 
	"STS w      ", 

	"SUBB #d    ", 
	"CMPB #d    ", 
	"SBCB #d    ", 
	"Illegal    ", 
	"ANDB #d    ", 
	"BITB #d    ", 
	"LDAB #d    ", 
	"Illegal    ", 
	"EORB #d    ", 
	"ADCB #d    ", 
	"ORAB #d    ", 
	"ADDB #d    ", 
	"Illegal    ", 
	"Illegal    ", 
	"LDX #w     ", 
	"Illegal    ", 

	"SUBB d     ", 
	"CMPB d     ", 
	"SBCB d     ", 
	"Illegal    ", 
	"ANDB d     ", 
	"BITB d     ", 
	"LDAB d     ", 
	"STAB d     ", 
	"EORB d     ", 
	"ADCB d     ", 
	"ORAB d     ", 
	"ADDB d     ", 
	"Illegal    ", 
	"Illegal    ", 
	"LDX d      ", 
	"STX d      ", 

	"SUBB d,X   ", 
	"CMPB d,X   ", 
	"SBCB d,X   ", 
	"Illegal    ", 
	"ANDB d,X   ", 
	"BITB d,X   ", 
	"LDAB d,X   ", 
	"STAB d,X   ", 
	"EORB d,X   ", 
	"ADCB d,X   ", 
	"ORAB d,X   ", 
	"ADDB d,X   ", 
	"Illegal   ", 
	"Illegal    ", 
	"LDX d,X    ", 
	"STX d,X    ", 

	"SUBB w     ", 
	"CMPB w     ", 
	"SBCB w     ", 
	"Illegal    ", 
	"ANDB w     ", 
	"BITB w     ", 
	"LDAB w     ", 
	"STAB w     ", 
	"EORB w     ", 
	"ADCB w     ", 
	"ORAB w     ", 
	"ADDB w     ", 
	"Illegal    ", 
	"Illegal    ", 
	"LDX w      ", 
	"STX w      "
};







////////////////////////////////////////////////////////////////////////////////
//                      Intel 6800 Simulator/Emulator (Start)                 //
////////////////////////////////////////////////////////////////////////////////


BYTE fetch()
{
	BYTE byte = 0;

	if ((ProgramCounter >= 0) && (ProgramCounter <= MEMORY_SIZE))
	{
		memory_in_range = true;
		byte = Memory[ProgramCounter];
		ProgramCounter++;
	}
	else
	{
		memory_in_range = false;
	}
	return byte;
}


// Add any instruction implementing routines here...

/*
*Function: set_flag_z, set_flag_n
*Description: sets flags depending on value defined 
*Parameters: inReg - BYTE
*Returns: none 
*Warnings: none
*/

void set_flag_z(BYTE inReg) 
{
	BYTE reg;
	int  bit_set_count;

	reg = inReg;

	if (reg == 0)			// set to zero
	{
		Flags = Flags | FLAG_Z;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_Z);
	}
}

void set_flag_n(BYTE inReg) 
{
	BYTE reg;
	int  bit_set_count;

	reg = inReg;

	if ((reg & 0x80) !=0) // mbit set
	{
		Flags = Flags | FLAG_N;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_N);
	}
}

/*
*Function: set_flag_v
*Description: sets flags depending on value defined 
*Parameters: in1, in2, out1 - BYTE
*Returns: none 
*Warnings: none
*/

void set_flag_v(BYTE in1, BYTE in2, BYTE out1) 
{
	BYTE reg1in;
	BYTE reg2in;
	BYTE regOut;

	reg1in = in1;
	reg2in = in2;
	regOut = out1;

	if ((((reg1in & 0x80) == 0x80) && ((reg2in & 0x80) == 0x80) && ((regOut & 0x80) != 0x80)) // overflow
		|| (((reg1in & 0x80) != 0x80) && ((reg2in & 0x80) != 0x80) && ((regOut & 0x80) == 0x80))) //overflow
	{
		Flags = Flags | FLAG_V;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_V);
	}
}







void Group_1_Dual_Operand_Instructions(BYTE opcode)
{
	//////////////////////////
	/// GROUP 1 VARIABLES ///
	////////////////////////

	BYTE byte_address;
	BYTE hb, lb;
	BYTE regA, regB;
	WORD address;
	WORD hi_address, lo_address; 
	WORD offset; 
	WORD temp_word;
	WORD wdata, wresult;

	switch (opcode)
	{

		/////////////////////////////
		/// GROUP 1 INSTRUCTIONS ///
		///////////////////////////

		/****************************/
		/* Load/Store Accumulators */
		/**************************/

	case 0x86:  // LDAA immediate - load Accumulator A using immediate addressing
		Registers[REGISTER_A] = fetch();

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0x96:  // LDAA direct - load Accumulator A using direct addressing
		address = (WORD)fetch();

		// Check that address held is valid
		if((address>=0)&&(address<MEMORY_SIZE))

			// Put into REGISTER_A using address in memory
		{
			Registers[REGISTER_A] = Memory[address];
		}
		else
		{
			Registers[REGISTER_A] = 0;
		}

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xA6: // LDAA indexed - load Accumulator A using indexed addressing
		address = IndexRegister + (WORD)fetch();

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Put into REGISTER_A using address in memory
		{
			Registers[REGISTER_A] = Memory[address];
		}
		else
		{
			Registers[REGISTER_A] = 0;
		}

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xB6: // LDAA extended - load Accumulator A using extended addressing
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Check that address held is valid
		if((address >= 0)&&(address<MEMORY_SIZE))

			// Put into REGISTER_A using address in memory
		{
			Registers[REGISTER_A] = Memory[address];
		}
		else
		{
			Registers[REGISTER_A] = 0;
		}

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xC6:  // LDAB immediate - load Accumulator B using immediate addressing
		Registers[REGISTER_B] = fetch();

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xD6:  // LDAB direct - load Accumulator B using direct addressing
		address = (WORD)fetch();

		// Check that address held is valid
		if((address>=0)&&(address<MEMORY_SIZE))

			// Put into REGISTER_B using address in memory
		{
			Registers[REGISTER_B] = Memory[address];
		}
		else
		{
			Registers[REGISTER_B] = 0;
		}

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xE6: // LDAB indexed - load Accumulator B using indexed addressing
		address = IndexRegister + (WORD)fetch();

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Put into REGISTER_B using address in memory
		{
			Registers[REGISTER_B] = Memory[address];
		}
		else
		{
			Registers[REGISTER_B] = 0;
		}

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		Flags = Flags & (0xFF - FLAG_V);
		break; 

	case 0xF6: // LDAB extended - load Accumulator B using extended addressing
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Check that address held is valid
		if((address >= 0)&&(address<MEMORY_SIZE))

			// Put into REGISTER_B using address in memory
		{
			Registers[REGISTER_B] = Memory[address];
		}
		else
		{
			Registers[REGISTER_B] = 0;
		}

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		Flags = Flags & (0xFF - FLAG_V);
		break;


	case 0x97:  // STAA direct - store Accumulator A using direct addressing
		address = (WORD)fetch();

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Put into memory using address in REGISTER_A
		{
			Memory[address] = Registers[REGISTER_A];
		}
		else
		{
			Registers[REGISTER_A] = 0;
		}

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xA7: // STAA indexed - store Accumulator A using indexed addressing
		address = IndexRegister + (WORD)fetch();

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Put into memory using address in REGISTER_A
		{
			Memory[address] = Registers[REGISTER_A];
		}
		else
		{
			Registers[REGISTER_A] = 0;
		}

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xB7: // STAA extended - store Accumulator A using extended addressing
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Put into memory using address in REGISTER_A
		{
			Memory[address] = Registers[REGISTER_A];
		}
		else
		{
			Registers[REGISTER_A] = 0;
		}

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xD7: // STAB direct - store Accumulator B using direct addressing
		address = (WORD)fetch();

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Put into memory using address in REGISTER_B
		{
			Memory[address] = Registers[REGISTER_B];
		}
		else
		{
			Registers[REGISTER_B] = 0;
		}

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xE7: // STAB indexed - store Accumulator B using indexed addressing
		address = IndexRegister + (WORD)fetch();

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Put into memory using address in REGISTER_B
		{
			Memory[address] = Registers[REGISTER_B];
		}
		else
		{
			Registers[REGISTER_B] = 0;
		}

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xF7: // STAB extended - store Accumulator B using extended addressing
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Put into memory using address in REGISTER_B
		{
			Memory[address] = Registers[REGISTER_B];
		}
		else
		{
			Registers[REGISTER_B] = 0;
		}

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		Flags = Flags & (0xFF - FLAG_V);
		break; 


		/*****************************/
		/* Load/Store IndexRegister */
		/***************************/


	case 0xCE: // LDX immediate - load IndexRegister using immediate addressing
		IndexRegister = (((WORD)fetch()) << 8) + fetch();

		/** FLAGS **/

		if ((IndexRegister & 0x8000) != 0)
		{
			Flags = Flags | FLAG_N;	
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_N);	
		}

		set_flag_z(IndexRegister);
		Flags = Flags & (0xFF - FLAG_V);

		break;

	case 0xDE:  // LDX direct - load IndexRegister using direct addressing
		address = (WORD)fetch();

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Put into IndexRegister using address in memory
		{
			IndexRegister = (((WORD)Memory[address])
				<< 8) + (WORD)Memory[address+1];
		}
		else
		{
			IndexRegister = 0;
		}

		/** FLAGS **/

		if ((IndexRegister & 0x8000) != 0)
		{
			Flags = Flags | FLAG_N;	
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_N);	
		}

		set_flag_z(IndexRegister);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xEE: // LDX indexed - load IndexRegister using indexed addressing
		address = IndexRegister + (WORD)fetch();

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Put into IndexRegister using address in memory
		{
			IndexRegister = (((WORD)Memory[address])
				<< 8)  + (WORD)Memory[address+1];
		}
		else
		{
			IndexRegister = 0;
		}

		/** FLAGS **/

		if ((IndexRegister & 0x8000) != 0)
		{
			Flags = Flags | FLAG_N;	
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_N);	
		}

		set_flag_z(IndexRegister);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xFE: // LDX extended - load IndexRegister using extended addressing
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Put into IndexRegister using address in memory
		{
			IndexRegister = (((WORD)Memory[address])
				<< 8) + (WORD)Memory[address+1];
		}
		else
		{
			IndexRegister = 0;
		}

		/** FLAGS **/

		if ((IndexRegister & 0x8000) != 0)
		{
			Flags = Flags | FLAG_N;	
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_N);	
		}

		set_flag_z(IndexRegister);
		Flags = Flags & (0xFF - FLAG_V);
		break; 


	case 0xDF: // STX direct - store IndexRegister using direct addressing
		address = (WORD)fetch();

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE-1))

			// Put into memory using address in IndexRegister
		{
			Memory[address] = (BYTE)((IndexRegister >> 
				8) & 0xFF);
			Memory[address+1] = (BYTE)(IndexRegister & 
				0xFF);
		}

		/** FLAGS **/

		if ((IndexRegister & 0x8000) != 0)
		{
			Flags = Flags | FLAG_N;	
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_N);	
		}

		set_flag_z(IndexRegister);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xEF: // STX indexed - store IndexRegister using indexed addressing
		address = IndexRegister + (WORD)fetch();

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE-1))

			// Put into memory using address in IndexRegister
		{
			Memory[address] = (BYTE)((IndexRegister >> 
				8) & 0xFF);
			Memory[address+1] = (BYTE)(IndexRegister & 
				0xFF);
		}

		/** FLAGS **/

		if ((IndexRegister & 0x8000) != 0)
		{
			Flags = Flags | FLAG_N;	
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_N);	
		}

		set_flag_z(IndexRegister);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xFF: // STX extended - store IndexRegister using extended addressing
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE-1))

			// Put into memory using address in IndexRegister
		{
			Memory[address] = (BYTE)((IndexRegister >> 
				8) & 0xFF);
			Memory[address+1] = (BYTE)(IndexRegister & 
				0xFF);
		}

		/** FLAGS **/

		if ((IndexRegister & 0x8000) != 0)
		{
			Flags = Flags | FLAG_N;	
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_N);	
		}

		set_flag_z(IndexRegister);
		Flags = Flags & (0xFF - FLAG_V);
		break;



		/****************************/
		/* Load/Store StackPointer */
		/**************************/


	case 0x8E: // LDS immediate - load StackPointer using immediate addressing
		StackPointer = (((WORD)fetch()) << 8) + fetch();

		/** FLAGS **/

		if ((StackPointer & 0x8000) != 0)
		{
			Flags = Flags | FLAG_N;	
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_N);	
		}

		set_flag_z(StackPointer);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0x9E: // LDS direct - load StackPointer using direct addressing
		address = (WORD)fetch();

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Put into StackPointer using address in memory
		{
			StackPointer = (((WORD)Memory[address])
				<< 8) + (WORD)Memory[address+1];
		}
		else
		{
			StackPointer = 0;
		}

		/** FLAGS **/

		if ((StackPointer & 0x8000) != 0)
		{
			Flags = Flags | FLAG_N;	
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_N);	
		}

		set_flag_z(StackPointer);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xAE: // LDS indexed - load StackPointer using indexed addressing
		address = StackPointer + (WORD)fetch();

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Put into StackPointer using address in memory
		{
			StackPointer = (((WORD)Memory[address])
				<< 8)  + (WORD)Memory[address+1];
		}
		else
		{
			StackPointer = 0;
		}

		/** FLAGS **/

		if ((StackPointer & 0x8000) != 0)
		{
			Flags = Flags | FLAG_N;	
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_N);	
		}

		set_flag_z(StackPointer);
		Flags = Flags & (0xFF - FLAG_V);
		break; 

	case 0xBE: // LDS extended - load StackPointer using extended addressing
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Put into StackPointer using address in memory
		{
			StackPointer = (((WORD)Memory[address])
				<< 8) + (WORD)Memory[address+1];
		}
		else
		{
			StackPointer = 0;
		}

		/** FLAGS **/

		if ((StackPointer & 0x8000) != 0)
		{
			Flags = Flags | FLAG_N;	
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_N);	
		}

		set_flag_z(StackPointer);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0x9F: // STS direct - store StackPointer using direct addressing 
		address = (WORD)fetch();

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE-1))

			// Put into memory using address in StackPointer
		{
			Memory[address] = (BYTE)((StackPointer >> 
				8) & 0xFF);
			Memory[address+1] = (BYTE)(StackPointer & 
				0xFF);
		}

		/** FLAGS **/

		if ((StackPointer & 0x8000) != 0)
		{
			Flags = Flags | FLAG_N;	
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_N);	
		}

		set_flag_z(StackPointer);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xAF: // STS indexed - store StackPointer using indexed addressing 
		address = StackPointer + (WORD)fetch();

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE-1))

			// Put into memory using address in StackPointer
		{
			Memory[address] = (BYTE)((StackPointer >> 
				8) & 0xFF);
			Memory[address+1] = (BYTE)(StackPointer & 
				0xFF);
		}

		/** FLAGS **/

		if ((StackPointer & 0x8000) != 0)
		{
			Flags = Flags | FLAG_N;	
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_N);	
		}

		set_flag_z(StackPointer);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xBF: // STS extended - store StackPointer using extended addressing 
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE-1))

			// Put into memory using address in StackPointer
		{
			Memory[address] = (BYTE)((StackPointer >> 
				8) & 0xFF);
			Memory[address+1] = (BYTE)(StackPointer & 
				0xFF);
		}

		/** FLAGS **/

		if ((StackPointer & 0x8000) != 0)
		{
			Flags = Flags | FLAG_N;	
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_N);	
		}

		set_flag_z(StackPointer);
		Flags = Flags & (0xFF - FLAG_V);
		break;


		/***********************/
		/* Add/Add with Carry */
		/*********************/


	case 0x8B: // ADDA immediate - add Accumulator A to memory using immediate addressing
		regA = Registers[REGISTER_A];
		lb = fetch();

		// Declaring temp_word for flag testing, written back into REGISTER_A on last line
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);

		Registers[REGISTER_A] = (BYTE)temp_word; 
		break;

	case 0x9B: // ADDA direct - add Accumulator A to memory using direct addressing
		regA = Registers[REGISTER_A];
		address = (WORD) fetch();

		// Check that address held is valid
		if((address>=0)&&(address<MEMORY_SIZE))

			// Put into low byte using address in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		// Declaring temp_word for flag testing, written back into REGISTER_A on last line
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);

		Registers[REGISTER_A] = (BYTE)temp_word; 
		break;

	case 0xAB: //ADDA indexed - add Accumulator A to memory using indexed addressing
		regA = Registers[REGISTER_A];
		address = IndexRegister + (WORD)fetch();

		// Check that address held is valid
		if ((address >= 0) && (address < MEMORY_SIZE))

			// Put into low byte using address in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		} 

		// Declaring temp_word for flag testing, written back into REGISTER_A on last line
		temp_word = (WORD) Registers[REGISTER_A] + (WORD)lb;

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);

		Registers[REGISTER_A] = (BYTE)temp_word; 
		break;

	case 0xBB: // ADDA extended - add Accumulator A to memory using extended addressing
		regA = Registers[REGISTER_A];

		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Check that address held is valid
		if((address >= 0)&&(address<MEMORY_SIZE))

			// Put into low byte using address in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		// Declaring temp_word for flag testing, written back into REGISTER_A on last line
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);

		Registers[REGISTER_A] = (BYTE)temp_word; 
		break;

	case 0xCB: // ADDB immediate - add Accumulator B to memory using immediate addressing
		regB = Registers[REGISTER_B];
		lb = fetch();

		// Declaring temp_word for flag testing, written back into REGISTER_B on last line
		temp_word = (WORD)Registers[REGISTER_B] + (WORD)lb;

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);

		Registers[REGISTER_B] = (BYTE)temp_word; 
		break; 

	case 0xDB: // ADDB direct - add Accumulator B to memory using direct addressing
		regB = Registers[REGISTER_B];
		address = (WORD) fetch();

		// Check that address held is valid
		if((address>=0)&&(address<MEMORY_SIZE))

			// Put into low byte using address in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		// Declaring temp_word for flag testing, written back into REGISTER_B on last line
		temp_word = (WORD) Registers[REGISTER_B] + (WORD)lb;

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);

		Registers[REGISTER_B] = (BYTE)temp_word; 
		break;

	case 0xEB: //ADDB indexed - add Accumulator B to memory using indexed addressing
		regB = Registers[REGISTER_B];
		address = IndexRegister + (WORD)fetch();

		// Check that address held is valid
		if ((address >= 0) && (address < MEMORY_SIZE))

			// Put into low byte using address in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		} 

		// Declaring temp_word for flag testing, written back into REGISTER_B on last line
		temp_word = (WORD) Registers[REGISTER_B] + (WORD)lb;

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);

		Registers[REGISTER_B] = (BYTE)temp_word; 
		break;

	case 0xFB: // ADDB extended - add Accumulator B to memory using extended addressing
		regB = Registers[REGISTER_B];

		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Check that address held is valid
		if((address >= 0)&&(address<MEMORY_SIZE))

			// Put into low byte using address in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		// Declaring temp_word for flag testing, written back into REGISTER_B on last line
		temp_word = (WORD)Registers[REGISTER_B] + (WORD)lb;

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);

		Registers[REGISTER_B] = (BYTE)temp_word; 
		break;

	case 0x89: // ADCA immediate - add Accumulator A with carry to memory using immediate addressing
		regA = Registers[REGISTER_A];
		lb = fetch();

		// Declaring temp_word for flag testing, written back into REGISTER_A on last line
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;

		if ((Flags & FLAG_C) != 0);
		{
			temp_word++;
		}

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);

		Registers[REGISTER_A] = (BYTE)temp_word; 
		break;

	case 0x99: // ADCA direct - add Accumulator A with carry to memory using direct addressing
		regA = Registers[REGISTER_A];
		address = (WORD)fetch();

		// Check that address held is valid
		if((address>=0)&&(address<MEMORY_SIZE))

			// Put into high byte using address in memory, low byte using FLAG_C
		{
			hb = Memory[address];
			lb = FLAG_C;
		}
		else
		{
			hb = 0;
			lb = 0;
		}

		// Declaring temp_word for flag testing, written back into REGISTER_A on last line
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;

		if ((Flags & FLAG_C) != 0);
		{
			temp_word++;
		}

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, hb, (BYTE)temp_word);

		Registers[REGISTER_A] = (BYTE)temp_word; 
		break; 

	case 0xA9: // ADCA indexed - add Accumulator A with carry to memory using indexed addressing
		regA = Registers[REGISTER_A];
		address = IndexRegister + (WORD)fetch();

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Put into high byte using address in memory, low byte using FLAG_C
		{
			hb = Memory[address];
			lb = FLAG_C;
		}
		else
		{
			hb = 0;
			lb = 0;
		}

		// Declaring temp_word for flag testing, written back into REGISTER_A on last line
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;


		if ((Flags & FLAG_C) != 0);
		{
			temp_word++;
		}

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, hb, (BYTE)temp_word);

		Registers[REGISTER_A] = (BYTE)temp_word; 
		break;

	case 0xB9: // ADCA extended - add Accumulator A with carry to memory using extended addressing
		regA = Registers[REGISTER_A];

		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Check that address held is valid
		if((address >= 0)&&(address<MEMORY_SIZE))

			// Put into high byte using address in memory, low byte using FLAG_C
		{
			hb = Memory[address];
			lb = FLAG_C;
		}
		else
		{
			hb = 0;
			lb = 0;
		}

		// Declaring temp_word for flag testing, written back into REGISTER_A on last line
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;

		if ((Flags & FLAG_C) != 0);
		{
			temp_word++;
		}

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, hb, (BYTE)temp_word);

		Registers[REGISTER_A] = (BYTE)temp_word; 
		break; 

	case 0xC9: // ADCB immediate - add Accumulator B with carry to memory using immediate addressing
		regB = Registers[REGISTER_B];
		lb = fetch();

		// Declaring temp_word for flag testing, written back into REGISTER_B on last line
		temp_word = (WORD)Registers[REGISTER_B] + (WORD)lb;

		if ((Flags & FLAG_C) != 0);
		{
			temp_word++;
		}

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);

		Registers[REGISTER_B] = (BYTE)temp_word; 
		break;

	case 0xD9: // ADCB direct - add Accumulator B with carry to memory using direct addressing
		regB = Registers[REGISTER_B];
		address = (WORD)fetch();

		// Check that address held is valid
		if((address>=0)&&(address<MEMORY_SIZE))

			// Put into high byte using address in memory, low byte using FLAG_C
		{
			hb = Memory[address];
			lb = FLAG_C;
		}
		else
		{
			hb = 0;
			lb = 0;
		}

		// Declaring temp_word for flag testing, written back into REGISTER_B on last line
		temp_word = (WORD)Registers[REGISTER_B] + (WORD)lb;

		if ((Flags & FLAG_C) != 0);
		{
			temp_word++;
		}

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, hb, (BYTE)temp_word);

		Registers[REGISTER_B] = (BYTE)temp_word; 
		break; 


	case 0xE9: // ADCB indexed - add Accumulator B with carry to memory using indexed addressing
		regB = Registers[REGISTER_B];
		address = IndexRegister + (WORD)fetch();

		// Check that address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Put into high byte using address in memory, low byte using FLAG_C
		{
			hb = Memory[address];
			lb = FLAG_C;
		}
		else
		{
			hb = 0;
			lb = 0;
		}

		// Declaring temp_word for flag testing, written back into REGISTER_B on last line
		temp_word = (WORD)Registers[REGISTER_B] + (WORD)lb;

		if ((Flags & FLAG_C) != 0);
		{
			temp_word++;
		}

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, hb, (BYTE)temp_word);

		Registers[REGISTER_B] = (BYTE)temp_word; 
		break;

	case 0xF9: // ADCB extended - add Accumulator B with carry to memory using extended addressing
		regB = Registers[REGISTER_B];

		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Check that address held is valid
		if((address >= 0)&&(address<MEMORY_SIZE))

			// Put into high byte using address in memory, low byte using FLAG_C
		{
			hb = Memory[address];
			lb = FLAG_C;
		}
		else
		{
			hb = 0;
			lb = 0;
		}

		// Declaring temp_word for flag testing, written back into REGISTER_B on last line
		temp_word = (WORD)Registers[REGISTER_B] + (WORD)lb;

		if ((Flags & FLAG_C) != 0);
		{
			temp_word++;
		}

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, hb, (BYTE)temp_word);

		Registers[REGISTER_B] = (BYTE)temp_word; 
		break; 


		/************/
		/* Compare */
		/**********/


	case 0x81: // CMPA immediate - compare Accumulator A to memory using immediate addressing
		regA  = Registers[REGISTER_A];
		lb = fetch();

		// Compares REGISTER_A to low byte, puts result into temp_word
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;

		/** FLAGS **/

		if(temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		break;

	case 0x91: // CMPA direct - compare Accumulator A to memory using direct addressing
		regA  = Registers[REGISTER_A];
		address = (WORD)fetch();

		// Check that the address held is valid
		if((address>=0)&&(address<MEMORY_SIZE))

			// Put into low byte using address in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		// Compares REGISTER_A to low byte, puts result into temp_word
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;

		/** FLAGS **/

		if(temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		break; 

	case 0xA1: // CMPA indexed - compare Accumulator A to memory using indexed addressing
		regA  = Registers[REGISTER_A];
		address = IndexRegister + (WORD)fetch();

		// Check that the address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Put into low byte using address in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		// Compares REGISTER_A to low byte, puts result into temp_word
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;

		/** FLAGS **/

		if(temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		break;

	case 0xB1: // CMPA extended - compare Accumulator A to memory using extended addressing
		regA  = Registers[REGISTER_A];

		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Check that the address held is valid
		if((address >= 0)&&(address<MEMORY_SIZE))

			// Puts into low byte using address held in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		// Compares REGISTER_A to low byte, puts result into temp_word
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;

		/** FLAGS **/

		if(temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);
		break;

	case 0xC1: // CMPB immediate - compare Accumulator B to memory using immediate addressing
		regB  = Registers[REGISTER_B];
		lb = fetch();

		// Compares REGISTER_B to low byte, puts result into temp_word
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;

		/** FLAGS **/

		if(temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		break;

	case 0xD1: // CMPB direct - compare Accumulator B to memory using direct addressing
		regB = Registers[REGISTER_B];
		address = (WORD)fetch();

		// Checks that the address held is valid
		if((address>=0)&&(address<MEMORY_SIZE))

			// Puts into low byte using address held in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		// Compares REGISTER_B to low byte, puts result into temp_word
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;

		/** FLAGS **/

		if(temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		break; 

	case 0xE1: // CMPB indexed - compare Accumulator B to memory using indexed addressing
		regB  = Registers[REGISTER_B];
		address = IndexRegister + (WORD)fetch();

		// Checks that he address held is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Puts into low bute using address held in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		// Compares REGISTER_B to low byte, puts result into temp_word
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;

		/** FLAGS **/

		if(temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		break;

	case 0xF1: // CMPB extended - compare Accumulator B to memory using extended addressing
		regB  = Registers[REGISTER_B];

		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Checks that the address held is valid
		if((address >= 0)&&(address<MEMORY_SIZE))

			// Puts into low byte using address held in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		// Compares REGISTER_B to low byte, puts result into temp_word
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;

		/** FLAGS **/

		if(temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // Set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
		}
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);
		break;


		/****************************/
		/* Subroutine instructions */
		/**************************/



	case 0xAD: // JSR indexed - jump to subroutine using indexed addressing
		address = IndexRegister + (WORD)fetch();

		// Checks that the address held in StackPointer is valid
		if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))

			// Puts into memory using ProgramCounter
		{
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
			StackPointer--;
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
			StackPointer--;
		}

		// Puts back into ProgramCounter using address held in memory
		ProgramCounter = (((WORD)Memory[address] << 8) + ((WORD)Memory[address+1]));
		break;

	case 0xBD: // JSR extended - jump to subroutine using extended addressing
		hb = fetch();
		lb = fetch();
		address = ((WORD)hb << 8) + (WORD)lb;

		// Checks that the address held in StackPointer is valid
		if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))

			// Puts into memory using ProgramCounter
		{
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
			StackPointer--;
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8 ) & 0xFF);
			StackPointer--; 
		}

		// Puts back into ProgramCounter using address
		ProgramCounter = address; 
		break;

	case 0x8D: // BSR - branch to subroutine using inherent addressing
		lb = fetch();
		offset = (WORD)lb;

		if ((offset & 0x80) != 0) // need to sign extend
		{
			offset = offset + 0xFF00;
		}
		// Adds ProgramCounter to the offset result and writes into address
		address = ProgramCounter + offset;


		// Checks that the address held in StackPointer is valid
		if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE))

			// Puts into memory using ProgramCounter 
		{
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF); 
			StackPointer--;
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF); 
			StackPointer--;
		}

		// Puts back into ProgramCounter using address
		ProgramCounter = address;
		break; 


		/*********************/
		/* AND instructions */
		/*******************/


	case 0x84: // ANDA immediate - '&' action on Accumulator A and memory using immediate addressing
		regA = Registers[REGISTER_A];
		lb = fetch();

		// Declares temp_word as the result of REGISTER_A and low byte
		temp_word = (WORD)Registers[REGISTER_A] & (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);

		// Puts into REGISTER_A using temp_word
		Registers[REGISTER_A] = (BYTE)temp_word; 
		break;

	case 0x94: // ANDA direct - '&' action on Accumulator A and memory using direct addressing
		regA = Registers[REGISTER_A];
		address = (WORD) fetch();

		// Checks that the address held in memory is valid
		if((address>=0)&&(address<MEMORY_SIZE))

			// Puts into low byte using address held in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		// Declares temp_word as the result of REGISTER_A and low byte
		temp_word = (WORD) Registers[REGISTER_A] & (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);

		// Puts into REGISTER_A using temp_word
		Registers[REGISTER_A] = (BYTE)temp_word; 
		break;

	case 0xA4: // ANDA indexed - '&' action on Accumulator A and memory using indexed addressing
		regA = Registers[REGISTER_A];
		address = IndexRegister + (WORD)fetch();

		// Checks that the address held in memory is valid
		if ((address >= 0) && (address < MEMORY_SIZE))

			// Puts into low byte using address held in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		} 

		// Declares temp_word as the result of REGISTER_A and low byte
		temp_word = (WORD) Registers[REGISTER_A] & (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);

		// Puts back into REGISTER_A using temp_words
		Registers[REGISTER_A] = (BYTE)temp_word; 
		break;

	case 0xB4: // ANDA extended - '&' action on Accumulator A and memory using extended addressing
		regA = Registers[REGISTER_A];

		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Check that the address held in memory is valid
		if((address >= 0)&&(address<MEMORY_SIZE))

			// Puts into low byte using address held in memory 
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		// Declares temp_word as the result of REGISTER_A and low byte
		temp_word = (WORD)Registers[REGISTER_A] & (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);

		// Puts back into REGISTER_A using temp_word
		Registers[REGISTER_A] = (BYTE)temp_word; 
		break;

	case 0xC4: // ANDB immediate - '&' action on Accumulator B and memory using immediate addressing
		regB = Registers[REGISTER_B];
		lb = fetch();

		// Declares temp_word as the result of REGISTER_B and low byte
		temp_word = (WORD)Registers[REGISTER_B] & (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);

		// Puts back into REGISTER_B using temp_word
		Registers[REGISTER_B] = (BYTE)temp_word; 
		break;

	case 0xD4: // ANDB direct - '&' action on Accumulator B and memory using direct addressing
		regB = Registers[REGISTER_B];
		address = (WORD) fetch();

		// Check that the address held in memory is valid
		if((address>=0)&&(address<MEMORY_SIZE))

			// Puts into low byte using address held in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		// Declares temp_word as the result of REGISTER_B and low byte
		temp_word = (WORD) Registers[REGISTER_B] & (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);

		// Puts back into REGISTER_B using temp_word
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xE4: //ANDB indexed - '&' action on Accumulator B and memory using indexed addressing
		regB = Registers[REGISTER_B];
		address = IndexRegister + (WORD)fetch();

		// Checks that the address held in memory is valid
		if ((address >= 0) && (address < MEMORY_SIZE))

			// Puts into low byte using address held in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		} 

		// Declares temp_word as the result of REGISTER_B and low byte
		temp_word = (WORD) Registers[REGISTER_B] & (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);

		// Puts back into REGISTER_B using temp_word
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;

	case 0xF4: // ANDB extended - '&' action on Accumulator B and memory using extended addressing
		regB = Registers[REGISTER_B];

		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Checks that the address held in memory is valid
		if((address >= 0)&&(address<MEMORY_SIZE))

			// Puts into low byte using address held in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		// Declares temp_word as the result of REGISTER_B and low byte
		temp_word = (WORD)Registers[REGISTER_B] & (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);

		// Puts back into REGISTER_B using temp_word
		Registers[REGISTER_B] = (BYTE)temp_word;
		break;


		/*************/
		/* Bit test */
		/***********/


	case 0x85: // BITA immediate - does a bit test on Accumulator A using immediate addressing
		regA = Registers[REGISTER_A];
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] & (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0x95: // BITA direct - does a bit test on Accumulator A using direct addressing
		regA = Registers[REGISTER_A];
		address = (WORD)fetch();


		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_A] & (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xA5: // BITA indexed - does a bit test on Accumulator A using indexed addressing
		regA = Registers[REGISTER_A];
		address = IndexRegister + (WORD)fetch();

		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		} 

		temp_word = (WORD)Registers[REGISTER_A] & (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xB5: // BITA extended - does a bit test on Accumulator A using extended addressing
		regA = Registers[REGISTER_A];

		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;
		if((address >= 0)&&(address<MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		temp_word = (WORD)Registers[REGISTER_A] & (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xC5: // BITB immediate - does a bit test on Accumulator B using immediate addressing
		regA = Registers[REGISTER_B];
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_B] & (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xD5: // BITB direct - does a bit test on Accumulator B using direct addressing
		regA = Registers[REGISTER_B];
		address = (WORD)fetch();

		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD)Registers[REGISTER_B] & (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xE5: // BITB indexed - does a bit test on Accumulator B using indexed addressing
		regA = Registers[REGISTER_B];
		address = IndexRegister + (WORD)fetch();

		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		} 

		temp_word = (WORD)Registers[REGISTER_B] & (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xF5: // BITB extended - does a bit test on Accumulator B using extended addressing
		regA = Registers[REGISTER_B];

		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;
		if((address >= 0)&&(address<MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		temp_word = (WORD)Registers[REGISTER_B] & (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		break;


		/*********************************/
		/* Subtract/Subtract with Carry */
		/*******************************/


	case 0x80: // SUBA immediate - subtract Accumulator A from memory using immediate addressing
		regA = Registers[REGISTER_A];
		lb = fetch();

		// Declares temp_word as the result of REGISTER_A minus low byte
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);

		// Puts back into REGISTER_A using temp_word
		Registers[REGISTER_A] = (BYTE)temp_word; 
		break;

	case 0x90: // SUBA direct - subtract Accumulator A from memory using direct addressing
		regA = Registers[REGISTER_A];
		address = (WORD) fetch();

		// Checks that the address in memory is valid
		if((address>=0)&&(address<MEMORY_SIZE))

			// Puts into low byte using address held in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		// Declares temp_word as the result of REGISTER_A minus low byte
		temp_word = (WORD) Registers[REGISTER_A] - (WORD)lb;

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);


		// Puts back into REGISTER_A using temp_word
		Registers[REGISTER_A] = (BYTE)temp_word; 

		break;

	case 0xA0: // SUBA indexed - subtract Accumulator A from memory using indexed addressing
		regA = Registers[REGISTER_A];
		address = IndexRegister + (WORD)fetch();

		// Checks that the address held in memory is valid
		if ((address >= 0) && (address < MEMORY_SIZE))

			// Puts into low byte using address held in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		} 

		// Declares temp_word as the result of REGISTER_A minus low byte
		temp_word = (WORD) Registers[REGISTER_A] - (WORD)lb;

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);

		// Puts back into REGISTER_A using temp_word
		Registers[REGISTER_A] = (BYTE)temp_word; 
		break;

	case 0xB0: // SUBA extended - subtract Accumulator A from memory using extended addressing
		regA = Registers[REGISTER_A];

		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Checks that the address held in memory is valid
		if((address >= 0)&&(address<MEMORY_SIZE))

			// Puts into low byte using address held memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		// Declares temp_word as the result of REGISTER_A minus low byte
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);

		// Puts back into REGISTER_A using temp_word
		Registers[REGISTER_A] = (BYTE)temp_word; 
		break;

	case 0xC0: // SUBB immediate - subtract Accumulator B from memory using immediate addressing
		regB = Registers[REGISTER_B];
		lb = fetch();

		// Declares temp_word as the result of REGISTER_B minus low byte
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);

		// Puts back into REGISTER_B using temp_word
		Registers[REGISTER_B] = (BYTE)temp_word; 
		break;

	case 0xD0: // SUBB direct - subtract Accumulator B from memory using direct addressing
		regB = Registers[REGISTER_B];
		address = (WORD) fetch();

		// Checks that the address held in memory is valid
		if((address>=0)&&(address<MEMORY_SIZE))

			// Puts into low byte using address held in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		// Declares temp_word as the result of REGISTER_B minus low byte
		temp_word = (WORD) Registers[REGISTER_B] - (WORD)lb;

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);

		// Puts back into REGISTER_B using temp_word
		Registers[REGISTER_B] = (BYTE)temp_word; 
		break;

	case 0xE0: // SUBB indexed - subtract Accumulator B from memory using indexed addressing
		regB = Registers[REGISTER_B];
		address = IndexRegister + (WORD)fetch();

		// Checks that the address held in memory is valid
		if ((address >= 0) && (address < MEMORY_SIZE))

			// Puts into low byte using address held in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		} 

		// Declares temp_word as the result of REGISTER_B minus low byte
		temp_word = (WORD) Registers[REGISTER_B] - (WORD)lb;

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);

		// Puts back into REGISTER_B using temp_word
		Registers[REGISTER_B] = (BYTE)temp_word; 
		break;

	case 0xF0: // SUBB extended - subtract Accumulator B from memory using extended addressing
		regB = Registers[REGISTER_B];

		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Checks that the address held in memory is valid
		if((address >= 0)&&(address<MEMORY_SIZE))

			// Puts into low byte using address held in memory
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}

		// Delcares temp_word as the result of REGISTER_B minus low byte
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);

		// Puts back into REGISTER_B using temp_word
		Registers[REGISTER_B] = (BYTE)temp_word; 
		break;

	case 0x82: // SBCA immediate - subtract Accumulator A with carry from memory using immediate addressing
		regA = Registers[REGISTER_A];
		lb = fetch();

		// Declares temp_word as the result of REGISTER_A minus low byte 
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;

		// Decrement temp_word based on the carry
		if ((Flags & FLAG_C) != 0);
		{
			temp_word--;
		}

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);

		// Puts back into REGISTER_A using temp_word
		Registers[REGISTER_A] = (BYTE)temp_word; 
		break;


	case 0x92: // SBCA direct - subtract Accumulator A with carry from memory using direct addressing
		regA = Registers[REGISTER_A];
		address = (WORD)fetch();

		// Checks that the address held in memory is valid
		if((address>=0)&&(address<MEMORY_SIZE))

			// Puts into high byte using address held in memory, low byte using the carry flag
		{
			hb = Memory[address];
			lb = FLAG_C;
		}
		else
		{
			hb = 0;
			lb = 0;
		}

		// Declares temp_word based on the result of REGISTER_A minus low byte
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;

		// Decrements temp_word based on the carry
		if ((Flags & FLAG_C) != 0);
		{
			temp_word--;
		}

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);

		// Puts back into REGISTER_A using temp_word
		Registers[REGISTER_A] = (BYTE)temp_word; 
		break; 

	case 0xA2: // SBCA indexed - subtract Accumulator A with carry from memory using indexed addressing
		regA = Registers[REGISTER_A];
		address = IndexRegister + (WORD)fetch();

		// Checks that the address held in memory is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Puts into high byte using address held in memory, low byte using carry flag
		{
			hb = Memory[address];
			lb = FLAG_C;
		}
		else
		{
			hb = 0;
			lb = 0;
		}

		// Declares temp_word as a result of REGISTER_A minus low byte, decrements based on carry flag
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;

		if ((Flags & FLAG_C) != 0);
		{
			temp_word--;
		}

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);

		// Puts back into REGISTER_A using temp_word
		Registers[REGISTER_A] = (BYTE)temp_word; 
		break;

	case 0xB2: // SBCA extended - subtract Accumulator A with carry from memory using extended addressing
		regA = Registers[REGISTER_A];

		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Checks that the address held in memory is valid
		if((address >= 0)&&(address<MEMORY_SIZE))

			// Puts into high byte using address held in memory, low byte using carry flag 
		{
			hb = Memory[address];
			lb = FLAG_C;
		}
		else
		{
			hb = 0;
			lb = 0;
		}


		// Declares temp_word as the result of REGISTER_A minus low byte, decrements temp_word based on the carry flag
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;

		if ((Flags & FLAG_C) != 0);
		{
			temp_word--;
		}

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regA, lb, (BYTE)temp_word);

		// Puts back into REGISTER_A using temp_word
		Registers[REGISTER_A] = (BYTE)temp_word; 
		break; 

	case 0xC2: // SBCB immediate - subtract Accumulator B with carry from memory using immediate addressing
		regB = Registers[REGISTER_B];
		lb = fetch();

		// Declares temp_word as the result of REGISTER_B minus low byte, decrements temp_word based on carry flag
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;

		if ((Flags & FLAG_C) != 0);
		{
			temp_word--;
		}

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);

		// Puts back into REGISTER_B using temp_word
		Registers[REGISTER_B] = (BYTE)temp_word; 
		break;

	case 0xD2: // SBCB direct - subtract Accumulator B with carry from memory using direct addressing
		regB = Registers[REGISTER_B];
		address = (WORD)fetch();

		// Checks that the address held in memory is valid
		if((address>=0)&&(address<MEMORY_SIZE))

			// Puts into high byte using address held in memory, low byte using carry flag
		{
			hb = Memory[address];
			lb = FLAG_C;
		}
		else
		{
			hb = 0;
			lb = 0;
		}

		// Declares temp_word as the result of REGISTER_B minus low byte, decrements temp_word based on carry flag
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;

		if ((Flags & FLAG_C) != 0);
		{
			temp_word--;
		}

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);

		// Puts back into REGISTER_B using temp_word
		Registers[REGISTER_B] = (BYTE)temp_word; 
		break; 

	case 0xE2: // SBCB indexed - subtract Accumulator B with carry from memory using indexed addressing
		regB = Registers[REGISTER_B];
		address = IndexRegister + (WORD)fetch();

		// Checks that the address held in memory is valid
		if((address >= 0)&&(address < MEMORY_SIZE))

			// Puts into high byte using address held in memory, low byte using carry flag
		{
			hb = Memory[address];
			lb = FLAG_C;
		}
		else
		{
			hb = 0;
			lb = 0;
		}

		// Declares temp_word using REGISTER_B minus low byte, decrements temp_word based on carry flag
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;

		if ((Flags & FLAG_C) != 0);
		{
			temp_word--;
		}

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);

		// Puts into REGISTER_B using temp_word
		Registers[REGISTER_B] = (BYTE)temp_word; 
		break;

	case 0xF2: // SBCB extended - subtract Accumulator B with carry from memory using extended addressing
		regB = Registers[REGISTER_B];

		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		// Checks that the address held in memory is valid
		if((address >= 0)&&(address<MEMORY_SIZE))

			// Puts into high byte using address held in memory, low byte using carry flag
		{
			hb = Memory[address];
			lb = FLAG_C;
		}
		else
		{
			hb = 0;
			lb = 0;
		}

		// Declares temp_word based on REGISTER_B minus low byte, decrements temp_word based on carry flag
		temp_word = (WORD)Registers[REGISTER_B] - (WORD)lb;

		if ((Flags & FLAG_C) != 0);
		{
			temp_word--;
		}

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(regB, lb, (BYTE)temp_word);

		// Puts back into REGISTER_B using temp_word
		Registers[REGISTER_B] = (BYTE)temp_word; 
		break; 


		/***************************/
		/* Exclusive/Inclusive OR */
		/*************************/


	case 0x88: // EORA immediate
		address = (WORD)fetch();

		if ((address >=0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		Registers[REGISTER_A] ^= lb;

		/** FLAGS **/

		set_flag_n(address);
		set_flag_z(address);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0x98: // EORA direct
		address = (WORD)fetch();

		if ((address >=0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		Registers[REGISTER_A] ^= lb;

		/** FLAGS **/

		set_flag_n(address);
		set_flag_z(address);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xA8: // EORA indexed
		lb = fetch();
		address = lb + (WORD)fetch();

		if ((address >=0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		Registers[REGISTER_A] ^= lb;

		/** FLAGS **/

		set_flag_n(address);
		set_flag_z(address);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xB8: // EORA extended
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		if ((address >=0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		Registers[REGISTER_A] ^= lb;

		/** FLAGS **/

		set_flag_n(address);
		set_flag_z(address);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xC8: // EORB immediate
		address = (WORD)fetch();

		if ((address >=0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		Registers[REGISTER_B] ^= lb;

		/** FLAGS **/

		set_flag_n(address);
		set_flag_z(address);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xD8: // EORB direct
		address = (WORD)fetch();

		if ((address >=0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		Registers[REGISTER_B] ^= lb;

		/** FLAGS **/

		set_flag_n(address);
		set_flag_z(address);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xE8: // EORB indexed
		lb = fetch();
		address = lb + (WORD)fetch();

		if ((address >=0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		Registers[REGISTER_B] ^= lb;

		/** FLAGS **/

		set_flag_n(address);
		set_flag_z(address);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0xF8: // EORB extended
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		if ((address >=0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		Registers[REGISTER_B] ^= lb;

		/** FLAGS **/

		set_flag_n(address);
		set_flag_z(address);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0x8A: // ORAA immediate
		lb=fetch();
		temp_word=(WORD)Registers[REGISTER_A]|(WORD)lb;

		/** FLAGS **/

		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Registers[REGISTER_A]=(BYTE)temp_word;
		break;

	case 0x9A: //ORAA direct
		address=(WORD)fetch();

		if ((address >= 0) | (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word=(WORD)Registers[REGISTER_A]|(WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_A]=(BYTE)temp_word;
		break;

	case 0xAA: // ORAA indexed
		address = IndexRegister + (WORD)fetch();

		if ((address >= 0) | (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD) Registers[REGISTER_A] | (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break;

	case 0xBA: // ORAA extended
		hb=(WORD)fetch();
		lb=(WORD)fetch();
		address=(hb<<8)+lb;
		address=(WORD)fetch();

		if ((address >= 0) | (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		temp_word = (WORD) Registers[REGISTER_A] | (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		Flags = Flags & (0xFF - FLAG_V);
		Registers[REGISTER_A]=(BYTE)temp_word;
		break;

	default:
		break;
	}
}


void Group_2_Single_Operand_Instructions(BYTE opcode)
{
	//////////////////////////
	/// GROUP 2 VARIABLES ///
	////////////////////////

	WORD hi_address, lo_address; 
	WORD address, temp_word;
	BYTE lb, hb;
	BYTE saved_flags; 

	switch(opcode)
	{

		/////////////////////////////
		/// GROUP 2 INSTRUCTIONS ///
		///////////////////////////

	case  0x6E: //JMP indexed
		address = IndexRegister + (WORD)fetch();
		hb = Memory[address];
		lb = Memory[address+1];
		address = ((WORD)hb << 8) +(WORD)lb;
		if (( address >=0) && (address < MEMORY_SIZE))
		{
			ProgramCounter = address;
		}
		break;

	case 0x7E: //JMP Extended
		hb = fetch();
		lb = fetch();
		address = ((WORD)hb << 8) + (WORD)lb;
		if (( address >=0) && (address < MEMORY_SIZE))
		{
			ProgramCounter = address;
		}
		break;

	case 0x6C: // INC indexed
		lb = fetch();	
		address = IndexRegister + (WORD)fetch();


		if ((address >=0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		lb++;

		temp_word = address + (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word); 
		set_flag_z((BYTE)temp_word); 

		if ((BYTE)temp_word == 0x80) 
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V); 
		}

		Memory[address] = lb; 
		break;

	case 0x7C: // INC extended
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		if ((address >=0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		lb++;

		/** FLAGS **/

		set_flag_n(lb); 
		set_flag_z(lb); 

		if (lb == 0x80) 
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V); //group 2
		}

		Memory[address] = lb; 

		break;

	case 0x4C: // INCA inherent

		Registers[REGISTER_A]++;

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);

		if (Registers[REGISTER_A] == 0x80)
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags  = Flags & (0xFF - FLAG_V);
		}

		break; 

	case 0x5C: // INCB inherent

		Registers[REGISTER_B]++;

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);


		if (Registers[REGISTER_B] == 0x80)
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags  = Flags & (0xFF - FLAG_V);
		}


		break; 

	case 0x6F: // CLR indexed
		address = IndexRegister + (WORD)fetch();
		if((address >= 0)&&(address < MEMORY_SIZE))
		{
			Memory[address] = 0;
		}

		/** FLAGS **/

		set_flag_z(0);
		Flags  = Flags & (0xFF - FLAG_N);
		Flags  = Flags & (0xFF - FLAG_V);
		Flags  = Flags & (0xFF - FLAG_C);
		break;

	case 0x7F: // CLR extended
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		if((address >= 0)&&(address < MEMORY_SIZE))
		{
			Memory[address] = 0;
		}

		/** FLAGS **/

		set_flag_z(0);
		Flags  = Flags & (0xFF - FLAG_N);
		Flags  = Flags & (0xFF - FLAG_V);
		Flags  = Flags & (0xFF - FLAG_C);
		break;


	case 0x4F: // CLRA inherent
		Registers[REGISTER_A] = 0x00;

		/** FLAGS **/

		set_flag_z(Registers[REGISTER_B]);
		Flags  = Flags & (0xFF - FLAG_N);
		Flags  = Flags & (0xFF - FLAG_V);
		Flags  = Flags & (0xFF - FLAG_C);
		break;

	case 0x5F: // CLRB inherent
		Registers[REGISTER_B] = 0x00;

		/** FLAGS **/

		set_flag_z(Registers[REGISTER_B]);
		Flags  = Flags & (0xFF - FLAG_N);
		Flags  = Flags & (0xFF - FLAG_V);
		Flags  = Flags & (0xFF - FLAG_C);
		break;

	case 0x6A: // DEC indexed
		lb = fetch();	
		address = IndexRegister + (WORD)fetch();


		if ((address >=0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		lb--;

		temp_word = address + (WORD)lb;

		/** FLAGS **/

		set_flag_n((BYTE)temp_word); 
		set_flag_z((BYTE)temp_word); 

		if ((BYTE)temp_word == 0x80) 
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V); 
		}

		Memory[address] = lb; 
		break;

	case 0x7A: // DEC extended
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		if ((address >=0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		lb--;

		/** FLAGS **/

		set_flag_n(lb); 
		set_flag_z(lb); 

		if (lb == 0x80) 
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V); //group 2
		}

		Memory[address] = lb; 

		break;
	case 0x4A: // DECA inherent
		Registers[REGISTER_A]--;

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);

		if (Registers[REGISTER_A] == 0x80)
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags  = Flags & (0xFF - FLAG_V);
		}

		break;

	case 0x5A: // DECB inherent
		Registers[REGISTER_B]--;

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);

		if (Registers[REGISTER_B] == 0x80)
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags  = Flags & (0xFF - FLAG_V);
		}

		break;

	case 0x63: // COM indexed
		lb = fetch();	
		address = lb + (WORD)fetch();

		if ((address >=0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		lb = ~lb;

		/** FLAGS **/

		Flags = Flags | FLAG_C;	// Set carry flag
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n(address);
		set_flag_z(address);
		break;

	case 0x73: // COM extended
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		if ((address >=0) && (address < MEMORY_SIZE))
		{
			lb = Memory[address];
		}
		else
		{
			lb = 0;
		}
		lb = ~lb;

		/** FLAGS **/

		Flags = Flags | FLAG_C;	// Set carry flag
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n(lb);
		set_flag_z(lb);
		break;

	case 0x43: // COMA inherent
		Registers[REGISTER_A] = ~Registers[REGISTER_A];

		/** FLAGS **/

		Flags = Flags | FLAG_C;	// Set carry flag
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0x53: // COMB inherent
		Registers[REGISTER_B] = ~Registers[REGISTER_B];

		/** FLAGS **/

		Flags = Flags | FLAG_C;	// Set carry flag
		Flags = Flags & (0xFF - FLAG_V);
		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		break;

	case 0x69: // ROL indexed
		lb = fetch();	
		address = lb + (WORD)fetch();

		saved_flags = Flags;

		if ((lb & 0x80) == 0x80) // carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		lb = (lb << 1) & 0xFE; 

		if(((lb & 0x80) == 0x80)
			^((Flags & FLAG_C) == FLAG_C)) // overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}

		if ((saved_flags & FLAG_C) == FLAG_C);
		{
			lb = lb | 0x01;
		}

		/** FLAGS **/

		set_flag_z(lb);
		set_flag_n(lb);
		break;

	case 0x79: // ROL extended
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();

		address = (hi_address << 8) + lo_address;

		lb = Memory[address];

		saved_flags = Flags;

		if ((lb & 0x80) == 0x80) // carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		lb = (lb << 1) & 0xFE; 

		if(((lb & 0x80) == 0x80)
			^((Flags & FLAG_C) == FLAG_C)) // overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}

		if ((saved_flags & FLAG_C) == FLAG_C);
		{
			lb = lb | 0x01;
		}

		/** FLAGS **/

		set_flag_z(lb);
		set_flag_n(lb);
		break;

	case 0x49: // ROLA inherent
		saved_flags = Flags;

		if ((Registers[REGISTER_A] & 0x80) == 0x80) // carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Registers[REGISTER_A] = (Registers[REGISTER_A] << 1) & 0xFE; 

		if(((Registers[REGISTER_A] & 0x80) == 0x80)
			^((Flags & FLAG_C) == FLAG_C)) // overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}

		if ((saved_flags & FLAG_C) == FLAG_C);
		{
			Registers[REGISTER_A] = Registers[REGISTER_A] | 0x01;
		}

		/** FLAGS **/

		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		break;

	case 0x59: // ROLB inherent
		saved_flags = Flags;

		if ((Registers[REGISTER_B] & 0x80) == 0x80) // carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Registers[REGISTER_B] = (Registers[REGISTER_B] << 1) & 0xFE; 

		if(((Registers[REGISTER_B] & 0x80) == 0x80)
			^((Flags & FLAG_C) == FLAG_C)) // overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}

		if ((saved_flags & FLAG_C) == FLAG_C);
		{
			Registers[REGISTER_B] = Registers[REGISTER_B] | 0x01;
		}

		/** FLAGS **/

		set_flag_z(Registers[REGISTER_B]);
		set_flag_n(Registers[REGISTER_B]);
		break;

	case 0x66: // ROR indexed
		lb = fetch();	
		address = lb + (WORD)fetch();

		saved_flags = Flags;

		if ((Memory[address] & 0x80) == 0x80) // carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Memory[address] = (Memory[address] >> 1) & 0xFE; 

		if(((Memory[address] & 0x80) == 0x80)
			^((Flags & FLAG_C) == FLAG_C)) // overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}

		if ((saved_flags & FLAG_C) == FLAG_C);
		{
			Memory[address] = Memory[address] | 0x01;
		}

		/** FLAGS **/

		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
		break;

	case 0x76: // ROR extended
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();

		address = (hi_address << 8) + lo_address;

		saved_flags = Flags;

		if ((Memory[address] & 0x80) == 0x80) // carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Memory[address] = (Memory[address] >> 1) & 0xFE; 

		if(((Memory[address] & 0x80) == 0x80)
			^((Flags & FLAG_C) == FLAG_C)) // overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}

		if ((saved_flags & FLAG_C) == FLAG_C);
		{
			Memory[address] = Memory[address] | 0x01;
		}

		/** FLAGS **/

		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
		break;

	case 0x46: // RORA inherent
		saved_flags = Flags;

		if ((Registers[REGISTER_A] & 0x80) == 0x80) // carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1) & 0xFE; 

		if(((Registers[REGISTER_A] & 0x80) == 0x80)
			^((Flags & FLAG_C) == FLAG_C)) // overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}

		if ((saved_flags & FLAG_C) == FLAG_C);
		{
			Registers[REGISTER_A] = Registers[REGISTER_A] | 0x01;
		}

		/** FLAGS **/

		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		break;

	case 0x56: // RORB inherent
		saved_flags = Flags;

		if ((Registers[REGISTER_B] & 0x80) == 0x80) // carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Registers[REGISTER_B] = (Registers[REGISTER_B] >> 1) & 0xFE; 

		if(((Registers[REGISTER_B] & 0x80) == 0x80)
			^((Flags & FLAG_C) == FLAG_C)) // overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}

		if ((saved_flags & FLAG_C) == FLAG_C);
		{
			Registers[REGISTER_B] = Registers[REGISTER_B] | 0x01;
		}

		/** FLAGS **/

		set_flag_z(Registers[REGISTER_B]);
		set_flag_n(Registers[REGISTER_B]);
		break;

	case 0x68: // ASL indexed
		lb = fetch();	
		address = lb + (WORD)fetch();

		saved_flags =Flags;
		if((Memory[address] & 0x80)==0x80) //carry
		{
			Flags= Flags | FLAG_C;
		}
		else
		{
			Flags=Flags&(0xFF - FLAG_C);
		}
		Memory[address] = (Memory[address] << 1) & 0xFE;
		if(((Memory[address]&0x80)==0x80)^((Flags &FLAG_C)==FLAG_C))//overflow
		{
			Flags =Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		if((saved_flags & FLAG_C)==FLAG_C)
		{
			Memory[address]=Memory[address]|0x01;
		}

		/** FLAGS **/

		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
		break;

	case 0x78: // ASL extended
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		saved_flags = Flags;

		if ((Memory[address] & 0x80) == 0x80) // carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Memory[address] = (Memory[address] << 1) & 0xFE; 

		if(((Memory[address] & 0x80) == 0x80)
			^((Flags & FLAG_C) == FLAG_C)) // overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}

		/** FLAGS **/

		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
		break;


	case 0x48: // ASLA inherent
		saved_flags = Flags;
		if((Registers[REGISTER_A]&0x80)==0x80) //carry
		{
			Flags= Flags | FLAG_C;
		}
		else
		{
			Flags=Flags&(0xFF - FLAG_C);
		}
		Registers[REGISTER_A] = (Registers[REGISTER_A] << 1) & 0xFE;
		if(((Registers[REGISTER_A]&0x80)==0x80)^((Flags &FLAG_C)==FLAG_C))//overflow
		{
			Flags =Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		if((saved_flags & FLAG_C)==FLAG_C)
		{
			Registers[REGISTER_A]=Registers[REGISTER_A]|0x01;
		}

		/** FLAGS **/

		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		break;


	case 0x58: // ASLB inherent
		saved_flags = Flags;
		if((Registers[REGISTER_B]&0x80)==0x80) //carry
		{
			Flags= Flags | FLAG_C;
		}
		else
		{
			Flags=Flags&(0xFF - FLAG_C);
		}
		Registers[REGISTER_B] = (Registers[REGISTER_B] << 1) & 0xFE;
		if(((Registers[REGISTER_B]&0x80)==0x80)^((Flags &FLAG_C)==FLAG_C))//overflow
		{
			Flags =Flags | FLAG_V;
		}
		else
		{
			Flags = Flags &(0xFF - FLAG_V);
		}
		if((saved_flags & FLAG_C)==FLAG_C)
		{
			Registers[REGISTER_B]=Registers[REGISTER_B]|0x01;
		}

		/** FLAGS **/

		set_flag_z(Registers[REGISTER_B]);
		set_flag_n(Registers[REGISTER_B]);
		break;

	case 0x67: // ASR indexed
		lb = fetch();	
		address = lb + (WORD)fetch();

		saved_flags = Flags;

		if ((Memory[address] & 0x80) == 0x80) // carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Memory[address] = (Memory[address] >> 1) & 0xFE; 

		if(((Memory[address] & 0x80) == 0x80)
			^((Flags & FLAG_C) == FLAG_C)) // overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}

		/** FLAGS **/

		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
		break;

	case 0x77: // ASR extended
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		saved_flags = Flags;

		if ((Memory[address] & 0x80) == 0x80) // carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Memory[address] = (Memory[address] >> 1) & 0xFE; 

		if(((Memory[address] & 0x80) == 0x80)
			^((Flags & FLAG_C) == FLAG_C)) // overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}

		/** FLAGS **/

		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);
		break;

	case 0x47: // ASRA inherent
		saved_flags = Flags;

		if ((Registers[REGISTER_A] & 0x80) == 0x80) // carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1) & 0xFE; 

		if(((Registers[REGISTER_A] & 0x80) == 0x80)
			^((Flags & FLAG_C) == FLAG_C)) // overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}

		/** FLAGS **/

		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);
		break;

	case 0x57: // ASRB inherent
		saved_flags = Flags;

		if ((Registers[REGISTER_B] & 0x80) == 0x80) // carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Registers[REGISTER_B] = (Registers[REGISTER_B] >> 1) & 0xFE; 

		if(((Registers[REGISTER_B] & 0x80) == 0x80)
			^((Flags & FLAG_C) == FLAG_C)) // overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}

		/** FLAGS **/

		set_flag_z(Registers[REGISTER_B]);
		set_flag_n(Registers[REGISTER_B]);
		break;

	case 0x64: // LSR indexed
		lb = fetch();	
		address = lb + (WORD)fetch();


		if ((Memory[address] & 0x01) == 0x01) // carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Memory[address] = (Memory[address] >> 1) & 0x7F;

		/** FLAGS **/

		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);

		if(((Memory[address] & 0x80) == 0x80)
			^((Flags & FLAG_C) == FLAG_C)) // overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}
		break;

	case 0x74: // LSR extended
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		if ((Memory[address] & 0x01) == 0x01) // carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Memory[address] = (Memory[address] >> 1) & 0x7F;

		/** FLAGS **/

		set_flag_z(Memory[address]);
		set_flag_n(Memory[address]);

		if(((Memory[address] & 0x80) == 0x80)
			^((Flags & FLAG_C) == FLAG_C)) // overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}
		break;

	case 0x44: // LSRA inherent
		if ((Registers[REGISTER_A] & 0x01) == 0x01) // carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Registers[REGISTER_A] = (Registers[REGISTER_A] >> 1) & 0x7F;

		/** FLAGS **/

		set_flag_z(Registers[REGISTER_A]);
		set_flag_n(Registers[REGISTER_A]);

		if(((Registers[REGISTER_A] & 0x80) == 0x80)
			^((Flags & FLAG_C) == FLAG_C)) // overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}
		break;

	case 0x54: // LSRB inherent
		if ((Registers[REGISTER_B] & 0x01) == 0x01) // carry
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}

		Registers[REGISTER_B] = (Registers[REGISTER_B] >> 1) & 0x7F;

		/** FLAGS **/

		set_flag_z(Registers[REGISTER_B]);
		set_flag_n(Registers[REGISTER_B]);

		if(((Registers[REGISTER_B] & 0x80) == 0x80)
			^((Flags & FLAG_C) == FLAG_C)) // overflow
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V);
		}
		break;

	case 0x6D: // TST indexed
		lb = fetch();
		address = lb + (WORD)fetch();

		Memory[address] - 0x00;

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_V);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x7D: // TST extended
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		Memory[address] - 0x00;

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_V);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x4D: // TSTA inherent
		Registers[REGISTER_A] - 0x00;

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_V);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x5D: // TSTB inherent
		Registers[REGISTER_B] - 0x00;

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		Flags = Flags & (0xFF - FLAG_V);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x60: // NEG indexed
		address = IndexRegister + (WORD)fetch();
		if((address >= 0)&&(address < MEMORY_SIZE))
		{
			Memory[address] = 0x00 - Memory[address];
		}

		/** FLAGS **/

		if (address >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		if (address == 0x80) 
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V); 
		}

		set_flag_n(address);
		set_flag_z(address);
		break;

	case 0x70: // NEG extended
		hi_address = (WORD)fetch();
		lo_address = (WORD)fetch();
		address = (hi_address << 8) + lo_address;

		if((address >= 0)&&(address < MEMORY_SIZE))
		{
			Memory[address] = 0x00 - Memory[address];
		}

		/** FLAGS **/

		if (address >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		if (address == 0x80) 
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V); 
		}

		set_flag_n(address);
		set_flag_z(address);
		break;

	case 0x40: // NEGA inherent
		Registers[REGISTER_A] = 0x00 - Registers[REGISTER_A];

		/** FLAGS **/

		if (Registers[REGISTER_A] >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		if (Registers[REGISTER_A] == 0x80) 
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V); 
		}

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		break;

	case 0x50: // NEGB inherent
		Registers[REGISTER_B] = 0x00 - Registers[REGISTER_B];

		/** FLAGS **/

		if (Registers[REGISTER_B] >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		if (Registers[REGISTER_B] == 0x80) 
		{
			Flags = Flags | FLAG_V;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_V); 
		}

		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		break;

	default:
		break;
	}
}


void Group_3_TPA_TAP_NOP_Instructions(BYTE opcode)
{
	switch(opcode)
	{

		/////////////////////////////
		/// GROUP 3 INSTRUCTIONS ///
		///////////////////////////

	case 0x01: // NOP
		break; 

	case 0x06: // TAP
		Flags = Registers[REGISTER_A];

		/** FLAGS **/

		Flags = Flags & (0xFF - FLAG_N);
		Flags = Flags & (0xFF - FLAG_Z);
		Flags = Flags & (0xFF - FLAG_V);
		Flags = Flags & (0xFF - FLAG_C);
		break;

	case 0x07: // TPA
		Registers[REGISTER_A] = Flags; 
		break;

	default:
		break;
	}
}


void Group_4_Condition_Code_Instructions(BYTE opcode)
{
	switch(opcode)
	{

		/////////////////////////////
		/// GROUP 4 INSTRUCTIONS ///
		///////////////////////////


	case 0x08: // INX inherent
		IndexRegister++;

		set_flag_z(IndexRegister);
		break;


	case 0x0C: // CLC
		Flags = Flags & (0xFF - FLAG_C); // Clear carry flag
		break;

	case 0x0A: // CLV
		Flags = Flags & (0xFF - FLAG_V); // Clear overflow flag
		break;

	case 0x0E: // CLI
		Flags = Flags & (0xFF - FLAG_I); // Clear interrupt flag
		break;

	case 0x0D: // SEC 
		Flags = Flags | FLAG_C; // Set carry flag
		break;

	case 0x0B: // SEV
		Flags = Flags | FLAG_V; // Set overflow flag
		break;

	case 0x0F: // SEI
		Flags = Flags | FLAG_I; // Set interrupt flag
		break;

	case 0x09: // DEX inherent
		IndexRegister--;

		set_flag_z(IndexRegister);
		break;

	default:
		break;
	}
}


void Group_5_Accumulator_Instructions(BYTE opcode)
{

	//////////////////////////
	/// GROUP 5 VARIABLES ///
	////////////////////////

	WORD temp_word; 

	switch(opcode)
	{

		/////////////////////////////
		/// GROUP 5 INSTRUCTIONS ///
		///////////////////////////

	case 0x1B: // ABA
		temp_word = Registers[REGISTER_A] + Registers[REGISTER_B];

		/** FLAGS **/

		if(temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);
		}
		break;

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		set_flag_v(Registers[REGISTER_A],Registers[REGISTER_B],(BYTE)temp_word);

		Registers[REGISTER_A] = (WORD)temp_word;

	case 0x10: // SBA inherent

		temp_word = (WORD)Registers[REGISTER_A] - (WORD)Registers[REGISTER_B];

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		set_flag_v(Registers[REGISTER_A],Registers[REGISTER_B],(BYTE)temp_word);
		Registers[REGISTER_A] = (BYTE)temp_word;
		break; 

	case 0x16: // TAB inherent
		Registers[REGISTER_B] = Registers[REGISTER_A];

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_B]);
		set_flag_z(Registers[REGISTER_B]);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0x17: // TBA inherent
		Registers[REGISTER_A] = Registers[REGISTER_B];

		/** FLAGS **/

		set_flag_n(Registers[REGISTER_A]);
		set_flag_z(Registers[REGISTER_A]);
		Flags = Flags & (0xFF - FLAG_V);
		break;

	case 0x11: // CBA inherent
		temp_word = Registers[REGISTER_A] - Registers[REGISTER_B];

		/** FLAGS **/

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;	// Set carry flag
		}
		else
		{ 
			Flags = Flags & (0xFF - FLAG_C);	// Clear carry flag
		}

		set_flag_n((BYTE)temp_word);
		set_flag_z((BYTE)temp_word);
		set_flag_v(Registers[REGISTER_A],Registers[REGISTER_B],(BYTE)temp_word);
		break;

	default:
		break;
	}
}


void Group_6_Branch_Instructions(BYTE opcode)
{
	//////////////////////////
	/// GROUP 6 VARIABLES ///
	////////////////////////

	BYTE N,Z,V,C;
	BYTE lb;
	WORD address, offset; 

	if ((Flags & FLAG_N) != 0)
	{
		N = 1;
	}
	else
	{
		N = 0;
	}

	if ((Flags & FLAG_Z) != 0)
	{
		Z = 1;
	}
	else
	{
		Z = 0;
	}

	if ((Flags & FLAG_V) != 0)
	{
		V = 1;
	}
	else
	{
		V = 0;
	}

	if ((Flags & FLAG_C) != 0)
	{
		C = 1;
	}
	else
	{
		C = 0;
	}

	switch (opcode)
	{

		/////////////////////////////
		/// GROUP 6 INSTRUCTIONS ///
		///////////////////////////

	case 0x20: //BRA Direct
		lb = fetch();
		offset = (WORD) lb;
		if((offset & 0x80) !=0) // need to sign extend
		{
			offset = offset + 0xFF00;
		}
		address = ProgramCounter + offset;
		ProgramCounter = address;
		break;

	case 0x24: //BCC Direct
		lb = fetch();
		if ((Flags & FLAG_C) != FLAG_C)
		{
			offset = (WORD) lb;
			if((offset & 0x80) !=0)
			{
				offset = offset + 0xFF00; 
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x25: //BCS Direct
		lb = fetch();
		if ((Flags & FLAG_C) == FLAG_C)
		{
			offset = (WORD) lb;
			if((offset & 0x80) !=0)
			{
				offset = offset + 0xFF00; 
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x27: //BEQ Direct
		lb = fetch();
		if ((Flags & FLAG_Z) == FLAG_Z)
		{
			offset = (WORD) lb;
			if((offset & 0x80) !=0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x2C: //BGE Direct
		lb = fetch();
		if ((N ^ V) == 0)
		{
			offset = (WORD) lb;
			if((offset & 0x80) !=0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x2E: //BGT Direct
		lb = fetch();
		if ((Z | N ^ V) == 0)
		{
			offset = (WORD) lb;
			if((offset & 0x80) !=0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x22: //BHI Direct
		lb = fetch();
		if (C | Z == 0)
		{
			offset = (WORD) lb;
			if((offset & 0x80) !=0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x2F: //BLE Direct
		lb = fetch();
		if ((Z | N ^ V) == 1)
		{
			offset = (WORD) lb;
			if((offset & 0x80) !=0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x23: //BLS Direct
		lb = fetch();
		if (C | Z == 1)
		{
			offset = (WORD) lb;
			if((offset & 0x80) !=0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x2D: //BLT DIRECT
		lb = fetch();
		if ( N ^ V == 1)
		{
			offset = (WORD) lb;
			if((offset & 0x80) !=0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x2B: //BMI Direct
		lb = fetch();
		if ((Flags & FLAG_N) == FLAG_N)
		{
			offset = (WORD) lb;
			if((offset & 0x80) !=0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x26: //BNE Direct
		lb = fetch();
		if ((Flags & FLAG_Z) != FLAG_Z)
		{
			offset = (WORD) lb;
			if((offset & 0x80) !=0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x28: //BVC Direct
		lb = fetch();
		if ((Flags & FLAG_V) != FLAG_V)
		{
			offset = (WORD) lb;
			if((offset & 0x80) !=0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x29: //BVS
		lb = fetch();
		if ((Flags & FLAG_V) == FLAG_V)
		{
			offset = (WORD) lb;
			if((offset & 0x80) !=0)
			{
				offset = offset + 0xFF00; 
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	case 0x2A: //BPL Direct
		lb = fetch();
		if ((Flags & FLAG_N) != FLAG_N)
		{
			offset = (WORD) lb;
			if((offset & 0x80) !=0)
			{
				offset = offset + 0xFF00;
			}
			address = ProgramCounter + offset;
			ProgramCounter = address;
		}
		break;

	default:
		break;
	}
}


void Group_7_Stack_and_Index_Register_Instructions(BYTE opcode)
{
	switch(opcode)
	{

		/////////////////////////////
		/// GROUP 7 INSTRUCTIONS ///
		///////////////////////////

	case 0x35: // TXS inherent
		StackPointer = IndexRegister -1; 
		break;

	case 0x30: // TSX inherent
		IndexRegister = StackPointer +1;
		break;

	case 0x36: // PSHA inherent
		if((StackPointer >= 1) && (StackPointer <(MEMORY_SIZE)))
		{
			Memory[StackPointer] = Registers[REGISTER_A];
			StackPointer--;
		}
		break;

	case 0x37: // PSHB inherent
		if((StackPointer >= 1) && (StackPointer <(MEMORY_SIZE)))
		{
			Memory[StackPointer] = Registers[REGISTER_B];
			StackPointer--;
		}
		break;

	case 0x32: // PULA inherent
		if((StackPointer >= 0) && (StackPointer <(MEMORY_SIZE-1)))
		{
			StackPointer++; 
			Registers[REGISTER_A] = Memory[StackPointer];
		}
		break;

	case 0x33: // PULB inherent
		if((StackPointer >= 0) && (StackPointer <(MEMORY_SIZE-1)))
		{
			StackPointer++; 
			Registers[REGISTER_B] = Memory[StackPointer];
		}
		break;

	case 0x34: // DES inherent
		StackPointer--;
		break;

	case 0x31: // INS inherent
		StackPointer++;
		break;

	default:
		break;


	}
}

void Group_8_Interrupt_and_Subroutine_Instructions(BYTE opcode)
{
	//////////////////////////
	/// GROUP 8 VARIABLES ///
	////////////////////////

	BYTE hb, lb;

	switch(opcode)
	{

		/////////////////////////////
		/// GROUP 8 INSTRUCTIONS ///
		///////////////////////////

	case 0x3E:  // WAI
		halt = true;
		break;

	case 0x39: // RTS
		if ((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE -2)))
		{
			StackPointer++;
			hb = Memory[StackPointer];
			StackPointer++;
			lb = Memory[StackPointer];

			ProgramCounter = ((WORD)hb << 8) + (WORD)lb;
		}
		break;

	case 0x3B: // RTI 
		StackPointer++;
		Flags = Memory[StackPointer];
		StackPointer++;
		Registers[REGISTER_B] = Memory[StackPointer];
		StackPointer++;
		Registers[REGISTER_A] = Memory[StackPointer];
		StackPointer++;

		IndexRegister = (WORD)(Memory[StackPointer] << 8) & 0xFF00;
		StackPointer++;
		IndexRegister += (WORD)(Memory[StackPointer]) & 0x00FF;
		StackPointer++;
		ProgramCounter = (WORD)(Memory[StackPointer] << 8) & 0xFF00;
		StackPointer++;
		ProgramCounter += (WORD)(Memory[StackPointer]) & 0x00FF;
		break; 

	default:
		break;
	}
}







void execute(BYTE opcode)
{
	BYTE high_nibble;
	BYTE low_nibble;

	high_nibble = (opcode >> 4) & 0xFF;
	low_nibble  = opcode & 0xFF;

	if ((opcode & 0x80) == 0x80)  // Group 1: Dual Operand Instructions
	{
		Group_1_Dual_Operand_Instructions(opcode);
	}
	else if ((opcode & 0xC0) == 0x40)  // Group 2: Single Operand Instructions
	{
		Group_2_Single_Operand_Instructions(opcode);
	}
	else if ((opcode & 0xF8) == 0x00)  // Group 3: TPA, TAP, NOP Instructions
	{
		Group_3_TPA_TAP_NOP_Instructions(opcode);
	}
	else if ((opcode & 0xF8) == 0x08)  // Group 4: Condition Code Instructions
	{
		Group_4_Condition_Code_Instructions(opcode);
	}
	else if ((opcode & 0xF0) == 0x10)  // Group 5: Accumulator Instructions
	{
		Group_5_Accumulator_Instructions(opcode);
	}
	else if ((opcode & 0xF0) == 0x20)  // Group 6: Branch Instructions
	{
		Group_6_Branch_Instructions(opcode);
	}
	else if ((opcode & 0xF8) == 0x30)  // Group 7: Stack and Index Register Instructions
	{
		Group_7_Stack_and_Index_Register_Instructions(opcode);
	}
	else if ((opcode & 0xF8) == 0x38)  // Group 8: Interrupt and Subroutine Instructions
	{
		Group_8_Interrupt_and_Subroutine_Instructions(opcode);
	}
	else
	{
		printf("ERROR> Unrecognised Op-code %X\n", opcode);
	}
}


void emulate_6800()
{
	BYTE opcode;

	ProgramCounter = 0;
	halt = false;
	memory_in_range = true;

	printf("                  A  B   IX   SP\n");

	while ((!halt) && (memory_in_range))
	{
		printf("%04X ", ProgramCounter);           // Print current address

		opcode = fetch();
		execute(opcode);

		printf("%s  ", opcode_mneumonics[opcode]);  // Print current opcode

		printf("%02X ", Registers[REGISTER_A]);     // Print Accumulator
		printf("%02X ", Registers[REGISTER_B]);     // Print Register B

		printf("%04X ", IndexRegister);              // Print Stack Pointer

		printf("%04X ", StackPointer);              // Print Stack Pointer

		if ((Flags & FLAG_H) == FLAG_H)	            // Print Half Carry Flag
		{
			printf("H=1 ");
		}
		else
		{
			printf("H=0 ");
		}

		if ((Flags & FLAG_I) == FLAG_I)	            // Print Interrupt Flag
		{
			printf("I=1 ");
		}
		else
		{
			printf("I=0 ");
		}

		if ((Flags & FLAG_N) == FLAG_N)	            // Print Sign Flag
		{
			printf("N=1 ");
		}
		else
		{
			printf("N=0 ");
		}

		if ((Flags & FLAG_Z) == FLAG_Z)	            // Print Zero Flag
		{
			printf("Z=1 ");
		}
		else
		{
			printf("Z=0 ");
		}

		if ((Flags & FLAG_V) == FLAG_V)	            // Print Overflow Flag
		{
			printf("V=1 ");
		}
		else
		{
			printf("V=0 ");
		}

		if ((Flags & FLAG_C) == FLAG_C)	            // Print Carry Flag
		{
			printf("C=1 ");
		}
		else
		{
			printf("C=0 ");
		}

		printf("\n");  // New line
	}

	printf("\n");  // New line
}




////////////////////////////////////////////////////////////////////////////////
//                      Intel 6800 Simulator/Emulator (End)                   //
////////////////////////////////////////////////////////////////////////////////













void initialise_filenames()
{
	int i;

	for (i=0; i<MAX_FILENAME_SIZE; i++)
	{
		hex_file [i] = '\0';
		trc_file [i] = '\0';
	}
}




int find_dot_position(char *filename)
{
	int  dot_position;
	int  i;
	char chr;

	dot_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0')
	{
		if (chr == '.') 
		{
			dot_position = i;
		}
		i++;
		chr = filename[i];
	}

	return (dot_position);
}


int find_end_position(char *filename)
{
	int  end_position;
	int  i;
	char chr;

	end_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0')
	{
		end_position = i;
		i++;
		chr = filename[i];
	}

	return (end_position);
}


bool file_exists(char *filename)
{
	bool exists;
	FILE *ifp;

	exists = false;

	if ( ( ifp = fopen( filename, "r" ) ) != NULL ) 
	{
		exists = true;

		fclose(ifp);
	}

	return (exists);
}



void create_file(char *filename)
{
	FILE *ofp;

	if ( ( ofp = fopen( filename, "w" ) ) != NULL ) 
	{
		fclose(ofp);
	}
}



bool getline(FILE *fp, char *buffer)
{
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect)
	{
		c = getc(fp);

		switch (c)
		{
		case EOF:
			if (i > 0)
			{
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0)
			{
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}





void load_and_run()
{
	char chr;
	int  ln;
	int  dot_position;
	int  end_position;
	long i;
	FILE *ifp;
	long address;
	long load_at;
	int  code;

	// Prompt for the .hex file

	printf("\n");
	printf("Enter the hex filename (.hex): ");

	ln = 0;
	chr = '\0';
	while (chr != '\n')
	{
		chr = getchar();

		switch(chr)
		{
		case '\n':
			break;
		default:
			if (ln < MAX_FILENAME_SIZE)
			{
				hex_file [ln] = chr;
				trc_file [ln] = chr;
				ln++;
			}
			break;
		}
	}

	// Tidy up the file names

	dot_position = find_dot_position(hex_file);
	if (dot_position == 0)
	{
		end_position = find_end_position(hex_file);

		hex_file[end_position + 1] = '.';
		hex_file[end_position + 2] = 'h';
		hex_file[end_position + 3] = 'e';
		hex_file[end_position + 4] = 'x';
		hex_file[end_position + 5] = '\0';
	}
	else
	{
		hex_file[dot_position + 0] = '.';
		hex_file[dot_position + 1] = 'h';
		hex_file[dot_position + 2] = 'e';
		hex_file[dot_position + 3] = 'x';
		hex_file[dot_position + 4] = '\0';
	}

	dot_position = find_dot_position(trc_file);
	if (dot_position == 0)
	{
		end_position = find_end_position(trc_file);

		trc_file[end_position + 1] = '.';
		trc_file[end_position + 2] = 't';
		trc_file[end_position + 3] = 'r';
		trc_file[end_position + 4] = 'c';
		trc_file[end_position + 5] = '\0';
	}
	else
	{
		trc_file[dot_position + 0] = '.';
		trc_file[dot_position + 1] = 't';
		trc_file[dot_position + 2] = 'r';
		trc_file[dot_position + 3] = 'c';
		trc_file[dot_position + 4] = '\0';
	}

	if (file_exists(hex_file))
	{
		// Clear Registers and Memory

		Flags = 0;
		ProgramCounter = 0;
		StackPointer = 0;
		IndexRegister = 0;
		for (i=0; i<2; i++)
		{
			Registers[i] = 0;
		}
		for (i=0; i<MEMORY_SIZE; i++)
		{
			Memory[i] = 0xFF;
		}

		// Load hex file

		if ( ( ifp = fopen( hex_file, "r" ) ) != NULL ) 
		{
			printf("Loading file...\n\n");

			load_at = 0;

			while (getline(ifp, InputBuffer))
			{
				if (sscanf(InputBuffer, "L=%x", &address) == 1)
				{
					load_at = address;
				}
				else if (sscanf(InputBuffer, "%x", &code) == 1)
				{
					if ((load_at >= 0) && (load_at <= MEMORY_SIZE))
					{
						Memory[load_at] = (BYTE)code;
					}
					load_at++;
				}
				else
				{
					printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
				}
			}

			fclose(ifp);
		}

		// Emulate the 6800

		emulate_6800();
	}
	else
	{
		printf("\n");
		printf("ERROR> Input file %s does not exist!\n", hex_file);
		printf("\n");
	}
}




void test_and_mark()
{
	char buffer[1024];
	bool testing_complete;
	int  len = sizeof(SOCKADDR);
	char chr;
	int  i;
	int  j;
	bool end_of_program;
	long address;
	long load_at;
	int  code;
	int  mark;

	printf("\n");
	printf("Automatic Testing and Marking\n");
	printf("\n");

	testing_complete = false;

	sprintf(buffer, "Test Student %s", STUDENT_NUMBER);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));

	while (!testing_complete)
	{
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer)-1, 0, (SOCKADDR *)&client_addr, &len) != SOCKET_ERROR)
		{
			printf("Incoming Data: %s \n", buffer);

			//if (strcmp(buffer, "Testing complete") == 0)
			if (sscanf(buffer, "Testing complete %d", &mark) == 1)
			{
				testing_complete = true;
				printf("Current mark = %d\n", mark);
			}
			else if (strcmp(buffer, "Error") == 0)
			{
				printf("ERROR> Testing abnormally terminated\n");
				testing_complete = true;
			}
			else
			{
				// Clear Registers and Memory

				Flags = 0;
				ProgramCounter = 0;
				StackPointer = 0;
				for (i=0; i<2; i++)
				{
					Registers[i] = 0;
				}
				for (i=0; i<MEMORY_SIZE; i++)
				{
					Memory[i] = 0;
				}

				// Load hex file

				i = 0;
				j = 0;
				load_at = 0;
				end_of_program = false;
				while (!end_of_program)
				{
					chr = buffer[i];
					switch (chr)
					{
					case '\0':
						end_of_program = true;

					case ',':
						if (sscanf(InputBuffer, "L=%x", &address) == 1)
						{
							load_at = address;
						}
						else if (sscanf(InputBuffer, "%x", &code) == 1)
						{
							if ((load_at >= 0) && (load_at <= MEMORY_SIZE))
							{
								Memory[load_at] = (BYTE)code;
							}
							load_at++;
						}
						else
						{
							printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
						}
						j = 0;
						break;

					default:
						InputBuffer[j] = chr;
						j++;
						break;
					}
					i++;
				}

				// Emulate the 6800

				if (load_at > 1)
				{
					emulate_6800();

					// Send results

					sprintf(buffer, "%02X%02X %02X%02X %04X %04X %02X%02X %02X%02X", Registers[REGISTER_A], Registers[REGISTER_B], Flags, Memory[TEST_ADDRESS_1], IndexRegister, StackPointer, Memory[TEST_ADDRESS_2], Memory[TEST_ADDRESS_3], Memory[TEST_ADDRESS_4], Memory[TEST_ADDRESS_5]);
					sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
				}
			}
		}
	}
}



int _tmain(int argc, _TCHAR* argv[])
{
	char chr;
	char dummy;

	printf("\n");
	printf("Intel 6800 Microprocessor Emulator\n");
	printf("UWE Computer and Network Systems Assignment 1 (2014-15)\n");
	printf("\n");

	initialise_filenames();

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock)
	{
		// Creation failed!
	}

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	//int ret = bind(sock, (SOCKADDR *)&client_addr, sizeof(SOCKADDR));
	//if (ret)
	//{
	//   //printf("Bind failed! \n");  // Bind failed!
	//}



	chr = '\0';
	while ((chr != 'e') && (chr != 'E'))
	{
		printf("\n");
		printf("Please select option\n");
		printf("L - Load and run a hex file\n");
		printf("T - Have the server test and mark your emulator\n");
		printf("E - Exit\n");
		printf("Enter option: ");
		chr = getchar();
		if (chr != 0x0A)
		{
			dummy = getchar();  // read in the <CR>
		}
		printf("\n");

		switch (chr)
		{
		case 'L':
		case 'l':
			load_and_run();
			break;

		case 'T':
		case 't':
			test_and_mark();
			break;

		default:
			break;
		}
	}
	
	closesocket(sock);
	WSACleanup();


	return 0;
}

