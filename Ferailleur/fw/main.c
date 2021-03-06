/*********************************************************************
 *               analog example for Versa1.0
 *	Analog capture on connectors K1, K2, K3 and K5. 
 *********************************************************************/

#define BOARD Versa1

#include <fruit.h>
#include <analog.h>
#include <switch.h>

t_delay mainDelay;

#define LED0 K4
//#define LED1 K11
//#define LED2 K12*/

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	
	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

	pinModeDigitalOut(LED0); 	// set the LED pin mode to digital out
	digitalClear(LED0);
	/*pinModeDigitalOut(LED1); 	// set the LED pin mode to digital out
	digitalClear(LED1);
	pinModeDigitalOut(LED2); 	// set the LED pin mode to digital out
	digitalClear(LED2);*/

//----------- Analog setup ----------------
	analogInit();		// init analog module
	analogSelect(0,K5);	
	analogSelect(1,K6);	
	analogSelect(2,K7);	

	switchInit();
	INTCON2bits.RBPU = 0; // enable pullups on PORTB
	
	switchSelect(0,K10);	// assign connector K1 to analog channel 0
	switchSelect(1,K9);
	switchSelect(2,MB2);
	switchSelect(3,MBEN2);
	switchSelect(4,MBEN);
	switchSelect(5,MB1);
	switchSelect(6,K1);
	switchSelect(7,K2);
	switchSelect(8,K3);
	switchSelect(9,K12);
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
	if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		digitalWrite(LED, c!='0');		
	}
	else if(c=='l'){		//switch LED on/off 
		c=fraiseGetChar();
		c2=fraiseGetChar();
		if(c == '0') digitalWrite(LED0, c2 != '0');		
		/*else if(c == '1') digitalWrite(LED1, c2 != '0');		
		else if(c == '2') digitalWrite(LED2, c2 != '0'); */
	}
	else if(c=='E') { 	// echo text (send it back to host)
		printf("C");
		c = fraiseGetLen(); 			// get length of current packet
		while(c--) printf("%c",fraiseGetChar());// send each received byte
		putchar('\n');				// end of line
	}	
}

