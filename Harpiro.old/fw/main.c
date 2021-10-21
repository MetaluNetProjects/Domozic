/*********************************************************************
 *               analog example for Versa1.0
 *	Analog capture on connectors K1, K2, K3 and K5. 
 *********************************************************************/

#define BOARD Versa2

#include <fruit.h>
#include <analog.h>
#include <switch.h>

t_delay mainDelay;

#define VOLPOT K1
#define FXPOT K2

#define MODELED K4

#define MODESW K10
#define SWA K11
#define SWB K12


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

	switchInit();
	INTCON2bits.RBPU = 0; // enable pullups on PORTB
	
	switchSelect(0,MODESW);
	switchSelect(1,SWA);
	switchSelect(2,SWB);
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


