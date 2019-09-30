#include <iostream>
#include <fstream>
#include "stdlib.h"
#include <iomanip>
#include <string>
#include <cstring>
#include <ctime>

using namespace std;
#define memWi 8
#define memHi 1024
#define TimeLimit 3
const unsigned int memSize = memWi*memHi;


int regs[32] = { 0 };
unsigned int pc = 0x0;

char memory[memSize];	// only 8KB of memory located at address 0
bool stop = false;
bool printMode = true;
bool stepByStep = true;

time_t beforeExec = time(NULL);	

void emitError(const string& s) {
	cout << s;
	stop = true;
}

void printPrefix(unsigned int instA, unsigned int instW) {
	cout << "0x" << hex << std::setfill('0') << std::setw(8) << instA << "\t0x" << std::setw(8) << instW;
}

void ecall(unsigned int instWord)
{
	time_t now = time(NULL);
	unsigned int instPC = pc - 4;

	printPrefix(instPC, instWord);

	cout << "\tecall\n";
	if (regs[17] == 10)//terminate
	{
		stop = true;
	}
	else if (!printMode){
		if (regs[17] == 1)//print int
		{
			cout << "<<< " << dec << regs[10] << endl;
		}
		else if (regs[17] == 4)// print string
		{
			cout << "<<< ";
			char value = memory[regs[10]];
			int j = 0;
			while (value != '\0')
			{
				cout << value;
				j++;
				value = memory[regs[10] + j];
			}
			cout << endl;
		}
		else if (regs[17] == 5)//read int
		{
			cout << ">>> ";
			cin >> regs[10];
		}
	}
	beforeExec += difftime(time(NULL),now);
}

void SB(unsigned int instWord, unsigned int opcode)
{
	unsigned int rd, rs1, rs2, funct3;
	unsigned int SB_imm;
	unsigned int address;
	unsigned int instPC = pc - 4;
	printPrefix(instPC, instWord);

	rd = (instWord >> 7) & 0x0000001F;
	funct3 = (instWord >> 12) & 0x00000007;
	rs1 = (instWord >> 15) & 0x0000001F;
	rs2 = (instWord >> 20) & 0x0000001F;
	SB_imm = ((((instWord >> 8) & 0xF | ((instWord & 0x7E000000) >> 21) | (((instWord & 0x80) << 3)) | (instWord & 0x80) | ((instWord >> 31) ? 0xFFFFE000 : 0x0))));
	SB_imm = SB_imm << 1;
	if (!(SB_imm % 4)) {
		switch (funct3) {
			case 0:						//beq
				cout << "\tbeq\tx" << dec << rs1 << ", x" << rs2 << ", " << int(SB_imm) << "\n";
				if (!printMode && regs[rs1] == regs[rs2])
					pc = instPC + int(SB_imm);
				break;
			case 1:						//bne
				cout << "\tbne\tx" << dec << rs1 << ", x" << rs2 << ", " << int(SB_imm) << "\n";
				if (!printMode && regs[rs1] != regs[rs2])
					pc = instPC + int(SB_imm);
				break;
			case 4:						//blt
				cout << "\tblt\tx" << dec << rs1 << ", x" << rs2 << ", " << int(SB_imm) << "\n";
				if (!printMode && regs[rs1] < regs[rs2])
					pc = instPC + int(SB_imm);
				break;
			case 5:						 //bge
				cout << "\tbge\tx" << dec << rs1 << ", x" << rs2 << ", " << int(SB_imm) << "\n";
				if (!printMode && regs[rs1] >= regs[rs2])
					pc = instPC + int(SB_imm);
				break;
			case 6:						 //bltu
				cout << "\tbltu\tx" << dec << rs1 << ", x" << rs2 << ", " << int(SB_imm) << "\n";
				if (!printMode && (unsigned int)regs[rs1] < (unsigned int)regs[rs2])
					pc = instPC + int(SB_imm);
				break;
			case 7:						//bgeu
				cout << "\tbgeu\tx" << dec << rs1 << ", x" << rs2 << ", " << int(SB_imm) << "\n";
				if (!printMode && (unsigned int)regs[rs1] >= (unsigned int)regs[rs2])
					pc = instPC + int(SB_imm);
				break;
			default:
				cout << "\tUnkown S Instruction \n";
		}
	}
	else cout << "Misalligned branch \n";
}

void S(unsigned int instWord, unsigned int opcode)
{
	unsigned int rd, rs1, rs2, funct3;
	unsigned int S_imm;
	unsigned int address;
	unsigned int instPC = pc - 4;
	printPrefix(instPC, instWord);

	rd = (instWord >> 7) & 0x0000001F;
	funct3 = (instWord >> 12) & 0x00000007;
	rs1 = (instWord >> 15) & 0x0000001F;
	rs2 = (instWord >> 20) & 0x0000001F;
	S_imm = ((((instWord >> 7) & 0x1F) | ((instWord & 0xFE000000)) >> 20) | (((instWord >> 31) ? 0xFFFFF000 : 0x0)));

	//int i = 0; int temp = regs[rs2];
	int i = 0;
	unsigned char temp;

	switch (funct3) {
		case 0://sb
			cout << dec << "\tsb\tx" << rs2 << ", " << int(S_imm) << "(x" << rs1 << ")\n";
			if (!printMode){
				temp = regs[rs2] & 0x000000FF;
				memory[regs[rs1] + int(S_imm)] = temp;
			}
			break;
		case 1://sh
			cout << dec << "\tsh\tx" << rs2 << ", " << int(S_imm) << "(x" << rs1 << ")\n";
			while (!printMode && i < 2) {
				temp = ((regs[rs2] >> (8 * i)) & 0x000000FF);
				memory[regs[rs1] + int(S_imm) + i] = temp;
				i++;
			}
			break;
		case 2://sw
			cout << dec << "\tsw\tx" << rs2 << ", " << int(S_imm) << "(x" << rs1 << ")\n";
			while (!printMode && i < 4 ) {
				temp = ((regs[rs2] >> (8 * i)) & 0x000000FF);
				memory[regs[rs1] + int(S_imm) + i] = temp;
				i++;
			}
			break;
		default:
			cout << "\tUnkown S Instruction \n";
	}
}

void I(unsigned int instWord, unsigned int opcode)
{
	unsigned int rd, rs1, rs2, funct3, funct7;
	unsigned int I_imm;
	unsigned int address;

	unsigned int instPC = pc - 4;
	printPrefix(instPC, instWord);

	//almost present in all formats:
	rd = (instWord >> 7) & 0x1F;//0x1F = 0b1_1111
	funct3 = (instWord >> 12) & 0x7; //0x7 = 0b111
	rs1 = (instWord >> 15) & 0x1F;
	rs2 = (instWord >> 20) & 0x1F;

	// — inst[31] — inst[30:25] inst[24:21] inst[20]


	I_imm = ((instWord >> 20) & 0x7FF)  // read the first 11 bits
		| (((instWord >> 31) ? 0xFFFFF800 : 0x0)); //perform sign extension (always sign exteneded first)
	cout << dec; //necessary since std::hex is "sticky"

	if (opcode == 0x13) {	// I instructions

		switch (funct3) {
			case 0x0: //addi
				cout << "\taddi\tx" << rd << ", x" << rs1 << ", " << dec << int(I_imm) << "\n";
				if (!printMode && rd) regs[rd] = regs[rs1] + int(I_imm);
				break;
			case 0x2: //slti
				cout << "\tslti\tx" << rd << ", x" << rs1 << ", " << dec << int(I_imm) << "\n";
				if (!printMode &&rd) regs[rd] = regs[rs1] < int(I_imm);
				break;
			case 0x3: //sltiu
				cout << "\tsltiu\tx" << rd << ", x" << rs1 << ", " << dec << int(I_imm) << "\n";
				if (!printMode &&rd) regs[rd] = (unsigned int)regs[rs1] < I_imm;
				break;
			case 0x4: //xori
				cout << "\txori\tx" << rd << ", x" << rs1 << ", " << dec << int(I_imm) << "\n";
				if (!printMode &&rd) regs[rd] = regs[rs1] ^ I_imm;
				break;
			case 0x6: //ori
				cout << "\tori\tx" << rd << ", x" << rs1 << ", " << dec << int(I_imm) << "\n";
				if (!printMode &&rd) regs[rd] = regs[rs1] | I_imm;
				break;
			case 0x7: //andi
				cout << "\tandi\tx" << rd << ", x" << rs1 << ", " << dec << int(I_imm) << "\n";
				if (!printMode &&rd) regs[rd] = regs[rs1] & I_imm;
				break;
			case 0x1: //slli
				if ((I_imm >> 5) & 0x7F) cout << "slli with a wrong format detected\n"; //?
				cout << "\tslli\tx" << rd << ", x" << rs1 << ", " << dec << ((I_imm) & 0x1F) << "\n";
				if (!printMode &&rd) regs[rd] = regs[rs1] << (I_imm & 0x1F);
				break;
			default: //srli and srai
				if (((I_imm >> 5) & 0x7F) == 0x0) {
					cout << "\tsrli\tx" << rd << ", x" << rs1 << ", " << dec << ((I_imm) & 0x1F) << "\n";
					if (!printMode &&rd) regs[rd] = (unsigned int)regs[rs1] >> (I_imm & 0x1F); //c++ : 0-ext of unsigned ints
				}
				else if (((I_imm >> 5) & 0x7F) == 0x20) { //0b0100000
					cout << "\tsrai\tx" << rd << ", x" << rs1 << ", " << dec << ((I_imm) & 0x1F) << "\n";
					if (!printMode &&rd) regs[rd] = regs[rs1] >> (I_imm & 0x1F); //c++ : sign-extension of ints
				}
				else
					cout << "Unknown shift instruction detected\n";
		}
	}
	else if (opcode == 0x3) { //Load instructions (I-Type)
		if (!printMode &&rd) {
			regs[rd] = 0;
			for (int i = 0; i < (1 << (funct3 % 4)); i++) { // functionality - compact
				if (funct3 == 0x4 || funct3 == 0x5)
					regs[rd] |= (((unsigned char)memory[int(I_imm) + regs[rs1] + i]) << i * 8);
				else
					regs[rd] |= ((memory[int(I_imm) + regs[rs1] + i]) << i * 8);
			}
		}

		switch (funct3) { //print
			case 0x0: //lb
				cout << "\tlb\tx" << rd << ", " << dec << int(I_imm) << "(x" << rs1 << ")\n";
				break;
			case 0x1: //lh
				cout << "\tlh\tx" << rd << ", " << dec << int(I_imm) << "(x" << rs1 << ")\n";
				break;
			case 0x2: //lw
				cout << "\tlw\tx" << rd << ", " << dec << int(I_imm) << "(x" << rs1 << ")\n";
				break;
			case 0x4: //lbu
				cout << "\tlbu\tx" << rd << ", " << dec << int(I_imm) << "(x" << rs1 << ")\n";
				break;
			case 0x5: //lhu
				cout << "\tlhu\tx" << rd << ", " << dec << int(I_imm) << "(x" << rs1 << ")\n";
				break;
			default:
				cout << "Invalid Load instruction detected\n";
		}
	}
	else if (opcode == 0x67) { //jalr - I-Type
		if (funct3) cout << "jalr with a wrong format detected\n";
		if (rd) regs[rd] = pc;
		if (!printMode) pc = (regs[rs1] + int(I_imm))&(~1);
		if (pc > (1 << 13) - 1 || pc % 4)
			emitError("Misaligned or out-of-range address with jalr detected\n");
		cout << "\tjalr\tx" << rd << ", x" << rs1 << ", " << dec << int(I_imm) << "\n";
	}
	else
		cout << "\tUnkown Instruction Type\n";
}

void U_UJ(unsigned int instWord, unsigned int opcode)
{
	unsigned int rd;
	unsigned int U_imm;
	unsigned int address;

	unsigned int instPC = pc - 4;
	printPrefix(instPC, instWord);

	opcode = instWord & 0x7F; // 0x7F = 0b111_1111
	rd = (instWord >> 7) & 0x1F;//0x1F = 0b1_1111

	U_imm = ((instWord >> 12) & 0xFFFFF)
		| (((instWord >> 31) ? 0xFFF00000 : 0x0));
	cout << dec;
	switch (opcode) {
		case 0x37:				// lui
			cout << "\tlui\tx" << rd << ", " << hex << U_imm << endl;
			if (!printMode &&rd)
			{
				regs[rd] = (U_imm << 12) & 0xFFFFFFFF;
			}
			break;
		case 0x17:				// auipc
			cout << "\tauipc\tx" << rd << ", " << hex << U_imm << endl;
			if (!printMode &&rd)
			{
				regs[rd] = ((U_imm << 12) & 0xFFFFFFFF) + instPC;
			}
			break;
		case 0x6F:				// jal
			U_imm = (((U_imm >> 19) << 19) | ((U_imm & 0xFF) << 11) | (((U_imm >> 8) & 0x1) << 10) | ((U_imm >> 9) & 0x3FF));
			U_imm = U_imm << 1;
			cout << "\tjal\tx" << rd << ", " << hex <<int(U_imm) << endl;
			if (U_imm % 4) emitError("Misalligned address with jal detected\n");
			else if (!printMode) {
				if (rd) regs[rd] = pc;
				pc = instPC + int(U_imm);
			}
			break;
		default:
			cout << "\tUnkown Instruction Type\n";
	}
}

void R(unsigned int instWord, unsigned int opcode)
{

	unsigned int instPC = pc - 4;
	printPrefix(instPC, instWord);
	unsigned int rd, rs1, rs2, funct3, funct7;

	rd = (instWord >> 7) & 0x0000001F;
	funct3 = (instWord >> 12) & 0x00000007;
	rs1 = (instWord >> 15) & 0x0000001F;
	rs2 = (instWord >> 20) & 0x0000001F;
	funct7 = (instWord >> 25) & 0x00007F;

	cout << dec;
	switch (funct3)
	{
		case 0:
			switch (funct7)
			{
				case 0:
					if (!printMode)
						regs[rd] = regs[rs1] + regs[rs2];		//add $rd, $rs1, $rs2;
					cout << "\tadd\tx" << rd << ", x" << rs1 << ", x" << rs2 << endl;
					break;
				case 32:
					if (!printMode)
						regs[rd] = regs[rs1] - regs[rs2];		// sub $rd, $rs1, $rs2;
					cout << "\tsub\tx" << rd << ", x" << rs1 << ", x" << rs2 << endl;
					break;
				default: printf("Invalid R-type Instruction Detected\n");
			}
			break;
		case 1:
			if (!funct7)	//funct7 = (0000000)2
			{
				if (!printMode)
					regs[rd] = (regs[rs1] << regs[rs2]);	// sll $rd, $rs1, $rs2;
				cout << "\tsll\tx" << rd << ", x" << rs1 << ", " << rs2 << endl;
			}
			else printf("Invalid R-type Instruction Detected\n");
			break;
		case 2:
			if (!funct7)
			{
				if (!printMode)
					regs[rd] = (regs[rs1] < regs[rs2]);		// slt $rd, $rs1, $rs2;
				cout << "\tslt\tx" << rd << ", x" << rs1 << ", x" << rs2 << endl;
			}
			else printf("Invalid R-type Instruction Detected\n");
			break;
		case 3:
			if (!funct7)
			{
				if (!printMode)
					regs[rd] = ((unsigned)regs[rs1] < (unsigned)regs[rs2]);		// sltu $rd, $rs1, $rs2;
				cout << "\tsltu\tx" << rd << ", x" << rs1 << ", x" << rs2 << endl;
			}
			else printf("Invalid R-type Instruction Detected\n");
			break;
		case 4:
			if (!funct7)
			{
				if (!printMode)
					regs[rd] = regs[rs1] ^ regs[rs2];		// xor $rd, $rs1, $rs2;
				cout << "\txor\tx" << rd << ", x" << rs1 << ", x" << rs2 << endl;
			}
			else printf("Invalid R-type Instruction Detected\n");
			break;
		case 5:
			switch (funct7)
			{
				case 0:
					if (!printMode)
						regs[rd] = (regs[rs1] >> regs[rs2]);		//srl $rd, $rs1, $rs2;
					cout << "\tsrl\tx" << rd << ", x" << rs1 << ", " << rs2 << endl;
					break;
				case 32:
					if (!printMode){
						regs[rd] = (regs[rs1] >> regs[rs2]);		// sra $rd, $rs1, $rs2;
						if (regs[rs1] >> 31)
							regs[rd] = regs[rd] | (0xFFFFFFFF << (32 - regs[rs2]));
					}
					cout << "\tsra\tx" << rd << ", x" << rs1 << ", " << rs2 << endl;
					break;
				default: printf("Invalid R-type Instruction Detected\n");
			}
			break;

		case 6:
			if (!funct7)
			{
				if (!printMode)
					regs[rd] = regs[rs1] | regs[rs2];		// or $rd, $rs1, $rs2;
				cout << "\tor\tx" << rd << ", x" << rs1 << ", x" << rs2 << endl;
			}
			else printf("Invalid R-type Instruction Detected\n");
			break;
		case 7:
			if (!funct7)
			{
				if (!printMode)
					regs[rd] = regs[rs1] & regs[rs2];		// and $rd, $rs1, $rs2;
				cout << "\tand\tx" << rd << ", x" << rs1 << ", x" << rs2 << endl;
			}
			else printf("Invalid R-type Instruction Detected\n");
			break;
		default: printf("Invalid R-type Instruction Detected\n");
	}
}

void instDecExec(unsigned int instWord)
{
	unsigned int opcode = instWord & 0x0000007F;

	if (opcode == 0x73)//ecall
	{
		ecall(instWord);
	}
	else if (opcode == 0x63)// sb type instructions
	{
		SB(instWord, opcode);
	}
	else if (opcode == 0x23)//s type instructions
	{
		S(instWord, opcode);
	}
	else if (opcode == 0x13 || opcode == 0x3 || opcode == 0x67)
	{
		I(instWord, opcode);
	}
	else if (opcode == 0x37 || opcode == 0x17 || opcode == 0x6F)
	{
		U_UJ(instWord, opcode);
	}
	else if (opcode == 0x33)
	{
		R(instWord, opcode);
	}
	else cout << "Invalid Instruction with opcode: 0x" <<  hex << opcode
		<< ":\t0x" << hex << std::setfill('0') << std::setw(8) << instWord << endl;
}

void dumpRegs (int s = 0, int e = 31){
	if (s < 0) s = 0;
	if (e > 31) e = 31;
	for (int i = s; i<=e; i++)
		cout << "x" << dec << i << ": \t" << "0x" << hex << std::setfill('0') << std::setw(8) << regs[i] << "\n";
}

void dumpMem (int s = 0, int e = memSize-1){
	if (s < 0) s = 0;
	if (e > memSize-1) e = memSize-1;
	for (int i = s; i<= e; i++)
		cout << "mem[" << dec << i << "] \t" << (unsigned int)memory[i] << endl;
}

int main(int argc, char* argv[]) {
	//use <chrono> to set a timer and detect infinite loops

	unsigned int instWord = 0;
	char command;
	int s, e;

	regs[2] = 1024 * 8 - 1;   // sp

	ifstream inFile;
	ofstream outFile;
	if(argc<1){
		emitError("use: rv32i_sim machine_code_file_name [-no_r_dump] [-mem_dump]\n");
		return -1;
	}

	inFile.open(argv[1], ios::in | ios::binary | ios::ate);
	if (inFile.is_open())
	{
		int fsize = inFile.tellg();

		inFile.seekg(0, inFile.beg);
		if (!inFile.read((char *)memory, fsize)) emitError("Cannot read from input file\n");

		for (int i = 0; i < 2; i++){
			if (printMode) cout << "Disassembly:\n===========\n";
			else	cout << "Simulation:"
						  << " | Commands:\tn: next instruction\t\t\tr <s> <e>: dump the register from index s to e\n"
			  		     <<"\n===========" 
						  << " | m <s> <e>: dump the memory from byte s to byte e\t-Any other character to stop step-by-step execution\n";

			beforeExec = time(NULL);	
			while (true) {
				instWord = (unsigned char)memory[pc] |
					(((unsigned char)memory[pc + 1]) << 8) |
					(((unsigned char)memory[pc + 2]) << 16) |
					(((unsigned char)memory[pc + 3]) << 24);
				pc += 4;
				if (!instWord || stop) break;

				bool session = true;
				while (stepByStep && !printMode && session){
					cout << "\nCommand: ";
					cin >> command;
					switch (command){
						case 'r':
							cin >> s >> e;
							dumpRegs(s,e);
							break;
						case 'm':
							cin >> s >> e;
							dumpMem(s,e);
							break;
						case 'n': 
							session = false;
							break;
						default:
						  	stepByStep = false;
							beforeExec = time(NULL);
					}
				}

				instDecExec(instWord);

				if (!stepByStep){
					if (difftime(time(NULL), beforeExec) > TimeLimit)
						emitError("\nError: Time Limit Exceeded - A Potential Infinite Loop\n\n");
				}
				else
					beforeExec = time(NULL);
			}
			cout << setw(60) << setfill('*') << "" << endl;
			if (!printMode)
				break;
			printMode = false;
			stop = false;
			pc = 0x0;
		}

		// dump the registers
		
		if ((argc<3 || strcmp(argv[2],"-no_r_dump")) && (argc < 4 || strcmp(argv[3], "-no_r_dump")))
			dumpRegs();

		if ((argc>2 && !strcmp(argv[2],"-mem_dump")) || (argc > 3 && !strcmp(argv[3], "-mem_dump")))
			dumpMem();

	}
	else emitError("Cannot access input file\n");
	system("pause");
	return 0;
}
