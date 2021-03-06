#include <iostream>

using namespace std;

typedef unsigned char Byte;
typedef unsigned short Word;

typedef unsigned int u32;



struct Mem {

    static constexpr u32 MAX_MEM = 1024 * 64;
    Byte Data[MAX_MEM];

    void Initialise()
    {
        for (u32 i = 0; i < MAX_MEM; i++)
        {

            Data[i] = 0;
        }
    }
     
    //aceasta functie este folosita pentru a returna un bit din memorie
    ///este practic un operator definit de utilizator

    Byte operator[](u32 Address) const
    {
        return Data[Address];
    }

    ///scrie un bit in memorie
    Byte& operator[](u32 Address)
    {
        return Data[Address];
    }
};

struct CPU {



    ///program counter
    Word PC;
    ///stack pointer -> ne arata unde mai exact suntem in program
    Word SP;

    Byte A, X, Y; ///registrii cp-ului

    Byte C : 1; ///flag de status
    Byte Z : 1; ///flag de status si restul sun flaguri de status
    Byte I : 1;
    Byte D : 1;
    Byte B : 1;
    Byte V : 1;
    Byte N : 1;

    ///codul de aici este folosit pentru a reseta procesorul
    ///si practic ce facem este ca ii atribuim niste adrese de memorie exact
    ///asa cum face si un procesor adevarat!
    void Reset(Mem& memory)
    {

        PC = 0xFFFC;
        SP = 0x00FFF;
        C = Z = I = D = B = V = N = 0;
        A = X = Y = 0;
        memory.Initialise();
    }

    Byte FetchByte(u32 Cycles, Mem& memory)
    {

        Byte Data = memory[PC];
        PC++;
        Cycles--;
        return Data;

    }

    Word FetchWord(u32& Cycles, Mem& memory)
    {
        ///6502 is little endian
        Word Data = memory[PC];
        PC++;
        Cycles--;

        Data |= (memory[PC] << 8);
        PC++;
        Cycles-=2;


        return Data;
    }

    Byte ReadByte(u32& Cycles,Byte Address, Mem& memory)
    {
        Byte Data = memory[Address];
        Cycles--;
        return Data;

    }

    ///take a look again at INS_JSR because it need to be reviewed a little bit
    static constexpr Byte INS_LDA_IM = 0xA9,
        INS_LDA_ZP = 0xA5,
        INS_LDA_ZPX = 0xB5,
        INS_JSR = 0x32;



    void LDASetStatus()
    {
        Z = (A == 0);
        N = (A & 0b10000000) > 0;

    }

    void Execute(u32 Cycles, Mem& memory)
    {
        while (Cycles > 0)
        {
            Byte Ins = FetchByte(Cycles, memory);
            
            switch (Ins)
            {

            case INS_LDA_IM:
            {
                Byte Value = FetchByte(Cycles,memory);
                A = Value;
                Z = (A == 0);
                N = (A & 0b10000000) > 0;
                LDASetStatus();
            } break;
            
            case INS_LDA_ZP:
            {
                Byte ZeroPageAddress = FetchByte(Cycles, memory);
                A = ReadByte(Cycles,ZeroPageAddress, memory);
                Z = (A == 0);
                N = (A & 0b10000000) > 0;
                LDASetStatus();
            } break;

            case INS_LDA_ZPX:
            {
                Byte ZeroPageAddr = FetchByte(Cycles, memory);
                ZeroPageAddr += X;
                Cycles--;
                A = ReadByte(Cycles, ZeroPageAddr, memory);
                LDASetStatus();
            }break;

            case INS_JSR:
            {
                Word SubAddr = FetchWord(Cycles, memory);
                memory[SP] = PC-1 ;
                Cycles--;
                PC = SubAddr; 
                Cycles--;
            } break;

            default: 
            {
                printf("Instruction not handled %d", Ins);
            } break;

            }
        }

    }

     
 
};

int main()
{
    ///ne-am creeat procesorul , bine doar ca data structre
    Mem mem;
    CPU cpu;
    cpu.Reset(mem);
    mem[0xFFFC] = CPU::INS_LDA_ZP;
    mem[0xFFFD] = 0x42;
    mem[0x0042] = 0x84;
    cpu.Execute(3, mem);
    return 0;
}
