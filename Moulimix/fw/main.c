/*********************************************************************
 *               analog example for Versa1.0
 *	Analog capture on connectors K1, K2, K3 and K5. 
 *********************************************************************/

#define BOARD 8X2A

#include <fruit.h>
#include <analog.h>
#include <switch.h>
#include <dcmotor.h>

t_delay mainDelay;

#define LED0 K4
//#define LED1 K11
//#define LED2 K12*/

// ----------------- MOTOR DECLARATIONS
//-------------  Timer1 macros :  ---------------------------------------- 
//prescaler=PS fTMR1=FOSC/(4*PS) nbCycles=0xffff-TMR1init T=nbCycles/fTMR1=(0xffff-TMR1init)*4PS/FOSC
//TMR1init=0xffff-(T*FOSC/4PS) ; max=65536*4PS/FOSC : 
//ex: PS=8 : T=0.01s : TMR1init=0xffff-15000
//Maximum 1s !!
#define	TMR1init(T) (0xffff-((T*FOSC)/32000)) //ms ; maximum: 8MHz:262ms 48MHz:43ms 64MHz:32ms
#define	TMR1initUS(T) (0xffff-((T*FOSC)/32000000)) //us ; 
#define InitTimer(T) do{ TMR1H=TMR1init(T)/256 ; TMR1L=TMR1init(T)%256; PIR1bits.TMR1IF=0; }while(0)
#define InitTimerUS(T) do{ TMR1H=TMR1initUS(T)/256 ; TMR1L=TMR1initUS(T)%256; PIR1bits.TMR1IF=0; }while(0)
#define TimerOut() (PIR1bits.TMR1IF)

DCMOTOR_DECLARE(D);
void highInterrupts()
{
	if(PIR1bits.TMR1IF) {
		DCMOTOR_CAPTURE_SERVICE(D);
		InitTimerUS(31);
	}
}
// -----------------



void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	
	pinModeDigitalOut(MODELED); 	// set the LED pin mode to digital out
	digitalClear(MODELED);
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms 

/*	pinModeDigitalOut(LED0); 	// set the LED pin mode to digital out
	digitalClear(LED0);
	pinModeDigitalOut(LED1); 	// set the LED pin mode to digital out
	digitalClear(LED1);
	pinModeDigitalOut(LED2); 	// set the LED pin mode to digital out
	digitalClear(LED2);*/

//----------- Analog setup ----------------
	analogInit();		// init analog module
	analogSelect(0,VOLPOT);	
	analogSelect(1,FXPOT);	
	analogSelect(2,CROQUEPOT);	

	switchInit();
	INTCON2bits.RBPU = 0; // enable pullups on PORTB
	
	switchSelect(0,MODESW);
	/*switchSelect(1,K9);
	switchSelect(2,MB2);
	switchSelect(3,MBEN2);
	switchSelect(4,MBEN);
	switchSelect(5,MB1);
	switchSelect(6,K1);
	switchSelect(7,K2);
	switchSelect(8,K3);
	switchSelect(9,K12);*/

// ---------- capture timer : TMR1 ------------
	T1CON=0b00110011;//src=fosc/4,ps=8,16bit r/w,on.
	PIE1bits.TMR1IE=1;  //1;
	IPR1bits.TMR1IP=1;
	dcmotorInit(D);
	EEreadMain();
}

void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
	analogService();	// analog management routine
	switchService();	// 
	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 5000); 	// re-init mainDelay
		if(!switchSend()) analogSend();		// send switches channels that changed, if none send analogs that changed
		DCMOTOR_COMPUTE(D, ASYM);
		fraiseService();	// listen to Fraise events
		printf("CM %d\n",DCMOTOR_GETPOS(D)%0xffff);
	}
}

// Receiving

void fraiseReceiveChar() // receive text
{
	unsigned char c,c2;
	
	c=fraiseGetChar();
	/*if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		digitalWrite(LED, c!='0');		
	}
	else */
	if(c=='l'){		//switch LED on/off 
		c=fraiseGetChar();
		c2=fraiseGetChar();
		if(c == '0') digitalWrite(MODELED, c2 != '0');		
		/*else if(c == '1') digitalWrite(LED1, c2 != '0');		
		else if(c == '2') digitalWrite(LED2, c2 != '0'); */
	}
	else if(c=='E') { 	// echo text (send it back to host)
		printf("C");
		c = fraiseGetLen(); 			// get length of current packet
		while(c--) printf("%c",fraiseGetChar());// send each received byte
		putchar('\n');				// end of line
	}
	else if(c=='S') { //EEPROM save
		if((fraiseGetChar() == 'A')
		&& (fraiseGetChar() == 'V')
		&& (fraiseGetChar() == 'E'))
			EEwriteMain();
	}	
	
}

void fraiseReceive()
{
	unsigned char c;//,c2;
	unsigned int i;
	
	c=fraiseGetChar();

	switch(c) {
		case 120 : DCMOTOR_INPUT(D); break;
	}
}

void EEdeclareMain()
{
	DCMOTOR_DECLARE_EE(D);
}
