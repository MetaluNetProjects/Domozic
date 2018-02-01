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
		InitTimerUS(30);//13
	}
}
// -----------------

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	
	pinModeDigitalOut(MODELED); 	// set the LED pin mode to digital out
	digitalClear(MODELED);
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms 

//----------- Analog setup ----------------
	analogInit();		// init analog module
	analogSelect(0,SPEEDPOT);	
	analogSelect(1,VOLPOT);	
	analogSelect(2,FXPOT);	
	analogSelect(3,SONPOT);	

	analogSelect(4,CROQUESON);	
	analogSelect(5,CROQUEFX);	
	analogSelect(6,CROQUEVOL);	
	analogSelect(7,CROQUEPOT);	

	switchInit();
	INTCON2bits.RBPU = 0; // enable pullups on PORTB
	
	switchSelect(0,MODESW);
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
		//fraiseService();	// listen to Fraise events
		printf("CM %ld %d %ld %ld\n",DCMOTOR_GETSPEED(D), (int)dcmotorDeltaPos, dcmotorDeltaT, DCMOTOR_GETPOS(D));
		//printf("CP %ld\n",DCMOTOR_GETPOS(D));
		//printf("CM %ld\n",DCMOTOR_GETSPEED(D)/*(long)DCMOTOR(D).VolVars.computedSpeed*/);
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
