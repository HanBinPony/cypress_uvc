//-----------------------------------------------------------------------------
//   File:      bulkloop.c
//   Contents:  Hooks required to implement USB peripheral function.
//
// 
//
//
//-----------------------------------------------------------------------------
// Copyright (c) 2011, Cypress Semiconductor Corporation All rights reserved
//-----------------------------------------------------------------------------
#pragma noiv               // Do not generate interrupt vectors

#include "Fx2.h"
#include "fx2regs.h"
#include "syncdly.h"            // SYNCDELAY macro

extern BOOL GotSUD;             // Received setup data flag
extern BOOL Sleep;
extern BOOL Rwuen;
extern BOOL Selfpwr;

BYTE Configuration;             // Current configuration
BYTE AlternateSetting;          // Alternate settings

#define VR_NAKALL_ON    0xD0
#define VR_NAKALL_OFF   0xD1


//-----------------------------------------------------------------------------
// Task Dispatcher hooks
//   The following hooks are called by the task dispatcher.
//-----------------------------------------------------------------------------

void TD_Init(void)             // Called once at startup
{
   // set the CPU clock to 48MHz
   CPUCS = ((CPUCS & ~bmCLKSPD) | bmCLKSPD1) ;

   // set the slave FIFO interface to 48MHz
   IFCONFIG |= 0x40;

  // Registers which require a synchronization delay, see section 15.14
  // FIFORESET        FIFOPINPOLAR
  // INPKTEND         OUTPKTEND
  // EPxBCH:L         REVCTL
  // GPIFTCB3         GPIFTCB2
  // GPIFTCB1         GPIFTCB0
  // EPxFIFOPFH:L     EPxAUTOINLENH:L
  // EPxFIFOCFG       EPxGPIFFLGSEL
  // PINFLAGSxx       EPxFIFOIRQ
  // EPxFIFOIE        GPIFIRQ
  // GPIFIE           GPIFADRH:L
  // UDMACRCH:L       EPxGPIFTRIG
  // GPIFTRIG
  
  // Note: The pre-REVE EPxGPIFTCH/L register are affected, as well...
  //      ...these have been replaced by GPIFTC[B3:B0] registers

  // default: all endpoints have their VALID bit set
  // default: TYPE1 = 1 and TYPE0 = 0 --> BULK  
  // default: EP2 and EP4 DIR bits are 0 (OUT direction)
  // default: EP6 and EP8 DIR bits are 1 (IN direction)
  // default: EP2, EP4, EP6, and EP8 are double buffered

  // we are just using the default values, yes this is not necessary...
  EP1OUTCFG = 0xA0;
  EP1INCFG = 0xA0;
  SYNCDELAY;                    // see TRM section 15.14
  EP2CFG = 0xA2;
  SYNCDELAY;                    
  EP4CFG = 0xA0;
  SYNCDELAY;                    
  EP6CFG = 0xE2;
  SYNCDELAY;                    
  EP8CFG = 0xE0;

  // out endpoints do not come up armed
  
  // since the defaults are double buffered we must write dummy byte counts twice
  SYNCDELAY;                    
  EP2BCL = 0x80;                // arm EP2OUT by writing byte count w/skip.
  SYNCDELAY;                    
  EP2BCL = 0x80;
  SYNCDELAY;                    
  EP4BCL = 0x80;                // arm EP4OUT by writing byte count w/skip.
  SYNCDELAY;                    
  EP4BCL = 0x80;    

  // enable dual autopointer feature
  AUTOPTRSETUP |= 0x01;

}


void TD_Poll(void)              // Called repeatedly while the device is idle
{
  WORD i;
  WORD count;

  if(!(EP2468STAT & bmEP2EMPTY))
  { // check EP2 EMPTY(busy) bit in EP2468STAT (SFR), core set's this bit when FIFO is empty
     if(!(EP2468STAT & bmEP6FULL))
     {  // check EP6 FULL(busy) bit in EP2468STAT (SFR), core set's this bit when FIFO is full
        APTR1H = MSB( &EP2FIFOBUF );
        APTR1L = LSB( &EP2FIFOBUF );

        AUTOPTRH2 = MSB( &EP6FIFOBUF );
        AUTOPTRL2 = LSB( &EP6FIFOBUF );

        count = (EP2BCH << 8) + EP2BCL;

        // loop EP2OUT buffer data to EP6IN
        for( i = 0x0000; i < count; i++ )
        {
           // setup to transfer EP2OUT buffer to EP6IN buffer using AUTOPOINTER(s)
           EXTAUTODAT2 = EXTAUTODAT1;
        }
        EP6BCH = EP2BCH;  
        SYNCDELAY;  
        EP6BCL = EP2BCL;        // arm EP6IN
        SYNCDELAY;                    
        EP2BCL = 0x80;          // re(arm) EP2OUT
     }
  }

  if(!(EP2468STAT & bmEP4EMPTY))
  { // check EP4 EMPTY(busy) bit in EP2468STAT (SFR), core set's this bit when FIFO is empty
     if(!(EP2468STAT & bmEP8FULL))
     {  // check EP8 FULL(busy) bit in EP2468STAT (SFR), core set's this bit when FIFO is full
        APTR1H = MSB( &EP4FIFOBUF );
        APTR1L = LSB( &EP4FIFOBUF );

        AUTOPTRH2 = MSB( &EP8FIFOBUF );
        AUTOPTRL2 = LSB( &EP8FIFOBUF );

        count = (EP4BCH << 8) + EP4BCL;

        // loop EP4OUT buffer data to EP8IN
        for( i = 0x0000; i < count; i++ )
        {
           // setup to transfer EP4OUT buffer to EP8IN buffer using AUTOPOINTER(s)
           EXTAUTODAT2 = EXTAUTODAT1;
        }
        EP8BCH = EP4BCH;  
        SYNCDELAY;  
        EP8BCL = EP4BCL;        // arm EP8IN
        SYNCDELAY;                    
        EP4BCL = 0x80;          // re(arm) EP4OUT
     }
  }
}

BOOL TD_Suspend(void)          // Called before the device goes into suspend mode
{
   return(TRUE);
}

BOOL TD_Resume(void)          // Called after the device resumes
{
   return(TRUE);
}

//-----------------------------------------------------------------------------
// Device Request hooks
//   The following hooks are called by the end point 0 device request parser.
//-----------------------------------------------------------------------------

BOOL DR_GetDescriptor(void)
{
   return(TRUE);
}

BOOL DR_SetConfiguration(void)   // Called when a Set Configuration command is received
{
   Configuration = SETUPDAT[2];
   return(TRUE);            // Handled by user code
}

BOOL DR_GetConfiguration(void)   // Called when a Get Configuration command is received
{
   EP0BUF[0] = Configuration;
   EP0BCH = 0;
   EP0BCL = 1;
   return(TRUE);            // Handled by user code
}

BOOL DR_SetInterface(void)       // Called when a Set Interface command is received
{
   AlternateSetting = SETUPDAT[2];
   return(TRUE);            // Handled by user code
}

BOOL DR_GetInterface(void)       // Called when a Set Interface command is received
{
   EP0BUF[0] = AlternateSetting;
   EP0BCH = 0;
   EP0BCL = 1;
   return(TRUE);            // Handled by user code
}

BOOL DR_GetStatus(void)
{
   return(TRUE);
}

BOOL DR_ClearFeature(void)
{
   return(TRUE);
}

BOOL DR_SetFeature(void)
{
   return(TRUE);
}

BOOL DR_VendorCmnd(void)
{
  BYTE tmp;
  
  switch (SETUPDAT[1])
  {
     case VR_NAKALL_ON:
        tmp = FIFORESET;
        tmp |= bmNAKALL;      
        SYNCDELAY;                    
        FIFORESET = tmp;
        break;
     case VR_NAKALL_OFF:
        tmp = FIFORESET;
        tmp &= ~bmNAKALL;      
        SYNCDELAY;                    
        FIFORESET = tmp;
        break;
     default:
        return(TRUE);
  }

  return(FALSE);
}

//-----------------------------------------------------------------------------
// USB Interrupt Handlers
//   The following functions are called by the USB interrupt jump table.
//-----------------------------------------------------------------------------

// Setup Data Available Interrupt Handler
void ISR_Sudav(void) __interrupt 0
{
   GotSUD = TRUE;            // Set flag
   EZUSB_IRQ_CLEAR();
   USBIRQ = bmSUDAV;         // Clear SUDAV IRQ
}

// Setup Token __interrupt Handler
void ISR_Sutok(void) __interrupt 0
{
   EZUSB_IRQ_CLEAR();
   USBIRQ = bmSUTOK;         // Clear SUTOK IRQ
}

void ISR_Sof(void) __interrupt 0
{
   EZUSB_IRQ_CLEAR();
   USBIRQ = bmSOF;            // Clear SOF IRQ
}

void ISR_Ures(void) __interrupt 0
{
   // whenever we get a USB reset, we should revert to full speed mode
   pConfigDscr = pFullSpeedConfigDscr;
   ((CONFIGDSCR __xdata *) pConfigDscr)->type = CONFIG_DSCR;
   pOtherConfigDscr = pHighSpeedConfigDscr;
   ((CONFIGDSCR __xdata *) pOtherConfigDscr)->type = OTHERSPEED_DSCR;

   EZUSB_IRQ_CLEAR();
   USBIRQ = bmURES;         // Clear URES IRQ
}

void ISR_Susp(void) __interrupt 0
{
   Sleep = TRUE;
   EZUSB_IRQ_CLEAR();
   USBIRQ = bmSUSP;
}

void ISR_Highspeed(void) __interrupt 0
{
   if (EZUSB_HIGHSPEED())
   {
      pConfigDscr = pHighSpeedConfigDscr;
      ((CONFIGDSCR __xdata *) pConfigDscr)->type = CONFIG_DSCR;
      pOtherConfigDscr = pFullSpeedConfigDscr;
      ((CONFIGDSCR __xdata *) pOtherConfigDscr)->type = OTHERSPEED_DSCR;
   }

   EZUSB_IRQ_CLEAR();
   USBIRQ = bmHSGRANT;
}
void ISR_Ep0ack(void) __interrupt 0
{
}
void ISR_Stub(void) __interrupt 0
{
}
void ISR_Ep0in(void) __interrupt 0
{
}
void ISR_Ep0out(void) __interrupt 0
{
}
void ISR_Ep1in(void) __interrupt 0
{
}
void ISR_Ep1out(void) __interrupt 0
{
}
void ISR_Ep2inout(void) __interrupt 0
{
}
void ISR_Ep4inout(void) __interrupt 0
{
}
void ISR_Ep6inout(void) __interrupt 0
{
}
void ISR_Ep8inout(void) __interrupt 0
{
}
void ISR_Ibn(void) __interrupt 0
{
}
void ISR_Ep0pingnak(void) __interrupt 0
{
}
void ISR_Ep1pingnak(void) __interrupt 0
{
}
void ISR_Ep2pingnak(void) __interrupt 0
{
}
void ISR_Ep4pingnak(void) __interrupt 0
{
}
void ISR_Ep6pingnak(void) __interrupt 0
{
}
void ISR_Ep8pingnak(void) __interrupt 0
{
}
void ISR_Errorlimit(void) __interrupt 0
{
}
void ISR_Ep2piderror(void) __interrupt 0
{
}
void ISR_Ep4piderror(void) __interrupt 0
{
}
void ISR_Ep6piderror(void) __interrupt 0
{
}
void ISR_Ep8piderror(void) __interrupt 0
{
}
void ISR_Ep2pflag(void) __interrupt 0
{
}
void ISR_Ep4pflag(void) __interrupt 0
{
}
void ISR_Ep6pflag(void) __interrupt 0
{
}
void ISR_Ep8pflag(void) __interrupt 0
{
}
void ISR_Ep2eflag(void) __interrupt 0
{
}
void ISR_Ep4eflag(void) __interrupt 0
{
}
void ISR_Ep6eflag(void) __interrupt 0
{
}
void ISR_Ep8eflag(void) __interrupt 0
{
}
void ISR_Ep2fflag(void) __interrupt 0
{
}
void ISR_Ep4fflag(void) __interrupt 0
{
}
void ISR_Ep6fflag(void) __interrupt 0
{
}
void ISR_Ep8fflag(void) __interrupt 0
{
}
void ISR_GpifComplete(void) __interrupt 0
{
}
void ISR_GpifWaveform(void) __interrupt 0
{
}
