#include "core.h"

//#define  DEBUG_GPU

#define J_ALWAYS        0x00
#define J_NZ            0x01
#define J_Z             0x02
#define J_NC            0x04
#define J_NC_NZ         0x05
#define J_NC_Z          0x06
#define J_C             0x08
#define J_C_NZ          0x09
#define J_C_Z           0x0A
#define J_NN            0x14
#define J_NN_NZ         0x15
#define J_NN_Z          0x16
#define J_N             0x18
#define J_N_NZ          0x19
#define J_N_Z           0x1A
#define J_NEVER         0x1F

   // Opcode Function Prototypes

   void G_ADD( void );          void G_ADDC( void );            
   void G_ADDQ( void );         void G_ADDQT( void );
   void G_SUB( void );          void G_SUBC( void );
   void G_SUBQ( void );         void G_SUBQT( void );
   void G_NEG( void );          void G_AND( void );
   void G_OR( void );           void G_XOR( void );
   void G_NOT( void );          void G_BTST( void );
   void G_BSET( void );         void G_BCLR( void );
   void G_MULT( void );         void G_IMULT( void );
   void G_IMULTN( void );       void G_RESMAC( void );
   void G_IMACN( void );        void G_DIV( void );
   void G_ABS( void );          void G_SH( void );
   void G_SHLQ( void );         void G_SHRQ( void );
   void G_SHA( void );          void G_SHARQ( void );
   void G_ROR( void );          void G_RORQ( void );
   void G_CMP( void );          void G_CMPQ( void );
   void G_SAT8( void );         void G_SAT16( void );        
   void G_MOVE( void );         void G_MOVEQ( void );
   void G_MOVETA( void );       void G_MOVEFA( void );
   void G_MOVEI( void );        void G_LOADB( void );
   void G_LOADW( void );        void G_LOAD( void );
   void G_LOADP( void );        void G_STORE_14I( void );
   void G_LOAD_14I( void );     void G_LOAD_15I( void );
   void G_STOREB( void );       void G_STOREW( void );
   void G_STORE( void );        void G_STOREP( void );
   void G_STORE_15I( void );    void G_MOVE_PC( void );
   void G_JUMP( void );         void G_JR( void );
   void G_MMULT( void );        void G_MTOI( void );
   void G_NORMI( void );        void G_NOP( void );
   void G_LOAD_14R( void );     void G_LOAD_15R( void );
   void G_STORE_14R( void );    void G_STORE_15R( void );
   void G_SAT24( void );        void G_PACK_UNPACK( void );

   // Other Prototypes

   unsigned mem_readword( unsigned );
   unsigned mem_readbyte( unsigned );
   void mem_writeword( unsigned, unsigned );
   void mem_writebyte( unsigned, unsigned );

   // CPU Cycles for Each GPU Opcode

   byte gpuCycles[64] = {
      3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
      3,  3,  1,  3,  1, 18,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
      3,  3,  2,  2,  2,  2,  3,  4,  5,  4,  5,  6,  6,  1,  1,  1,
      1,  2,  2,  2,  1,  1,  9,  3,  3,  0,  6,  6,  2,  2,  3,  3

   };

   // GPU Opcode Function Table

   void (*gpuOp[64])()= {	
      G_ADD,        G_ADDC,        G_ADDQ,       G_ADDQT,
      G_SUB,        G_SUBC,        G_SUBQ,       G_SUBQT,
      G_NEG,        G_AND,         G_OR,         G_XOR,
      G_NOT,        G_BTST,        G_BSET,       G_BCLR,
      G_MULT,       G_IMULT,       G_IMULTN,     G_RESMAC,
      G_IMACN,      G_DIV,         G_ABS,        G_SH,
      G_SHLQ,       G_SHRQ,        G_SHA,        G_SHARQ,
      G_ROR,        G_RORQ,        G_CMP,        G_CMPQ,
      G_SAT8,       G_SAT16,       G_MOVE,       G_MOVEQ,
      G_MOVETA,     G_MOVEFA,      G_MOVEI,      G_LOADB,
      G_LOADW,      G_LOAD,        G_LOADP,      G_LOAD_14I,
      G_LOAD_15I,   G_STOREB,      G_STOREW,     G_STORE,
      G_STOREP,     G_STORE_14I,   G_STORE_15I,  G_MOVE_PC,
      G_JUMP,       G_JR,          G_MMULT,      G_MTOI,
      G_NORMI,      G_NOP,         G_LOAD_14R,   G_LOAD_15R,
      G_STORE_14R,  G_STORE_15R,   G_SAT24,      G_PACK_UNPACK
   };

   GPUSTATE gst;                       // GPU State Information
   dword GincPC;                       // PC Increment Flag
   dword GjmpPC;                       // PC Jump gdstination
   dword Gbranch;                      // Branch Flag

#ifdef DEBUG_GPU
   FILE *oo;
#endif
/*
      sdword arb[32];                  // GPU Active Register Bank
      sdword srb[32];                  // GPU Secondary Register Bank
      dword  acc;                      // Accumulator
      dword  pc;                       // Program Counter
      dword  src;                      // Source Register
      dword  dst;                      // Destination Register
      BOOL   z;                        // Zero Flag
      BOOL   n;                        // Negative Flag
      BOOL   c;                        // Carry Flag
      dword  ctrl;                     // GPU Control Word    (0xF02114 RW)
      BOOL   div16;                    // 16.16 Division Flag (0xF0211C WO)
      dword  divrem;                   // Division Remainder  (0xF0211C RO)
*/

   dword gpc;                          // Program Counter
   dword gsrc;                         // Instruction Word Source       
   dword gdst;                         // Instruction Word Destination
   BOOL gz;                            // Zero Flag
   BOOL gn;                            // Negative Flag
   BOOL gc;                            // Carry Flag


   // GPU Processor Emulation

   DWORD gpu_exec( LPVOID lpParam )
   {
      dword iw;                        // Instruction Word

#ifdef DEBUG_GPU
         oo = fopen( "gpudbg.out", "w" );

         {
            FILE *f;
            int i;

            f = fopen( "gpudump.bin", "wb" );
            for( i = 0x3000; i < 0x4000; i += 2 )  // Dump GPU RAM
            {
               fwrite( &st.tom[i+1], 1, 1, f );
               fwrite( &st.tom[i],   1, 1, f );
            }
            fclose( f );
         }
#endif

      // Counter = InterruptPeriod;

      while( gst.gpuActive )
      {
         if( gst.step )                // While Single Step Enabled
         {
            while( !gst.stepgo ) ;     // Wait Until StepGo Issued   
            printf( "   GPU Released for One Instruction (SINGLE STEP)\n" );
         }

         iw = mem_readword( gpc );       // Get Instruction Word
         gsrc = (iw & 0x03E0) >> 5;  // Get Source
         gdst = (iw & 0x001F);       // Get Destination

         gpuOp[iw >> 10]();            // Jump to and Execute Opcode Routine

#ifdef DEBUG_GPU
            fflush( oo );
#endif

         // Counter -= Cycles[Opcode];

         // if( Counter <= 0 )
         // {
         //    /* Check for Interrupts and do Other Cyclic tasks here 
         //    Counter += InterruptPeriod
         // }

         switch( Gbranch )             // Increment Program Counter
         {
            case 2:                    // Next Instruction is the Branch Address
               Gbranch = 0;
               gpc = GjmpPC;
               break;
         
            // A Branch Address was Stored in the Branch Program Counter.
            // The Next Instruction is a PC + 4 (Case 0 is Executed).
            // The Next Instruction is at the Branch Program Counter

            case 1:
               Gbranch = 2;
            case 0:                    // Normal Execution
               switch( GincPC )
               {
                  case 1:              // Opcode Used a 32-Bit Word
                     gpc += 6;
                     GincPC = 0;
                     break;
                  case 0:              // Normal Execution
                     gpc += 2;
                     break;
               }
               break;
         } 

         if( gst.step )                // If Single Step Mode Enabled
         {
            gst.stepgo = 0;            // Reset StepGo Flag
            printf( "   GPU Paused Until New SINGLE_GO (SINGLE STEP)\n" );
         }
      }

      #ifdef DEBUG_GPU
         fclose( oo );
      #endif

      return( TRUE );
   }

   void G_ABS( void )
   {
      if( gst.arb[gdst] == 0x80000000 )
         gn = 1;
      else
         if( gst.arb[gdst] < 0 )
         {
            __asm {
               mov   ecx,[gdst]
               mov   eax,[ecx*4+gst.arb]  
               neg   eax
               mov   [ecx*4+gst.arb],eax  
            };

            gz = (gst.arb[gdst] == 0) ? 1 : 0;
            gn = 0;
            gc = 1;
         }

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X ABS R%i\t\t\tR%02i = 0x%08X  Z:%i N:%i C:%i\n",
                 gpc, gdst, gdst, gst.arb[gdst],
                 gz, gn, gc );
      #endif
   }

   void G_ADD( void )
   {
      __asm {
         mov   ecx,[gsrc]
         mov   edx,[ecx*4+gst.arb]
         mov   ecx,[gdst]
         mov   eax,[ecx*4+gst.arb]
         add   eax,edx
         mov   [ecx*4+gst.arb],eax
         mov   ecx,1
         jc    addend
         mov   ecx,0
      addend:
         mov   [gc],ecx
      };

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X ADD R%i,R%i\t\tR%02i = 0x%08X  Z:%i N:%i C:%i\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst],
                 gz, gn, gc );
      #endif
   }

   void G_ADDC( void )
   {
      __asm {
         clc
         mov   ecx,[gc]
         jcxz  addc
         stc                     
      addc:
         mov   ecx,[gsrc]
         mov   edx,[ecx*4+gst.arb]  
         mov   ecx,[gdst]
         mov   eax,[ecx*4+gst.arb]  
         adc   eax,edx
         mov   [ecx*4+gst.arb],eax  
         mov   ecx,1
         jc    addcend
         mov   ecx,0
      addcend:
         mov   [gc],ecx
      };

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X ADDC R%i,R%i\t\tR%02i = 0x%08X  "
                 "Z:%i N:%i C:%i\n", gpc, gsrc, gdst, gdst, 
                 gst.arb[gdst], gz, gn, gc );
      #endif
   }
    
   void G_ADDQ( void )
   {
      __asm {
         mov   ecx,[gdst]
         mov   eax,[ecx*4+gst.arb]
         mov   edx,[gsrc]         
         add   eax,edx
         mov   [ecx*4+gst.arb],eax
         mov   ecx,1
         jc    addqend
         mov   ecx,0
      addqend:
         mov   [gc],ecx
      };

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X ADDQ 0x%02X,R%i\t\tR%02i = 0x%08X  "
                 "Z:%i N:%i C:%i\n", gpc, gsrc, gdst, gdst, 
                 gst.arb[gdst], gz, gn, gc );
      #endif
   }

   void G_ADDQT( void )
   {
      __asm {
         mov   ecx,[gdst]
         mov   eax,[ecx*4+gst.arb]  
         mov   edx,[gsrc]        
         add   eax,edx
         mov   [ecx*4+gst.arb],eax  
      };

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X ADDQT 0x%02X,R%i\t\tR%02i = 0x%08X\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst] );
      #endif
   }

   void G_AND( void )
   {
      gst.arb[gdst] &= gst.arb[gsrc];
      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X AND R%i,R%i\t\tR%02i = 0x%08X  Z:%i N:%i C:%i\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst],
                 gz, gn, gc );
      #endif
   }

   void G_BCLR( void )
   {
      __asm {
         mov   ecx,[gdst]
         mov   eax,[ecx*4+gst.arb]
         mov   edx,[gsrc]
         btr   eax,edx           
         mov   [ecx*4+gst.arb],eax 
      };

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X BCLR 0x%04X,R%i\t\t\t                  "
                 "Z:%i N:%i C:%i\n", 
                 gpc, gsrc, gdst, gz, gn, gc );
      #endif
   }

   void G_BSET( void )
   {
      __asm {
         mov   ecx,[gdst]
         mov   eax,[ecx*4+gst.arb]  
         mov   edx,[gsrc]
         bts   eax,edx          
         mov   [ecx*4+gst.arb],eax 
      };

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X BSET 0x%04X,R%i\t\t\t                  "
                 "Z:%i N:%i C:%i\n", 
                 gpc, gsrc, gdst, gz, gn, gc );
      #endif
   }

   void G_BTST( void )
   {
      __asm {
         mov   ecx,[gdst]
         mov   eax,[ecx*4+gst.arb] 
         mov   ecx,[gsrc]
         bt    eax,ecx           
         mov   ecx,1
         jc    btstend
         mov   ecx,0
      btstend:
         mov   [gz],ecx
      };

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X BTST 0x%04X,R%i\t\t\t                  "
                 "Z:%i N:%i C:%i\n", 
                 gpc, gsrc, gdst, gz, gn, gc );
      #endif
   }
               
   void G_CMP( void )
   {
      sdword tmpW;

      __asm {
         mov   ecx,[gsrc]
         mov   edx,[ecx*4+gst.arb]  
         mov   ecx,[gdst]
         mov   eax,[ecx*4+gst.arb]  
         sub   eax,edx
         lahf                    
         mov   [tmpW],eax
      };

      gc = (tmpW & 0x0100) ? 1 : 0;
      gz = (tmpW & 0x4000) ? 1 : 0;
      gn = (tmpW & 0x8000) ? 1 : 0;
               
      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X CMP R%i,R%i\t\t                  "
                 "Z:%i N:%i C:%i\n",
                 gpc, gsrc, gdst, gz, gn, gc );
      #endif
   }

   void G_CMPQ( void )
   {
      sdword tmpW;

      __asm {
         mov   edx,[gsrc]
         bt    edx,4
         jnc   cmpq
         bts   edx,31
      cmpq:
         mov   ecx,[gdst]
         mov   eax,[ecx*4+gst.arb]  
         sub   eax,edx
         lahf                    
         mov   [tmpW],eax
      };

      gc = (tmpW & 0x0100) ? 1 : 0;
      gz = (tmpW & 0x4000) ? 1 : 0;
      gn = (tmpW & 0x8000) ? 1 : 0;
               
      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X CMPQ 0x%02X,R%i\t\t                  "
                 "Z:%i N:%i C:%i\n",
                 gpc, gsrc, gdst, gz, gn, gc );
      #endif*/
   }

   void G_DIV( void )
   {
      if( gst.div16 )
         printf( "GPU DIVIDE 16.16 REQUIRED !!!!\n" );
      else
      {
         (dword)gst.arb[gdst] = (dword)gst.arb[gdst] / 
                                   (dword)gst.arb[gsrc];

         gst.divrem = (dword)gst.arb[gdst] % (dword)gst.arb[gsrc];
      }

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X DIV R%i,R%i\t\tR%02i = 0x%08X  ",
                  gpc, gsrc, gdst, gdst, gst.arb[gdst] );
      #endif
   }

   void G_IMACN( void )
   {
      // Like IMULT but product is added to the previous arithmetic operation.
      // Intended to be used after IMULTN.

      gst.acc += (sdword)((sword)gst.arb[gsrc] * 
                          (sword)gst.arb[gdst] );

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X IMACN R%i,R%i\t\tACC = 0x%08X\n",
                 gpc, gsrc, gdst, gst.acc );
      #endif
   }

   void G_IMULT( void )
   {
      gst.arb[gdst] = (sdword)((sword)gst.arb[gsrc] * 
                                  (sword)gst.arb[gdst] );

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X IMULT R%i,R%i\t\tR%02i = 0x%08X  "
                 "Z:%i N:%i C:%i\n", gpc, gsrc, gdst, gdst, 
                 gst.arb[gdst], gz, gn, gc );
      #endif
   }
    
   void G_IMULTN( void )
   {
      // Like IMULT but result not written back to gst.arb[gdst]
      // but to an accumulator. Used as first part of multiply/accumulate group.

      gst.acc = (sdword)((sword)gst.arb[gsrc] * 
                         (sword)gst.arb[gdst] );

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X IMULTN R%i,R%i\t\tACC = 0x%08X  "
                 "Z:%i N:%i C:%i\n", gpc, gsrc, gdst, gst.acc,
                  gz, gn, gc );
      #endif
   }

   void G_JR( void )
   {
      if( gsrc >= 16 )         // Set Jump Direction
         gsrc -= 32;

      GjmpPC = (gpc + 2) + (gsrc * 2);

      switch( gdst )
      {
         case 0x00:           // No Conditions
            Gbranch = 1;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JR 0x%08X\n", gpc, GjmpPC );
            #endif
            break;

         case 0x01:           // NE - Not Equal
            if( !gz )  // Zero Flag Unset
               Gbranch = 1;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JR NE,0x%08X\n",gpc, GjmpPC );
            #endif
            break;

         case 0x02:           // EQ - Equal
            if( gz)    // Zero Flag Set
               Gbranch = 1;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JR EQ,0x%08X\n", gpc, GjmpPC );
            #endif
            break;

         case 0x04:           // Flag Selected Cleared to Jump
            if( !gc ) Gbranch = 1; else Gbranch = 0;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JR CC,0x%08X\n", gpc, GjmpPC );
            #endif
            break;

         case 0x08:           // Flag Selected Set to Jump
            if( gc ) Gbranch = 1; else Gbranch = 0;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JR CS,0x%08X\n", gpc, GjmpPC );
            #endif
            break;

         case 0x14:
            if( !gn ) Gbranch = 1; else Gbranch = 0;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JR NN,0x%08X\n", gpc, GjmpPC );
            #endif
            break;

         case 0x18:
            if( gn ) Gbranch = 1; else Gbranch = 0;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JR N,0x%08X\n", gpc, GjmpPC );
            #endif
            break;

         default:
            #ifdef DEBUG_GPU
               fprintf( oo, "Unknown JR Condition at 0x%08X\n", gpc );
            #endif
            break;
               
      }
   }

   void G_JUMP( void )
   {
      GjmpPC = gst.arb[gsrc];

      switch( gdst )
      {
         case J_ALWAYS:
            Gbranch = 1;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JUMP (R%02i)                    DEST = 0x%08X\n",
                       gpc, gsrc, GjmpPC );
            #endif
            break;

         case J_NZ:
            if( !gz ) Gbranch = 1;
            else         Gbranch = 0;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JUMP NZ,(R%02i)                 DEST = 0x%08X\n",
                        gpc, gsrc, GjmpPC );
            #endif
            break;

         case J_Z:
            if( gz) Gbranch = 1;
            else       Gbranch = 0;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JUMP Z,(R%02i)                  DEST = 0x%08X\n",
                        gpc, gsrc, GjmpPC );
            #endif
            break;

         case J_NC:
               if( !gc ) Gbranch = 1; 
               else         Gbranch = 0;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JUMP NC,(R%02i)                 DEST = 0x%08X\n",
                        gpc, gsrc, GjmpPC );
            #endif
            break;

         case J_NC_NZ:
               if( !gc && !gz ) Gbranch = 1; 
               else                   Gbranch = 0;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JUMP NC_NZ,(R%02i)              DEST = 0x%08X\n",
                        gpc, gsrc, GjmpPC );
            #endif
            break;

         case J_NC_Z:
               if( !gc && gz ) Gbranch = 1; 
               else                  Gbranch = 0;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JUMP NC_Z,(R%02i)               DEST = 0x%08X\n",
                        gpc, gsrc, GjmpPC );
            #endif
            break;

         case J_C:
               if( gc ) Gbranch = 1; 
               else        Gbranch = 0;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JUMP C,(R%02i)                  DEST = 0x%08X\n",
                        gpc, gsrc, GjmpPC );
            #endif
            break;

         case J_C_NZ:
               if( gc && !gz ) Gbranch = 1; 
               else                  Gbranch = 0;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JUMP C_NZ,(R%02i)               DEST = 0x%08X\n",
                        gpc, gsrc, GjmpPC );
            #endif
            break;

         case J_C_Z:
               if( gc && gz ) Gbranch = 1; 
               else                 Gbranch = 0;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JUMP C_Z,(R%02i)                DEST = 0x%08X\n",
                        gpc, gsrc, GjmpPC );
            #endif
            break;

         case J_NN:
            if( !gn ) Gbranch = 1; 
            else         Gbranch = 0;                     
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JUMP NN,(R%02i)                 DEST = 0x%08X\n",
                        gpc, gsrc, GjmpPC );
            #endif
            break;

         case J_NN_NZ:
            if( !gn && !gz ) Gbranch = 1; 
            else                   Gbranch = 0;                     
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JUMP NN_NZ,(R%02i)              DEST = 0x%08X\n",
                        gpc, gsrc, GjmpPC );
            #endif
            break;

         case J_NN_Z:
            if( !gn && gz ) Gbranch = 1; 
            else                  Gbranch = 0;                     
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JUMP NN_Z,(R%02i)               DEST = 0x%08X\n",
                        gpc, gsrc, GjmpPC );
            #endif
            break;

         case J_N:
            if( gn ) Gbranch = 1; 
            else        Gbranch = 0;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JUMP N,(R%02i)                  DEST = 0x%08X\n",
                        gpc, gsrc, GjmpPC );
            #endif
            break;

         case J_N_NZ:
            if( gn && !gz ) Gbranch = 1; 
            else                  Gbranch = 0;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JUMP N_NZ,(R%02i)               DEST = 0x%08X\n",
                        gpc, gsrc, GjmpPC );
            #endif
            break;

         case J_N_Z:
            if( gn && gz ) Gbranch = 1; 
            else                 Gbranch = 0;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JUMP N_Z,(R%02i)                DEST = 0x%08X\n",
                        gpc, gsrc, GjmpPC );
            #endif
            break;

         case J_NEVER:
            Gbranch = 0;
            #ifdef DEBUG_GPU
               fprintf( oo, "0x%06X JUMP NEVER (R%02i)              DEST = 0x%08X\n",
                       gpc, gsrc, GjmpPC );
            #endif
            break;

         default:
            #ifdef DEBUG_GPU
               fprintf( oo, "Unknown JUMP Condition\n" );
            #endif
            break;
      }
   }

   void G_LOAD( void )
   {
      gst.arb[gdst]  = mem_readword( gst.arb[gsrc] ) << 16;
      gst.arb[gdst] |= mem_readword( gst.arb[gsrc] + 2 );

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X LOAD (R%i),R%i\t\tR%02i = 0x%08X\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst] );
      #endif
   }

   void G_LOAD_14I( void )
   {
      sdword tmpW;

      tmpW = gst.arb[0x0E] + (gsrc * 4);
      gst.arb[gdst]  = mem_readword( tmpW ) << 16;
      gst.arb[gdst] |= mem_readword( tmpW + 2 );

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X LOAD (R14+%02i),R%i\t\tR%02i = 0x%08X\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst] );
      #endif
   }

   void G_LOAD_15I( void )
   {
      sdword tmpW;

      tmpW = gst.arb[0x0F] + (gsrc * 4);
      gst.arb[gdst]  = mem_readword( tmpW ) << 16;
      gst.arb[gdst] |= mem_readword( tmpW + 2 );

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X LOAD (R15+%02i),R%i\t\tR%02i = 0x%08X\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst] );
      #endif
   }
               
   void G_LOAD_14R( void )
   {               
      sdword tmpW;

      tmpW = gst.arb[0x0E] + gst.arb[gsrc];
      gst.arb[gdst]  = mem_readword( tmpW ) << 16;
      gst.arb[gdst] |= mem_readword( tmpW + 2 );

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X LOAD (R14+R%i),R%i\tR%02i = 0x%08X\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst] );
      #endif
   }

   void G_LOAD_15R( void )
   {
      sdword tmpW;

      // NOTE: Manual seems to indicate that this opcode 
      // uses Register 14 as the base offset address.

      tmpW = gst.arb[0x0E] + gst.arb[gsrc];
      gst.arb[gdst]  = mem_readword( tmpW ) << 16;
      gst.arb[gdst] |= mem_readword( tmpW + 2 );

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X LOAD (R15+R%i),R%i\tR%02i = 0x%08X\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst] );
      #endif
   }

   void G_LOADB( void )
   {
      if( gst.arb[gsrc] >= 0xF03000 && gst.arb[gsrc] < 0xF04000 )
      {
         gst.arb[gdst]  = mem_readword( gst.arb[gsrc] ) << 16;
         gst.arb[gdst] |= mem_readword( gst.arb[gsrc] + 2 );
      }
      else
         gst.arb[gdst] = mem_readbyte( gst.arb[gsrc] );

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X LOADB (R%i),R%i\t\tR%02i = 0x%08X\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst] );
      #endif
   }

   void G_LOADW( void )
   {
      if( gst.arb[gsrc] >= 0xF03000 && gst.arb[gsrc] < 0xF04000 )
      {
         gst.arb[gdst]  = mem_readword( gst.arb[gsrc] ) << 16;
         gst.arb[gdst] |= mem_readword( gst.arb[gsrc] + 2 );
      }
      else
         gst.arb[gdst] = mem_readword( gst.arb[gsrc] );

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X LOADW (R%i),R%i\t\tR%02i = 0x%08X\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst] );
      #endif
   }

   void G_LOADP( void )
   {/*
         phr.hi  = mem_readword( tolp ) << 16;          
         phr.hi |= mem_readword( tolp + 0x02 );             
         phr.lo  = mem_readword( tolp + 0x04 ) << 16;       
         phr.lo |= mem_readword( tolp + 0x06 );             
*/
      #ifdef DEBUG_GPU
         fprintf( oo, "GPU - Unimplemented Opcode (LOADP)\n" );
      #endif

      gst.gpuActive = FALSE;      
   }

   void G_MMULT( void )
   {
      #ifdef DEBUG_GPU
         fprintf( oo, "GPU - Unimplemented Opcode (MMULT)\n" );
      #endif
      gst.gpuActive = FALSE;      
   }

   void G_MOVE( void )
   {
      gst.arb[gdst] = gst.arb[gsrc];

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X MOVE R%i,R%i\t\tR%02i = 0x%08X\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst] );
      #endif
   }

   void G_MOVE_PC( void )
   {
      gst.arb[gdst] = gpc;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X MOVE PC,R%i\t\tR%02i = 0x%08X\n",
                  gpc, gdst, gdst, gst.arb[gdst] );
      #endif
   }

   void G_MOVEFA( void )
   {
      gst.arb[gdst] = gst.srb[gsrc];

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X MOVEFA R%i,R%i\t\tR%02i = 0x%08X\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst] );
      #endif
   }

   void G_MOVEI( void )
   {
      gst.arb[gdst]  = mem_readword(gpc + 2);
      gst.arb[gdst] |= mem_readword(gpc + 4) << 16;
                              
      GincPC = 1;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X MOVEI 0x%08X,R%i\tR%02i = 0x%08X\n",
                 gpc, gst.arb[gdst], gdst, gdst, gst.arb[gdst] );
      #endif
   }

   void G_MOVEQ( void )
   {
      gst.arb[gdst] = gsrc;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X MOVEQ 0x%02X,R%i\t\tR%02i = 0x%08X\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst] );
      #endif
   }

   void G_MOVETA( void )
   {
      gst.srb[gdst] = gst.arb[gsrc];

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X MOVETA R%i,R%i\t\tR%02i = 0x%08X(0)\n",
                 gpc, gsrc, gdst, gdst, gst.srb[gdst] );
      #endif
   }

   void G_MTOI( void )
   {
      #ifdef DEBUG_GPU
         fprintf( oo, "GPU - Unimplemented Opcode (MTOI)\n" );
      #endif
      gst.gpuActive = FALSE;      
   }

   void G_MULT( void )
   {
      gst.arb[gdst] = (sdword)((word)gst.arb[gsrc] * 
                                  (word)gst.arb[gdst] );

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X MULT R%i,R%i\t\tR%02i = 0x%08X  "
                 "Z:%i N:%i C:%i\n", gpc, gsrc, gdst, gdst, 
                 gst.arb[gdst], gz, gn, gc );
      #endif
   }

   void G_NEG( void )
   {
      __asm {
         mov   ecx,[gdst]
         mov   eax,[ecx*4+gst.arb]  
         neg   eax
         mov   [ecx*4+gst.arb],eax  
         mov   ecx,1
         jc    negend
         mov   ecx,0
      negend:
         mov   [gc],ecx
      };

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X NEG R%i\t\t\tR%02i = 0x%08X  Z:%i N:%i C:%i\n",
                 gpc, gdst, gdst, gst.arb[gdst],
                 gz, gn, gc );
      #endif
   }

   void G_NOP( void )
   {
      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X NOP\n", gpc );
      #endif
   }

   void G_NORMI( void )
   {
      #ifdef DEBUG_GPU
         fprintf( oo, "GPU - Unimplemented Opcode (NORMI)\n" );
      #endif
      gst.gpuActive = FALSE;      
   }

   void G_NOT( void )                  // Fix donated by YaK
   {
      gst.arb[gdst] = gst.arb[gdst] ^ 0xFFFFFFFF;
      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X NOT R%i\t\t\tR%02i = 0x%08X  Z:%i N:%i C:%i\n",
                 gpc, gdst, gdst, gst.arb[gdst],
                 gz, gn, gc );
      #endif
   }

   void G_OR( void )
   {
      gst.arb[gdst] |= gst.arb[gsrc];
      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X OR R%i,R%i\t\tR%02i = 0x%08X  Z:%i N:%i C:%i\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst],
                 gz, gn, gc );
      #endif
   }

   void G_PACK_UNPACK( void )
   {
      dword tmpW;

      tmpW = gst.arb[gdst];

      if( gsrc )                    // UNPACK
      {
         gst.arb[gdst]  = (tmpW & 0x000000FF);
         gst.arb[gdst] |= (tmpW & 0x00000F00) << 5;
         gst.arb[gdst] |= (tmpW & 0x0000F000) << 10;

         #ifdef DEBUG_GPU
            fprintf( oo, "0x%06X UNPACK R%i\t\t\tR%02i = 0x%08X\n",
                    gpc, gdst, gdst, gst.arb[gdst] );
         #endif
      }
      else                             // PACK
      {
         gst.arb[gdst]  = (tmpW & 0x000000FF);
         gst.arb[gdst] |= (tmpW & 0x0001E000) >> 5;
         gst.arb[gdst] |= (tmpW & 0x03C00000) >> 10;

         #ifdef DEBUG_GPU
            fprintf( oo, "0x%06X PACK R%i\t\t\tR%02i = 0x%08X\n",
                    gpc, gdst, gdst, gst.arb[gdst] );
         #endif
      }
   }

   void G_RESMAC( void )
   {
      // Write result register to Register Rn
      // Used as last part of multiply/accumulate group.

      gst.arb[gdst] = gst.acc;
               
      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X RESMAC R%i\t\tR%02X = 0x%08X\n",
                  gpc, gdst, gdst, gst.arb[gdst] );
      #endif
   }

   void G_ROR( void )
   {
      gc = (gst.arb[gdst] >> 31) & 0x01;

      __asm {
         mov   ecx,[gdst]
         mov   eax,[ecx*4+gst.arb]
         mov   edx,[gsrc]
         mov   ecx,[edx*4+gst.arb]
         and   ecx,0x1F
         ror   eax,cl
         mov   ecx,[gdst]
         mov   [ecx*4+gst.arb],eax
      };

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X ROR R%i,R%i\t\tR%02i = 0x%08X  Z:%i N:%i C:%i\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst],
                 gz, gn, gc );
      #endif
   }

   void G_RORQ( void )
   {
      sdword tmpW;

      gc = (gst.arb[gdst] >> 31) & 0x01;
      tmpW = gst.arb[gdst];
      
      __asm {
         mov   eax,[tmpW]
         mov   ecx,[gsrc]
         ror   eax,cl
         mov   [tmpW],eax
      };

      gst.arb[gdst] = tmpW;

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X RORQ 0x%02X,R%i\t\tR%02i = 0x%08X  "
                 "Z:%i N:%i C:%i\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst],
                 gz, gn, gc );
      #endif
   }

   void G_SAT8( void )
   {
      if( gst.arb[gdst] < 0   ) gst.arb[gdst] =0x00000000;
      if( gst.arb[gdst] > 255 ) gst.arb[gdst] =0x000000FF;

      gn = 0;
      gz = (gst.arb[gdst] == 0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X SAT8 R%02i\t\t\tR%02i = 0x%08X  "
                 "Z:%i N:%i C:%i\n",
                 gpc, gdst, gdst, gst.arb[gdst],
                 gz, gn, gc );
      #endif
   }

   void G_SAT16( void )
   {
      if( gst.arb[gdst] < 0     ) gst.arb[gdst] =0x00000000;
      if( gst.arb[gdst] > 65535 ) gst.arb[gdst] =0x0000FFFF;

      gn = 0;
      gz = (gst.arb[gdst] == 0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X SAT16 R%02i\t\t\tR%02i = 0x%08X  "
                 "Z:%i N:%i C:%i\n",
                 gpc, gdst, gdst, gst.arb[gdst],
                 gz, gn, gc );
      #endif
   }

   void G_SAT24( void )
   {
      if( gst.arb[gdst] < 0        ) gst.arb[gdst] =0x00000000;
      if( gst.arb[gdst] > 16777215 ) gst.arb[gdst] =0x00FFFFFF;

      gn = 0;
      gz = (gst.arb[gdst] == 0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X SAT24 R%02i\t\t\tR%02i = 0x%08X  "
                 "Z:%i N:%i C:%i\n",
                 gpc, gdst, gdst, gst.arb[gdst],
                 gz, gn, gc );
      #endif
   }

   void G_SH( void )
   {
      sdword tmpW;

      // NOTE: Watch the values here carefully

      if( gst.arb[gsrc] >= 0 )    // Shift Right
      {
         gc = gst.arb[gdst] & 0x01;
         tmpW = gst.arb[gsrc];
         __asm {
            mov   edx,[gdst]
            mov   eax,[edx*4+gst.arb] 
            mov   ecx,[tmpW]
            shr   eax,cl
            mov   [edx*4+gst.arb],eax 
         };
      }
      else                    // Shift Left
      {
         gc = (gst.arb[gdst] >> 31) & 0x01;
         // Fix donated by YaK
         tmpW = (0xFFFFFFFF - gst.arb[gsrc]) + 1;
         __asm {
            mov   edx,[gdst]
            mov   eax,[edx*4+gst.arb] 
            mov   ecx,[tmpW]
            shl   eax,cl
            mov   [edx*4+gst.arb],eax 
         };
      }

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X SH R%i,R%i\t\tR%02i = 0x%08X  "
                 "Z:%i N:%i C:%i\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst],
                 gz, gn, gc );
      #endif*/
   }

   void G_SHA( void )
   {
      sdword tmpW;

      // NOTE: Watch the values here carefully

      if( gst.arb[gsrc] >= 0 )    // Shift Right
      {
         gc = gst.arb[gdst] & 0x01;
         tmpW = gst.arb[gsrc];
         __asm {
            mov   edx,[gdst]
            mov   eax,[edx*4+gst.arb] 
            mov   ecx,[tmpW]
            sar   eax,cl
            mov   [edx*4+gst.arb],eax 
         };
      }
      else                    // Shift Left
      {
         gc = (gst.arb[gdst] >> 31) & 0x01;
         // Fix donated by YaK
         tmpW = (0xFFFFFFFF - gst.arb[gsrc]) + 1;
         __asm {
            mov   edx,[gdst]
            mov   eax,[edx*4+gst.arb] 
            mov   ecx,[tmpW]
            shl   eax,cl
            mov   [edx*4+gst.arb],eax 
         };
      }

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X SHA R%i,R%i\t\tR%02i = 0x%08X  "
                 "Z:%i N:%i C:%i\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst],
                 gz, gn, gc );
      #endif*/
   }

   void G_SHARQ( void )
   {
      sdword tmpW;

      gc = (gst.arb[gdst] & 0x01);
      tmpW = gst.arb[gdst];
      
      __asm {
         mov   eax,[tmpW]
         mov   ecx,[gsrc]
         sar   eax,cl
         mov   [tmpW],eax
      };
      
      gst.arb[gdst] = tmpW;

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X SHARQ 0x%02X,R%i\t\tR%02i = 0x%08X  "
                 "Z:%i N:%i C:%i\n", gpc, gsrc, gdst, gdst, 
                 gst.arb[gdst], gz, gn, gc );
      #endif
   }
               
   void G_SHLQ( void )
   {
      gc = (gst.arb[gdst] >> 31) & 0x01;
      gst.arb[gdst] <<= (32 - gsrc);

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X SHLQ 0x%02X,R%i\t\tR%02i = 0x%08X  "
                 "Z:%i N:%i C:%i\n", gpc, gsrc, gdst, gdst, 
                 gst.arb[gdst], gz, gn, gc );
      #endif
   }

   void G_SHRQ( void )
   {
      gc = gst.arb[gdst] & 0x01;
      gst.arb[gdst] >>= (32 - gsrc);

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X SHRQ 0x%02X,R%i\t\tR%02i = 0x%08X  "
                 "Z:%i N:%i C:%i\n", gpc, gsrc, gdst, gdst, 
                 gst.arb[gdst], gz, gn, gc );
      #endif
   }
               
   void G_STORE( void )
   {
      mem_writeword( gst.arb[gsrc],     (gst.arb[gdst] >> 16)    );
      mem_writeword( gst.arb[gsrc] + 2, (gst.arb[gdst] & 0xFFFF) );

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X STORE R%i,(R%i)\t\tgdst = 0x%08X\n",
                 gpc, gdst, gsrc, gst.arb[gsrc] );
      #endif
   }

   void G_STORE_14I( void )
   {
      sdword tmpW;

      tmpW = gst.arb[0x0E] + (gsrc * 4);

      mem_writeword( tmpW,     (gst.arb[gdst] >> 16)    );
      mem_writeword( tmpW + 2, (gst.arb[gdst] & 0xFFFF) );

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X STORE R%i,(R14+%02i)\t\tgdst = 0x%08X\n",
                 gpc, gsrc, gdst, tmpW );
      #endif
   }

   void G_STORE_15I( void )
   {
      sdword tmpW;

      tmpW = gst.arb[0x0F] + (gsrc * 4);

      mem_writeword( tmpW,     (gst.arb[gdst] >> 16)    );
      mem_writeword( tmpW + 2, (gst.arb[gdst] & 0xFFFF) );

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X STORE R%i,(R15+%02i)\t\tgdst = 0x%08X\n",
                 gpc, gsrc, gdst, tmpW );
      #endif
   }

   void G_STORE_14R( void )
   {
      sdword tmpW;

      tmpW = gst.arb[0x0E] + gst.arb[gsrc];
      mem_writeword( tmpW,     (gst.arb[gdst] >> 16)    );
      mem_writeword( tmpW + 2, (gst.arb[gdst] & 0xFFFF) );

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X STORE R%i,(R14+R%i)\tgdst = 0x%08X\n",
                  gpc, gdst, gsrc, tmpW );
      #endif
   }

   void G_STORE_15R( void )
   {
      sdword tmpW;

      // NOTE: Manual seems to indicate that this opcode
      // uses Register 14 as the base offset address.

      tmpW = gst.arb[0x0E] + gst.arb[gsrc];
      mem_writeword( tmpW,     (gst.arb[gdst] >> 16)    );
      mem_writeword( tmpW + 2, (gst.arb[gdst] & 0xFFFF) );

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X STORE R%i,(R14+R%i)\tgdst = 0x%08X\n",
                 gpc, gdst, gsrc, tmpW );
      #endif
   }

   void G_STOREB( void )
   {
      if( gst.arb[gsrc] >= 0xF03000 && gst.arb[gsrc] < 0xF04000 )
      {
         mem_writeword( gst.arb[gsrc],     (gst.arb[gdst] >> 16)     );
         mem_writeword( gst.arb[gsrc] + 2, (gst.arb[gdst] & 0xFFFF) );
      }
      else
         mem_writebyte( gst.arb[gsrc], (gst.arb[gdst] & 0xFF) );

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X STOREB R%i,(R%i)\tgdst = 0x%08X\n",
                 gpc, gdst, gsrc, gsrc );
      #endif
   }

   void G_STOREW( void )
   {
      if( gst.arb[gsrc] >= 0xF03000 && gst.arb[gsrc] < 0xF04000 )
      {
         mem_writeword( gst.arb[gsrc],     (gst.arb[gdst] >> 16)     );
         mem_writeword( gst.arb[gsrc] + 2, (gst.arb[gdst] & 0xFFFF) );
      }
      else
         mem_writeword( gst.arb[gsrc], (gst.arb[gdst] & 0xFFFF) );

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X STOREW R%i,(R%i)\tgdst = 0x%08X\n",
                 gpc, gdst, gsrc, gsrc );
      #endif
   }

   void G_STOREP( void )
   {
      #ifdef DEBUG_GPU
         fprintf( oo, "GPU - Unimplemented Opcode (STOREP)\n" );
      #endif
      gst.gpuActive = FALSE;      
   }

   void G_SUB( void )
   {
      __asm {
         mov   ecx,[gsrc]
         mov   edx,[ecx*4+gst.arb]  
         mov   ecx,[gdst]
         mov   eax,[ecx*4+gst.arb]  
         sub   eax,edx
         mov   [ecx*4+gst.arb],eax  
         mov   ecx,1
         jc    subend
         mov   ecx,0
      subend:
         mov   [gc],ecx
      };

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X SUB R%i,R%i\t\tR%02i = 0x%08X  Z:%i N:%i C:%i\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst],
                 gz, gn, gc );
      #endif
   }

   void G_SUBC( void )
   {
      __asm {
         clc
         mov   ecx,[gc]
         jcxz  subc
         stc                     
      subc:
         mov   ecx,[gsrc]
         mov   edx,[ecx*4+gst.arb]
         mov   ecx,[gdst]
         mov   eax,[ecx*4+gst.arb]
         sbb   eax,edx
         mov   [ecx*4+gst.arb],eax
         mov   ecx,1
         jc    subcend
         mov   ecx,0
      subcend:
         mov   [gc],ecx
      };

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
          fprintf( oo, "0x%06X SUBC R%i,R%i\t\tR%02i = 0x%08X  "
                  "Z:%i N:%i C:%i\n", gpc, gsrc, gdst, gdst, 
                  gst.arb[gdst], gz, gn, gc );
      #endif
   }

   void G_SUBQ( void )
   {
      __asm {
         mov   ecx,[gdst]
         mov   eax,[ecx*4+gst.arb]  
         mov   edx,[gsrc]         
         sub   eax,edx
         mov   [ecx*4+gst.arb],eax
         mov   ecx,1
         jc    subqend
         mov   ecx,0
      subqend:
         mov   [gc],ecx
      };

      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X SUBQ 0x%02X,R%i\t\tR%02i = 0x%08X  "
                 "Z:%i N:%i C:%i\n", gpc, gsrc, gdst, gdst, 
                 gst.arb[gdst], gz, gn, gc );
      #endif
   }

   void G_SUBQT( void )
   {
      __asm {
         mov   ecx,[gdst]
         mov   eax,[ecx*4+gst.arb]
         mov   edx,[gsrc]         
         sub   eax,edx
         mov   [ecx*4+gst.arb],eax  
      };

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X SUBQT 0x%02X,R%i\t\tR%02i = 0x%08X\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst] );
      #endif
   }

   void G_XOR( void )
   {
      gst.arb[gdst] ^= gst.arb[gsrc];
      gz = (gst.arb[gdst] == 0) ? 1 : 0;
      gn = (gst.arb[gdst] <  0) ? 1 : 0;

      #ifdef DEBUG_GPU
         fprintf( oo, "0x%06X XOR R%i,R%i\t\tR%02i = 0x%08X  Z:%i N:%i C:%i\n",
                 gpc, gsrc, gdst, gdst, gst.arb[gdst],
                 gz, gn, gc );
      #endif
   }
