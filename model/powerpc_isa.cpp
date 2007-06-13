/**
 * @file      powerpc_isa.cpp
 * @author    Bruno Corsi dos Santos
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:50:48 -0300
 * 
 * @brief     The ArchC POWERPC functional model.
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

//IMPLEMENTATION NOTES:
// PowerPC 32 bits family.
// The PowerPC 405 instruction family are not implemented.
// Based on IBM and Xilinx manuals of PowerPC 405.
// mtspr and mfspr instructions not completely implemented.
// sc instruction not completely implemented and never used.

#include  "powerpc_isa.H"
#include  "powerpc_isa_init.cpp"
#include  "powerpc_bhv_macros.H"

//If you want debug information for this model, uncomment next line
//#define DEBUG_MODEL
#include  "ac_debug_model.H"

using namespace powerpc_parms;

//Compute CR0 fields LT, GT, EQ, SO
//XER.SO must be updated by instruction before the use of this routine!
//Arguments:
//int result -> The result register
inline void CR0_update(ac_reg<ac_word> &CR, ac_reg<ac_word> &XER, unsigned int result) {

    /* LT field */
    if((result & 0x80000000) >> 31)
        CR.write(CR.read() | 0x80000000); /* 1 */
    else
        CR.write(CR.read() & 0x7FFFFFFF); /* 0 */

    /* GT field */
    if(((~result & 0x80000000) >> 31) && (result!=0))
        CR.write(CR.read() | 0x40000000); /* 1 */
    else
        CR.write(CR.read() & 0xBFFFFFFF); /* 0 */

    /* EQ field */
    if(result==0)
        CR.write(CR.read() | 0x20000000); /* 1 */
    else
        CR.write(CR.read() & 0xDFFFFFFF); /* 0 */

    /* SO field */
    if(XER.read() & 0x80000000)
        CR.write(CR.read() | 0x10000000); /* 1 */
    else
        CR.write(CR.read() & 0xEFFFFFFF); /* 0 */
}


//Compute XER overflow fields SO, OV
//Arguments:
//int result -> The result register
//int s1 -> Source 1
//int s2 -> Source 2
//int s3 -> Source 3 (if only two sources, use 0)
inline void add_XER_OV_SO_update(ac_reg<ac_word> &XER, int result,int s1,int s2,int s3) {

    long long int longresult =
        (long long int)(int)s1 + (long long int)(int)s2 + (long long int)(int)s3;

    if(longresult != (long long int)(int)result) {
        XER.write(XER.read() | 0x40000000); /* Write 1 to bit 1 OV */
        XER.write(XER.read() | 0x80000000); /* Write 1 to bit 0 SO */
    }
    else {
        XER.write(XER.read() & 0xBFFFFFFF); /* Write 0 to bit 1 OV */
    }
}


//Compute XER carry field CA
//Arguments:
//int result -> The result register
//int s1 -> Source 1
//int s2 -> Source 2
//int s3 -> Source 3 (if only two sources, use 0)
inline void add_XER_CA_update(ac_reg<ac_word> &XER, int result,int s1,int s2,int s3) {

    unsigned long long int longresult =
        (unsigned long long int)(unsigned int)s1 + 
        (unsigned long long int)(unsigned int)s2 + 
        (unsigned long long int)(unsigned int)s3;

    if(longresult > 0xFFFFFFFF)
        XER.write(XER.read() | 0x20000000); /* Write 1 to bit 2 CA */
    else 
        XER.write(XER.read() & 0xDFFFFFFF); /* Write 0 to bit 2 CA */

} 

//Compute XER overflow fields SO, OV
//Arguments:
//int result -> The result register
//int s1 -> Source 1
//int s2 -> Source 2
inline void divws_XER_OV_SO_update(ac_reg<ac_word> &XER, int result,int s1,int s2) {

    long long int longresult =
        (long long int)(int)s1 / (long long int)(int)s2;

    if(longresult != (long long int)(int)result) {
        XER.write(XER.read() | 0x40000000); /* Write 1 to bit 1 OV */
        XER.write(XER.read() | 0x80000000); /* Write 1 to bit 0 SO */
    }
    else
        XER.write(XER.read() & 0xBFFFFFFF); /* Write 0 to bit 1 OV */    

}


//Compute XER overflow fields SO, OV
//Arguments:
//int result -> The result register
//int s1 -> Source 1
//int s2 -> Source 2
inline void divwu_XER_OV_SO_update(ac_reg<ac_word> &XER, int result,int s1,int s2) {

    unsigned long long int longresult =
        (unsigned long long int)(unsigned int)s1 / 
        (unsigned long long int)(unsigned int)s2;

    if(longresult != (unsigned long long int)(unsigned int)result) {
        XER.write(XER.read() | 0x40000000); /* Write 1 to bit 1 OV */
        XER.write(XER.read() | 0x80000000); /* Write 1 to bit 0 SO */
    }
    else
        XER.write(XER.read() & 0xBFFFFFFF); /* Write 0 to bit 1 OV */

}


//Function to do_branch
inline void do_Branch(ac_reg<ac_word> &ac_pc, ac_reg<ac_word> &LR, signed int ili,unsigned int iaa,unsigned int ilk) {

    int displacement;
    unsigned int nia;

    ac_pc-=4; /* Because pre-increment */

    if(iaa==1) {
        displacement=ili<<2;
        nia=displacement;
    }
    else { /* iaa=0 */
        displacement=ili<<2;
        nia=ac_pc+displacement;
    }

    if(ilk==1)
        LR.write(ac_pc+4);

    ac_pc=nia;

}

//Function to do conditional branch
inline void do_Branch_Cond(ac_reg<ac_word> &ac_pc, ac_reg<ac_word> &LR, ac_reg<ac_word> &CR, ac_reg<ac_word> &CTR,  unsigned int ibo,unsigned int ibi,
        signed int ibd,unsigned int iaa,
        unsigned int ilk) {

    int displacement;
    unsigned int nia;
    unsigned int masc;

    masc=0x80000000;
    masc=masc>>ibi;

    ac_pc-=4; /* Because pre-increment */

    if((ibo & 0x04) == 0x00) {
        CTR.write(CTR.read()-1);
    }

    if(((ibo & 0x04) || /* Branch */
                ((CTR.read()==0) && (ibo & 0x02)) || 
                (!(CTR.read()==0) && !(ibo & 0x02)))
            && 
            ((ibo & 0x10) ||
             (((CR.read() & masc) && (ibo & 0x08)) ||
              (!(CR.read() & masc) && !(ibo & 0x08))))) {

        if(iaa == 1) {
            displacement=ibd<<2;
            nia=displacement;
        }
        else {
            displacement=ibd<<2;
            nia=ac_pc+displacement;
        }
    }
    else { /* No branch */
        nia=ac_pc+4;
    }

    if(ilk==1)
        LR.write(ac_pc+4);

    ac_pc=nia;

}

//Function to do conditional branch to count register
inline void do_Branch_Cond_Count_Reg(ac_reg<ac_word> &ac_pc, ac_reg<ac_word> &LR, ac_reg<ac_word> &CR, ac_reg<ac_word> &CTR, unsigned int ibo, unsigned int ibi,
        unsigned int ilk) {

    unsigned int nia;
    unsigned int masc;

    masc=0x80000000;
    masc=masc>>ibi;

    ac_pc-=4; /* Because pre-increment */

    if((ibo & 0x04) == 0x00)
        CTR.write(CTR.read()-1);

    if(((ibo & 0x04) || /* Branch */
                ((CTR.read()==0) && (ibo & 0x02)) || 
                (!(CTR.read()==0) && !(ibo & 0x02)))
            && 
            ((ibo & 0x10) ||
             (((CR.read() & masc) && (ibo & 0x08)) ||
              (!(CR.read() & masc) && !(ibo & 0x08))))) {

        nia=CTR.read() & 0xFFFFFFFC;

    }
    else { /* No Branch */
        nia=ac_pc+4;
    }

    if(ilk==1)
        LR.write(ac_pc+4);

    ac_pc=nia;

}

//Function to do conditional branch to link register
inline void do_Branch_Cond_Link_Reg(ac_reg<ac_word> &ac_pc, ac_reg<ac_word> &LR, ac_reg<ac_word> &CR, ac_reg<ac_word> &CTR,unsigned int ibo,unsigned int ibi,
        unsigned int ilk) {

    unsigned int nia;
    unsigned int masc;

    masc=0x80000000;
    masc=masc>>ibi;

    ac_pc-=4; /* Because pre-increment */

    if((ibo & 0x04) == 0x00)
        CTR.write(CTR.read()-1);

    if(((ibo & 0x04) || /* Branch */
                ((CTR.read()==0) && (ibo & 0x02)) || 
                (!(CTR.read()==0) && !(ibo & 0x02)))
            && 
            ((ibo & 0x10) ||
             (((CR.read() & masc) && (ibo & 0x08)) ||
              (!(CR.read() & masc) && !(ibo & 0x08))))) {

        nia=LR.read() & 0xFFFFFFFC;

    }
    else { /* No Branch */
        nia=ac_pc+4;
    }

    if(ilk==1)
        LR.write(ac_pc+4);

    ac_pc=nia;

}


//Ceil function
inline int ceil(int value, int divisor) {
    int res;
    if ((value % divisor)!=0)
        res=int(value/divisor)+1;
    else res=value/divisor;
    return res;
}

//Rotl function
inline unsigned int rotl(unsigned int reg,unsigned int n) {
    unsigned int tmp1=reg;
    unsigned int tmp2=reg;
    unsigned int rotated=(tmp1 << n) | (tmp2 >> 32-n);

    return(rotated);
}

//Mask32rlw function
inline unsigned int mask32rlw(unsigned int i,unsigned int f) {

    unsigned int maski,maskf;

    if(i<=f) {
        maski=(0xFFFFFFFF>>i);
        maskf=(0xFFFFFFFF<<(31-f));
        return(maski & maskf);
    }
    else {
        maski=(0xFFFFFFFF>>f);
        maski=maski>>1;
        maskf=(0xFFFFFFFF<<(31-i));
        maskf=maskf<<1;
        return(~(maski & maskf));
    }

}


//Function dump General Purpose Registers
inline void dumpGPR(ac_regbank<32, ac_word, ac_Dword> &GPR) {
    int i;
    for(i=0 ; i<32 ; i++)
        dbg_printf("r%d -> %#x \n",i,GPR.read(i));
}

//Function that returns the value of XER TBC
inline unsigned int XER_TBC_read(ac_reg<ac_word> &XER) {
    return(XER.read() & 0x0000007F);
}


//Function that returns the value of XER OV
    inline unsigned int XER_OV_read(ac_reg<ac_word> &XER) {
        if(XER.read() & 0x40000000)
            return 1;
        else
            return 0;
    }

//Function that returns the value of XER SO
    inline unsigned int XER_SO_read(ac_reg<ac_word> &XER) {
        if(XER.read() & 0x80000000)
            return 1;
        else
            return 0;
    }

//Function that returns the value of XER CA
    inline unsigned int XER_CA_read(ac_reg<ac_word> &XER) {
        if(XER.read() & 0x20000000)
            return 1;
        else
            return 0;
    }


//Function dump various registers
inline void dumpREG(ac_reg<ac_word> &XER, ac_reg<ac_word> &CR, ac_reg<ac_word> &LR, ac_reg<ac_word> &CTR) {
    dbg_printf("XER.OV = %d\n",XER_OV_read(XER));
    dbg_printf("XER.SO = %d\n",XER_SO_read(XER));
    dbg_printf("XER.CA = %d\n",XER_CA_read(XER));
    dbg_printf("CR = %#x\n",CR.read());
    dbg_printf("LR = %#x\n",LR.read());
    dbg_printf("CTR = %#x\n",CTR.read());
}



//!Generic instruction behavior method.
void ac_behavior( instruction )
{
    dbg_printf("\n program counter=%#x\n",(int)ac_pc);
    ac_pc+=4;
    //dumpGPR();
    //dumpREG();
}

//!Generic begin behavior method.
void ac_behavior( begin )
{
    dbg_printf("Starting simulator...\n");

    /* Here the stack is started in a */
    GPR.write(1,AC_RAM_END - 1024);

    /* Make a jump out of memory if it doesn't have an abi */
    LR.write(0xFFFFFFFF);

}

void ac_behavior(end)
{
    dbg_printf("@@@ end behavior @@@\n");
}

//! Instruction Format behavior methods.
void ac_behavior( I1 ){}
void ac_behavior( B1 ){}
void ac_behavior( SC1 ){}
void ac_behavior( D1 ){}
void ac_behavior( D2 ){}
void ac_behavior( D3 ){}
void ac_behavior( D4 ){}
void ac_behavior( D5 ){}
void ac_behavior( D6 ){}
void ac_behavior( D7 ){}
void ac_behavior( X1 ){}
void ac_behavior( X2 ){}
void ac_behavior( X3 ){}
void ac_behavior( X4 ){}
void ac_behavior( X5 ){}
void ac_behavior( X6 ){}
void ac_behavior( X7 ){}
void ac_behavior( X8 ){}
void ac_behavior( X9 ){}
void ac_behavior( X10 ){}
void ac_behavior( X11 ){}
void ac_behavior( X12 ){}
void ac_behavior( X13 ){}
void ac_behavior( X14 ){}
void ac_behavior( X15 ){}
void ac_behavior( X16 ){}
void ac_behavior( X17 ){}
void ac_behavior( X18 ){}
void ac_behavior( X19 ){}
void ac_behavior( X20 ){}
void ac_behavior( X21 ){}
void ac_behavior( X22 ){}
void ac_behavior( X23 ){}
void ac_behavior( X24 ){}
void ac_behavior( X25 ){}
void ac_behavior( XL1 ){}
void ac_behavior( XL2 ){}
void ac_behavior( XL3 ){}
void ac_behavior( XL4 ){}
void ac_behavior( XFX1 ){}
void ac_behavior( XFX2 ){}
void ac_behavior( XFX3 ){}
void ac_behavior( XFX4 ){}
void ac_behavior( XFX5 ){}
void ac_behavior( XO1 ){}
void ac_behavior( XO2 ){}
void ac_behavior( XO3 ){}
void ac_behavior( M1 ){}
void ac_behavior( M2 ){}


//!Instruction add behavior method.
void ac_behavior( add )
{
    dbg_printf(" add r%d, r%d, r%d\n\n",rt,ra,rb);
    GPR.write(rt,GPR.read(ra) + GPR.read(rb));

};

//!Instruction add_ behavior method.
void ac_behavior( add_ )
{
    dbg_printf(" add. r%d, r%d, r%d\n\n",rt,ra,rb);
    int result=GPR.read(ra) + GPR.read(rb);

    CR0_update(CR, XER, result);
    GPR.write(rt,result);

};

//!Instruction addo behavior method.
void ac_behavior( addo )
{
    dbg_printf(" addo r%d, r%d, r%d\n\n",rt,ra,rb);
    int result=GPR.read(ra) + GPR.read(rb);

    add_XER_OV_SO_update(XER, result,GPR.read(ra),GPR.read(rb),0);

    GPR.write(rt,result);

};

//!Instruction addo_ behavior method.
void ac_behavior( addo_ )
{
    dbg_printf(" addo. r%d, r%d, r%d\n\n",rt,ra,rb);
    int result=GPR.read(ra) + GPR.read(rb);

    /* Note: XER_OV_SO_update before CR0_update */
    add_XER_OV_SO_update(XER, result,GPR.read(ra),GPR.read(rb),0);
    CR0_update(CR, XER, result);
    GPR.write(rt,result);

};

//!Instruction addc behavior method.
void ac_behavior( addc )
{
    dbg_printf(" addc r%d, r%d, r%d\n\n",rt,ra,rb);
    int result=GPR.read(ra) + GPR.read(rb);

    add_XER_CA_update(XER, result,GPR.read(ra),GPR.read(rb),0);
    GPR.write(rt,result);

};

//!Instruction addc_ behavior method.
void ac_behavior( addc_ )
{
    dbg_printf(" addc. r%d, r%d, r%d\n\n",rt,ra,rb);
    int result=GPR.read(ra) + GPR.read(rb);

    add_XER_CA_update(XER, result,GPR.read(ra),GPR.read(rb),0);

    CR0_update(CR, XER, result);

};

//!Instruction addco behavior method.
void ac_behavior( addco )
{
    dbg_printf(" addco r%d, r%d, r%d\n\n",rt,ra,rb);
    int result=GPR.read(ra) + GPR.read(rb);

    add_XER_CA_update(XER, result,GPR.read(ra),GPR.read(rb),0);

    add_XER_OV_SO_update(XER, result,GPR.read(ra),GPR.read(rb),0);

    GPR.write(rt,result);

};

//!Instruction addco_ behavior method.
void ac_behavior( addco_ )
{
    dbg_printf(" addco. r%d, r%d, r%d\n\n",rt,ra,rb);
    int result=GPR.read(ra) + GPR.read(rb);

    add_XER_CA_update(XER, result,GPR.read(ra),GPR.read(rb),0);

    /* Note: XER_OV_SO_update before CR0_update */
    add_XER_OV_SO_update(XER, result,GPR.read(ra),GPR.read(rb),0);
    CR0_update(CR, XER, result);

    GPR.write(rt,result);

};

//!Instruction adde behavior method.
void ac_behavior( adde )
{
    dbg_printf(" adde r%d, r%d, r%d\n\n",rt,ra,rb);

    int result=GPR.read(ra) + GPR.read(rb) + XER_CA_read(XER);

    add_XER_CA_update(XER, result,GPR.read(ra),GPR.read(rb),XER_CA_read(XER));

    GPR.write(rt,result);

};

//!Instruction adde_ behavior method.
void ac_behavior( adde_ )
{
    dbg_printf(" adde. r%d, r%d, r%d\n\n",rt,ra,rb);

    int result=GPR.read(ra) + GPR.read(rb) + XER_CA_read(XER);

    add_XER_CA_update(XER, result,GPR.read(ra),GPR.read(rb),XER_CA_read(XER));

    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction addeo behavior method.
void ac_behavior( addeo )
{
    dbg_printf(" addeo r%d, r%d, r%d\n\n",rt,ra,rb);

    int result=GPR.read(ra) + GPR.read(rb) + XER_CA_read(XER);

    add_XER_CA_update(XER, result,GPR.read(ra),GPR.read(rb),XER_CA_read(XER));

    add_XER_OV_SO_update(XER, result,GPR.read(ra),GPR.read(rb),XER_CA_read(XER));

    GPR.write(rt,result);
};

//!Instruction addeo_ behavior method.
void ac_behavior( addeo_ )
{
    dbg_printf(" addeo. r%d, r%d, r%d\n\n",rt,ra,rb);

    int result=GPR.read(ra) + GPR.read(rb) + XER_CA_read(XER);

    add_XER_CA_update(XER, GPR.read(rt),GPR.read(ra),GPR.read(rb),XER_CA_read(XER));

    /* Note: XER_OV_SO_update before CR0_update */
    add_XER_OV_SO_update(XER, result,GPR.read(ra),GPR.read(rb),XER_CA_read(XER));
    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction addi behavior method.
void ac_behavior( addi )
{
    dbg_printf(" addi r%d, r%d, %d\n\n",rt,ra,d);

    int ime32=d;

    if(ra == 0)
        GPR.write(rt,ime32);
    else
        GPR.write(rt,GPR.read(ra)+ime32);

};

//!Instruction addic behavior method.
void ac_behavior( addic )
{
    dbg_printf(" addic r%d, r%d, %d\n\n",rt,ra,d);
    int ime32=d;
    int result=GPR.read(ra)+ime32;

    add_XER_CA_update(XER, result,GPR.read(ra),ime32,0);

    GPR.write(rt,result);
};

//!Instruction addic_ behavior method.
void ac_behavior( addic_ )
{
    dbg_printf(" addic. r%d, r%d, %d\n\n",rt,ra,d);
    /* Do not have rc field and update CR0 */
    int ime32=d;
    int result=GPR.read(ra)+ime32;

    add_XER_CA_update(XER, result,GPR.read(ra),ime32,0);

    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction addis behavior method.
void ac_behavior( addis )
{
    dbg_printf(" addis r%d, r%d, %d\n\n",rt,ra,d);
    int ime32=d;
    ime32=ime32<<16;

    if(ra == 0) 
        GPR.write(rt,ime32);
    else
        GPR.write(rt,GPR.read(ra)+ime32);

};

//!Instruction addme behavior method.
void ac_behavior( addme )
{
    dbg_printf(" addme r%d, r%d\n\n",rt,ra);
    int result=GPR.read(ra)+XER_CA_read(XER)+(-1);

    add_XER_CA_update(XER, result,GPR.read(ra),XER_CA_read(XER),-1);

    GPR.write(rt,result);
};

//!Instruction addme_ behavior method.
void ac_behavior( addme_ )
{
    dbg_printf(" addme. r%d, r%d\n\n",rt,ra);
    int result=GPR.read(ra)+XER_CA_read(XER)+(-1);

    add_XER_CA_update(XER, result,GPR.read(ra),XER_CA_read(XER),-1);

    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction addmeo behavior method.
void ac_behavior( addmeo )
{
    dbg_printf(" addmeo r%d, r%d\n\n",rt,ra);
    int result=GPR.read(ra)+XER_CA_read(XER)+(-1);

    add_XER_CA_update(XER, result,GPR.read(ra),XER_CA_read(XER),-1);

    add_XER_OV_SO_update(XER, result,GPR.read(ra),XER_CA_read(XER),-1);

    GPR.write(rt,result);
};

//!Instruction addmeo_ behavior method.
void ac_behavior( addmeo_ )
{
    dbg_printf(" addmeo. r%d, r%d\n\n",rt,ra);
    int result=GPR.read(ra)+XER_CA_read(XER)+(-1);

    add_XER_CA_update(XER, result,GPR.read(ra),XER_CA_read(XER),-1);

    /* Note: XER_OV_SO_update before CR0_update */
    add_XER_OV_SO_update(XER, result,GPR.read(ra),XER_CA_read(XER),-1);
    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction addze behavior method.
void ac_behavior( addze )
{
    dbg_printf(" addze r%d, r%d\n\n",rt,ra);
    int result=GPR.read(ra)+XER_CA_read(XER);

    add_XER_CA_update(XER, result,GPR.read(ra),XER_CA_read(XER),0);

    GPR.write(rt,result);
};

//!Instruction addze_ behavior method.
void ac_behavior( addze_ )
{
    dbg_printf(" addze. %d, %d\n\n",rt,ra);
    int result=GPR.read(ra)+XER_CA_read(XER);

    add_XER_CA_update(XER, result,GPR.read(ra),XER_CA_read(XER),0);

    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction addzeo behavior method.
void ac_behavior( addzeo )
{
    dbg_printf(" addzeo r%d, r%d\n\n",rt,ra);
    int result=GPR.read(ra)+XER_CA_read(XER);

    add_XER_CA_update(XER, result,GPR.read(ra),XER_CA_read(XER),0);

    add_XER_OV_SO_update(XER, result,GPR.read(ra),XER_CA_read(XER),0);  

    GPR.write(rt,result);
};

//!Instruction addzeo_ behavior method.
void ac_behavior( addzeo_ )
{
    dbg_printf(" addzeo. r%d, r%d\n\n",rt,ra);
    int result=GPR.read(ra)+XER_CA_read(XER);

    add_XER_CA_update(XER, result,GPR.read(ra),XER_CA_read(XER),0);

    /* Note: XER_OV_SO_update before CR0_update */
    add_XER_OV_SO_update(XER, result,GPR.read(ra),XER_CA_read(XER),0);  
    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction ande behavior method.
void ac_behavior( ande )
{
    dbg_printf(" and r%d, r%d, r%d\n\n",ra,rs,rb);
    GPR.write(ra,GPR.read(rs) & GPR.read(rb));

};

//!Instruction ande_ behavior method.
void ac_behavior( ande_ )
{
    dbg_printf(" and. r%d, r%d, r%d\n\n",ra,rs,rb);
    int result=GPR.read(rs) & GPR.read(rb);

    CR0_update(CR, XER, result);

    GPR.write(ra,result);
};

//!Instruction andc behavior method.
void ac_behavior( andc )
{
    dbg_printf(" andc r%d, r%d, r%d\n\n",ra,rs,rb);
    GPR.write(ra,GPR.read(rs) & ~GPR.read(rb));

};

//!Instruction ande_ behavior method.
void ac_behavior( andc_ )
{
    dbg_printf(" andc. r%d, r%d, r%d\n\n",ra,rs,rb);
    int result=GPR.read(rs) & ~GPR.read(rb);

    CR0_update(CR, XER, result);

    GPR.write(ra,result);
};

//!Instruction andi_ behavior method.
void ac_behavior( andi_ )
{
    dbg_printf(" andi. r%d, r%d, %d\n\n",ra,rs,ui);
    unsigned int ime32=(unsigned short int)ui;
    int result=GPR.read(rs) & ime32;

    CR0_update(CR, XER, result);

    GPR.write(ra,result);
};

//!Instruction andis_ behavior method.
void ac_behavior( andis_ )
{
    dbg_printf(" andis. r%d, r%d, %d\n\n",ra,rs,ui);
    unsigned int ime32=(unsigned short int)ui;

    ime32=ime32<<16;
    int result=GPR.read(rs) & ime32;

    CR0_update(CR, XER, result);

    GPR.write(ra,result);
};

//!Instruction b behavior method.
void ac_behavior( b )
{
    dbg_printf(" b %d\n\n",li);
    do_Branch(ac_pc, LR, li,aa,lk);

};

//!Instruction ba behavior method.
void ac_behavior( ba )
{
    dbg_printf(" ba %d\n\n",li);
    do_Branch(ac_pc, LR, li,aa,lk);

};

//!Instruction bl behavior method.
void ac_behavior( bl )
{
    dbg_printf(" bl %d\n\n",li);
    do_Branch(ac_pc, LR, li,aa,lk);

};

//!Instruction bla behavior method.
void ac_behavior( bla )
{
    dbg_printf(" bla %d\n\n",li);
    do_Branch(ac_pc, LR, li,aa,lk);

};

//!Instruction bc behavior method.
void ac_behavior( bc )
{
    dbg_printf(" bc %d, %d, %d\n\n",bo,bi,bd);
    do_Branch_Cond(ac_pc, LR, CR, CTR, bo,bi,bd,aa,lk);

};

//!Instruction bca behavior method.
void ac_behavior( bca )
{
    dbg_printf(" bca %d, %d, %d\n\n",bo,bi,bd);
    do_Branch_Cond(ac_pc, LR, CR, CTR, bo,bi,bd,aa,lk);

};

//!Instruction bcl behavior method.
void ac_behavior( bcl )
{
    dbg_printf(" bcl %d, %d, %d\n\n",bo,bi,bd);
    do_Branch_Cond(ac_pc, LR, CR, CTR, bo,bi,bd,aa,lk);

};

//!Instruction bcla behavior method.
void ac_behavior( bcla )
{
    dbg_printf(" bcla %d, %d, %d\n\n",bo,bi,bd);
    do_Branch_Cond(ac_pc, LR, CR, CTR, bo,bi,bd,aa,lk);

};

//!Instruction bcctr behavior method.
void ac_behavior( bcctr )
{
    dbg_printf(" bcctr %d, %d\n\n",bo,bi);
    do_Branch_Cond_Count_Reg(ac_pc, LR, CR, CTR,bo,bi,lk);

};

//!Instruction bcctrl behavior method.
void ac_behavior( bcctrl )
{
    dbg_printf(" bcctrl %d, %d\n\n",bo,bi);
    do_Branch_Cond_Count_Reg(ac_pc, LR, CR, CTR,bo,bi,lk);

};

//!Instruction bclr behavior method.
void ac_behavior( bclr )
{
    dbg_printf(" bclr %d, %d\n\n",bo,bi);
    do_Branch_Cond_Link_Reg(ac_pc, LR, CR, CTR,bo,bi,lk);

};

//!Instruction bclrl behavior method.
void ac_behavior( bclrl )
{
    dbg_printf(" bclrl %d, %d\n\n",bo,bi);
    do_Branch_Cond_Link_Reg(ac_pc, LR, CR, CTR,bo,bi,lk);

};

//!Instruction cmp behavior method.
void ac_behavior( cmp )
{
    dbg_printf(" cmp crf%d, 0, r%d, r%d\n\n",bf,ra,rb);
    unsigned int c=0x00;
    unsigned int n=bf;
    unsigned int masc=0xF0000000;

    if((int)GPR.read(ra) < (int)GPR.read(rb)) 
        c = c | 0x80000000;
    if((int)GPR.read(ra) > (int)GPR.read(rb))
        c = c | 0x40000000;
    if((int)GPR.read(ra) == (int)GPR.read(rb))
        c = c | 0x20000000;
    if(XER_SO_read(XER)==1)
        c = c | 0x10000000;

    c = c >> (n*4);
    masc = ~(masc >> (n*4));

    CR.write((CR.read() & masc) | c);

};

//!Instruction cmpi behavior method.
void ac_behavior( cmpi )
{
    dbg_printf(" cmpi crf%d, 0, r%d, %d\n\n",bf,ra,si);
    unsigned int c=0x00;
    unsigned int n=bf;
    unsigned int masc=0xF0000000;

    int ime32=(short int)(si);

    if((int)GPR.read(ra) < ime32) 
        c = c | 0x80000000;
    if((int)GPR.read(ra) > ime32)
        c = c | 0x40000000;
    if((int)GPR.read(ra) == ime32)
        c = c | 0x20000000;
    if(XER_SO_read(XER)==1)
        c = c | 0x10000000;

    c = c >> (n*4);
    masc = ~(masc >> (n*4));

    CR.write((CR.read() & masc) | c);

};

//!Instruction cmpl behavior method.
void ac_behavior( cmpl )
{
    dbg_printf(" cmpl crf%d, 0, r%d, r%d\n\n",bf,ra,rb);
    unsigned int c=0x00;
    unsigned int n=bf;
    unsigned int masc=0xF0000000;

    unsigned int uintra=GPR.read(ra);
    unsigned int uintrb=GPR.read(rb);

    if(uintra < uintrb) 
        c = c | 0x80000000;
    if(uintra > uintrb)
        c = c | 0x40000000;
    if(uintra == uintrb)
        c = c | 0x20000000;
    if(XER_SO_read(XER)==1)
        c = c | 0x10000000;

    c = c >> (n*4);
    masc = ~(masc >> (n*4));

    CR.write((CR.read() & masc) | c);

};

//!Instruction cmpli behavior method.
void ac_behavior( cmpli )
{
    dbg_printf(" cmpli crf%d, 0, r%d, %d\n\n",bf,ra,ui);
    unsigned int c=0x00;
    unsigned int n=bf;
    unsigned int masc=0xF0000000;

    unsigned int ime32=(unsigned short int)(ui);

    if(GPR.read(ra) < ime32) 
        c = c | 0x80000000;
    if(GPR.read(ra) > ime32)
        c = c | 0x40000000;
    if(GPR.read(ra) == ime32)
        c = c | 0x20000000;
    if(XER_SO_read(XER)==1)
        c = c | 0x10000000;

    c = c >> (n*4);
    masc = ~(masc >> (n*4));

    CR.write((CR.read() & masc) | c);

};

//!Instruction cntlzw behavior method.
void ac_behavior( cntlzw )
{
    dbg_printf(" cntlzw r%d, r%d\n\n",ra,rs);

    unsigned int urs=GPR.read(rs);
    unsigned int masc=0x80000000;
    unsigned int n=0;

    while(n < 32) {
        if(urs & masc)
            break;
        n++;
        masc=masc>>1;
    }

    GPR.write(ra,n);

};

//!Instruction cntlzw_ behavior method.
void ac_behavior( cntlzw_ )
{
    dbg_printf(" cntlzw. r%d, r%d\n\n",ra,rs);

    unsigned int urs=GPR.read(rs);
    unsigned int masc=0x80000000;
    unsigned int n=0;

    while(n < 32) {
        if(urs & masc)
            break;
        n++;
        masc=masc>>1;
    }

    GPR.write(ra,n);
    CR0_update(CR, XER, n); 

};

//!Instruction crand behavior method.
void ac_behavior( crand )
{
    dbg_printf(" crand %d, %d, %d\n\n",bt,ba,bb);

    unsigned int CRbt;
    unsigned int CRba;
    unsigned int CRbb;

    CRba=CR.read();
    CRbb=CR.read();

    /* Shift source bit to first position and zero another */
    CRba=(CRba<<ba)&0x80000000;
    CRbb=(CRbb<<bb)&0x80000000;

    CRbt=(CRba & CRbb) & 0x80000000;
    CRbt=CRbt>>bt;

    if(CRbt)
        CR.write(CR.read() | CRbt);
    else
        CR.write(CR.read() & ~(0x80000000 >> bt));    

};

//!Instruction crandc behavior method.
void ac_behavior( crandc )
{
    dbg_printf(" crandc %d, %d, %d\n\n",bt,ba,bb);

    unsigned int CRbt;
    unsigned int CRba;
    unsigned int CRbb;

    CRba=CR.read();
    CRbb=CR.read();

    /* Shift source bit to first position and zero another */
    CRba=(CRba<<ba)&0x80000000;
    CRbb=(CRbb<<bb)&0x80000000;

    CRbt=(CRba & ~CRbb) & 0x80000000;
    CRbt=CRbt>>bt;

    if(CRbt)
        CR.write(CR.read() | CRbt);
    else
        CR.write(CR.read() & ~(0x80000000 >> bt));    

};

//!Instruction creqv behavior method.
void ac_behavior( creqv )
{
    dbg_printf(" creqv %d, %d, %d\n\n",bt,ba,bb);

    unsigned int CRbt;
    unsigned int CRba;
    unsigned int CRbb;

    CRba=CR.read();
    CRbb=CR.read();

    /* Shift source bit to first position and zero another */
    CRba=(CRba<<ba)&0x80000000;
    CRbb=(CRbb<<bb)&0x80000000;

    CRbt=~(CRba ^ CRbb) & 0x80000000;
    CRbt=CRbt>>bt;

    if(CRbt)
        CR.write(CR.read() | CRbt);
    else
        CR.write(CR.read() & ~(0x80000000 >> bt));    

};

//!Instruction crnand behavior method.
void ac_behavior( crnand )
{
    dbg_printf(" crnand %d, %d, %d\n\n",bt,ba,bb);

    unsigned int CRbt;
    unsigned int CRba;
    unsigned int CRbb;

    CRba=CR.read();
    CRbb=CR.read();

    /* Shift source bit to first position and zero another */
    CRba=(CRba<<ba)&0x80000000;
    CRbb=(CRbb<<bb)&0x80000000;

    CRbt=~(CRba & CRbb) & 0x80000000;
    CRbt=CRbt>>bt;

    if(CRbt)
        CR.write(CR.read() | CRbt);
    else
        CR.write(CR.read() & ~(0x80000000 >> bt));    

};

//!Instruction crnor behavior method.
void ac_behavior( crnor )
{
    dbg_printf(" crnor %d, %d, %d\n\n",bt,ba,bb);

    unsigned int CRbt;
    unsigned int CRba;
    unsigned int CRbb;

    CRba=CR.read();
    CRbb=CR.read();

    /* Shift source bit to first position and zero another */
    CRba=(CRba<<ba)&0x80000000;
    CRbb=(CRbb<<bb)&0x80000000;

    CRbt=~(CRba | CRbb) & 0x80000000;
    CRbt=CRbt>>bt;

    if(CRbt)
        CR.write(CR.read() | CRbt);
    else
        CR.write(CR.read() & ~(0x80000000 >> bt));    

};

//!Instruction cror behavior method.
void ac_behavior( cror )
{
    dbg_printf(" cror %d, %d, %d\n\n",bt,ba,bb);

    unsigned int CRbt;
    unsigned int CRba;
    unsigned int CRbb;

    CRba=CR.read();
    CRbb=CR.read();

    /* Shift source bit to first position and zero another */
    CRba=(CRba<<ba)&0x80000000;
    CRbb=(CRbb<<bb)&0x80000000;

    CRbt=(CRba | CRbb) & 0x80000000;
    CRbt=CRbt>>bt;

    if(CRbt)
        CR.write(CR.read() | CRbt);
    else
        CR.write(CR.read() & ~(0x80000000 >> bt));    

};

//!Instruction crorc behavior method.
void ac_behavior( crorc )
{
    dbg_printf(" crorc %d, %d, %d\n\n",bt,ba,bb);

    unsigned int CRbt;
    unsigned int CRba;
    unsigned int CRbb;

    CRba=CR.read();
    CRbb=CR.read();

    /* Shift source bit to first position and zero another */
    CRba=(CRba<<ba)&0x80000000;
    CRbb=(CRbb<<bb)&0x80000000;

    CRbt=(CRba | ~CRbb) & 0x80000000;
    CRbt=CRbt>>bt;

    if(CRbt)
        CR.write(CR.read() | CRbt);
    else
        CR.write(CR.read() & ~(0x80000000 >> bt));    

};

//!Instruction crxor behavior method.
void ac_behavior( crxor )
{
    dbg_printf(" crxor %d, %d, %d\n\n",bt,ba,bb);

    unsigned int CRbt;
    unsigned int CRba;
    unsigned int CRbb;

    CRba=CR.read();
    CRbb=CR.read();

    /* Shift source bit to first position and zero another */
    CRba=(CRba<<ba)&0x80000000;
    CRbb=(CRbb<<bb)&0x80000000;

    CRbt=(CRba ^ CRbb) & 0x80000000;
    CRbt=CRbt>>bt;

    if(CRbt)
        CR.write(CR.read() | CRbt);
    else
        CR.write(CR.read() & ~(0x80000000 >> bt));    

};

//!Instruction divw behavior method.
void ac_behavior( divw )
{
    dbg_printf(" divw r%d, r%d, r%d\n\n",rt,ra,rb);

    GPR.write(rt,(int)GPR.read(ra)/(int)GPR.read(rb));

};

//!Instruction divw_ behavior method.
void ac_behavior( divw_ )
{
    dbg_printf(" divw. r%d, r%d, r%d\n\n",rt,ra,rb);
    int result=(int)GPR.read(ra)/(int)GPR.read(rb);

    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction divwo behavior method.
void ac_behavior( divwo )
{
    dbg_printf(" divwo r%d, r%d, r%d\n\n",rt,ra,rb);
    int result=(int)GPR.read(ra)/(int)GPR.read(rb);  

    divws_XER_OV_SO_update(XER, result,GPR.read(ra),GPR.read(rb));

    GPR.write(rt,result);
};

//!Instruction divwo_ behavior method.
void ac_behavior( divwo_ )
{
    dbg_printf(" divwo_ r%d, r%d, r%d\n\n",rt,ra,rb);

    int result=(int)GPR.read(ra)/(int)GPR.read(rb);  

    /* Note: XER_OV_SO_update before CR0_update */
    divws_XER_OV_SO_update(XER, result,GPR.read(ra),GPR.read(rb));
    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction divw behavior method.
void ac_behavior( divwu )
{
    dbg_printf(" divwu r%d, r%d, r%d\n\n",rt,ra,rb);

    GPR.write(rt,(unsigned int)GPR.read(ra)/(unsigned int)GPR.read(rb));

};

//!Instruction divwu_ behavior method.
void ac_behavior( divwu_ )
{
    dbg_printf(" divwu. r%d, r%d, r%d\n\n",rt,ra,rb);

    unsigned int result=(unsigned int)GPR.read(ra)/(unsigned int)GPR.read(rb);

    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction divwou behavior method.
void ac_behavior( divwou )
{
    dbg_printf(" divwou r%d, r%d, r%d\n\n",rt,ra,rb);

    unsigned int result=(unsigned int)GPR.read(ra)/(unsigned int)GPR.read(rb);  

    divwu_XER_OV_SO_update(XER, result,GPR.read(ra),GPR.read(rb));

    GPR.write(rt,result);
};

//!Instruction divwou_ behavior method.
void ac_behavior( divwou_ )
{
    dbg_printf(" divwou_ r%d, r%d, r%d\n\n",rt,ra,rb);

    unsigned int result=(unsigned int)GPR.read(ra)/(unsigned int)GPR.read(rb);  

    /* Note: XER_OV_SO_update before CR0_update */
    divwu_XER_OV_SO_update(XER, result,GPR.read(ra),GPR.read(rb));
    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction eqv behavior method.
void ac_behavior( eqv )
{
    dbg_printf(" eqv r%d, r%d, r%d\n\n",ra,rs,rb);

    GPR.write(ra,~(GPR.read(rs)^GPR.read(rb)));
};

//!Instruction eqv_ behavior method.
void ac_behavior( eqv_ )
{
    dbg_printf(" eqv. r%d, r%d, r%d\n\n",ra,rs,rb);

    GPR.write(ra,~(GPR.read(rs)^GPR.read(rb)));

    CR0_update(CR, XER, GPR.read(ra));
};

//!Instruction extsb behavior method.
void ac_behavior( extsb )
{
    dbg_printf(" extsb r%d, r%d\n\n",ra,rs);

    GPR.write(ra,(char)(GPR.read(rs)));

};

//!Instruction extsb_ behavior method.
void ac_behavior( extsb_ )
{
    dbg_printf(" extsb. r%d, r%d\n\n",ra,rs);

    GPR.write(ra,(char)(GPR.read(rs)));

    CR0_update(CR, XER, GPR.read(ra));
};

//!Instruction extsh behavior method.
void ac_behavior( extsh )
{
    dbg_printf(" extsh r%d, r%d\n\n",ra,rs);

    GPR.write(ra,(short int)(GPR.read(rs)));

};

//!Instruction extsh_ behavior method.
void ac_behavior( extsh_ )
{
    dbg_printf(" extsh. r%d, r%d\n\n",ra,rs);

    GPR.write(ra,(short int)(GPR.read(rs)));

    CR0_update(CR, XER, GPR.read(ra));

};


//!Instruction lbz behavior method.
void ac_behavior( lbz )
{
    dbg_printf(" lbz r%d, %d(r%d)\n\n",rt,d,ra);

    int ea;

    if(ra!=0)
        ea=GPR.read(ra)+(short int)d;
    else
        ea=(short int)d;

    GPR.write(rt,(unsigned int)MEM.read_byte(ea));

};

//!Instruction lbzu behavior method.
void ac_behavior( lbzu )
{
    dbg_printf(" lbzu r%d, %d(r%d)\n\n",rt,d,ra);

    int ea;

    ea=GPR.read(ra)+(short int)d;

    GPR.write(ra,ea);
    GPR.write(rt,(unsigned int)MEM.read_byte(ea));

};

//!Instruction lbzux behavior method.
void ac_behavior( lbzux )
{
    dbg_printf(" lbzux r%d, r%d, r%d\n\n",rt,ra,rb);

    int ea;

    ea=GPR.read(ra)+GPR.read(rb);

    GPR.write(ra,ea);
    GPR.write(rt,(unsigned int)MEM.read_byte(ea));

};

//!Instruction lbzx behavior method.
void ac_behavior( lbzx )
{
    dbg_printf(" lbzx r%d, r%d, r%d\n\n",rt,ra,rb);

    int ea;

    if(ra!=0)
        ea=GPR.read(ra)+GPR.read(rb);
    else
        ea=GPR.read(rb);

    GPR.write(rt,(unsigned int)MEM.read_byte(ea));

};

//!Instruction lha behavior method.
void ac_behavior( lha )
{
    dbg_printf(" lha r%d, %d(r%d)\n\n",rt,d,ra);

    int ea;

    if(ra!=0)
        ea=GPR.read(ra)+(short int)d;
    else
        ea=(short int)d;

    GPR.write(rt,(short int)MEM.read_half(ea));

};

//!Instruction lhau behavior method.
void ac_behavior( lhau )
{
    dbg_printf(" lhau r%d, %d(r%d)\n\n",rt,d,ra);

    int ea=GPR.read(ra)+(short int)d;

    GPR.write(ra,ea);
    GPR.write(rt,(short int)MEM.read_half(ea));

};

//!Instruction lhaux behavior method.
void ac_behavior( lhaux )
{
    dbg_printf(" lhaux r%d, r%d, r%d\n\n",rt,ra,rb);

    int ea=GPR.read(ra)+GPR.read(rb);

    GPR.write(ra,ea);
    GPR.write(rt,(short int)MEM.read_half(ea));

};

//!Instruction lhax behavior method.
void ac_behavior( lhax )
{
    dbg_printf(" lhax r%d, r%d, r%d\n\n",rt,ra,rb);

    int ea;

    if(ra !=0)
        ea=GPR.read(ra)+GPR.read(rb);
    else
        ea=GPR.read(rb);

    GPR.write(rt,(short int)MEM.read_half(ea));

};

//!Instruction lhbrx behavior method.
void ac_behavior( lhbrx )
{
    dbg_printf(" lhbrx r%d, r%d, r%d\n\n",rt,ra,rb);

    int ea;

    if(ra !=0)
        ea=GPR.read(ra)+GPR.read(rb);
    else
        ea=GPR.read(rb);

    GPR.write(rt,(((int)(MEM.read_byte(ea+1)) & 0x000000FF)<<8) | ((int)(MEM.read_byte(ea)) & 0x000000FF));

};

//!Instruction lhz behavior method.
void ac_behavior( lhz )
{
    dbg_printf(" lhz r%d, %d(r%d)\n\n",rt,d,ra);

    int ea;

    if(ra !=0)
        ea=GPR.read(ra)+(short int)d;
    else
        ea=(short int)d;

    GPR.write(rt,(unsigned short int)MEM.read_half(ea));

};

//!Instruction lhzu behavior method.
void ac_behavior( lhzu )
{
    dbg_printf(" lhzu r%d, %d(%d)\n\n",rt,d,ra);

    int ea=GPR.read(ra)+(short int)d;

    GPR.write(ra,ea);
    GPR.write(rt,(unsigned short int)MEM.read_half(ea));

};

//!Instruction lhzux behavior method.
void ac_behavior( lhzux )
{
    dbg_printf(" lhzux r%d, r%d, r%d\n\n",rt,ra,rb);

    int ea=GPR.read(ra)+GPR.read(rb);

    GPR.write(ra,ea);
    GPR.write(rt,(unsigned short int)MEM.read_half(ea));

};

//!Instruction lhzx behavior method.
void ac_behavior( lhzx )
{
    dbg_printf(" lhzx r%d, r%d, r%d\n\n",rt,ra,rb);

    int ea;

    if(ra !=0)
        ea=GPR.read(ra)+GPR.read(rb);
    else
        ea=GPR.read(rb);

    GPR.write(rt,(unsigned short int)MEM.read_half(ea));

};

//!Instruction lmw behavior method.
void ac_behavior( lmw )
{
    dbg_printf(" lmw r%d, %d(r%d)\n\n",rt,d,ra);

    int ea;
    unsigned int r;

    if(ra !=0)
        ea=GPR.read(ra)+(short int)d;
    else
        ea=(short int)d;

    r=rt;

    while(r<=31) {
        if((r!=ra)||(r==31))
            GPR.write(r,MEM.read(ea));
        r=r+1;
        ea=ea+4;
    }

};

//!Instruction lswi behavior method.
void ac_behavior( lswi )
{
    dbg_printf(" lswi r%d, r%d, %d\n\n",rt,ra,nb);

    int ea;
    unsigned int cnt,n;
    unsigned int rfinal,r;
    unsigned int i,masc;

    if(ra!=0)
        ea=GPR.read(ra);
    else
        ea=0;

    if(nb==0)
        cnt=32;
    else
        cnt=nb;

    n=cnt;
    rfinal=((rt + ceil(cnt,4) - 1) % 32);
    r=rt-1;
    i=0;

    while(n>0) {
        if(i==0) {
            r=r+1;
            if(r==32)
                r=0;
            if((r!=ra)||(r==rfinal))
                GPR.write(r,0);
        }
        if((r!=ra)||(r==rfinal)) {
            masc=0xFF000000>>i;
            masc=~masc;
            GPR.write(r,(GPR.read(r) & masc));
            GPR.write(r,(((unsigned int)MEM.read_byte(ea)) << (24-i)) | GPR.read(r));
        }
        i=i+8;
        if(i==32)
            i=0;
        ea=ea+1;
        n=n-1;
    }

};

//!Instruction lswx behavior method.
void ac_behavior( lswx )
{
    dbg_printf(" lswx r%d, r%d, r%d\n\n",rt,ra,rb);

    int ea;
    unsigned int cnt,n;
    unsigned int rfinal,r;
    unsigned int i,masc;

    if(ra!=0)
        ea=GPR.read(ra)+GPR.read(rb);
    else
        ea=GPR.read(rb);

    cnt=XER_TBC_read(XER);
    n=cnt;
    rfinal=((rt + ceil(cnt,4) - 1) % 32);
    r=rt-1;
    i=0;

    while(n>0) {
        if(i==0) {
            r=r+1;
            if(r==32)
                r=0;
            if(((r!=ra) && (r!=rb)) || (r==rfinal))
                GPR.write(r,0);
        }
        if(((r!=ra) && (r!=rb)) || (r==rfinal)) {
            masc=0xFF000000>>i;
            masc=~masc;
            GPR.write(r,(GPR.read(r) & masc));
            GPR.write(r,(((unsigned int)MEM.read_byte(ea)) << (24-i)) | GPR.read(r));
        }
        i=i+8;
        if(i==32)
            i=0;
        ea=ea+1;
        n=n-1;    
    }

};


//!Instruction lwbrx behavior method.
void ac_behavior( lwbrx )
{
    dbg_printf(" lwbrx r%d, r%d, r%d\n\n",rt,ra,rb);

    int ea;

    if(ra!=0)
        ea=GPR.read(ra)+GPR.read(rb);
    else
        ea=GPR.read(rb);

    GPR.write(rt,(((unsigned int)MEM.read_byte(ea+3) & 0x000000FF) << 24) | 
            (((unsigned int)MEM.read_byte(ea+2) & 0x000000FF) << 16) | 
            (((unsigned int)MEM.read_byte(ea+1) & 0x000000FF) << 8) | 
            ((unsigned int)MEM.read_byte(ea) & 0x000000FF));

};

//!Instruction lwz behavior method.
void ac_behavior( lwz )
{
    dbg_printf(" lwz r%d, %d(r%d)\n\n",rt,d,ra);

    int ea;

    if(ra !=0)
        ea=GPR.read(ra)+(short int)d;
    else
        ea=(short int)d;

    GPR.write(rt,MEM.read(ea));
};

//!Instruction lwzu behavior method.
void ac_behavior( lwzu )
{
    dbg_printf(" lwzu r%d, %d(r%d)\n\n",rt,d,ra);

    int ea=GPR.read(ra)+(short int)d;

    GPR.write(ra,ea);
    GPR.write(rt,MEM.read(ea));

};

//!Instruction lwzux behavior method.
void ac_behavior( lwzux )
{
    dbg_printf(" lwzux r%d, r%d, r%d\n\n",rt,ra,rb);

    int ea=GPR.read(ra)+GPR.read(rb);

    GPR.write(ra,ea);
    GPR.write(rt,MEM.read(ea));

};

//!Instruction lwzx behavior method.
void ac_behavior( lwzx )
{
    dbg_printf(" lwzx %d, %d, %d\n\n",rt,ra,rb);

    int ea;

    if(ra !=0)
        ea=GPR.read(ra)+GPR.read(rb);
    else
        ea=GPR.read(rb);

    GPR.write(rt,MEM.read(ea));

};

//!Instruction mcrf behavior method.
void ac_behavior( mcrf )
{
    dbg_printf(" mcrf %d, %d\n\n",bf,bfa);

    unsigned int m=bfa;
    unsigned int n=bf;
    unsigned int tmp,i;
    unsigned int masc;
    /* n <- m */

    /* Clean all bits except m-bits in tmp */
    masc=0xF0000000;
    for(i=0 ; i<m ; i++)
        masc=masc>>4;
    tmp=CR.read();
    tmp=tmp & masc;

    /* Put m-bits in n position */
    if(n<m)
        for(i=0 ; i < m-n ; i++)
            tmp=tmp<<4;
    else 
        if(n>m) /* Else nothing */
            for(i=0 ; i < n-m ; i++)
                tmp=tmp>>4;

    /* Clean all bits of n-bits and make an or with tmp */
    masc=0xF0000000;
    for(i=0 ; i<n ; i++)
        masc=masc>>4;
    masc=~masc;
    CR.write((CR.read() & masc)|tmp);

};

//!Instruction mcrxr behavior method.
void ac_behavior( mcrxr )
{
    dbg_printf(" mcrxr %d\n\n",bf);

    unsigned int n=bf;
    unsigned int i;
    unsigned int tmp=0x00;

    /* Calculate tmp bits by XER */
    if(XER_SO_read(XER))
        tmp=tmp | 0x80000000;
    if(XER_OV_read(XER))
        tmp=tmp | 0x40000000;
    if(XER_CA_read(XER))
        tmp=tmp | 0x20000000;

    /* Move altered bits to correct CR field */
    for(i=0 ; i<n ; i++)
        tmp=tmp>>4;
    CR.write(tmp);

    /* Clean XER bits */
    XER.write(XER.read() & 0xBFFFFFFF); /* Write 0 to bit 1 OV */
    XER.write(XER.read() & 0x7FFFFFFF); /* Write 0 to bit 0 SO */
    XER.write(XER.read() & 0xDFFFFFFF); /* Write 0 to bit 2 CA */
};

//!Instruction mfcr behavior method.
void ac_behavior( mfcr )
{
    dbg_printf(" mfcr r%d\n\n",rt);
    GPR.write(rt,CR.read());

};

//!Instruction mfspr behavior method.
void ac_behavior( mfspr )
{
    /* This instruction is a fix, other implementations can be better */
    dbg_printf(" mfspr r%d,%d\n\n",rt,sprf);
    unsigned int spvalue=sprf;
    spvalue=((spvalue>>5) & 0x0000001f ) |
        ((spvalue<<5) & 0x000003e0 );


    switch(spvalue) {

        /* LR */
        case 0x008:
            GPR.write(rt,LR.read());
            break;

            /* CTR */
        case 0x009:
            GPR.write(rt,CTR.read());
            break;

            /* USPRG0 */
        case 0x100:
            GPR.write(rt,USPRG0.read());
            break;

            /* SPRG4 */
        case 0x104:
            GPR.write(rt,SPRG4.read());
            break;

            /* SPRG5 */
        case 0x105:
            GPR.write(rt,SPRG5.read());
            break;

            /* SPRG6 */
        case 0x106:
            GPR.write(rt,SPRG6.read());
            break;

            /* SPRG7 */
        case 0x107:
            GPR.write(rt,SPRG7.read());
            break;



            /* Not implemented yet! */
        default:
            dbg_printf("\nERROR!\n");
            exit(-1);
            break;


    }
};


//!Instruction mtcrf behavior method.
void ac_behavior( mtcrf ) {
    dbg_printf(" mtcrf %d, r%d\n\n",xfm,rs);

    unsigned int tmpop,tmpmask;
    unsigned int mask;
    unsigned int i;
    unsigned int crm=xfm;

    tmpmask=0xF0000000;
    tmpop=0x80;
    mask=0;
    for(i=0;i<8;i++) {
        if(crm & tmpop) 
            mask=mask | tmpmask;
        tmpmask=tmpmask>>4;
        tmpop=tmpop>>1;
    }

    CR.write((GPR.read(rs) & mask) | (CR.read() & ~mask));

};



//!Instruction mtspr behavior method.
void ac_behavior( mtspr )
{
    /* This instruction is a fix, other implementations can be better */
    dbg_printf(" mtspr %d,r%d\n\n",sprf,rs);
    unsigned int spvalue=sprf;
    spvalue=((spvalue>>5) & 0x0000001f ) |
        ((spvalue<<5) & 0x000003e0 );

    switch(spvalue) {

        /* LR */
        case 0x008:
            LR.write(GPR.read(rs));
            break;

            /* CTR */
        case 0x009:
            CTR.write(GPR.read(rs));
            break;

            /* USPRG0 */
        case 0x100:
            USPRG0.write(GPR.read(rs));
            break;

            /* SPRG4 */
        case 0x104:
            SPRG4.write(GPR.read(rs));
            break;

            /* SPRG5 */
        case 0x105:
            SPRG5.write(GPR.read(rs));
            break;

            /* SPRG6 */
        case 0x106:
            SPRG6.write(GPR.read(rs));
            break;

            /* SPRG7 */
        case 0x107:
            SPRG7.write(GPR.read(rs));
            break;


            /* Not implemented yet! */
        default:
            dbg_printf("\nERROR!\n");
            exit(-1);
            break;

    }


};


//!Instruction mulhw behavior method.
void ac_behavior( mulhw )
{
    dbg_printf(" mulhw r%d, r%d, r%d\n\n",rt,ra,rb);

    int high;
    long long int prod = 
        (long long int)(int)GPR.read(ra)*(long long int)(int)GPR.read(rb);

    unsigned long long int shprod=prod;
    shprod=shprod>>32;
    high=shprod;

    GPR.write(rt,high);

};


//!Instruction mulhw_ behavior method.
void ac_behavior( mulhw_ )
{
    dbg_printf(" mulhw. r%d, r%d, r%d\n\n",rt,ra,rb);

    int high;
    long long int prod = 
        (long long int)(int)GPR.read(ra)*(long long int)(int)GPR.read(rb);

    unsigned long long int shprod=prod;
    shprod=shprod>>32;
    high=shprod;

    GPR.write(rt,high);
    CR0_update(CR, XER, high); 
};

//!Instruction mulhwu behavior method.
void ac_behavior( mulhwu )
{
    dbg_printf(" mulhwu r%d, r%d, r%d\n\n",rt,ra,rb);

    unsigned int high;
    unsigned long long int prod = 
        (unsigned long long int)(unsigned int)GPR.read(ra)*
        (unsigned long long int)(unsigned int)GPR.read(rb);

    prod=prod>>32;
    high=prod;

    GPR.write(rt,high);

};

//!Instruction mulhwu_ behavior method.
void ac_behavior( mulhwu_ )
{
    dbg_printf(" mulhwu. r%d, r%d, r%d\n\n",rt,ra,rb);

    unsigned int high;
    unsigned long long int prod = 
        (unsigned long long int)(unsigned int)GPR.read(ra)*
        (unsigned long long int)(unsigned int)GPR.read(rb);

    prod=prod>>32;
    high=prod;

    GPR.write(rt,high);
    CR0_update(CR, XER, high); 
};

//!Instruction mullhw behavior method.
void ac_behavior( mullhw )
{
    dbg_printf(" mullhw r%d, r%d, r%d\n\n",rt,ra,rb);

    int prod = 
        (int)(short int)(GPR.read(ra) & 0x0000FFFF)*
        (int)(short int)(GPR.read(rb) & 0x0000FFFF);

    GPR.write(rt,prod);

};

//!Instruction mullhw_ behavior method.
void ac_behavior( mullhw_ )
{
    dbg_printf(" mullhw. r%d, r%d, r%d\n\n",rt,ra,rb);

    int prod = 
        (int)(short int)(GPR.read(ra) & 0x0000FFFF)*
        (int)(short int)(GPR.read(rb) & 0x0000FFFF);

    GPR.write(rt,prod);
    CR0_update(CR, XER, prod); 
};

//!Instruction mullhwu behavior method.
void ac_behavior( mullhwu )
{
    dbg_printf(" mullhwu r%d, r%d, r%d\n\n",rt,ra,rb);

    unsigned int prod = 
        (unsigned int)(unsigned short int)(GPR.read(ra) & 0x0000FFFF)*
        (unsigned int)(unsigned short int)(GPR.read(rb) & 0x0000FFFF);

    GPR.write(rt,prod);

};

//!Instruction mullhwu_ behavior method.
void ac_behavior( mullhwu_ )
{
    dbg_printf(" mullhwu. r%d, r%d, r%d\n\n",rt,ra,rb);

    unsigned int prod = 
        (unsigned int)(unsigned short int)(GPR.read(ra) & 0x0000FFFF)*
        (unsigned int)(unsigned short int)(GPR.read(rb) & 0x0000FFFF);

    GPR.write(rt,prod);
    CR0_update(CR, XER, prod); 
};

//!Instruction mulli behavior method.
void ac_behavior( mulli )
{
    dbg_printf(" mulli r%d, r%d, %d\n\n",rt,ra,d);

    long long int prod = 
        (long long int)(int)GPR.read(ra) * (long long int)(short int)(d);

    int low;
    low=prod;

    GPR.write(rt,low);

};

//!Instruction mullw behavior method.
void ac_behavior( mullw )
{
    dbg_printf(" mullw r%d, r%d, r%d\n\n",rt,ra,rb);

    int low;
    long long int prod = 
        (long long int)(int)GPR.read(ra)*(long long int)(int)GPR.read(rb);

    low=prod;

    GPR.write(rt,low);

};

//!Instruction mullw_ behavior method.
void ac_behavior( mullw_ )
{
    dbg_printf(" mullw. r%d, r%d, r%d\n\n",rt,ra,rb);

    int low;
    long long int prod = 
        (long long int)(int)GPR.read(ra)*(long long int)(int)GPR.read(rb);

    low=prod;

    GPR.write(rt,low);
    CR0_update(CR, XER, low);

};

//!Instruction mullwo behavior method.
void ac_behavior( mullwo )
{
    dbg_printf(" mullwo r%d, r%d, r%d\n\n",rt,ra,rb);

    int low;
    long long int prod = 
        (long long int)(int)GPR.read(ra)*(long long int)(int)GPR.read(rb);

    low=prod;

    if(prod != (long long int)(int)low) {
        XER.write(XER.read() | 0x40000000); /* Write 1 to bit 1 OV */
        XER.write(XER.read() | 0x80000000); /* Write 1 to bit 0 SO */
    }
    else
        XER.write(XER.read() & 0xBFFFFFFF); /* Write 0 to bit 1 OV */


    GPR.write(rt,low);

};

//!Instruction mullw behavior method.
void ac_behavior( mullwo_ )
{
    dbg_printf(" mullwo_ r%d, r%d, r%d\n\n",rt,ra,rb);

    int low;
    long long int prod = 
        (long long int)(int)GPR.read(ra)*(long long int)(int)GPR.read(rb);

    low=prod;

    GPR.write(rt,low);

    if(prod != (long long int)(int)low) {
        XER.write(XER.read() | 0x40000000); /* Write 1 to bit 1 OV */
        XER.write(XER.read() | 0x80000000); /* Write 1 to bit 0 SO */
    }
    else
        XER.write(XER.read() & 0xBFFFFFFF); /* Write 0 to bit 1 OV */

    CR0_update(CR, XER, low);
};

//!Instruction nand behavior method.
void ac_behavior( nand )
{
    dbg_printf(" nand r%d, r%d, r%d\n\n",ra,rs,rb);
    GPR.write(ra,~(GPR.read(rs) & GPR.read(rb)));

};

//!Instruction nand_ behavior method.
void ac_behavior( nand_ )
{
    dbg_printf(" nand. r%d, r%d, r%d\n\n",ra,rs,rb);
    int result=~(GPR.read(rs) & GPR.read(rb));

    GPR.write(ra,result);
    CR0_update(CR, XER, result);

};

//!Instruction neg behavior method.
void ac_behavior( neg )
{
    dbg_printf(" neg r%d, r%d\n\n",rt,ra);

    GPR.write(rt,~(GPR.read(ra))+1);
};

//!Instruction neg_ behavior method.
void ac_behavior( neg_ )
{
    dbg_printf(" neg. r%d, r%d\n\n",rt,ra);

    int result=~(GPR.read(ra))+1;
    GPR.write(rt,result);
    CR0_update(CR, XER, result);
};

//!Instruction nego behavior method.
void ac_behavior( nego )
{
    dbg_printf(" nego r%d, r%d\n\n",rt,ra);

    long long int longresult=~((unsigned int)GPR.read(ra))+1;
    int result=~(GPR.read(ra))+1;

    if(longresult != (long long int)result) {
        XER.write(XER.read() | 0x40000000); /* Write 1 to bit 1 OV */
        XER.write(XER.read() | 0x80000000); /* Write 1 to bit 0 SO */
    }
    else
        XER.write(XER.read() & 0xBFFFFFFF); /* Write 0 to bit 1 OV */

    GPR.write(rt,result);
};

//!Instruction nego_ behavior method.
void ac_behavior( nego_ )
{
    dbg_printf(" nego. r%d, r%d\n\n",rt,ra);

    long long int longresult=~((unsigned int)GPR.read(ra))+1;
    int result=~(GPR.read(ra))+1;

    if(longresult != (long long int)result) {
        XER.write(XER.read() | 0x40000000); /* Write 1 to bit 1 OV */
        XER.write(XER.read() | 0x80000000); /* Write 1 to bit 0 SO */
    }
    else
        XER.write(XER.read() & 0xBFFFFFFF); /* Write 0 to bit 1 OV */

    GPR.write(rt,result);
    CR0_update(CR, XER, result);

};

//!Instruction nor behavior method.
void ac_behavior( nor )
{
    dbg_printf(" nor r%d, r%d, r%d\n\n",ra,rs,rb);
    GPR.write(ra,~(GPR.read(rs) | GPR.read(rb)));

};

//!Instruction nor_ behavior method.
void ac_behavior( nor_ )
{
    dbg_printf(" nor. r%d, r%d, r%d\n\n",ra,rs,rb);
    int result=~(GPR.read(rs) | GPR.read(rb));

    GPR.write(ra,result);
    CR0_update(CR, XER, result);

};

//!Instruction ore behavior method.
void ac_behavior( ore )
{
    dbg_printf(" or r%d, r%d, r%d\n\n",ra,rs,rb);
    GPR.write(ra,GPR.read(rs) | GPR.read(rb));

};

//!Instruction ore_ behavior method.
void ac_behavior( ore_ )
{
    dbg_printf(" or. r%d, r%d, r%d\n\n",ra,rs,rb);
    int result=GPR.read(rs) | GPR.read(rb);

    GPR.write(ra,result);
    CR0_update(CR, XER, result);

};

//!Instruction orc behavior method.
void ac_behavior( orc )
{
    dbg_printf(" orc r%d, r%d, r%d\n\n",ra,rs,rb);
    GPR.write(ra,GPR.read(rs) | ~GPR.read(rb));

};

//!Instruction orc_ behavior method.
void ac_behavior( orc_ )
{
    dbg_printf(" orc. r%d, r%d, r%d\n\n",ra,rs,rb);
    int result=GPR.read(rs) | ~GPR.read(rb);

    GPR.write(ra,result);
    CR0_update(CR, XER, result);

};

//!Instruction ori behavior method.
void ac_behavior( ori )
{
    dbg_printf(" ori r%d, r%d, %d\n\n",ra,rs,ui);

    GPR.write(ra,GPR.read(rs) | (int)((unsigned short int)ui));
};

//!Instruction oris behavior method.
void ac_behavior( oris )
{
    dbg_printf(" oris r%d, r%d, r%d\n\n",ra,rs,ui);

    GPR.write(ra,GPR.read(rs) | (((int)((unsigned short int)ui)) << 16));
};

//!Instruction rlwimi behavior method.
void ac_behavior( rlwimi )
{
    dbg_printf(" rlwimi r%d, r%d, %d, %d, %d\n\n",ra,rs,sh,mb,me);

    unsigned int r=rotl(GPR.read(rs),sh);
    unsigned int m=mask32rlw(mb,me);

    GPR.write(ra,(r & m) | (GPR.read(ra) & ~m));

};

//!Instruction rlwimi_ behavior method.
void ac_behavior( rlwimi_ )
{
    dbg_printf(" rlwimi. r%d, r%d, %d, %d, %d\n\n",ra,rs,sh,mb,me);

    unsigned int r=rotl(GPR.read(rs),sh);
    unsigned int m=mask32rlw(mb,me);

    GPR.write(ra,(r & m) | (GPR.read(ra) & ~m));

    CR0_update(CR, XER, GPR.read(ra));  

};

//!Instruction rlwinm behavior method.
void ac_behavior( rlwinm )
{
    dbg_printf(" rlwinm r%d, r%d, %d, %d, %d\n\n",ra,rs,sh,mb,me);

    unsigned int r=rotl(GPR.read(rs),sh);
    unsigned int m=mask32rlw(mb,me);

    GPR.write(ra,(r & m));

};

//!Instruction rlwinm_ behavior method.
void ac_behavior( rlwinm_ )
{
    dbg_printf(" rlwinm. r%d, r%d, %d, %d, %d\n\n",ra,rs,sh,mb,me);

    unsigned int r=rotl(GPR.read(rs),sh);
    unsigned int m=mask32rlw(mb,me);

    GPR.write(ra,(r & m));

    CR0_update(CR, XER, GPR.read(ra));  

};

//!Instruction rlwnm behavior method.
void ac_behavior( rlwnm )
{
    dbg_printf(" rlwnm r%d, r%d, %d, %d, %d\n\n",ra,rs,rb,mb,me);

    unsigned int r=rotl(GPR.read(rs),(GPR.read(rb) | 0x0000001F));
    unsigned int m=mask32rlw(mb,me);

    GPR.write(ra,(r & m));

};

//!Instruction rlwnm_ behavior method.
void ac_behavior( rlwnm_ )
{
    dbg_printf(" rlwnm. r%d, r%d, %d, %d, %d\n\n",ra,rs,rb,mb,me);

    unsigned int r=rotl(GPR.read(rs),(GPR.read(rb) | 0x0000001F));
    unsigned int m=mask32rlw(mb,me);

    GPR.write(ra,(r & m));

    CR0_update(CR, XER, GPR.read(ra));  

};

//!Instruction sc behavior method.
/* The registers used in this intruction may be defined better */
void ac_behavior( sc )
{
    dbg_printf(" sc\n\n");

    SRR1.write(MSR.read()); /* It uses a function to join MSR fields */

    /* Write WE, EE, PR, DR and IR as 0 in MSR */
    MSR.write(MSR.read() | 0xFFFB3FCF);

    SRR0.write((ac_pc-4)+4); /* Only to understand pre-increment */

    ac_pc=((EVPR.read() & 0xFFFF0000) | 0x00000C00);

};

//!Instruction slw behavior method.
void ac_behavior( slw )
{
    dbg_printf(" slw r%d, r%d, r%d\n\n",ra,rs,rb);

    unsigned int n=(GPR.read(rb) & 0x0000001F);
    unsigned int r=rotl(GPR.read(rs),n);
    unsigned int m;

    if((GPR.read(rb) & 0x00000020)==0x00) /* Check bit 26 */
        m=mask32rlw(0,31-n);
    else
        m=0;

    GPR.write(ra,r & m);

};

//!Instruction slw_ behavior method.
void ac_behavior( slw_ )
{
    dbg_printf(" slw. r%d, r%d, r%d\n\n",ra,rs,rb);

    unsigned int n=(GPR.read(rb) & 0x0000001F);
    unsigned int r=rotl(GPR.read(rs),n);
    unsigned int m;

    if((GPR.read(rb) & 0x00000020)==0x00) /* Check bit 26 */
        m=mask32rlw(0,31-n);
    else
        m=0;

    int result=r & m;
    GPR.write(ra,result);

    CR0_update(CR, XER, result);

};

//!Instruction sraw behavior method.
void ac_behavior( sraw )
{
    dbg_printf(" sraw r%d, r%d, r%d\n\n",ra,rs,rb);

    unsigned int n=(GPR.read(rb) & 0x0000001F);
    unsigned int r=rotl(GPR.read(rs),32-n);
    unsigned int m;
    unsigned int s;

    if((GPR.read(rb) & 0x00000020)==0x00) /* Check bit 26 */
        m=mask32rlw(n,31);
    else
        m=0;

    s=(GPR.read(rs) & 0x80000000);
    if(s!=0)
        s=0xFFFFFFFF;

    int result=(r & m) | (s & ~m);

    /* Write ra */
    GPR.write(ra,result);

    /* Update XER CA */
    if(s && ((r & ~m)!=0))
        XER.write(XER.read() | 0x20000000); /* Write 1 to bit 2 CA */
    else
        XER.write(XER.read() & 0xDFFFFFFF); /* Write 0 to bit 2 CA */

};

//!Instruction sraw_ behavior method.
void ac_behavior( sraw_ )
{
    dbg_printf(" sraw. r%d, r%d, r%d\n\n",ra,rs,rb);

    unsigned int n=(GPR.read(rb) & 0x0000001F);
    unsigned int r=rotl(GPR.read(rs),32-n);
    unsigned int m;
    unsigned int s;

    if((GPR.read(rb) & 0x00000020)==0x00) /* Check bit 26 */
        m=mask32rlw(n,31);
    else
        m=0;

    s=(GPR.read(rs) & 0x80000000);
    if(s!=0)
        s=0xFFFFFFFF;

    /* Write ra */
    int result=(r & m) | (s & ~m);

    GPR.write(ra,result);

    /* Update XER CA */
    if(s && ((r & ~m)!=0))
        XER.write(XER.read() | 0x20000000); /* Write 1 to bit 2 CA */
    else
        XER.write(XER.read() & 0xDFFFFFFF); /* Write 0 to bit 2 CA */

    /* Update CR register */
    CR0_update(CR, XER, result);

};

//!Instruction srawi behavior method.
void ac_behavior( srawi )
{
    dbg_printf(" srawi r%d, r%d, %d\n\n",ra,rs,sh);

    unsigned int n=sh;
    unsigned int r=rotl(GPR.read(rs),32-n);
    unsigned int m=mask32rlw(n,31);
    unsigned int s=(GPR.read(rs) & 0x80000000);

    if(s!=0)
        s=0xFFFFFFFF;

    int result=(r & m) | (s & ~m);

    GPR.write(ra,result);

    /* Update XER CA */
    if(s && ((r & ~m)!=0))
        XER.write(XER.read() | 0x20000000); /* Write 1 to bit 2 CA */
    else
        XER.write(XER.read() & 0xDFFFFFFF); /* Write 0 to bit 2 CA */

};

//!Instruction srawi_ behavior method.
void ac_behavior( srawi_ )
{
    dbg_printf(" srawi. r%d, r%d, %d\n\n",ra,rs,sh);

    unsigned int n=sh;
    unsigned int r=rotl(GPR.read(rs),32-n);;
    unsigned int m=mask32rlw(n,31);
    unsigned int s=(GPR.read(rs) & 0x80000000);
    if(s!=0)
        s=0xFFFFFFFF;

    /* Write ra */
    int result=(r & m) | (s & ~m);
    GPR.write(ra,result);

    /* Update XER CA */
    if(s && ((r & ~m)!=0))
        XER.write(XER.read() | 0x20000000); /* Write 1 to bit 2 CA */
    else
        XER.write(XER.read() & 0xDFFFFFFF); /* Write 0 to bit 2 CA */

    /* Update CR register */
    CR0_update(CR, XER, result);
};

//!Instruction srw behavior method.
void ac_behavior( srw )
{
    dbg_printf(" srw r%d, r%d, r%d\n\n",ra,rs,rb);

    unsigned int n=(GPR.read(rb) & 0x0000001F);
    unsigned int r=rotl(GPR.read(rs),32-n);
    unsigned int m;

    if((GPR.read(rb) & 0x00000020)==0x00) /* Check bit 26 */
        m=mask32rlw(n,31);
    else
        m=0;

    GPR.write(ra,r & m);

};

//!Instruction srw_ behavior method.
void ac_behavior( srw_ )
{
    dbg_printf(" srw. r%d, r%d, r%d\n\n",ra,rs,rb);

    unsigned int n=(GPR.read(rb) & 0x0000001F);
    unsigned int r=rotl(GPR.read(rs),32-n);
    unsigned int m;

    if((GPR.read(rb) & 0x00000020)==0x00) /* Check bit 26 */
        m=mask32rlw(n,31);
    else
        m=0;

    int result=r & m;

    GPR.write(ra,result);

    CR0_update(CR, XER, result);

};

//!Instruction stb behavior method.
void ac_behavior( stb )
{
    dbg_printf(" stb r%d, %d(r%d)\n\n",rs,d,ra);

    int ea;

    if(ra!=0)
        ea=GPR.read(ra)+(short int)d;
    else
        ea=(short int)d;

    MEM.write_byte(ea,(unsigned char)GPR.read(rs));

};

//!Instruction stbu behavior method.
void ac_behavior( stbu )
{
    dbg_printf(" stbu r%d, %d(r%d)\n\n",rs,d,ra);

    int ea=GPR.read(ra)+(short int)d;

    MEM.write_byte(ea,(unsigned char)GPR.read(rs));
    GPR.write(ra,ea);

};

//!Instruction stbux behavior method.
void ac_behavior( stbux )
{
    dbg_printf(" stbux r%d, r%d, r%d\n\n",rs,ra,rb);

    int ea=GPR.read(ra)+GPR.read(rb);

    MEM.write_byte(ea,(unsigned char)GPR.read(rs));
    GPR.write(ra,ea);

};

//!Instruction stbx behavior method.
void ac_behavior( stbx )
{
    dbg_printf(" stbx r%d, r%d, r%d\n\n",rs,ra,rb);

    int ea;

    if(ra!=0)
        ea=GPR.read(ra)+GPR.read(rb);
    else
        ea=GPR.read(rb);

    MEM.write_byte(ea,(unsigned char)GPR.read(rs));

};

//!Instruction sth behavior method.
void ac_behavior( sth )
{
    dbg_printf(" sth r%d, %d(r%d)\n\n",rs,d,ra);

    int ea;

    if(ra!=0)
        ea=GPR.read(ra)+(short int)d;
    else
        ea=(short int)d;

    MEM.write_half(ea,(unsigned short int)GPR.read(rs));

};

//!Instruction sthbrx behavior method.
void ac_behavior( sthbrx )
{
    dbg_printf(" sthbrx r%d, r%d, r%d\n\n",rs,ra,rb);

    int ea;

    if(ra!=0)
        ea=GPR.read(ra)+GPR.read(rb);
    else
        ea=GPR.read(rb);

    MEM.write_half(ea,(unsigned short int)
            ( ((GPR.read(rs) & 0x000000FF) << 8) | 
              ((GPR.read(rs) & 0x0000FF00) >> 8) ));

}

//!Instruction sthu behavior method.
void ac_behavior( sthu )
{
    dbg_printf(" sthu r%d, %d(r%d)\n\n",rs,d,ra);

    int ea=GPR.read(ra)+(short int)d;

    MEM.write_half(ea,(unsigned short int)GPR.read(rs));
    GPR.write(ra,ea);

};

//!Instruction sthux behavior method.
void ac_behavior( sthux )
{
    dbg_printf("sthux r%d, r%d, r%d\n\n",rs,ra,rb);

    int ea=GPR.read(ra)+GPR.read(rb);

    MEM.write_half(ea,(unsigned short int)GPR.read(rs));
    GPR.write(ra,ea);

};

//!Instruction sthx behavior method.
void ac_behavior( sthx )
{
    dbg_printf(" shhx r%d, r%d, r%d\n\n",rs,ra,rb);

    int ea;

    if(ra!=0)
        ea=GPR.read(ra)+GPR.read(rb);
    else
        ea=GPR.read(rb);

    MEM.write_half(ea,(unsigned short int)GPR.read(rs));

};

//!Instruction stmw behavior method.
void ac_behavior( stmw )
{
    dbg_printf(" stmw r%d, %d(r%d)\n\n",rs,d,ra);

    int ea;
    unsigned int r;

    if(ra!=0)
        ea=GPR.read(ra)+(short int)d;
    else
        ea=(short int)d;

    r=rs;

    while(r<=31) {
        MEM.write(ea,GPR.read(r));
        r+=1;
        ea+=4;
    }

};

//!Instruction stswi behavior method.
void ac_behavior( stswi )
{
    dbg_printf(" stswi r%d, r%d, %d\n\n",rs,ra,nb);

    int ea;
    unsigned int n;
    unsigned int r;
    unsigned int i,masc;

    if(ra!=0)
        ea=GPR.read(ra);
    else
        ea=0;

    if(nb==0)
        n=32;
    else
        n=nb;

    r=rs-1;
    i=0;

    while(n>0) {
        if(i==0) 
            r=r+1;
        if(r==32)
            r=0; 
        masc=mask32rlw(i,i+7);
        MEM.write_byte(ea,(unsigned char)((GPR.read(r) & masc) >> (24-i)));
        i=i+8;
        if(i==32)
            i=0;
        ea=ea+1;
        n=n-1;
    }

};

//!Instruction stswx behavior method.
void ac_behavior( stswx )
{
    dbg_printf(" stswx r%d, r%d, r%d\n\n",rs,ra,rb);

    int ea;
    unsigned int n;
    unsigned int r;
    unsigned int i,masc;

    if(ra!=0)
        ea=GPR.read(ra)+GPR.read(rb);
    else
        ea=GPR.read(rb);

    n=XER_TBC_read(XER);

    r=rs-1;
    i=0;

    while(n>0) {
        if(i==0) 
            r=r+1;
        if(r==32)
            r=0; 
        masc=mask32rlw(i,i+7);
        MEM.write_byte(ea,(unsigned char)((GPR.read(r) & masc) >> (24-i)));
        i=i+8;
        if(i==32)
            i=0;
        ea=ea+1;
        n=n-1;
    }

};

//!Instruction stw behavior method.
void ac_behavior( stw )
{
    dbg_printf(" stw r%d, %d(r%d)\n\n",rs,d,ra);

    int ea;

    if(ra!=0)
        ea=GPR.read(ra)+(short int)d;
    else
        ea=(short int)d;

    MEM.write(ea,(unsigned int)GPR.read(rs));
};

//!Instruction stwbrx behavior method.
void ac_behavior( stwbrx )
{
    dbg_printf(" stwbrx r%d, r%d, r%d\n\n",rs,ra,rb);

    int ea;

    if(ra!=0)
        ea=GPR.read(ra)+GPR.read(rb);
    else
        ea=GPR.read(rb);

    MEM.write(ea,(((GPR.read(rs) & 0x000000FF) << 24)  |
                ((GPR.read(rs) & 0x0000FF00) << 16 ) |
                ((GPR.read(rs) & 0x00FF0000) << 8 ) |
                (GPR.read(rs) & 0xFF000000)));

};


//!Instruction stwu behavior method.
void ac_behavior( stwu )
{
    dbg_printf(" stwu r%d, %d(r%d)\n\n",rs,d,ra);

    int ea=GPR.read(ra)+(short int)d;

    MEM.write(ea,(unsigned int)GPR.read(rs));
    GPR.write(ra,ea);

};

//!Instruction stwux behavior method.
void ac_behavior( stwux )
{
    dbg_printf("stwux r%d, r%d, r%d\n\n",rs,ra,rb);

    int ea=GPR.read(ra)+GPR.read(rb);

    MEM.write(ea,GPR.read(rs));
    GPR.write(ra,ea);

};

//!Instruction stwx behavior method.
void ac_behavior( stwx )
{
    dbg_printf(" stwx r%d, r%d, r%d\n\n",rs,ra,rb);

    int ea;

    if(ra!=0)
        ea=GPR.read(ra)+GPR.read(rb);
    else
        ea=GPR.read(rb);

    MEM.write(ea,(unsigned int)GPR.read(rs));

};







//!Instruction subf behavior method.
void ac_behavior( subf )
{
    dbg_printf(" subf r%d, r%d, r%d\n\n",rt,ra,rb);

    GPR.write(rt,~GPR.read(ra) + GPR.read(rb) + 1);

};

//!Instruction subf_ behavior method.
void ac_behavior( subf_ )
{
    dbg_printf(" subf. r%d, r%d, r%d\n\n",rt,ra,rb);
    int result=~GPR.read(ra) + GPR.read(rb) + 1;

    GPR.write(rt,result);
    CR0_update(CR, XER, result);

};

//!Instruction subfo behavior method.
void ac_behavior( subfo )
{
    dbg_printf(" subfo r%d, r%d, r%d\n\n",rt,ra,rb);
    int result=~GPR.read(ra) + GPR.read(rb) + 1;

    add_XER_OV_SO_update(XER, result,~GPR.read(ra),GPR.read(rb),1);
    GPR.write(rt,result);
};

//!Instruction subfo_ behavior method.
void ac_behavior( subfo_ )
{
    dbg_printf(" subfo. r%d, r%d, r%d\n\n",rt,ra,rb);
    int result=~GPR.read(ra) + GPR.read(rb) + 1;

    /* Note: XER_OV_SO_update before CR0_update */
    add_XER_OV_SO_update(XER, result,~GPR.read(ra),GPR.read(rb),1);
    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction subfc behavior method.
void ac_behavior( subfc )
{
    dbg_printf(" subfc r%d, r%d, r%d\n\n",rt,ra,rb);
    int result=~GPR.read(ra) + GPR.read(rb) + 1;

    add_XER_CA_update(XER, result,~GPR.read(ra),GPR.read(rb),1);

    GPR.write(rt,result);
};

//!Instruction subfc_ behavior method.
void ac_behavior( subfc_ )
{
    dbg_printf(" subfc. r%d, r%d, r%d\n\n",rt,ra,rb);
    int result=~GPR.read(ra) + GPR.read(rb) + 1;

    add_XER_CA_update(XER, result,~GPR.read(ra),GPR.read(rb),1);

    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction subfco behavior method.
void ac_behavior( subfco )
{
    dbg_printf(" subfco r%d, r%d, r%d\n\n",rt,ra,rb);
    int result=~GPR.read(ra) + GPR.read(rb) + 1;

    add_XER_CA_update(XER, result,~GPR.read(ra),GPR.read(rb),1);

    add_XER_OV_SO_update(XER, result,~GPR.read(ra),GPR.read(rb),1);

    GPR.write(rt,result);
};

//!Instruction subfco_ behavior method.
void ac_behavior( subfco_ )
{
    dbg_printf(" subfco. r%d, r%d, r%d\n\n",rt,ra,rb);
    int result=~GPR.read(ra) + GPR.read(rb) + 1;

    add_XER_CA_update(XER, result,~GPR.read(ra),GPR.read(rb),1);

    /* Note: XER_OV_SO_update before CR0_update */
    add_XER_OV_SO_update(XER, result,~GPR.read(ra),GPR.read(rb),1);
    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction subfe behavior method.
void ac_behavior( subfe )
{
    dbg_printf(" subfe r%d, r%d, r%d\n\n",rt,ra,rb);

    int result=~GPR.read(ra) + GPR.read(rb) + XER_CA_read(XER);

    add_XER_CA_update(XER, result,~GPR.read(ra),GPR.read(rb),XER_CA_read(XER));

    GPR.write(rt,result);
};

//!Instruction subfe_ behavior method.
void ac_behavior( subfe_ )
{
    dbg_printf(" subfe. r%d, r%d, r%d\n\n",rt,ra,rb);

    int result=~GPR.read(ra) + GPR.read(rb) + XER_CA_read(XER);

    add_XER_CA_update(XER, result,~GPR.read(ra),GPR.read(rb),XER_CA_read(XER));

    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction subfeo behavior method.
void ac_behavior( subfeo )
{
    dbg_printf(" subfeo r%d, r%d, r%d\n\n",rt,ra,rb);

    int result=~GPR.read(ra) + GPR.read(rb) + XER_CA_read(XER);

    add_XER_CA_update(XER, result,~GPR.read(ra),GPR.read(rb),XER_CA_read(XER));

    add_XER_OV_SO_update(XER, result,~GPR.read(ra),GPR.read(rb),XER_CA_read(XER));

    GPR.write(rt,result);
};

//!Instruction subfeo_ behavior method.
void ac_behavior( subfeo_ )
{
    dbg_printf(" subfeo. r%d, r%d, r%d\n\n",rt,ra,rb);

    int result=~GPR.read(ra) + GPR.read(rb) + XER_CA_read(XER);

    add_XER_CA_update(XER, result,~GPR.read(ra),GPR.read(rb),XER_CA_read(XER));

    /* Note: XER_OV_SO_update before CR0_update */
    add_XER_OV_SO_update(XER, result,~GPR.read(ra),GPR.read(rb),XER_CA_read(XER));
    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction subfic behavior method.
void ac_behavior( subfic )
{
    dbg_printf(" subfic r%d, r%d, %d\n\n",rt,ra,d);
    int ime32=d;
    int result=~GPR.read(ra)+ime32+1;

    add_XER_CA_update(XER, result,~GPR.read(ra),ime32,1);

    GPR.write(rt,result);
};

//!Instruction subfme behavior method.
void ac_behavior( subfme )
{
    dbg_printf(" subfme r%d, r%d\n\n",rt,ra);
    int result=~GPR.read(ra)+XER_CA_read(XER)+(-1);

    add_XER_CA_update(XER, result,~GPR.read(ra),XER_CA_read(XER),-1);

    GPR.write(rt,result);
};

//!Instruction subfme_ behavior method.
void ac_behavior( subfme_ )
{
    dbg_printf(" subfme. r%d, r%d\n\n",rt,ra);
    int result=~GPR.read(ra)+XER_CA_read(XER)+(-1);

    add_XER_CA_update(XER, result,~GPR.read(ra),XER_CA_read(XER),-1);

    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction subfmeo behavior method.
void ac_behavior( subfmeo )
{
    dbg_printf(" subfmeo r%d, r%d\n\n",rt,ra);
    int result=~GPR.read(ra)+XER_CA_read(XER)+(-1);

    add_XER_CA_update(XER, result,~GPR.read(ra),XER_CA_read(XER),-1);

    add_XER_OV_SO_update(XER, result,~GPR.read(ra),XER_CA_read(XER),-1);

    GPR.write(rt,result);
};

//!Instruction subfmeo_ behavior method.
void ac_behavior( subfmeo_ )
{
    dbg_printf(" subfmeo. r%d, r%d\n\n",rt,ra);
    int result=~GPR.read(ra)+XER_CA_read(XER)+(-1);

    add_XER_CA_update(XER, result,~GPR.read(ra),XER_CA_read(XER),-1);

    /* Note: XER_OV_SO_update before CR0_update */
    add_XER_OV_SO_update(XER, result,~GPR.read(ra),XER_CA_read(XER),-1);
    CR0_update(CR, XER, result);

    GPR.write(rt,result);  
};

//!Instruction subfze behavior method.
void ac_behavior( subfze )
{
    dbg_printf(" subfze r%d, r%d\n\n",rt,ra);
    int result=~GPR.read(ra)+XER_CA_read(XER);

    add_XER_CA_update(XER, result,~GPR.read(ra),XER_CA_read(XER),0);

    GPR.write(rt,result);
};

//!Instruction subfze_ behavior method.
void ac_behavior( subfze_ )
{
    dbg_printf(" subfze. r%d, r%d\n\n",rt,ra);
    int result=~GPR.read(ra)+XER_CA_read(XER);

    add_XER_CA_update(XER, result,~GPR.read(ra),XER_CA_read(XER),0);

    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction subfzeo behavior method.
void ac_behavior( subfzeo )
{
    dbg_printf(" subfzeo r%d, r%d\n\n",rt,ra);
    int result=~GPR.read(ra)+XER_CA_read(XER);

    add_XER_CA_update(XER, result,~GPR.read(ra),XER_CA_read(XER),0);

    add_XER_OV_SO_update(XER, result,~GPR.read(ra),XER_CA_read(XER),0);

    GPR.write(rt,result);
};

//!Instruction subfzeo_ behavior method.
void ac_behavior( subfzeo_ )
{
    dbg_printf(" subfzeo. r%d, r%d\n\n",rt,ra);
    int result=~GPR.read(ra)+XER_CA_read(XER);

    add_XER_CA_update(XER, result,~GPR.read(ra),XER_CA_read(XER),0);

    /* Note: XER_OV_SO_update before CR0_update */
    add_XER_OV_SO_update(XER, result,~GPR.read(ra),XER_CA_read(XER),0);  
    CR0_update(CR, XER, result);

    GPR.write(rt,result);
};

//!Instruction xor behavior method.
void ac_behavior( xxor )
{
    dbg_printf(" xor r%d, r%d, r%d\n\n",ra,rs,rb);
    GPR.write(ra,GPR.read(rs) ^ GPR.read(rb));

};

//!Instruction xor_ behavior method.
void ac_behavior( xxor_ )
{
    dbg_printf(" xor. r%d, r%d, r%d\n\n",ra,rs,rb);
    int result=GPR.read(rs) ^ GPR.read(rb);

    CR0_update(CR, XER, result);

    GPR.write(ra,result);
};

//!Instruction xori behavior method.
void ac_behavior( xori )
{
    dbg_printf(" xori r%d, r%d, %d\n\n",ra,rs,ui);

    GPR.write(ra,GPR.read(rs) ^ (int)((unsigned short int)ui));
};

//!Instruction xoris behavior method.
void ac_behavior( xoris )
{
    dbg_printf(" xoris r%d, r%d, %d\n\n",ra,rs,ui);

    GPR.write(ra,GPR.read(rs) ^ (((int)((unsigned short int)ui)) << 16));
};


// AltiVec implementations

typedef ac_multireg<unsigned long, 4> vec;
typedef ac_regbank<32, ac_multireg<unsigned long, 4>, ac_multireg<unsigned long, 4> > vecbank;

void ac_behavior( AV_X1 ){}
void ac_behavior( AV_X2 ){}
void ac_behavior( AV_VX1 ){}
void ac_behavior( AV_VX2 ){}
void ac_behavior( AV_VX3 ){}
void ac_behavior( AV_VX4 ){}
void ac_behavior( AV_VX5 ){}
void ac_behavior( AV_VX6 ){}
void ac_behavior( AV_VX7 ){}
void ac_behavior( AV_VX8 ){}
void ac_behavior( AV_VA1 ){}
void ac_behavior( AV_VA2 ){}
void ac_behavior( AV_VC ){}

// Helpers

// Given t and f (defined by PowerISA for vector instructions which deal with
// CR6), update CR properly. (TODO: May need changes for float point)
inline void CR6_update(ac_reg<ac_word> &CR, uint8_t t, uint8_t f) {
    uint32_t cr6_mask = 0xFFFFFF0F; /* start with CR6 clean */
    uint32_t new_cr6 = (((uint32_t) t << 3) & ((uint32_t) f << 1)) << 4;

    CR.write((CR.read() & cr6_mask) | new_cr6);
}

// Set SATuration bit in VSCR
inline void VSCR_SAT(ac_reg<ac_word> &VSCR, uint8_t sat) {
    uint32_t sat_mask = 0xFFFFFFFE; /* start with SAT clean */
    uint32_t new_sat = 0x00000001 & sat;

    VSCR.write((VSCR.read() & sat_mask) | new_sat);
}


//!Instruction lvebx behavior method.
void ac_behavior( lvebx ) {
    dbg_printf(" lvebx v%d, r%d, r%d\n\n", vrt, ra, rb);

    short int b;
    if (ra == 0) {
        b = 0;
    } else {
        b = GPR.read(ra);
    }

    short int ea = b + GPR.read(rb);
    short int eb = ea & (0x000F);

    printf("lvebx: ea = %d, eb = %d\n", ea, eb);
    printf("lvebx: dado lido byte: 0x%02x\n", (unsigned char) MEM.read_byte(ea));

    /* 'eb >> 2' tells what word the value is and
     * 'eb & 0x3' tells what byte inside the word
     */

    int word_pos = eb >> 2;
    int byte_pos = eb & 0x3;

    vec temp;
    temp.data[word_pos] = ((unsigned long) MEM.read_byte(ea)) << (8 * (3 - byte_pos));
    printf("lvebx: word final construida: 0x%08x\n", (unsigned long) temp.data[word_pos]);
    VR.write(vrt, temp);
}

//!Instruction lvehx behavior method.
void ac_behavior( lvehx ){
    dbg_printf(" lvehx v%d, r%d, r%d\n\n", vrt, ra, rb);

    uint16_t b;
    if (ra == 0) {
        b = 0;
    } else {
        b = GPR.read(ra);
    }

    uint16_t ea = (b + GPR.read(rb)) & (0xFFFE);
    uint16_t eb = ea & (0x000F);

    printf("lvehx: ea = %d, eb = %d\n", ea, eb);
    printf("lvehx: dado lido halfword: 0x%04x\n", (uint16_t) MEM.read_half(ea));

    /* 'eb >> 2' tells what word the value is and
     * 'eb & 0x3' tells what byte inside the word,
     * we use '(eb & 0x3) >> 1' to get the which half inside the word
     */

    int word_pos = eb >> 2;
    int half_pos = (eb & 0x3) >> 1;

    vec temp;
    temp.data[word_pos] = ((uint32_t) MEM.read_half(ea)) << (16 * (7 - half_pos));
    printf("lvehx: word final construida: 0x%08x\n", (uint32_t) temp.data[word_pos]);
    VR.write(vrt, temp);
}

//!Instruction lvewx behavior method.
void ac_behavior( lvewx ){
    dbg_printf(" lvewx v%d, r%d, r%d\n\n", vrt, ra, rb);

    uint16_t b;
    if (ra == 0) {
        b = 0;
    } else {
        b = GPR.read(ra);
    }

    uint16_t ea = (b + GPR.read(rb)) & (0xFFFC);
    uint16_t eb = ea & (0x000F);

    printf("lvewx: ea = %d, eb = %d, ou seja a word lida eh %d\n", ea, eb, (eb >> 2));
    printf("lvewx: dado lido: 0x%08x\n", (uint32_t) MEM.read(ea));

    /* Here we use 'eb >> 2' instead of 'eb' because we
     * index our vectors by words not bytes. */
    vec temp = VR.read(vrt);
    temp.data[eb >> 2] = MEM.read(ea);
    VR.write(vrt, temp);
}

//!Instruction lvx behavior method.
void ac_behavior( lvx ){
    dbg_printf(" lvx v%d, r%d, r%d\n\n", vrt, ra, rb);

    uint16_t b;
    if (ra == 0) {
        b = 0;
    } else {
        b = GPR.read(ra);
    }

    uint16_t ea = (b + GPR.read(rb)) & (0xFFF0);

    vec t = VR.read(vrt);
    t.data[0] = MEM.read(ea);
    t.data[1] = MEM.read(ea+4);
    t.data[2] = MEM.read(ea+8);
    t.data[3] = MEM.read(ea+12);
    printf("lvx: estou escrevendo 0x%08x  0x%08x  0x%08x  0x%08x\n", t.data[0], t.data[1], t.data[2], t.data[3]);
    VR.write(vrt, t);
}

//!Instruction lvxl behavior method.
void ac_behavior( lvxl ){
    // Teoricamente sera igual a lvxl
}

//!Instruction lvsl behavior method.
void ac_behavior( lvsl ){

}

//!Instruction lvsr behavior method.
void ac_behavior( lvsr ){}

//!Instruction stvebx behavior method.
void ac_behavior( stvebx ){
    dbg_printf(" stvebx v%d, r%d, r%d\n\n", vrs, ra, rb);

    uint16_t b;
    if (ra == 0) {
        b = 0;
    } else {
        b = GPR.read(ra);
    }

    uint16_t ea = b + GPR.read(rb);
    uint16_t eb = ea & (0x000F);

    printf("stvebx: ea = %d, eb = %d\n", ea, eb);

    /* 'eb >> 2' tells what word the value is and
     * 'eb & 0x3' tells what byte inside the word
     */

    int word_pos = eb >> 2;
    int byte_pos = eb & 0x3;

    vec s = VR.read(vrs);
    uint8_t byte = (uint8_t) (s.data[word_pos] >> (8 * byte_pos));
    MEM.write_byte(ea, byte);
    printf("stvebx: byte que eu escrevi 0x%02x\n", byte);
}

//!Instruction stvehx behavior method.
void ac_behavior( stvehx ) {
    dbg_printf(" stvehx v%d, r%d, r%d\n\n", vrs, ra, rb);

    uint16_t b;
    if (ra == 0) {
        b = 0;
    } else {
        b = GPR.read(ra);
    }

    uint16_t ea = b + GPR.read(rb);
    uint16_t eb = ea & (0x000F);

    printf("stvehx: ea = %d, eb = %d\n", ea, eb);

    /* 'eb >> 2' tells what word the value is and
     * 'eb & 0x3' tells what byte inside the word
     * we use '(eb & 0x3) >> 1' to get the which half inside the word
     */

    int word_pos = eb >> 2;
    int half_pos = (eb & 0x3) >> 1;

    vec s = VR.read(vrs);
    uint16_t half = (uint16_t) (s.data[word_pos] >> (16 * half_pos));
    MEM.write_half(ea, half);
    printf("stvehx: half que eu escrevi 0x%04x\n", half);
}

//!Instruction stvewx behavior method.
void ac_behavior( stvewx ) {
    dbg_printf(" stvewx v%d, r%d, r%d\n\n", vrs, ra, rb);

    uint16_t b;
    if (ra == 0) {
        b = 0;
    } else {
        b = GPR.read(ra);
    }

    uint16_t ea = (b + GPR.read(rb)) & (0xFFFC);
    uint16_t eb = ea & (0x000F);

    printf("stvewx: ea = %d, eb = %d, ou seja a word escrita eh %d\n", ea, eb, (eb >> 2));

    /* Here we use 'eb >> 2' instead of 'eb' because we
     * index our vectors by words not bytes. */
    vec temp = VR.read(vrs);
    MEM.write(ea, temp.data[eb >> 2]);
    
    printf("stvewx: escrevi %u\n", temp.data[eb >> 2]);
}

//!Instruction stvx behavior method.
void ac_behavior( stvx ){
    dbg_printf(" stvx v%d, r%d, r%d\n\n", vrs, ra, rb);

    uint16_t b;
    if (ra == 0) {
        b = 0;
    } else {
        b = GPR.read(ra);
    }

    uint16_t ea = (b + GPR.read(rb)) & (0xFFF0);

    vec t = VR.read(vrs);
    MEM.write(ea, t.data[0]);
    MEM.write(ea+4, t.data[1]);
    MEM.write(ea+8, t.data[2]);
    MEM.write(ea+12, t.data[3]);
    printf("stvx: estou escrevendo 0x%08x  0x%08x  0x%08x  0x%08x\n", t.data[0], t.data[1], t.data[2], t.data[3]);
}

//!Instruction stvxl behavior method.
void ac_behavior( stvxl ){
    // Teoricamente sera igual ao stvx
}

//!Instruction vpkpx behavior method.
void ac_behavior( vpkpx ){}

//!Instruction vpkshss behavior method.
void ac_behavior( vpkshss ){}

//!Instruction vpkswss behavior method.
void ac_behavior( vpkswss ){}

//!Instruction vpkshus behavior method.
void ac_behavior( vpkshus ){}

//!Instruction vpkswus behavior method.
void ac_behavior( vpkswus ){}

//!Instruction vpkuhum behavior method.
void ac_behavior( vpkuhum ){}

//!Instruction vpkuwum behavior method.
void ac_behavior( vpkuwum ){}

//!Instruction vpkuhus behavior method.
void ac_behavior( vpkuhus ){}

//!Instruction vpkuwus behavior method.
void ac_behavior( vpkuwus ){}

//!Instruction vmrghb behavior method.
void ac_behavior( vmrghb ){}

//!Instruction vmrghw behavior method.
void ac_behavior( vmrghw ){}

//!Instruction vmrghh behavior method.
void ac_behavior( vmrghh ){}

//!Instruction vmrglb behavior method.
void ac_behavior( vmrglb ){}

//!Instruction vmrglw behavior method.
void ac_behavior( vmrglw ){}

//!Instruction vmrglh behavior method.
void ac_behavior( vmrglh ){}

//!Instruction vupkhpx behavior method.
void ac_behavior( vupkhpx ){}

//!Instruction vupkhsb behavior method.
void ac_behavior( vupkhsb ){}

//!Instruction vupkhsh behavior method.
void ac_behavior( vupkhsh ){}

//!Instruction vupklpx behavior method.
void ac_behavior( vupklpx ){}

//!Instruction vupklsb behavior method.
void ac_behavior( vupklsb ){}

//!Instruction vupklsh behavior method.
void ac_behavior( vupklsh ){}

//!Instruction vspltb behavior method.
void ac_behavior( vspltb ){}

//!Instruction vsplth behavior method.
void ac_behavior( vsplth ){}

//!Instruction vspltw behavior method.
void ac_behavior( vspltw ){}

//!Instruction vspltb behavior method.
void ac_behavior( vspltisb ){}

//!Instruction vsplth behavior method.
void ac_behavior( vspltish ){}

//!Instruction vspltw behavior method.
void ac_behavior( vspltisw ){}

//!Instruction perm behavior method.
void ac_behavior( perm ){}

//!Instruction vsl behavior method.
void ac_behavior( vsl ){
    dbg_printf(" vsl v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    uint8_t bb = 0,
            sh = 0;
    uint32_t bt32 = 0;
    bool undefined=false;

    sh = (uint8_t)(0x00000007 & (b.data[3]));
    printf("sh: %02X; b.data[3]: %08X\n",sh,b.data[3]);

    // accesses words and bytes in the right order 
    // (i==0 means word 0, j==0 means byte 0 as in the specs)
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            // we only want bits 5:7 of bc
            bb = (uint8_t) ((0x07000000 & (b.data[i] << (8 * j)))>>3*8);
            printf("sh: %02X; bb: %02X\n",sh,bb);

            if ( bb != sh ) {
                // if the shift counts aren't all the same, result is undefined
                undefined = true;
                t.data[i] = 0xDEADBEEF;
                break;
            }
        }
    }

    if (!undefined) {
        for (i = 0; i < 3; i++) {
            t.data[i] = (a.data[i] << sh) | (a.data[i+1] >> (32-sh));
        }
        t.data[3] = a.data[3] << sh;
    }

    VR.write(vrt, t);

}

//!Instruction vslo behavior method.
void ac_behavior( vslo ){}

//!Instruction vsr behavior method.
void ac_behavior( vsr ){}

//!Instruction vsro behavior method.
void ac_behavior( vsro ){}

//!Instruction vaddcuw behavior method.
void ac_behavior( vaddcuw ){
    dbg_printf(" vaddcuw v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    uint32_t sum;
    int i;
    for (i = 0; i < 4; i++) {
        sum = a.data[i] + b.data[i];
        t.data[i] = sum < a.data[i] ? 1 : 0;
    }

    VR.write(vrt, t);
}

//!Instruction vaddsbs behavior method.
void ac_behavior( vaddsbs ) {
    dbg_printf(" vaddsbs v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    int8_t ba, bb;
    int16_t bt;
    uint8_t raw;

    int saturated = 0;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            ba = (int8_t) (0x000000FF & (a.data[i] >> (8 * j)));
            bb = (int8_t) (0x000000FF & (b.data[i] >> (8 * j)));
         
            bt = (int16_t) ba + (int16_t) bb;

            if (abs(bt) > 0x80) {
                raw = 0x80;
                saturated++;
            } else if (bt > 0x7F) {
                raw = 0x7F;
                saturated++;
            } else {
                raw = (uint8_t) bt;
            }

            t.data[i] |= (((uint32_t) raw) & (0x000000FF)) << (8 * j);
        }
    }

    VR.write(vrt, t);
    if (saturated) VSCR_SAT(VSCR, 1);
}

//!Instruction vaddshs behavior method.
void ac_behavior( vaddshs )
{
    dbg_printf(" vaddshs v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    int16_t ba, bb;
    int32_t bt;
    uint16_t raw;

    int saturated = 0;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) {
            ba = (int16_t) (0x0000FFFF & (a.data[i] >> (16 * j)));
            bb = (int16_t) (0x0000FFFF & (b.data[i] >> (16 * j)));
         
            bt = (int32_t) ba + (int32_t) bb;

            if (abs(bt) > 0x8000) {
                raw = 0x8000;
                saturated++;
            } else if (bt > 0x7FFF) {
                raw = 0x7FFF;
                saturated++;
            } else {
                raw = (uint16_t) bt;
            }

            t.data[i] |= (((uint32_t) raw) & (0x0000FFFF)) << (16 * j);
        }
    }

    VR.write(vrt, t);
    if (saturated) VSCR_SAT(VSCR, 1);
}

//!Instruction vaddsws behavior method.
void ac_behavior( vaddsws )
{
    dbg_printf(" vaddsws v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i;
    int32_t wa, wb;
    int64_t sum;
    uint32_t raw;
    int saturated = 0;

    for (i = 0; i < 4; i++) {
        wa = (int32_t) a.data[i];
        wb = (int32_t) b.data[i];

        sum = (int64_t) wa + (int64_t) wb;

        if (abs(sum) > 0x080000000) {
            printf("0x%08X\n", abs(sum));
            raw = 0x80000000;
            saturated++;
        } else if (sum > 0x7FFFFFFF) {
            raw = 0x7FFFFFFF;
            saturated++;
        } else {
            raw = (uint32_t) sum;
        }

        t.data[i] = raw;
    }

    VR.write(vrt, t);
    if (saturated) VSCR_SAT(VSCR, 1);
}

//!Instruction vaddubm behavior method.
void ac_behavior( vaddubm ) {
    dbg_printf(" vaddubm v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    uint8_t ba, bb, bt;
    uint32_t bt32;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            ba = (uint8_t) (0x000000FF & (a.data[i] >> (8 * j)));
            bb = (uint8_t) (0x000000FF & (b.data[i] >> (8 * j)));
         
            //printf("vaddubm: i=%d j=%d - 0x%02X + 0x%02X\n", i, j, ba, bb);

            bt = ba + bb;
            bt32 = (((uint32_t) bt) & (0x000000FF)) << (8 * j);
            //printf("vaddubm: bt32=0x%08X - t.data[%d]=0x%08X\n", bt32, i, t.data[i]);
            t.data[i] |= bt32;
            //printf("vaddubm: bt32=0x%08X - t.data[%d]=0x%08X\n", bt32, i, t.data[i]);
        }
        //printf("vaddubm: t.data[%d] = 0x%08X\n", i, t.data[i]);
    }

    VR.write(vrt, t);
}

//!Instruction vadduwm behavior method.
void ac_behavior( vadduwm ) {
    dbg_printf(" vadduwm v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i;
    for (i = 0; i < 4; i++) {
        t.data[i] = a.data[i] + b.data[i];
    }

    VR.write(vrt, t);
}

//!Instruction vadduhm behavior method.
void ac_behavior( vadduhm ){
    dbg_printf(" vadduhm v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    uint16_t ha, hb, ht;
    uint32_t ht32;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) {
            ha = (uint16_t) (0x0000FFFF & (a.data[i] >> (16 * j)));
            hb = (uint16_t) (0x0000FFFF & (b.data[i] >> (16 * j)));
          
            ht = ha + hb;
            ht32 = (((uint32_t) ht) & 0x0000FFFF) << (16 * j);
            t.data[i] |= ht32;
        }
    }

    VR.write(vrt, t);
}

//!Instruction vaddubs behavior method.
void ac_behavior( vaddubs ){
    dbg_printf(" vaddubs v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    uint8_t ba, bb, bt;
    uint32_t bt32;
    int saturated = 0;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            ba = (uint8_t) (0x000000FF & (a.data[i] >> (8 * j)));
            bb = (uint8_t) (0x000000FF & (b.data[i] >> (8 * j)));
         
            bt = ba + bb;

            if (bt < ba) {
                bt = 0xFF;
                saturated++;
            }

            bt32 = (((uint32_t) bt) & (0x000000FF)) << (8 * j);
            t.data[i] |= bt32;
        }
    }

    VR.write(vrt, t);
    if (saturated) VSCR_SAT(VSCR, 1);
}

//!Instruction vadduhs behavior method.
void ac_behavior( vadduhs ){
    dbg_printf(" vadduhs v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    uint16_t ha, hb, ht;
    uint32_t ht32;
    int saturated = 0;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) {
            ha = (uint16_t) (0x0000FFFF & (a.data[i] >> (16 * j)));
            hb = (uint16_t) (0x0000FFFF & (b.data[i] >> (16 * j)));
          
            ht = ha + hb;

            if (ht < ha) {
                ht = 0xFFFF;
                saturated++;
            }

            ht32 = (((uint32_t) ht) & 0x0000FFFF) << (16 * j);
            t.data[i] |= ht32;
        }
    }

    VR.write(vrt, t);
    if (saturated) VSCR_SAT(VSCR, 1);
}

//!Instruction vadduws behavior method.
void ac_behavior( vadduws ){
    dbg_printf(" vadduws v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    uint32_t sum;
    int i;
    int saturated = 0;

    for (i = 0; i < 4; i++) {
        sum = a.data[i] + b.data[i];

        if (sum < a.data[i]) {
            sum = 0xFFFFFFFF;
            saturated++;
        }

        t.data[i] = sum;
    }

    VR.write(vrt, t);
    if (saturated) VSCR_SAT(VSCR, 1);
}

//!Instruction vsubcuw behavior method.
void ac_behavior( vsubcuw ){
    dbg_printf(" vsubcuw v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    uint32_t diff;
    int i;
    for (i = 0; i < 4; i++) {
        diff = a.data[i] - b.data[i];
        t.data[i] = diff > a.data[i] ? 1 : 0;
    }

    VR.write(vrt, t);
}

//!Instruction vsubsbs behavior method.
void ac_behavior( vsubsbs ){
   dbg_printf(" vsubsbs v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    int8_t ba, bb;
    int32_t bt;
    uint32_t bt32;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            ba = (int8_t) ((0xFF000000 & (a.data[i] << (8 * j)))>>3*8);
            bb = (int8_t) ((0xFF000000 & (b.data[i] << (8 * j)))>>3*8);

            bt = (int32_t)ba - (int32_t)bb;
            printf("before -> word:%02d; byte:%02d; ba: %02X; bb: %02X; bt: %02X\n",i,j,ba,bb,bt);

            bool pos_saturated=false, neg_saturated=false;
            if ( bt >= 0){
                pos_saturated = (bt >  0x7f);
                printf("bt >= 0; pos_saturated: %s\n",pos_saturated?"true":"false");
            }
            else {
                neg_saturated = (abs(bt) > 0x80);
                printf("bt < 0; neg_saturated: %s\n",neg_saturated?"true":"false");
            }
            //FIXME: strangely this doesn't work, can anyone explain:
            //neg_saturated = (t_i_i < -1*0x80000000);
            //neither this:
            //neg_saturated = (t_i_i < -2147483648);
            //see the fixme before.
            bool saturated = pos_saturated || neg_saturated;
            uint8_t t_i =
                (uint8_t)(saturated ?
                        (pos_saturated ? 0x7f: 0x80)
                        : bt);
            printf("after  -> word:%02d; byte:%02d; ba: %02X; bb: %02X; bt: %02X\n",i,j,ba,bb,t_i);

            if(saturated){
                if (saturated) VSCR_SAT(VSCR, 1);
            }


            bt32 = ((uint32_t)(t_i) & (0x000000FF)) << (8 * (3-j));
            t.data[i] |= bt32;
        }
    }

    VR.write(vrt, t);

}

//!Instruction vsubshs behavior method.
void ac_behavior( vsubshs ){
   dbg_printf(" vsubshs v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    int16_t ba, bb;
    int32_t bt;
    uint32_t bt32;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) {
            ba = (int16_t) ((0xFFFF0000 & (a.data[i] << (2*8 * j)))>>2*8);
            bb = (int16_t) ((0xFFFF0000 & (b.data[i] << (2*8 * j)))>>2*8);

            bt = (int32_t)ba - (int32_t)bb;
            printf("before -> word:%02d; halfword:%02d; ba: %04X; bb: %04X; bt: %04X\n",i,j,ba,bb,bt);

            bool pos_saturated=false, neg_saturated=false;
            if ( bt >= 0){
                pos_saturated = (bt >  0x7FFF);
                printf("bt >= 0; pos_saturated: %s\n",pos_saturated?"true":"false");
            }
            else {
                neg_saturated = (abs(bt) > 0x8000);
                printf("bt < 0; neg_saturated: %s\n",neg_saturated?"true":"false");
            }
            //FIXME: strangely this doesn't work, can anyone explain:
            //neg_saturated = (t_i_i < -1*0x80000000);
            //neither this:
            //neg_saturated = (t_i_i < -2147483648);
            //see the fixme before.
            bool saturated = pos_saturated || neg_saturated;
            uint16_t t_i =
                (uint16_t)(saturated ?
                        (pos_saturated ? 0x7FFF: 0x8000)
                        : bt);
            printf("after  -> word:%02d; halfword:%02d; ba: %04X; bb: %04X; bt: %04X\n",i,j,ba,bb,t_i);

            if(saturated){
                if (saturated) VSCR_SAT(VSCR, 1);
            }


            bt32 = ((uint32_t)(t_i) & (0x0000FFFF)) << (16 *(1-j));
            t.data[i] |= bt32;
        }
    }

    VR.write(vrt, t);

}

//!Instruction vsubsws behavior method.
void ac_behavior( vsubsws ){
   dbg_printf(" vsubsws v%d, v%d, v%d\n\n", vrt, vra, vrb);

   vec t(0);
   vec a = VR.read(vra);
   vec b = VR.read(vrb);

   int i;
   int32_t ba, bb;
   int64_t bt;
   uint32_t bt32;
   for (i = 0; i < 4; i++) {
       ba = (int32_t) a.data[i];
       bb = (int32_t) b.data[i];

       bt = (int64_t)ba - (int64_t)bb;
       printf("before -> word:%02d; ba: %08X; bb: %08X; bt: %08X\n",i,ba,bb,bt);

       bool pos_saturated=false, neg_saturated=false;
       if ( bt >= 0){
           pos_saturated = (bt >  0x7FFFFFFF);
           printf("bt >= 0; pos_saturated: %s\n",pos_saturated?"true":"false");
       }
       else {
           neg_saturated = (abs(bt) > 0x80000000);
           printf("bt < 0; neg_saturated: %s\n",neg_saturated?"true":"false");
       }
       //FIXME: strangely this doesn't work, can anyone explain:
       //neg_saturated = (t_i_i < -1*0x80000000);
       //neither this:
       //neg_saturated = (t_i_i < -2147483648);
       //see the fixme before.
       bool saturated = pos_saturated || neg_saturated;
       uint32_t t_i =
           (uint32_t)(saturated ?
                   (pos_saturated ? 0x7FFFFFFF: 0x80000000)
                   : bt);
       printf("after  -> word:%02d; ba: %08X; bb: %08X; bt: %08X\n",i,ba,bb,t_i);

       if(saturated){
           if (saturated) VSCR_SAT(VSCR, 1);

       }

       bt32 = t_i;
       t.data[i] = bt32;
   }

   VR.write(vrt, t);

}

//!Instruction vsububm behavior method.
void ac_behavior( vsububm ){
    dbg_printf(" vsububm v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    uint8_t ba, bb, bt;
    uint32_t bt32;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            ba = (uint8_t) (0x000000FF & (a.data[i] >> (8 * j)));
            bb = (uint8_t) (0x000000FF & (b.data[i] >> (8 * j)));

            bt = ba - bb;

            bt32 = (((uint32_t) bt) & (0x000000FF)) << (8 * j);
            t.data[i] |= bt32;
        }
    }

    VR.write(vrt, t);
}

//!Instruction vsubuwm behavior method.
void ac_behavior( vsubuwm ){
    dbg_printf(" vsubuwm v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i;
    for (i = 0; i < 4; i++) {
        t.data[i] = a.data[i] - b.data[i];
    }

    VR.write(vrt, t);
}

//!Instruction vsubuhm behavior method.
void ac_behavior( vsubuhm ){
    dbg_printf(" vsubuhm v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    uint16_t ha, hb, ht;
    uint32_t ht32;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) {
            ha = (uint16_t) (0x0000FFFF & (a.data[i] >> (16 * j)));
            hb = (uint16_t) (0x0000FFFF & (b.data[i] >> (16 * j)));
          
            ht = ha - hb;
            ht32 = (((uint32_t) ht) & 0x0000FFFF) << (16 * j);
            t.data[i] |= ht32;
        }
    }

    VR.write(vrt, t);

}

//!Instruction vsububs behavior method.
void ac_behavior( vsububs ){
    dbg_printf(" vsububs v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    uint8_t ba, bb, bt;
    uint32_t bt32;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            ba = (uint8_t) (0x000000FF & (a.data[i] >> (8 * j)));
            bb = (uint8_t) (0x000000FF & (b.data[i] >> (8 * j)));
         
            bt = ba - bb;

            // TODO: Need to mark SAT bit at VSCR
            // saturate
            bt = bt > ba ? 0x00 : bt;

            bt32 = (((uint32_t) bt) & (0x000000FF)) << (8 * j);
            t.data[i] |= bt32;
        }
    }

    VR.write(vrt, t);
}

//!Instruction vsubuhs behavior method.
void ac_behavior( vsubuhs ){
    dbg_printf(" vsubuhs v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    uint16_t ha, hb, ht;
    uint32_t ht32;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) {
            ha = (uint16_t) (0x0000FFFF & (a.data[i] >> (16 * j)));
            hb = (uint16_t) (0x0000FFFF & (b.data[i] >> (16 * j)));
          
            ht = ha - hb;

            // TODO: Need to mark SAT bit at VSCR
            // saturate
            ht = ht > ha ? 0x0000 : ht;

            ht32 = (((uint32_t) ht) & 0x0000FFFF) << (16 * j);
            t.data[i] |= ht32;
        }
    }

    VR.write(vrt, t);
}

//!Instruction vsubuws behavior method.
void ac_behavior( vsubuws ){
    dbg_printf(" vsubuws v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    uint32_t dif;
    int i;
    for (i = 0; i < 4; i++) {
        dif = a.data[i] - b.data[i];
        // TODO: Need to mark SAT bit at VSCR
        // saturate
        t.data[i] = dif > a.data[i] ? 0x000000 : dif;
    }

    VR.write(vrt, t);
}

//!Instruction vmulesb behavior method.
void ac_behavior( vmulesb ){}

//!Instruction vmulesh behavior method.
void ac_behavior( vmulesh ){}

//!Instruction vmuleub behavior method.
void ac_behavior( vmuleub )
{
    dbg_printf(" vmuleub v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    uint8_t ba, bb;
    uint16_t ht;
    uint32_t ht32;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j+=2) {
            ba = (uint8_t) (0x000000FF & (a.data[i] >> (8 * j)));
            bb = (uint8_t) (0x000000FF & (b.data[i] >> (8 * j)));
         
            ht = ba * bb;

            ht32 = (((uint32_t) ht) & (0x0000FFFF)) << (8 * j);
            t.data[i] |= ht32;
        }
    }

    VR.write(vrt, t);
}

//!Instruction vmuleuh behavior method.
void ac_behavior( vmuleuh )
{
    dbg_printf(" vmuleuh v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i;
    uint16_t ha, hb;

    for (i = 0; i < 4; i++) {
        ha = (uint16_t) (0x0000FFFF & a.data[i]);
        hb = (uint16_t) (0x0000FFFF & b.data[i]);
         
        t.data[i] = (uint32_t) ha * hb;
    }

    VR.write(vrt, t);
}

//!Instruction vmulosb behavior method.
void ac_behavior( vmulosb ){}

//!Instruction vmulosh behavior method.
void ac_behavior( vmulosh ){}

//!Instruction vmuloub behavior method.
void ac_behavior( vmuloub)
{
    dbg_printf(" vmuloub v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    uint8_t ba, bb;
    uint16_t ht;
    uint32_t ht32;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j+=2) {
            ba = (uint8_t) (0x000000FF & (a.data[i] >> (8 * (j+1))));
            bb = (uint8_t) (0x000000FF & (b.data[i] >> (8 * (j+1))));
         
            ht = ba * bb;

            ht32 = (((uint32_t) ht) & (0x0000FFFF)) << (8 * j);
            t.data[i] |= ht32;
        }
    }

    VR.write(vrt, t);
}

//!Instruction vmulouh behavior method.
void ac_behavior( vmulouh )
{
    dbg_printf(" vmulouh v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i;
    uint16_t ha, hb;

    for (i = 0; i < 4; i++) {
        ha = (uint16_t) (0x0000FFFF & (a.data[i] >> 16));
        hb = (uint16_t) (0x0000FFFF & (b.data[i] >> 16));
         
        t.data[i] = (uint32_t) ha * hb;
    }

    VR.write(vrt, t);
}

//!Instruction vsumsws behavior method.
void ac_behavior( vsumsws ){}

//!Instruction vsum2sws behavior method.
void ac_behavior( vsum2sws ){}

//!Instruction vsum4sbs behavior method.
void ac_behavior( vsum4sbs ){}

//!Instruction vsum4shs behavior method.
void ac_behavior( vsum4shs ){}

//!Instruction vsum4ubs behavior method.
void ac_behavior( vsum4ubs ){}

// ****** Integer Maximum Instructions. 

//!Instruction vavgsb behavior method.
// Vector Average Signed Byte - powerisa spec pag 169. 
void ac_behavior( vavgsb ){
    dbg_printf("vavgsb v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    for (int i = 0; i < 4; i++){
        int k = 8 ;  //  shifts de sizeof(BYTE). 
        int8_t a_i_0 =  (int8_t) ((a.data[i] << 3*k) >> 3*k); 
        int8_t a_i_1 =  (int8_t) ((a.data[i] << 2*k) >> 3*k); 
        int8_t a_i_2 =  (int8_t) ((a.data[i] << 1*k) >> 3*k); 
        int8_t a_i_3 =  (int8_t) ((a.data[i] << 0  ) >> 3*k); 
        int8_t b_i_0 =  (int8_t) ((b.data[i] << 3*k) >> 3*k); 
        int8_t b_i_1 =  (int8_t) ((b.data[i] << 2*k) >> 3*k); 
        int8_t b_i_2 =  (int8_t) ((b.data[i] << 1*k) >> 3*k); 
        int8_t b_i_3 =  (int8_t) ((b.data[i] << 0  ) >> 3*k); 
        int8_t t_0 =  int8_t ( (((int16_t)a_i_0 + (int16_t)b_i_0)+1) >> 1); 
        int8_t t_1 =  int8_t ( (((int16_t)a_i_1 + (int16_t)b_i_1)+1) >> 1); 
        int8_t t_2 =  int8_t ( (((int16_t)a_i_2 + (int16_t)b_i_2)+1) >> 1); 
        int8_t t_3 =  int8_t ( (((int16_t)a_i_3 + (int16_t)b_i_3)+1) >> 1); 

        uint64_t t_i = ((uint64_t)t_3 << 3*k) + ((uint32_t)t_2 << 2*k) 
                     + ((uint16_t)t_1 <<   k) +  (uint8_t)t_0 ; 
        t.data[i] = t_i; 
        //dbg_printf: 
        printf("(a_0 = %ld + b_0 = %ld)/ 2 = %ld\n ", 
                       a_i_0, 
                       b_i_0, 
                         t_0); 
        printf("(a_1 = %ld + b_1 = %ld)/ 2 = %ld\n ", 
                       a_i_1, 
                       b_i_1, 
                         t_1); 
        printf("(a_2 = %ld + b_2 = %ld)/ 2 = %ld\n ", 
                       a_i_2, 
                       b_i_2, 
                         t_2); 
        printf("(a_3 = %ld + b_3 = %ld)/ 2 = %ld\n ", 
                       a_i_3, 
                       b_i_3, 
                         t_3); 
        printf("t = {%#x,%#x,%#x,%#x}, ", (unsigned char) t_3, 
                                      (unsigned char) t_2, 
                                      (unsigned char) t_1,
                                      (unsigned char) t_0); 
        printf("t_i = %X \n\n ", t_i); 
                      //(unsigned char) b_i_3, 
    }
    VR.write(vrt, t); 
}

//!Instruction vavgsh behavior method.
// Vector Average Signed Halfword - powerisa spec pag 169. 
void ac_behavior( vavgsh ){

    dbg_printf("vavgsh v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    for (int i = 0; i < 4; i++){
        int k = 16 ;  //  shifts de sizeof(HALFWORD). 
        int16_t a_i_0 = (int16_t) ((a.data[i] << k) >> k); 
        int16_t a_i_1 = (int16_t) ((a.data[i] << 0) >> k); 
        int16_t b_i_0 = (int16_t) ((b.data[i] << k) >> k); 
        int16_t b_i_1 = (int16_t) ((b.data[i] << 0) >> k); 
        //uint16_t t_0 = (uint16_t) ( (((int32_t)a_i_0 + (int32_t)b_i_0)+1) >> 1);
        //uint16_t t_1 = (uint16_t) ( (((int32_t)a_i_1 + (int32_t)b_i_1)+1) >> 1); 
        int16_t t_0 = (int16_t) ( (((int32_t)a_i_0 + (int32_t)b_i_0)+1) >> 1);
        int16_t t_1 = (int16_t) ( (((int32_t)a_i_1 + (int32_t)b_i_1)+1) >> 1); 
        uint64_t t_i =  ((uint32_t)t_1 <<  k) +  (uint16_t)t_0 ; 
        t.data[i] = t_i; 
        //dbg_printf: 
        printf(" ((%ld) + (%ld))/2 = %ld \n", a_i_0, b_i_0, (signed long)t_0); 
        printf(" ((%ld) + (%ld))/2 = %ld \n", a_i_1, b_i_1, (signed long)t_1); 
        printf("t_0 = %#x t_1 = %#x, t_i = %#x \n\n", t_0, t_1, t_i); 
    }
    VR.write(vrt, t); 
}

//!Instruction vavgsw behavior method.
// Vector Average Signed Word - powerisa spec pag 169. 
void ac_behavior( vavgsw ){

    dbg_printf("vavgsw v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    for (int i = 0; i <  4; i++){
        int32_t a_i =  (int32_t) a.data[i]; 
        int32_t b_i =  (int32_t) b.data[i]; 
        int64_t t_i =  int64_t ( (((int64_t)a_i + (int64_t)b_i) + 1) >> 1); 
        t.data[i] = (uint64_t) t_i; 
        //dbg_printf: 
        printf("((%ld) + (%ld))/2 = %ld \t", a_i, b_i, t_i); 
        printf("t.data[i] = %ld \n", t.data[i]);
    }
    VR.write(vrt, t); 
}

//!Instruction vavgub behavior method.
// Vector Average Unsigned Byte - powerisa spec pag 170. 
void ac_behavior( vavgub ){

    dbg_printf("vavgub v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    for (int i = 0; i < 4; i++){
        int k = 8 ;  //  shifts de sizeof(BYTE). 
        //FIXME: these should be "uint8_t", but int8_t doesn't hurt anyway.
        int8_t a_i_0 =  (int8_t) ((a.data[i] << 3*k) >> 3*k); 
        int8_t a_i_1 =  (int8_t) ((a.data[i] << 2*k) >> 3*k); 
        int8_t a_i_2 =  (int8_t) ((a.data[i] << 1*k) >> 3*k); 
        int8_t a_i_3 =  (int8_t) ((a.data[i] << 0  ) >> 3*k); 
        int8_t b_i_0 =  (int8_t) ((b.data[i] << 3*k) >> 3*k); 
        int8_t b_i_1 =  (int8_t) ((b.data[i] << 2*k) >> 3*k); 
        int8_t b_i_2 =  (int8_t) ((b.data[i] << 1*k) >> 3*k); 
        int8_t b_i_3 =  (int8_t) ((b.data[i] << 0  ) >> 3*k); 
        uint8_t t_0 =  uint8_t ( (((uint16_t)a_i_0 + (uint16_t)b_i_0)+1) >> 1); 
        uint8_t t_1 =  uint8_t ( (((uint16_t)a_i_1 + (uint16_t)b_i_1)+1) >> 1); 
        uint8_t t_2 =  uint8_t ( (((uint16_t)a_i_2 + (uint16_t)b_i_2)+1) >> 1); 
        uint8_t t_3 =  uint8_t ( (((uint16_t)a_i_3 + (uint16_t)b_i_3)+1) >> 1); 

        uint64_t t_i = ((uint64_t)t_3 << 3*k) + ((uint32_t)t_2 << 2*k) 
                     + ((uint16_t)t_1 <<   k) +  (uint8_t)t_0 ; 
        t.data[i] = t_i; 
        //dbg_printf: 
        printf("(a_0 = %#x + b_0 = %#x)/ 2 = %#x\n ", 
                      (unsigned char) a_i_0, 
                      (unsigned char) b_i_0, 
                      (unsigned char)   t_0); 
        printf("(a_1 = %#x + b_1 = %#x)/ 2 = %#x\n ", 
                      (unsigned char) a_i_1, 
                      (unsigned char) b_i_1, 
                      (unsigned char)   t_1); 
        printf("(a_2 = %#x + b_2 = %#x)/ 2 = %#x\n ", 
                      (unsigned char) a_i_2, 
                      (unsigned char) b_i_2, 
                      (unsigned char)   t_2); 
        printf("(a_3 = %#x + b_3 = %#x)/ 2 = %#x\n ", 
                      (unsigned char) a_i_3, 
                      (unsigned char) b_i_3, 
                      (unsigned char)   t_3); 
        printf("t = {%#x,%#x,%#x,%#x}, ", (unsigned char) t_3, 
                                      (unsigned char) t_2, 
                                      (unsigned char) t_1,
                                      (unsigned char) t_0); 
        printf("t_i = %X \n\n ", t_i); 
    }
    VR.write(vrt, t); 
}

//!Instruction vavguh behavior method.
// Vector Average Unsigned Halfword - powerisa spec pag 170. 
void ac_behavior( vavguh ){

    dbg_printf("vavguh v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    for (int i = 0 ; i < 4; i++){
        int k = 16 ;  //  shifts de sizeof(HALFWORD). 
        uint16_t a_i_0 =  (uint16_t) ((a.data[i] << k) >> k); 
        uint16_t b_i_0 =  (uint16_t) ((b.data[i] << k) >> k); 
        uint16_t a_i_1 =  (uint16_t) ((a.data[i] >> k)); 
        uint16_t b_i_1 =  (uint16_t) ((b.data[i] >> k)); 
        uint16_t t_0 = uint16_t ( (((uint32_t)a_i_0 + (uint32_t)b_i_0)+1) >> 1);
        uint16_t t_1 = uint16_t ( (((uint32_t)a_i_1 + (uint32_t)b_i_1)+1) >> 1); 
        uint64_t t_i =  ((uint32_t)t_1 << k) + t_0 ; 
        t.data[i] = t_i; 
        //dbg_printf: 
        //printf("(%#X + %#X)/2 = %#X \n", a_i_0, b_i_0, t_0); 
        //printf("(%#X + %#X)/2 = %#X \n", a_i_1, b_i_1, t_1); 
        printf("(%u + %u)/2 = %u \n", a_i_0, b_i_0, t_0); 
        printf("(%u + %u)/2 = %u \n", a_i_1, b_i_1, t_1); 
        printf("t_0 = %#X t_1 = %#X, t_i = %#X \n\n", t_0, t_1, t_i); 

    }

    VR.write(vrt, t); 

}

//!Instruction vavguw behavior method.
// Vector Average Unsigned Word - powerisa spec pag 170. 
void ac_behavior( vavguw ){

    dbg_printf("vavguw v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    for (int i = 0 ; i < 4; i++){
        uint64_t sum = a.data[i] + b.data[i]; 
        sum++; 
        t.data[i] = sum >> 1; 
        //printf("0x%x + 0x%x = 0x%x / 2 \n", a.data[i], 
        printf("(%u + %u)/2 = %u \n", a.data[i], 
                               b.data[i], t.data[i]); 
    }

    VR.write(vrt, t); 
}

// ****** Integer Maximum Instructions. 

//!Instruction vmaxsb behavior method.
// Vector Maximum Signed Byte - powerisa spec pag 171. 
void ac_behavior( vmaxsb ){

    dbg_printf("vmaxsb v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    for (int i = 0; i < 4; i++){
        int k = 8 ;  //  shifts de sizeof(BYTE). 
        int8_t a_i_0 =  (int8_t) ((a.data[i] << 3*k) >> 3*k); 
        int8_t a_i_1 =  (int8_t) ((a.data[i] << 2*k) >> 3*k); 
        int8_t a_i_2 =  (int8_t) ((a.data[i] << 1*k) >> 3*k); 
        int8_t a_i_3 =  (int8_t) ((a.data[i] << 0  ) >> 3*k); 
        int8_t b_i_0 =  (int8_t) ((b.data[i] << 3*k) >> 3*k); 
        int8_t b_i_1 =  (int8_t) ((b.data[i] << 2*k) >> 3*k); 
        int8_t b_i_2 =  (int8_t) ((b.data[i] << 1*k) >> 3*k); 
        int8_t b_i_3 =  (int8_t) ((b.data[i] << 0  ) >> 3*k); 
        uint8_t t_0 =  (uint8_t)(a_i_0 > b_i_0 ? a_i_0 : b_i_0) ; 
        uint8_t t_1 =  (uint8_t)(a_i_1 > b_i_1 ? a_i_1 : b_i_1) ; 
        uint8_t t_2 =  (uint8_t)(a_i_2 > b_i_2 ? a_i_2 : b_i_2) ; 
        uint8_t t_3 =  (uint8_t)(a_i_3 > b_i_3 ? a_i_3 : b_i_3) ; 
        uint64_t t_i = ((uint64_t)t_3 << 3*k) + ((uint32_t)t_2 << 2*k) 
                     + ((uint16_t)t_1 <<   k) +  (uint8_t)t_0 ; 
        t.data[i] = t_i; 
        //dbg_printf: 
        printf("a_0 = {%X,%X,%X,%X}, ", 
                      (unsigned char) a_i_3, 
                      (unsigned char) a_i_2, 
                      (unsigned char) a_i_1, 
                      (unsigned char) a_i_0); 
        printf("b_0 = {%X,%X,%X,%X}, ", 
                      (unsigned char) b_i_3, 
                      (unsigned char) b_i_2, 
                      (unsigned char) b_i_1, 
                      (unsigned char) b_i_0); 
        printf("t = {%X,%X,%X,%X}, ", (unsigned char) t_3, 
                                      (unsigned char) t_2, 
                                      (unsigned char) t_1,
                                      (unsigned char) t_0); 
        printf("t_i = %X \n\n ", t_i); 
    }
    VR.write(vrt, t); 
}


//!Instruction vmaxsh behavior method.
// Vector Maximum Signed Halfword - powerisa spec pag 171. 
void ac_behavior( vmaxsh ){

    dbg_printf("vmaxsh v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    for (int i = 0; i < 4; i++){
        int k = 16 ;  //  shifts de sizeof(HALFWORD). 
        int16_t a_i_0 = (int16_t) ((a.data[i] << k) >> k); 
        int16_t a_i_1 = (int16_t) ((a.data[i] << 0) >> k); 
        int16_t b_i_0 = (int16_t) ((b.data[i] << k) >> k); 
        int16_t b_i_1 = (int16_t) ((b.data[i] << 0) >> k); 
        uint16_t t_0 =  (uint16_t) (a_i_0 > b_i_0 ? a_i_0 : b_i_0) ; 
        uint16_t t_1 =  (uint16_t) (a_i_1 > b_i_1 ? a_i_1 : b_i_1) ; 
        uint64_t t_i =  ((uint32_t)t_1 <<  k) +  (uint16_t)t_0 ; 
        t.data[i] = t_i; 
        //dbg_printf: 
        printf("a_0 = {%X,%X}, ", 
                      (unsigned short) a_i_1, 
                      (unsigned short) a_i_0); 
        printf("b_0 = {%X,%X}, ", 
                      (unsigned short) b_i_1, 
                      (unsigned short) b_i_0); 
        printf("t = {%X,%X}, ", (unsigned short) t_1, 
                                      (unsigned short) t_0); 
        printf("t_i = %X \n\n", t_i); 
    }
    VR.write(vrt, t); 
}


//!Instruction vmaxsw behavior method.
// Vector Maximum Signed Word - powerisa spec pag 171. 
void ac_behavior( vmaxsw ){

    dbg_printf("vmaxsw v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    for (int i = 0; i <  4; i++){
        int32_t a_i =  (int32_t) a.data[i]; 
        int32_t b_i =  (int32_t) b.data[i]; 
        t.data[i] = a_i > b_i ? a.data[i] : b.data[i]; 
        //dbg_printf: 
        cerr << "a = "   << a_i 
             << ", b = " << b_i
             << ", t = " << (int32_t) t.data[i]
             << endl;
    }
    VR.write(vrt, t); 
}

//!Instruction vmaxub behavior method.
// Vector Maximum Unsigned Byte - powerisa spec pag 172. 
void ac_behavior( vmaxub ){

    dbg_printf("vmaxub v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    for (int i = 0; i < 4; i++){
        int k = 8 ;  //  shifts de sizeof(BYTE). 
        uint8_t a_i_0 =  (uint8_t) ((a.data[i] << 3*k) >> 3*k); 
        uint8_t a_i_1 =  (uint8_t) ((a.data[i] << 2*k) >> 3*k); 
        uint8_t a_i_2 =  (uint8_t) ((a.data[i] << 1*k) >> 3*k); 
        uint8_t a_i_3 =  (uint8_t) ((a.data[i] << 0  ) >> 3*k); 
        uint8_t b_i_0 =  (uint8_t) ((b.data[i] << 3*k) >> 3*k); 
        uint8_t b_i_1 =  (uint8_t) ((b.data[i] << 2*k) >> 3*k); 
        uint8_t b_i_2 =  (uint8_t) ((b.data[i] << 1*k) >> 3*k); 
        uint8_t b_i_3 =  (uint8_t) ((b.data[i] << 0  ) >> 3*k); 
        uint8_t t_0 =  (a_i_0 > b_i_0 ? a_i_0 : b_i_0) ; 
        uint8_t t_1 =  (a_i_1 > b_i_1 ? a_i_1 : b_i_1) ; 
        uint8_t t_2 =  (a_i_2 > b_i_2 ? a_i_2 : b_i_2) ; 
        uint8_t t_3 =  (a_i_3 > b_i_3 ? a_i_3 : b_i_3) ; 
        uint64_t t_i = ((uint64_t)t_3 << 3*k) + ((uint32_t)t_2 << 2*k) 
                     + ((uint16_t)t_1 <<   k) + t_0 ; 
        t.data[i] = t_i; 
        //dbg_printf: 
        printf("a_0 = {%X,%X,%X,%X}, ", 
                     a_i_3, a_i_2, a_i_1, a_i_0); 
        printf("b_0 = {%X,%X,%X,%X}, ",
                     b_i_3, b_i_2, b_i_1, b_i_0); 
        printf("t = {%X,%X,%X,%X}, ", t_3, t_2, t_1, t_0); 
        printf("t_i = %X \n\n ", t_i); 
    }
    VR.write(vrt, t); 
}

//!Instruction vmaxuh behavior method.
// Vector Maximum Unsigned Halfword - powerisa spec pag 172. 
void ac_behavior( vmaxuh ){
    dbg_printf("vmaxuh v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    for (int i = 0; i < 4; i++){
        int k = 16 ;  //  shifts de sizeof(HALFWORD). 
        uint16_t a_i_0 =  (uint16_t) ((a.data[i] << k) >> k); 
        uint16_t a_i_1 =  (uint16_t) ((a.data[i] >> k)); 
        uint16_t b_i_0 =  (uint16_t) ((b.data[i] << k) >> k); 
        uint16_t b_i_1 =  (uint16_t) ((b.data[i] >> k)); 
        uint16_t t_0 =  (a_i_0 > b_i_0 ? a_i_0 : b_i_0) ; 
        uint16_t t_1 =  (a_i_1 > b_i_1 ? a_i_1 : b_i_1) ; 
        uint32_t t_i =  ((uint32_t)t_1 << k) + t_0 ; 
        t.data[i] = t_i; 
        //dbg_printf: 
        printf("a_0 = %X, ", a_i_0); 
        printf("a_1 = %X, ", a_i_1); 
        printf("b_0 = %X, ", b_i_0); 
        printf("b_1 = %X, ", b_i_1); 
        printf("t_0 = %X, ", t_0); 
        printf("t_1 = %X, ", t_1); 
        printf("t_i = %X \n\n", t_i); 
    }
    VR.write(vrt, t); 
}

//!Instruction vmaxuw behavior method.
// Vector Maximum Unsigned Word - powerisa spec pag 172. 
void ac_behavior( vmaxuw ){
    dbg_printf("vmaxuw v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    for (int i = 0; i <  4; i++){
        uint32_t  a_i =  (uint32_t) a.data[i]; 
        uint32_t  b_i =  (uint32_t) b.data[i]; 
        t.data[i] = a_i > b_i ? a.data[i] : b.data[i]; 
        //dbg_printf: 
        cerr << "a = "   << a_i 
             << ", b = " << b_i
             << ", t = " << (uint32_t) t.data[i]
             << endl;        
    }
    VR.write(vrt, t); 
}

// ****** Integer Minimum Instructions. 

//!Instruction vminsb behavior method.
// Vector Minimum Signed Byte - powerisa spec pag 173. 
void ac_behavior( vminsb ){

    dbg_printf("vminsb v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    for (int i = 0; i < 4; i++){
        int k = 8 ;  //  shifts de sizeof(BYTE). 
        int8_t a_i_0 =  (int8_t) ((a.data[i] << 3*k) >> 3*k); 
        int8_t a_i_1 =  (int8_t) ((a.data[i] << 2*k) >> 3*k); 
        int8_t a_i_2 =  (int8_t) ((a.data[i] << 1*k) >> 3*k); 
        int8_t a_i_3 =  (int8_t) ((a.data[i] << 0  ) >> 3*k); 
        int8_t b_i_0 =  (int8_t) ((b.data[i] << 3*k) >> 3*k); 
        int8_t b_i_1 =  (int8_t) ((b.data[i] << 2*k) >> 3*k); 
        int8_t b_i_2 =  (int8_t) ((b.data[i] << 1*k) >> 3*k); 
        int8_t b_i_3 =  (int8_t) ((b.data[i] << 0  ) >> 3*k); 
        uint8_t t_0 =  (uint8_t)(a_i_0 < b_i_0 ? a_i_0 : b_i_0) ; 
        uint8_t t_1 =  (uint8_t)(a_i_1 < b_i_1 ? a_i_1 : b_i_1) ; 
        uint8_t t_2 =  (uint8_t)(a_i_2 < b_i_2 ? a_i_2 : b_i_2) ; 
        uint8_t t_3 =  (uint8_t)(a_i_3 < b_i_3 ? a_i_3 : b_i_3) ; 
        uint64_t t_i = ((uint64_t)t_3 << 3*k) + ((uint32_t)t_2 << 2*k) 
                     + ((uint16_t)t_1 <<   k) +  (uint8_t)t_0 ; 
        t.data[i] = t_i; 
        //dbg_printf: 
        printf("a_0 = {%X,%X,%X,%X}, ", 
                      (unsigned char) a_i_3, 
                      (unsigned char) a_i_2, 
                      (unsigned char) a_i_1, 
                      (unsigned char) a_i_0); 
        printf("b_0 = {%X,%X,%X,%X}, ", 
                      (unsigned char) b_i_3, 
                      (unsigned char) b_i_2, 
                      (unsigned char) b_i_1, 
                      (unsigned char) b_i_0); 
        printf("t = {%X,%X,%X,%X}, ", (unsigned char) t_3, 
                                      (unsigned char) t_2, 
                                      (unsigned char) t_1,
                                      (unsigned char) t_0); 
        printf("t_i = %X \n\n ", t_i); 
    }
    VR.write(vrt, t); 
}

//!Instruction vminsh behavior method.
// Vector Minimum Signed Halfword - powerisa spec pag 173. 
void ac_behavior( vminsh ){

    dbg_printf("vminsh v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    for (int i = 0; i < 4; i++){
        int k = 16 ;  //  shifts de sizeof(HALFWORD). 
        int16_t a_i_0 = (int16_t) ((a.data[i] << k) >> k); 
        int16_t a_i_1 = (int16_t) ((a.data[i] << 0) >> k); 
        int16_t b_i_0 = (int16_t) ((b.data[i] << k) >> k); 
        int16_t b_i_1 = (int16_t) ((b.data[i] << 0) >> k); 
        uint16_t t_0 =  (uint16_t) (a_i_0 < b_i_0 ? a_i_0 : b_i_0) ; 
        uint16_t t_1 =  (uint16_t) (a_i_1 < b_i_1 ? a_i_1 : b_i_1) ; 
        uint64_t t_i =  ((uint32_t)t_1 <<  k) +  (uint16_t)t_0 ; 
        t.data[i] = t_i; 
        //dbg_printf: 
        printf("a_0 = {%X,%X}, ", 
                      (unsigned short) a_i_1, 
                      (unsigned short) a_i_0); 
        printf("b_0 = {%X,%X}, ", 
                      (unsigned short) b_i_1, 
                      (unsigned short) b_i_0); 
        printf("t = {%X,%X}, ", (unsigned short) t_1, 
                                      (unsigned short) t_0); 
        printf("t_i = %X \n\n ", t_i); 
    }
    VR.write(vrt, t); 
}


//!Instruction vminsw behavior method.
// Vector Minimum Signed Word - powerisa spec pag 173. 
void ac_behavior( vminsw ){

    dbg_printf("vminsw v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    for (int i = 0; i <  4; i++){
        int32_t a_i =  (int32_t) a.data[i]; 
        int32_t b_i =  (int32_t) b.data[i]; 
        t.data[i] = a_i < b_i ? a.data[i] : b.data[i]; 
        //dbg_printf: 
        cerr << "a = "   << a_i 
             << ", b = " << b_i
             << ", t = " << (int32_t) t.data[i]
             << endl;
    }
    VR.write(vrt, t); 
}

//!Instruction vminub behavior method.
// Vector Minimum Unsigned Byte - powerisa spec pag 174. 
void ac_behavior( vminub ){

    dbg_printf("vminub v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    for (int i = 0; i < 4; i++){
        int k = 8 ;  //  shifts de sizeof(BYTE). 
        uint8_t a_i_0 =  (uint8_t) ((a.data[i] << 3*k) >> 3*k); 
        uint8_t a_i_1 =  (uint8_t) ((a.data[i] << 2*k) >> 3*k); 
        uint8_t a_i_2 =  (uint8_t) ((a.data[i] << 1*k) >> 3*k); 
        uint8_t a_i_3 =  (uint8_t) ((a.data[i] << 0  ) >> 3*k); 
        uint8_t b_i_0 =  (uint8_t) ((b.data[i] << 3*k) >> 3*k); 
        uint8_t b_i_1 =  (uint8_t) ((b.data[i] << 2*k) >> 3*k); 
        uint8_t b_i_2 =  (uint8_t) ((b.data[i] << 1*k) >> 3*k); 
        uint8_t b_i_3 =  (uint8_t) ((b.data[i] << 0  ) >> 3*k); 
        uint8_t t_0 =  (a_i_0 < b_i_0 ? a_i_0 : b_i_0) ; 
        uint8_t t_1 =  (a_i_1 < b_i_1 ? a_i_1 : b_i_1) ; 
        uint8_t t_2 =  (a_i_2 < b_i_2 ? a_i_2 : b_i_2) ; 
        uint8_t t_3 =  (a_i_3 < b_i_3 ? a_i_3 : b_i_3) ; 
        uint64_t t_i = ((uint64_t)t_3 << 3*k) + ((uint32_t)t_2 << 2*k) 
                     + ((uint16_t)t_1 <<   k) + t_0 ; 
        t.data[i] = t_i; 
        //dbg_printf: 
        printf("a_0 = {%X,%X,%X,%X}, ", 
                     a_i_3, a_i_2, a_i_1, a_i_0); 
        printf("b_0 = {%X,%X,%X,%X}, ",
                     b_i_3, b_i_2, b_i_1, b_i_0); 
        printf("t = {%X,%X,%X,%X}, ", t_3, t_2, t_1, t_0); 
        printf("t_i = %X \n\n ", t_i); 
    }
    VR.write(vrt, t); 
}

//!Instruction vminuh behavior method.
// Vector Minimum Unsigned Halfword - powerisa spec pag 174.
void ac_behavior( vminuh ){
    dbg_printf("vminuh v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    for (int i = 0; i < 4; i++){
        int k = 16 ;  //  shifts de sizeof(HALFWORD). 
        uint16_t a_i_0 =  (uint16_t) ((a.data[i] << k) >> k); 
        uint16_t a_i_1 =  (uint16_t) ((a.data[i] >> k)); 
        uint16_t b_i_0 =  (uint16_t) ((b.data[i] << k) >> k); 
        uint16_t b_i_1 =  (uint16_t) ((b.data[i] >> k)); 
        uint16_t t_0 =  (a_i_0 < b_i_0 ? a_i_0 : b_i_0) ; 
        uint16_t t_1 =  (a_i_1 < b_i_1 ? a_i_1 : b_i_1) ; 
        uint32_t t_i =  ((uint32_t)t_1 << k) + t_0 ; 
        t.data[i] = t_i; 
        //dbg_printf: 
        printf("a_0 = %X, ", a_i_0); 
        printf("a_1 = %X, ", a_i_1); 
        printf("b_0 = %X, ", b_i_0); 
        printf("b_1 = %X, ", b_i_1); 
        printf("t_0 = %X, ", t_0); 
        printf("t_1 = %X, ", t_1); 
        printf("t_i = %X \n\n", t_i); 
    }
    VR.write(vrt, t); 
}

//!Instruction vminuw behavior method.
// Vector Minimum Unsigned Word - powerisa spec pag 174. 
void ac_behavior( vminuw ){
    dbg_printf("vminuw v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    for (int i = 0; i <  4; i++){
        uint32_t  a_i =  (uint32_t) a.data[i]; 
        uint32_t  b_i =  (uint32_t) b.data[i]; 
        t.data[i] = a_i < b_i ? a.data[i] : b.data[i]; 
        //dbg_printf: 
        cerr << "a = "   << a_i 
             << ", b = " << b_i
             << ", t = " << (uint32_t) t.data[i]
             << endl;        
    }
    VR.write(vrt, t); 
}

//!Instruction vrlb behavior method.
void ac_behavior( vrlb )
{
    dbg_printf(" vrlb v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    uint8_t ba, bb, bt;
    uint32_t bt32;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            ba = (uint8_t) (0x000000FF & (a.data[i] >> (8 * j)));
            bb = (uint8_t) (0x00000007 & (b.data[i] >> (8 * j)));

            bt = (ba << bb) | (ba >> (8 - bb));
            
            bt32 = (((uint32_t) bt) & (0x000000FF)) << (8 * j);
            t.data[i] |= bt32;
        }
    }

    VR.write(vrt, t);
}

//!Instruction vrlh behavior method.
void ac_behavior( vrlh )
{
    dbg_printf(" vrlh v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    uint16_t ha, hb, ht;
    uint32_t ht32;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) {
            ha = (uint16_t) (0x0000FFFF & (a.data[i] >> (16 * j)));
            hb = (uint16_t) (0x0000000F & (b.data[i] >> (16 * j)));

            ht = (ha << hb) | (ha >> (16 - hb));
            
            ht32 = (((uint32_t) ht) & (0x0000FFFF)) << (16 * j);
            t.data[i] |= ht32;
        }
    }

    VR.write(vrt, t);
}

//!Instruction vrlw behavior method.
void ac_behavior( vrlw )
{
    dbg_printf(" vrlw v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i;
    uint32_t wa, wb;
    for (i = 0; i < 4; i++) {
        wa = a.data[i];
        wb = b.data[i] & 0x0000001F;

        t.data[i] = (wa << wb) | (wa >> (32 - wb));
    }

    VR.write(vrt, t);
}

//!Instruction vsrab behavior method.
void ac_behavior( vsrab )
{
    dbg_printf(" vsrab v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    int8_t ba, bt;
    uint8_t bb;
    uint32_t bt32;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            ba = (int8_t) (0x000000FF & (a.data[i] >> (8 * j)));
            bb = (uint8_t) (0x00000007 & (b.data[i] >> (8 * j)));

            bt = (ba >> bb);
            
            bt32 = (((uint32_t) bt) & (0x000000FF)) << (8 * j);
            t.data[i] |= bt32;
        }
    }

    VR.write(vrt, t);
}

//!Instruction vsrah behavior method.
void ac_behavior( vsrah ) {
    dbg_printf(" vsrah v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    int16_t ha, ht;
    uint16_t hb;
    uint32_t ht32;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) {
            ha = (int16_t) (0x0000FFFF & (a.data[i] >> (16 * j)));
            hb = (uint16_t) (0x0000000F & (b.data[i] >> (16 * j)));

            ht = (ha >> hb);
            
            ht32 = (((uint32_t) ht) & (0x0000FFFF)) << (16 * j);
            t.data[i] |= ht32;
        }
    }

    VR.write(vrt, t);
}

//!Instruction vsraw behavior method.
void ac_behavior( vsraw )
{
    dbg_printf(" vsraw v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    uint8_t sh;
    int32_t w;
    
    int i;
    for (i = 0; i < 4; i++) {
        sh = (uint8_t) (b.data[i] & 0x0000001F);
        w = a.data[i];
        w >>= sh;
        t.data[i] = (uint32_t) w;
    }

    VR.write(vrt, t);
}

//!Instruction vsrb behavior method.
void ac_behavior( vsrb )
{
    dbg_printf(" vsrb v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    uint8_t ba, bb, bt;
    uint32_t bt32;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            ba = (uint8_t) (0x000000FF & (a.data[i] >> (8 * j)));
            bb = (uint8_t) (0x00000007 & (b.data[i] >> (8 * j)));

            bt = (ba >> bb);
            
            bt32 = (((uint32_t) bt) & (0x000000FF)) << (8 * j);
            t.data[i] |= bt32;
        }
    }

    VR.write(vrt, t);
}

//!Instruction vsrh behavior method.
void ac_behavior( vsrh ) {
    dbg_printf(" vsrh v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    uint16_t ha, hb, ht;
    uint32_t ht32;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) {
            ha = (uint16_t) (0x0000FFFF & (a.data[i] >> (16 * j)));
            hb = (uint16_t) (0x0000000F & (b.data[i] >> (16 * j)));

            ht = (ha >> hb);
            
            ht32 = (((uint32_t) ht) & (0x0000FFFF)) << (16 * j);
            t.data[i] |= ht32;
        }
    }

    VR.write(vrt, t);
}

//!Instruction vsrw behavior method.
void ac_behavior( vsrw )
{
    dbg_printf(" vsrw v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    uint8_t sh;
    
    int i;
    for (i = 0; i < 4; i++) {
        sh = (uint8_t) (b.data[i] & 0x0000001F);
        t.data[i] = a.data[i] >> sh;
        printf("sh=%d a=0x%08X t=0x%08X\n", sh, a.data[i], t.data[i]);
    }

    VR.write(vrt, t);
}

//!Instruction vperm behavior method.
void ac_behavior( vperm ){
    dbg_printf(" vperm v%d, v%d, v%d\n\n", vrt, vra, vrb, vrc);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);
    vec c = VR.read(vrc);

    int i, j;
    uint8_t ba = 0,
            bb = 0,
            bc = 0,
            bt = 0;
    uint32_t bt32 = 0;

    // accesses words and bytes in the right order 
    // (i==0 means word 0, j==0 means byte 0 as in the specs)
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            // we only want bits 3:7 of bc, they are going to index
            // the desired output byte from vra or vrb
            bc = (uint8_t) ((0x1F000000 & (c.data[i] << (8 * j)))>>3*8);

            if ( bc <= 15 ) {
                int wo = bc / 4;
                int by = bc % 4;
                ba = (uint8_t) ((0xFF000000 & (a.data[wo] << (8 * by)))>>3*8);
                bt = ba;
                printf("bc: %02d; wo: %02d; by: %02d; bt: %02X\n",bc,wo,by,bt);
            }
            else {
                int wo = bc / 4;
                int by = bc % 4;
                bb = (uint8_t) ((0xFF000000 & (b.data[wo-4] << (8 * by)))>>3*8);
                bt = bb;
                printf("bc: %02d; wo: %02d; by: %02d; bt: %02X\n",bc,wo,by,bt);
           }

            bt32 = (((uint32_t) bt) & (0x000000FF)) << (8 * (3-j));
            t.data[i] |= bt32;
        }
    }

    VR.write(vrt, t);

}

//!Instruction vsel behavior method.
void ac_behavior( vsel ){
    dbg_printf(" vsel v%d, v%d, v%d, v%d\n\n", vrt, vra, vrb, vrc);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);
    vec c = VR.read(vrc);

    int i;
    uint32_t wa;
    uint32_t wb;
    uint32_t wc;
    uint32_t wt;
    uint32_t mask_a;
    uint32_t mask_b;
    uint32_t ta;
    uint32_t tb;
    for (i = 0; i < 4; i++) {
        wa = a.data[i];
        wb = b.data[i];
        wc = c.data[i];
        mask_a = ~ wc;
        mask_b = wc;
        ta = wa & mask_a;
        tb = wb & mask_b;
        wt = ta | tb;
        printf("wa: %08X; mask_a: %08X; ta: %08X\n"
               "wb: %08X; mask_b: %08X; tb: %08X\n"
               "              mask  : %08X; wt: %08X\n",
                wa,mask_a, ta,
                wb,mask_b, tb,
                wc,wt);
        t.data[i] = wt;
    }

    VR.write(vrt, t);
}

//!Instruction vmhaddshs behavior method.
// Vector Multiply-High-Add Signed Halfword Saturate - powerisa spec pag 163.
void ac_behavior( vmhaddshs ){

    dbg_printf("vmhaddshs v%d, v%d, v%d, v%d\n\n", vrt, vra, vrb, vrc);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);
    vec c = VR.read(vrc);


    for (int i = 0; i < 4; i++){
        int k = 16 ;  //  shifts de sizeof(HALFWORD). 
        int16_t a_i_0  = (int16_t) ((a.data[i] << k) >> k); 
        int16_t a_i_1  = (int16_t) ((a.data[i] >> k)); 
        int16_t b_i_0  = (int16_t) ((b.data[i] << k) >> k); 
        int16_t b_i_1  = (int16_t) ((b.data[i] >> k)); 
        int32_t t_0  = a_i_0*b_i_0; 
        int32_t t_1  = a_i_1*b_i_1; 
        int16_t t_0l = (int16_t)(t_0 >> k); 
        int16_t t_1l = (int16_t)(t_1 >> k); 
        int16_t c_i_0  = (int16_t) ((c.data[i] << k) >> k); 
        int16_t c_i_1  = (int16_t) ((c.data[i] >> k)); 
        int32_t t_i_0 = t_0l + c_i_0; 
        int32_t t_i_1 = t_1l + c_i_1; 
        uint32_t neg_saturated_0 = 0; 
        uint32_t neg_saturated_1 = 0; 
        uint32_t pos_saturated_0 = 0; 
        uint32_t pos_saturated_1 = 0; 
        // 0x7fff = 2^15 - 1 0x8000
        t_i_0 = (t_i_0 > 0x7fff ? (pos_saturated_0 = 0x7fff) : t_i_0); 
        t_i_1 = (t_i_1 > 0x7fff ? (pos_saturated_1 = 0x7fff) : t_i_1); 
        t_i_0 = (abs(t_i_0) > 0x8000 ? 
                 (int32_t)(neg_saturated_0 = 0x8000) : t_i_0); 
        t_i_1 = (abs(t_i_1) > 0x8000 ? 
                 (int32_t)(neg_saturated_1 = 0x8000) : t_i_1); 
        uint32_t t_i = (((uint32_t)t_i_1) << 16) + (uint16_t)t_i_0; 
        t.data[i] = t_i; 

        //debugging info: 
        printf("(%X*%X = %X);(%X*%X = %X)\n",  
                (unsigned short)a_i_1,  
                (unsigned short)b_i_1, 
                (unsigned int)t_1, 
                (unsigned short)a_i_0, 
                (unsigned short)b_i_0, 
                (unsigned int)t_0); 

        printf("(%ld*%ld = %ld);(%ld*%ld = %ld)\n",  
                a_i_1,  
                b_i_1, 
                t_1, 
                a_i_0, 
                b_i_0, 
                t_0); 

        printf("{ t_1l + c_i_1 = t_i_1 (bef. sat.) =  t_i_1 (aft. sat) } =>\n "
               "{ %X + %X = %X = %X} => " 
               "{ %ld + %ld = %ld = %ld }\n", 
               (unsigned short)  t_1l, 
               (unsigned short)c_i_1, 
               (unsigned int)(t_1l + c_i_1), 
               (unsigned int)t_i_1, 
               t_1l, c_i_1, (t_1l + c_i_1) , t_i_1); 
        if( pos_saturated_1 )
            printf("t_i_1 saturated (positive)\n"); 
        if( neg_saturated_1 )
            printf("t_i_1 saturated (negative)\n"); 

        printf("{ t_0l + c_i_0 = t_i_0 (bef. sat.) =  t_i_0 (aft. sat) } =>\n "
               "{ %X + %X = %X = %X } => " 
               "{ %ld + %ld = %ld = %ld }\n", 
               (unsigned short)  t_0l, 
               (unsigned short)c_i_0, 
               (unsigned int)(t_0l + c_i_0), 
               (unsigned int)t_i_0, 
               t_0l, c_i_0, (t_0l + c_i_0), t_i_0); 
        if( pos_saturated_0 )
            printf("t_i_1 saturated (positive)\n"); 
        if( neg_saturated_0 )
            printf("t_i_1 saturated (negative)\n"); 

        printf("t_i = %X = %lu\n", (unsigned long)t_i, t_i); 
        printf("\n"); 

    }
    VR.write(vrt, t); 

}

//!Instruction vmhraddshs behavior method.
// Vector Multiply-High-Round-Add Signed Halfword Saturate - powerisa spec pag 163.
void ac_behavior( vmhraddshs ){

    dbg_printf("vmhraddshs v%d, v%d, v%d, v%d\n\n", vrt, vra, vrb, vrc);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);
    vec c = VR.read(vrc);


    for (int i = 0; i < 4; i++){
        int k = 16 ;  //  shifts de sizeof(HALFWORD). 
        int16_t a_i_0  = (int16_t) ((a.data[i] << k) >> k); 
        int16_t a_i_1  = (int16_t) ((a.data[i] >> k)); 
        int16_t b_i_0  = (int16_t) ((b.data[i] << k) >> k); 
        int16_t b_i_1  = (int16_t) ((b.data[i] >> k)); 
        int32_t t_0  = a_i_0*b_i_0 + 0x4000; 
        int32_t t_1  = a_i_1*b_i_1 + 0x4000; 
        int16_t t_0l = (int16_t)(t_0 >> k); 
        int16_t t_1l = (int16_t)(t_1 >> k); 
        int16_t c_i_0  = (int16_t) ((c.data[i] << k) >> k); 
        int16_t c_i_1  = (int16_t) ((c.data[i] >> k)); 
        int32_t t_i_0 = t_0l + c_i_0; 
        int32_t t_i_1 = t_1l + c_i_1; 
        uint32_t neg_saturated_0 = 0; 
        uint32_t neg_saturated_1 = 0; 
        uint32_t pos_saturated_0 = 0; 
        uint32_t pos_saturated_1 = 0; 
        // 0x7fff = 2^15 - 1 0x8000
        t_i_0 = (t_i_0 > 0x7fff ? (pos_saturated_0 = 0x7fff) : t_i_0); 
        t_i_1 = (t_i_1 > 0x7fff ? (pos_saturated_1 = 0x7fff) : t_i_1); 
        t_i_0 = (abs(t_i_0) > 0x8000 ? 
                 (int32_t)(neg_saturated_0 = 0x8000) : t_i_0); 
        t_i_1 = (abs(t_i_1) > 0x8000 ? 
                 (int32_t)(neg_saturated_1 = 0x8000) : t_i_1); 
        uint32_t t_i = (((uint32_t)t_i_1) << 16) + (uint16_t)t_i_0; 
        t.data[i] = t_i; 

        //debugging info: 
        printf("(%X*%X + 4000 = %X);(%X*%X + 4000 = %X)\n",  
                (unsigned short)a_i_1,  
                (unsigned short)b_i_1, 
                (unsigned int)t_1, 
                (unsigned short)a_i_0, 
                (unsigned short)b_i_0, 
                (unsigned int)t_0); 

        printf("(%ld*%ld + 16384 = %ld);(%ld*%ld + 16384 = %ld)\n",  
                a_i_1,  
                b_i_1, 
                t_1, 
                a_i_0, 
                b_i_0, 
                t_0); 

        printf("{ t_1l + c_i_1 = t_i_1 (bef. sat.) =  t_i_1 (aft. sat) } =>\n "
               "{ %X + %X = %X = %X} => " 
               "{ %ld + %ld = %ld = %ld }\n", 
               (unsigned short)  t_1l, 
               (unsigned short)c_i_1, 
               (unsigned int)(t_1l + c_i_1), 
               (unsigned int)t_i_1, 
               t_1l, c_i_1, (t_1l + c_i_1) , t_i_1); 
        if( pos_saturated_1 )
            printf("t_i_1 saturated (positive)\n"); 
        if( neg_saturated_1 )
            printf("t_i_1 saturated (negative)\n"); 

        printf("{ t_0l + c_i_0 = t_i_0 (bef. sat.) =  t_i_0 (aft. sat) } =>\n "
               "{ %X + %X = %X = %X } => " 
               "{ %ld + %ld = %ld = %ld }\n", 
               (unsigned short)  t_0l, 
               (unsigned short)c_i_0, 
               (unsigned int)(t_0l + c_i_0), 
               (unsigned int)t_i_0, 
               t_0l, c_i_0, (t_0l + c_i_0), t_i_0); 
        if( pos_saturated_0 )
            printf("t_i_1 saturated (positive)\n"); 
        if( neg_saturated_0 )
            printf("t_i_1 saturated (negative)\n"); 

        printf("t_i = %X = %lu\n", (unsigned long)t_i, t_i); 
        printf("\n"); 

    }
    VR.write(vrt, t); 

}
//!Instruction vmladduhm behavior method.
// Vector Multiply-Low-Add Unsigned Halfword Modulo - powerisa spec pag 163.
// note: I'm considering that there is an error in the specification of 
// this instruction:  s/i from 0 to 3/i from 0 to 7/
void ac_behavior( vmladduhm ){

    dbg_printf("vmladduhm v%d, v%d, v%d, v%d\n\n", vrt, vra, vrb, vrc);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);
    vec c = VR.read(vrc);


    for (int i = 0; i < 4; i++){
        int k = 16 ;  //  shifts de sizeof(HALFWORD). 
        uint16_t a_i_0  = (uint16_t) ((a.data[i] << k) >> k); 
        uint16_t a_i_1  = (uint16_t) ((a.data[i] >> k)); 
        uint16_t b_i_0  = (uint16_t) ((b.data[i] << k) >> k); 
        uint16_t b_i_1  = (uint16_t) ((b.data[i] >> k)); 
        uint32_t t_0  = a_i_0*b_i_0; 
        uint32_t t_1  = a_i_1*b_i_1; 
        uint16_t t_0l = (uint16_t)t_0; 
        uint16_t t_1l = (uint16_t)t_1; 
        uint16_t c_i_0  = (uint16_t) ((c.data[i] << k) >> k); 
        uint16_t c_i_1  = (uint16_t) ((c.data[i] >> k)); 
        uint32_t t_i_0 = t_0l + c_i_0; 
        uint32_t t_i_1 = t_1l + c_i_1; 
        uint32_t t_i = (t_i_1 << 16) + (uint16_t)t_i_0; 
        t.data[i] = t_i; 
        //debugging info: 
        printf("(%X*%X = %X);(%X*%X = %X)\n",  
                (unsigned short)a_i_1,  
                (unsigned short)b_i_1, 
                (unsigned int)t_1, 
                (unsigned short)a_i_0, 
                (unsigned short)b_i_0, 
                (unsigned int)t_0); 

        printf("(%lu*%lu = %lu);(%lu*%lu = %lu)\n",  
                a_i_1,  
                b_i_1, 
                t_1, 
                a_i_0, 
                b_i_0, 
                t_0); 

        printf("{ t_1l + c_i_1 = t_i_1 } => "
               "{ %X + %X = %X } => " 
               "{ %lu + %lu = %lu }\n", 
               (unsigned short)  t_1l, 
               (unsigned short)c_i_1, 
               (unsigned int)t_i_1, 
               t_1l, c_i_1, t_i_1); 

        printf("{ t_0l + c_i_0 = t_i_0 } => "
               "{ %X + %X = %X } => " 
               "{ %lu + %lu = %lu }\n", 
               (unsigned short)  t_0l, 
               (unsigned short)c_i_0, 
               (unsigned int)t_i_0, 
               t_0l, c_i_0, t_i_0); 

        printf("t_i = %X = %lu\n", (unsigned long)t_i, t_i); 
        printf("\n"); 
    }
    VR.write(vrt, t); 

}

//!Instruction vmsumubm behavior method.
// Vector Multiply-Sum Unsigned Byte Modulo - powerisa spec pag 164.
void ac_behavior( vmsumubm ){

    dbg_printf("vmsumubm v%d, v%d, v%d, v%d\n\n", vrt, vra, vrb, vrc);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);
    vec c = VR.read(vrc);

    for (int i = 0; i < 4; i++){
        int k = 8 ;  //  shifts de sizeof(BYTE). 
        uint8_t a_i_0 = (uint8_t) ((a.data[i] << 3*k) >> 3*k); 
        uint8_t a_i_1 = (uint8_t) ((a.data[i] << 2*k) >> 3*k); 
        uint8_t a_i_2 = (uint8_t) ((a.data[i] << 1*k) >> 3*k); 
        uint8_t a_i_3 = (uint8_t) ((a.data[i] << 0  ) >> 3*k); 
        uint8_t b_i_0 = (uint8_t) ((b.data[i] << 3*k) >> 3*k); 
        uint8_t b_i_1 = (uint8_t) ((b.data[i] << 2*k) >> 3*k); 
        uint8_t b_i_2 = (uint8_t) ((b.data[i] << 1*k) >> 3*k); 
        uint8_t b_i_3 = (uint8_t) ((b.data[i] << 0  ) >> 3*k); 
        uint16_t t_0  = a_i_0 * b_i_0; 
        uint16_t t_1  = a_i_1 * b_i_1; 
        uint16_t t_2  = a_i_2 * b_i_2; 
        uint16_t t_3  = a_i_3 * b_i_3; 
        uint32_t c_i  = (uint32_t) c.data[i]; 
        uint32_t t_i  =  t_0 + t_1 + t_2 + t_3 + c_i; 
        t.data[i]     = (uint32_t)t_i; 
        //debug information:
        printf("(%X*%X);(%X*%X);(%X*%X);(%X*%X)\n",  
                (unsigned char)a_i_3,  
                (unsigned char)b_i_3, 
                (unsigned char)a_i_2,  
                (unsigned char)b_i_2, 
                (unsigned char)a_i_1,  
                (unsigned char)b_i_1, 
                (unsigned char)a_i_0, 
                (unsigned char)b_i_0); 

        printf("(%lu*%lu);(%lu*%lu);(%lu*%lu);(%lu*%lu)\n",  
                a_i_3,  
                b_i_3, 
                a_i_2,  
                b_i_2, 
                a_i_1,  
                b_i_1, 
                a_i_0, 
                b_i_0); 
        
        printf(" t_3 + t_2 + t_1 + t_0 + ci ="
                " %X +  %X +  %X +  %X + %X ="
                "t_i  = %X \n",  
                (unsigned short)t_3, 
                (unsigned short)t_2, 
                (unsigned short)t_1, 
                (unsigned short)t_0, 
                (unsigned int)  c_i, 
                (unsigned int)  t_i); 

        printf(" t_3 + t_2 + t_1 + t_0 + ci ="
                " %lu +  %lu +  %lu + %lu + %lu ="
                "t_i  = %lu \n",  
                (short)t_3, 
                (short)t_2, 
                (short)t_1, 
                (short)t_0, 
                (int)  c_i, 
                (int)  t_i); 
        printf("\n"); 
    }
    VR.write(vrt, t); 
}

//!Instruction vmsummbm behavior method.
// Vector Multiply-Sum Mixed Byte Modulo - powerisa spec pag 164.
void ac_behavior( vmsummbm ){

    dbg_printf("vmsummbm v%d, v%d, v%d, v%d\n\n", vrt, vra, vrb, vrc);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);
    vec c = VR.read(vrc);

    for (int i = 0; i < 4; i++){
        int k = 8 ;  //  shifts de sizeof(BYTE). 
        int8_t  a_i_0 = (int8_t)  ((a.data[i] << 3*k) >> 3*k); 
        int8_t  a_i_1 = (int8_t)  ((a.data[i] << 2*k) >> 3*k); 
        int8_t  a_i_2 = (int8_t)  ((a.data[i] << 1*k) >> 3*k); 
        int8_t  a_i_3 = (int8_t)  ((a.data[i] << 0  ) >> 3*k); 
        uint8_t b_i_0 = (uint8_t) ((b.data[i] << 3*k) >> 3*k); 
        uint8_t b_i_1 = (uint8_t) ((b.data[i] << 2*k) >> 3*k); 
        uint8_t b_i_2 = (uint8_t) ((b.data[i] << 1*k) >> 3*k); 
        uint8_t b_i_3 = (uint8_t) ((b.data[i] << 0  ) >> 3*k); 
        int16_t t_0   = a_i_0 * b_i_0; 
        int16_t t_1   = a_i_1 * b_i_1; 
        int16_t t_2   = a_i_2 * b_i_2; 
        int16_t t_3   = a_i_3 * b_i_3; 
        int32_t c_i   = (int32_t) c.data[i]; 
        int32_t t_i   =  t_0 + t_1 + t_2 + t_3 + c_i; 
        t.data[i]     = (uint32_t)t_i; 
        //debug information:
        printf("(%X*%X);(%X*%X);(%X*%X);(%X*%X)\n",  
                (unsigned char)a_i_3,  
                (unsigned char)b_i_3, 
                (unsigned char)a_i_2,  
                (unsigned char)b_i_2, 
                (unsigned char)a_i_1,  
                (unsigned char)b_i_1, 
                (unsigned char)a_i_0, 
                (unsigned char)b_i_0); 

        printf("(%ld*%lu);(%ld*%lu);(%ld*%lu);(%ld*%lu)\n",  
                a_i_3,  
                b_i_3, 
                a_i_2,  
                b_i_2, 
                a_i_1,  
                b_i_1, 
                a_i_0, 
                b_i_0); 
        
        printf(" t_3 + t_2 + t_1 + t_0 + ci ="
                " %X +  %X +  %X +  %X + %X ="
                "t_i  = %X \n",  
                (unsigned short)t_3, 
                (unsigned short)t_2, 
                (unsigned short)t_1, 
                (unsigned short)t_0, 
                (unsigned int)  c_i, 
                (unsigned int)  t_i); 

        printf(" t_3 + t_2 + t_1 + t_0 + ci ="
                " %ld +  %ld +  %ld + %ld + %ld ="
                "t_i  = %ld \n",  
                (short)t_3, 
                (short)t_2, 
                (short)t_1, 
                (short)t_0, 
                (int)  c_i, 
                (int)  t_i); 
        printf("\n"); 
    }
    VR.write(vrt, t); 
}

//!Instruction vmsumshm behavior method.
// Vector Multiply-Sum Signed Halfword Modulo - powerisa spec pag 166.
void ac_behavior( vmsumshm ){

    dbg_printf("vmsumshm v%d, v%d, v%d, v%d\n\n", vrt, vra, vrb, vrc);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);
    vec c = VR.read(vrc);


    for (int i = 0; i < 4; i++){
        int k = 16 ;  //  shifts de sizeof(HALFWORD). 
        int16_t a_i_0  = (int16_t) ((a.data[i] << k) >> k); 
        int16_t a_i_1  = (int16_t) ((a.data[i] >> k)); 
        int16_t b_i_0  = (int16_t) ((b.data[i] << k) >> k); 
        int16_t b_i_1  = (int16_t) ((b.data[i] >> k)); 
        int64_t a_i_0s = a_i_0;
        int64_t a_i_1s = a_i_1;
        int64_t b_i_0s = b_i_0;
        int64_t b_i_1s = b_i_1;
        int32_t c_i = (int32_t)c.data[i]; 
        int64_t t_l =  a_i_0s*b_i_0s; 
        int64_t t_h =  a_i_1s*b_i_1s; 
        int64_t t_i_i = ((int64_t)t_l) + ((int64_t)t_h) + ((int64_t)c_i); 
        bool pos_saturated = false; 
        bool neg_saturated = false; 
        if ( t_i_i >= 0){
            if(t_i_i < 0) 
                printf("* ERROR! WRONG! pos and neg!\n"); 
            // 0x7fff_ffff  == 2^31 - 1 
            pos_saturated = (t_i_i >  0x7fffffff); 
        }
        else
            // -0x8000_0000 ==  -2^31
            neg_saturated = (abs(t_i_i) > 0x80000000); 
        bool saturated = pos_saturated || neg_saturated; 
        t_i_i = (saturated ? 
                            (pos_saturated ? 
                               t_i_i - 0x80000000: t_i_i + 0x80000001) 
                            : t_i_i); 
        uint32_t t_i = (uint32_t)t_i_i;
        t.data[i] = t_i; 

        //debug information: 
        printf("(%X*%X);(%X*%X)\n",  
                (unsigned int)a_i_1s,  
                (unsigned int)b_i_1s, 
                (unsigned int)a_i_0s, 
                (unsigned int)b_i_0s); 

        if(saturated){
            if(pos_saturated) printf("pos "); 
            if(neg_saturated) printf("neg "); 
            printf("saturated (so it must be moduled).\n"); 
            if(pos_saturated && neg_saturated) printf("ERROR! WRONG! pos and neg!\n"); 
        }else  printf("not saturated.\n");
        //don't know how to print 64bit hexas... 
       printf("t_h + t_l + c_i = t_i_i; t_i => {%X + %X + %X = %X; %X}.\n\n", 
       //printf("t_h + t_l + c_i = t_i_i; t_i => {%ld + %ld + %ld = %ld; %lu}.\n\n", 
               (signed int)t_h, 
               (signed int)t_l, 
               (signed int)c_i, 
               (signed int)t_i_i, 
               (unsigned int)t_i); 
    }
    VR.write(vrt, t); 

}


//!Instruction vmsumshs behavior method.
// Vector Multiply-Sum Signed Halfword Saturate - powerisa spec pag 166.
void ac_behavior( vmsumshs ){

    dbg_printf("vmsumshs v%d, v%d, v%d, v%d\n\n", vrt, vra, vrb, vrc);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);
    vec c = VR.read(vrc);

    for (int i = 0; i < 4; i++){
        int k = 16 ;  //  shifts de sizeof(HALFWORD). 
        int16_t a_i_0  = (int16_t) ((a.data[i] << k) >> k); 
        int16_t a_i_1  = (int16_t) ((a.data[i] >> k)); 
        int16_t b_i_0  = (int16_t) ((b.data[i] << k) >> k); 
        int16_t b_i_1  = (int16_t) ((b.data[i] >> k)); 
        int64_t a_i_0s = a_i_0;
        int64_t a_i_1s = a_i_1;
        int64_t b_i_0s = b_i_0;
        int64_t b_i_1s = b_i_1;
        int32_t c_i = (int32_t)c.data[i]; 
        int64_t t_l =  a_i_0s*b_i_0s; 
        int64_t t_h =  a_i_1s*b_i_1s; 
        int64_t t_i_i = ((int64_t)t_l) + ((int64_t)t_h) + ((int64_t)c_i); 
        bool pos_saturated = false; 
        bool neg_saturated = false; 
        printf("t_i_i = %ld.\n" , t_i_i); 
        /*FIXME: WORRIED! With the test vmsumshs-1 I got this piece of code: 
        if(t_i_i < -2147483648){
            printf(" %ld is lesser then -2147483648.\n" , t_i_i);
        }
        to show this:
        "-2147483648 is lesser then -2147483648"
        something related to this warning: 
        powerpc_isa.cpp:4619: warning: this decimal constant is unsigned only in ISO C90
        */
        if ( t_i_i >= 0){
            if(t_i_i < 0) 
                printf("* ERROR! WRONG! pos and neg!\n"); 
            // 0x7fff_ffff  == 2^31 - 1 
            pos_saturated = (t_i_i >  0x7fffffff); 
        }
        else
            // -0x8000_0000 ==  -2^31
            neg_saturated = (abs(t_i_i) > 0x80000000); 
            //FIXME: strangely this doesn't work, can anyone explain:
            //neg_saturated = (t_i_i < -1*0x80000000); 
            //neither this: 
            //neg_saturated = (t_i_i < -2147483648); 
            //see the fixme before. 
        bool saturated = pos_saturated || neg_saturated; 
        uint32_t t_i = 
                (uint32_t)(saturated ? 
                            (pos_saturated ? 0x7fffffff: 0x80000000) 
                            : t_i_i); 
        t.data[i] = t_i; 
        if(saturated){
            //FIXME: undeclared function. 
            //altivec_mark_saturation(); 
        }
        //debug information: 

        printf("(%X*%X);(%X*%X)\n",  
                /*
                (unsigned short)a_i_1,  
                (unsigned short)b_i_1, 
                (unsigned short)a_i_0, 
                (unsigned short)b_i_0); 
                */
                (unsigned int)a_i_1s,  
                (unsigned int)b_i_1s, 
                (unsigned int)a_i_0s, 
                (unsigned int)b_i_0s); 
        if(saturated){
            if(pos_saturated) printf("pos "); 
            if(neg_saturated) printf("neg "); 
            printf("saturated.\n"); 
            if(pos_saturated && neg_saturated) printf("ERROR! WRONG! pos and neg!\n"); 
        }else  printf("not saturated.\n");
        if(t_i_i)
        //don't know how to print 64bit hexas... 
        printf("t_h + t_l + c_i = t_i_i; t_i => {%X + %X + %X = %X + 1; %X}.\n\n", 
                /*
                (unsigned int)t_h, 
                (unsigned int)t_l, 
                (unsigned int)c_i, 
                (unsigned long)t_i_i - 1, 
                (unsigned int)t_i); 
                */
                (int)t_h, 
                (int)t_l, 
                (int)c_i, 
                (long)t_i_i - 1, 
                (int)t_i); 
        else
        printf("t_h + t_l + c_i = t_i_i; t_i => {%X + %X + %X = zero; %X}.\n\n", 
                /*
                (unsigned int)t_h, 
                (unsigned int)t_l, 
                (unsigned int)c_i, 
                (unsigned int)t_i); 
                */
                (int)t_h, 
                (int)t_l, 
                (int)c_i, 
                (int)t_i); 
    }
    VR.write(vrt, t); 
}

//!Instruction vmsumuhm behavior method.
// Vector Multiply-Sum Unsigned Halfword Modulo - powerisa spec pag 166.
void ac_behavior( vmsumuhm ){

    dbg_printf("vmsumuhm v%d, v%d, v%d, v%d\n\n", vrt, vra, vrb, vrc);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);
    vec c = VR.read(vrc);

    for (int i = 0; i < 4; i++){
        int k = 16 ;  //  shifts de sizeof(HALFWORD). 
        uint16_t a_i_0 =  (uint16_t) ((a.data[i] << k) >> k); 
        uint16_t a_i_1 =  (uint16_t) ((a.data[i] >> k)); 
        uint16_t b_i_0 =  (uint16_t) ((b.data[i] << k) >> k); 
        uint16_t b_i_1 =  (uint16_t) ((b.data[i] >> k)); 
        uint32_t c_i = c.data[i]; 
        uint32_t t_l =   (uint32_t)a_i_0*(uint32_t)b_i_0 ; 
        uint32_t t_h =   (uint32_t)a_i_1*(uint32_t)b_i_1 ; 
        int32_t t_i = t_l + t_h + c_i; 
        t.data[i] = t_i; 
        //dbg_printf: 
        printf("(%X*%X),(%X*%X)\n",  
                (unsigned short)a_i_1,  
                (unsigned short)b_i_1, 
                (unsigned short)a_i_0, 
                (unsigned short)b_i_0); 
        printf("t_h + t_l + c_i = t_i => {%X + %X + %X = %X} \n\n", 
                (unsigned int)t_h, 
                (unsigned int)t_l, 
                (unsigned int)c_i, 
                (unsigned int)t_i); 
    }
    VR.write(vrt, t); 
}

//!Instruction vmsumuhs behavior method.
// Vector Multiply-Sum Unsigned Halfword Saturate - powerisa spec pag 166.
void ac_behavior( vmsumuhs ){

    dbg_printf("vmsumuhs v%d, v%d, v%d, v%d\n\n", vrt, vra, vrb, vrc);

    vec t; 
    vec a = VR.read(vra);
    vec b = VR.read(vrb);
    vec c = VR.read(vrc);

    for (int i = 0; i < 4; i++){
        int k = 16 ;  //  shifts de sizeof(HALFWORD). 
        uint16_t a_i_0 =  (uint16_t) ((a.data[i] << k) >> k); 
        uint16_t a_i_1 =  (uint16_t) ((a.data[i] >> k)); 
        uint16_t b_i_0 =  (uint16_t) ((b.data[i] << k) >> k); 
        uint16_t b_i_1 =  (uint16_t) ((b.data[i] >> k)); 
        uint32_t c_i = c.data[i]; 
        uint32_t t_l =  (uint32_t)a_i_0*(uint32_t)b_i_0; 
        uint32_t t_h =  (uint32_t)a_i_1*(uint32_t)b_i_1; 
        uint64_t t_i_i = ((uint64_t)t_l) + ((uint64_t)t_h) + ((uint64_t)c_i); 
        // 0xffff_ffff == 2^32
        bool saturated = (t_i_i > 0xffffffff); 
        uint32_t t_i = (uint32_t)(saturated ? 0xffffffff : t_i_i); 
        t.data[i] = t_i; 
        if(saturated){
            //FIXME: undeclared function. 
            //altivec_mark_saturation(); 
        }
        //dbg_printf: 
        
        printf("(%X*%X),(%X*%X)\n",  
                (unsigned short)a_i_1,  
                (unsigned short)b_i_1, 
                (unsigned short)a_i_0, 
                (unsigned short)b_i_0); 
        if(saturated){
            printf("saturated.\n"); 
        }
        if(t_i_i)
        //don't know how to print 64bit hexas... 
        printf("t_h + t_l + c_i = t_i_i; t_i => {%X + %X + %X = %X + 1; %X}.\n\n", 
                (unsigned int)t_h, 
                (unsigned int)t_l, 
                (unsigned int)c_i, 
                (unsigned long)t_i_i - 1, 
                (unsigned int)t_i); 
        else
        printf("t_h + t_l + c_i = t_i_i; t_i => {%X + %X + %X = zero; %X}.\n\n", 
                (unsigned int)t_h, 
                (unsigned int)t_l, 
                (unsigned int)c_i, 
                (unsigned int)t_i); 
    }
    VR.write(vrt, t); 
}

//!Instruction vsldoi behavior method.
void ac_behavior( vsldoi ){}

//!Instruction vslb behavior method.
void ac_behavior( vslb )
{
    dbg_printf(" vslb v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    uint8_t ba, bb, bt;
    uint32_t bt32;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            ba = (uint8_t) (0x000000FF & (a.data[i] >> (8 * j)));
            bb = (uint8_t) (0x00000007 & (b.data[i] >> (8 * j)));

            bt = (ba << bb);
            
            bt32 = (((uint32_t) bt) & (0x000000FF)) << (8 * j);
            t.data[i] |= bt32;
        }
    }

    VR.write(vrt, t);
}

//!Instruction vslh behavior method.
void ac_behavior( vslh )
{
    dbg_printf(" vslh v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i, j;
    uint16_t ha, hb, ht;
    uint32_t ht32;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) {
            ha = (uint16_t) (0x0000FFFF & (a.data[i] >> (16 * j)));
            hb = (uint16_t) (0x0000000F & (b.data[i] >> (16 * j)));

            ht = (ha << hb);
            
            ht32 = (((uint32_t) ht) & (0x0000FFFF)) << (16 * j);
            t.data[i] |= ht32;
        }
    }

    VR.write(vrt, t);
}

//!Instruction vslw behavior method.
void ac_behavior( vslw ){
    dbg_printf(" vslw v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    uint8_t sh;
    
    int i;
    for (i = 0; i < 4; i++) {
        sh = (uint8_t) (b.data[i] & 0x0000001F);
        t.data[i] = a.data[i] << sh;
        printf("sh=%d a=0x%08X t=0x%08X\n", sh, a.data[i], t.data[i]);
    }

    VR.write(vrt, t);
}

//!Instruction vand behavior method.
void ac_behavior( vand ){
    dbg_printf(" vand v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i;
    for (i = 0; i < 4; i++) {
        t.data[i] = a.data[i] & b.data[i];
    }

    VR.write(vrt, t);
}

//!Instruction vandc behavior method.
void ac_behavior( vandc ){
    dbg_printf(" vandc v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i;
    for (i = 0; i < 4; i++) {
        t.data[i] = a.data[i] & ~(b.data[i]);
    }

    VR.write(vrt, t);
}

//!Instruction vnor behavior method.
void ac_behavior( vnor ){
    dbg_printf(" vnor v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i;
    for (i = 0; i < 4; i++) {
        t.data[i] = ~(a.data[i] | b.data[i]);
    }

    VR.write(vrt, t);
}

//!Instruction vor behavior method.
void ac_behavior( vor ){
    dbg_printf(" vor v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i;
    for (i = 0; i < 4; i++) {
        t.data[i] = a.data[i] | b.data[i];
    }

    VR.write(vrt, t);
}

//!Instruction vxor behavior method.
void ac_behavior( vxor ){
    dbg_printf(" vxor v%d, v%d, v%d\n\n", vrt, vra, vrb);

    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);

    int i;
    for (i = 0; i < 4; i++) {
        t.data[i] = a.data[i] ^ b.data[i];
    }

    VR.write(vrt, t);
}

//!Instruction vaddfp behavior method.
void ac_behavior( vaddfp ){}

//!Instruction vsubfp behavior method.
void ac_behavior( vsubfp ){}

//!Instruction vmaxfp behavior method.
void ac_behavior( vmaxfp ){}

//!Instruction vinfp behavior method.
void ac_behavior( vinfp ){}

//!Instruction vctsxs behavior method.
void ac_behavior( vctsxs ){}

//!Instruction vctuxs behavior method.
void ac_behavior( vctuxs ){}

//!Instruction vcfsx behavior method.
void ac_behavior( vcfsx ){}

//!Instruction vcfux behavior method.
void ac_behavior( vcfux ){}

//!Instruction vrfim behavior method.
void ac_behavior( vrfim ){}

//!Instruction vrfin behavior method.
void ac_behavior( vrfin ){}

//!Instruction vrfip behavior method.
void ac_behavior( vrfip ){}

//!Instruction vrfiz behavior method.
void ac_behavior( vrfiz ){}

//!Instruction vexptefp behavior method.
void ac_behavior( vexptefp ){}

//!Instruction vlogefp behavior method.
void ac_behavior( vlogefp ){}

//!Instruction vrefp behavior method.
void ac_behavior( vrefp ){}

//!Instruction vsqrtefp behavior method.
void ac_behavior( vsqrtefp ){}

//!Instruction mtvscr behavior method.
void ac_behavior( mtvscr ) {
    dbg_printf(" mtvscr v%d\n\n", vrb);

    vec b = VR.read(vrb);

    VSCR.write(b.data[3]);
    printf("mtvscr: isto => %X\n", b.data[3]);
}

//!Instruction mfvscr behavior method.
void ac_behavior( mfvscr ) {
    dbg_printf(" mfvscr v%d\n\n", vrt);

    printf("mfvscr: isto => %X\n", VSCR.read());

    vec t(0);
    t.data[3] = VSCR.read();
    VR.write(vrt, t);
}

//!Instruction vmaddfp behavior method.
void ac_behavior( vmaddfp ){}

//!Instruction vmsubfp behavior method.
void ac_behavior( vmsubfp ){}

void inline vcmpequb_impl(ac_reg<ac_word> &CR, vecbank &VR, int update_cr6, int vrt, int vra, int vrb)
{
    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);
    
    int i, j;
    uint8_t ba, bb, bt;
    uint32_t bt32;
    int remaining_eq = 16;
    int remaining_ne = 16;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            ba = (uint8_t) (0x000000FF & (a.data[i] >> (8 * j)));
            bb = (uint8_t) (0x000000FF & (b.data[i] >> (8 * j)));
         
            bt = (ba == bb);

            if (bt) {
                remaining_eq--;
            } else {
                remaining_ne--;
            }

            bt32 = (((uint32_t) bt) & (0x000000FF)) << (8 * j);
            t.data[i] |= bt32;
        }
    }

    VR.write(vrt, t);
    
    if (update_cr6) CR6_update(CR, !remaining_eq, !remaining_ne);
}

//!Instruction vcmpequb behavior method.
void ac_behavior( vcmpequb ) {
    dbg_printf(" vcmpequb v%d, v%d, v%d\n\n", vrt, vra, vrb);
    vcmpequb_impl(CR, VR, 0, vrt, vra, vrb);
}

//!Instruction vcmpequb_ behavior method.
void ac_behavior( vcmpequb_ ) {
    dbg_printf(" vcmpequb. v%d, v%d, v%d\n\n", vrt, vra, vrb);
    vcmpequb_impl(CR, VR, 1, vrt, vra, vrb);
}

void inline vcmpequh_impl(ac_reg<ac_word> &CR, vecbank &VR, int update_cr6, int vrt, int vra, int vrb)
{
    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);
    
    int i, j;
    uint16_t ha, hb, ht;
    uint32_t ht32;
    int remaining_eq = 8;
    int remaining_ne = 8;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) {
            ha = (uint16_t) (0x0000FFFF & (a.data[i] >> (16 * j)));
            hb = (uint16_t) (0x0000FFFF & (b.data[i] >> (16 * j)));
          
            ht = (ha == hb);

            if (ht) {
                remaining_eq--;
            } else {
                remaining_ne--;
            }

            ht32 = (((uint32_t) ht) & 0x0000FFFF) << (16 * j);
            t.data[i] |= ht32;
        }
    }

    VR.write(vrt, t);
    
    if (update_cr6) CR6_update(CR, !remaining_eq, !remaining_ne);
}

//!Instruction vcmpequh behavior method.
void ac_behavior( vcmpequh ) {
    dbg_printf(" vcmpequh v%d, v%d, v%d\n\n", vrt, vra, vrb);
    vcmpequh_impl(CR, VR, 0, vrt, vra, vrb);
}

//!Instruction vcmpequh_ behavior method.
void ac_behavior( vcmpequh_ ) {
    dbg_printf(" vcmpequh. v%d, v%d, v%d\n\n", vrt, vra, vrb);
    vcmpequh_impl(CR, VR, 1, vrt, vra, vrb);
}

void inline vcmpequw_impl(ac_reg<ac_word> &CR, vecbank &VR, int update_cr6, int vrt, int vra, int vrb)
{
    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);
    
    int i;
    uint32_t equal;
    int remaining_eq = 4;
    int remaining_ne = 4;

    for (i = 0; i < 4; i++) {
        equal = (a.data[i] == b.data[i]);

        if (equal) {
            remaining_eq--;
        } else {
            remaining_ne--;
        }

        t.data[i] = equal;
    }
    
    VR.write(vrt, t);
    if (update_cr6) CR6_update(CR, !remaining_eq, !remaining_ne);
}


//!Instruction vcmpequw behavior method.
void ac_behavior( vcmpequw )
{
    dbg_printf(" vcmpequw v%d, v%d, v%d\n\n", vrt, vra, vrb);
    vcmpequw_impl(CR, VR, 0, vrt, vra, vrb);
}

//!Instruction vcmpequw_ behavior method.
void ac_behavior( vcmpequw_ )
{
    dbg_printf(" vcmpequw. v%d, v%d, v%d\n\n", vrt, vra, vrb);
    vcmpequw_impl(CR, VR, 1, vrt, vra, vrb);
}

//!Instruction vcmpgtsb behavior method.
void ac_behavior( vcmpgtsb ){}

//!Instruction vcmpgtsh behavior method.
void ac_behavior( vcmpgtsh ){}

//!Instruction vcmpgtsw behavior method.
void ac_behavior( vcmpgtsw ){}

//!Instruction vcmpgtsb_ behavior method.
void ac_behavior( vcmpgtsb_ ){}

//!Instruction vcmpgtsh_ behavior method.
void ac_behavior( vcmpgtsh_ ){}

//!Instruction vcmpgtsw_ behavior method.
void ac_behavior( vcmpgtsw_ ){}

void inline vcmpgtub_impl(ac_reg<ac_word> &CR, vecbank &VR, int update_cr6, int vrt, int vra, int vrb)
{
    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);
    
    int i, j;
    uint8_t ba, bb, bt;
    uint32_t bt32;
    int remaining_gt = 16;
    int remaining_le = 16;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            ba = (uint8_t) (0x000000FF & (a.data[i] >> (8 * j)));
            bb = (uint8_t) (0x000000FF & (b.data[i] >> (8 * j)));
         
            bt = (ba > bb);

            if (bt) {
                remaining_gt--;
            } else {
                remaining_le--;
            }

            bt32 = (((uint32_t) bt) & (0x000000FF)) << (8 * j);
            t.data[i] |= bt32;
        }
    }

    VR.write(vrt, t);
    
    if (update_cr6) CR6_update(CR, !remaining_gt, !remaining_le);
}

//!Instruction vcmpgtqub behavior method.
void ac_behavior( vcmpgtub ) {
    dbg_printf(" vcmpgtub v%d, v%d, v%d\n\n", vrt, vra, vrb);
    vcmpgtub_impl(CR, VR, 0, vrt, vra, vrb);
}

//!Instruction vcmpgtub_ behavior method.
void ac_behavior( vcmpgtub_ ) {
    dbg_printf(" vcmpgtub. v%d, v%d, v%d\n\n", vrt, vra, vrb);
    vcmpgtub_impl(CR, VR, 1, vrt, vra, vrb);
}

void inline vcmpgtuh_impl(ac_reg<ac_word> &CR, vecbank &VR, int update_cr6, int vrt, int vra, int vrb)
{
    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);
    
    int i, j;
    uint16_t ha, hb, ht;
    uint32_t ht32;
    int remaining_gt = 8;
    int remaining_le = 8;

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) {
            ha = (uint16_t) (0x0000FFFF & (a.data[i] >> (16 * j)));
            hb = (uint16_t) (0x0000FFFF & (b.data[i] >> (16 * j)));
          
            ht = (ha > hb);

            if (ht) {
                remaining_gt--;
            } else {
                remaining_le--;
            }

            ht32 = (((uint32_t) ht) & 0x0000FFFF) << (16 * j);
            t.data[i] |= ht32;
        }
    }

    VR.write(vrt, t);
    
    if (update_cr6) CR6_update(CR, !remaining_gt, !remaining_le);
}

//!Instruction vcmpgtuh behavior method.
void ac_behavior( vcmpgtuh ) {
    dbg_printf(" vcmpgtuh v%d, v%d, v%d\n\n", vrt, vra, vrb);
    vcmpgtuh_impl(CR, VR, 0, vrt, vra, vrb);
}

//!Instruction vcmpgtuh_ behavior method.
void ac_behavior( vcmpgtuh_ ) {
    dbg_printf(" vcmpgtuh. v%d, v%d, v%d\n\n", vrt, vra, vrb);
    vcmpgtuh_impl(CR, VR, 1, vrt, vra, vrb);
}

void inline vcmpgtuw_impl(ac_reg<ac_word> &CR, vecbank &VR, int update_cr6, int vrt, int vra, int vrb)
{
    vec t(0);
    vec a = VR.read(vra);
    vec b = VR.read(vrb);
    
    int i;
    uint32_t greater;
    int remaining_gt = 4;
    int remaining_le = 4;

    for (i = 0; i < 4; i++) {
        greater = (a.data[i] > b.data[i]);

        if (greater) {
            remaining_gt--;
        } else {
            remaining_le--;
        }

        t.data[i] = greater;
    }
    
    VR.write(vrt, t);
    if (update_cr6) CR6_update(CR, !remaining_gt, !remaining_le);
}


//!Instruction vcmpgtuw behavior method.
void ac_behavior( vcmpgtuw )
{
    dbg_printf(" vcmpgtuw v%d, v%d, v%d\n\n", vrt, vra, vrb);
    vcmpgtuw_impl(CR, VR, 0, vrt, vra, vrb);
}

//!Instruction vcmpgtuw_ behavior method.
void ac_behavior( vcmpgtuw_ )
{
    dbg_printf(" vcmpgtuw. v%d, v%d, v%d\n\n", vrt, vra, vrb);
    vcmpgtuw_impl(CR, VR, 1, vrt, vra, vrb);
}

//!Instruction vcmpbfp behavior method.
void ac_behavior( vcmpbfp ){}

//!Instruction vcmpeqfp behavior method.
void ac_behavior( vcmpeqfp ){}

//!Instruction vcmpgefp behavior method.
void ac_behavior( vcmpgefp ){}

//!Instruction vcmpgtfp behavior method.
void ac_behavior( vcmpgtfp ){}

//!Instruction vcmpbfp_ behavior method.
void ac_behavior( vcmpbfp_ ){}

//!Instruction vcmpeqfp_ behavior method.
void ac_behavior( vcmpeqfp_ ){}

//!Instruction vcmpgefp_ behavior method.
void ac_behavior( vcmpgefp_ ){}

//!Instruction vcmpgtfp_ behavior method.
void ac_behavior( vcmpgtfp_ ){}




// vim: sw=4 ts=4 sts=4 et
