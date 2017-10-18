/*********************************************************************
 *               analog example for Versa1.0
 *	Analog capture on connectors K1, K2, K3 and K5. 
 *********************************************************************/

#define BOARD Versa1

#include <fruit.h>
#include <analog.h>
#include <switch.h>
#include <i2c_master.h>
#include <PCA9655.h>

t_delay mainDelay;

//	PCAs addresses:			 AD2 AD1 AD0
#define PCA1ADD (0x40>>1) // GND GND GND 1st 8-buttons-of-16

PCA9655 pca1;

void I2C_Service()
{
	static unsigned char State = 0;
	
	State++;
	switch(State) {
	case 1 : // write to pca1 :
		PCA9655_Write(&pca1, 2);
		break;
	case 2 :// read from pca1 :
		PCA9655_Read(&pca1, 1);
		break;
	default :
		State = 0;
	}
}

#define SendIfChanged(new, old, id) do { if(old != new) { buf[len++] = id; buf[len++] = old = new;} } while(0)
unsigned char lastIncs;
void SendBoutons(void)
{
	static byte oldpca1, oldpca2;
	static byte buf[15];
	byte len = 0;
	
	buf[len++] = 'B';
	
	SendIfChanged(pca1.in1._byte, oldpca1, 10);
	
	if(len > 1) fraiseSend(buf,len+1);
}


void setup(void) {
//----------- Setup ----------------
	fruitInit();

	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	pinModeDigitalOut(LED1); 	// set the LED1 pin mode to digital out
	digitalClear(LED);		// clear the LED
	digitalClear(LED1);		// clear the LED1
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

//----------- Analog setup ----------------
	analogInit();		// init analog module

	analogSelect(0,K1);	// assign connector K1 to analog channel 0

	switchInit();
	INTCON2bits.RBPU = 0; // activate PORTB pullups

	switchSelect(0,K9);
	switchSelect(1,K10);
	switchSelect(2,K11);
	switchSelect(3,K12);
	switchSelect(4,MB1);
	switchSelect(5,MBEN);

//----------- setup I2C master ----------------
	i2cm_init(I2C_MASTER, I2C_SLEW_ON, FOSC/400000/4-1);
	
	PCA9655_Config(&pca1,PCA1ADD,255,0);

}

void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
	analogService();	// analog management routine
	switchService();	// switch routine
	fraiseService();	// listen to Fraise events
	I2C_Service();
	fraiseService();	// listen to Fraise events
	
	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 5000); 	// re-init mainDelay
		if(!switchSend()) analogSend();		// send analog channels that changed
		SendBoutons();
	}
}

// Receiving

void fraiseReceiveChar() // receive text
{
	unsigned char c;
	
	c=fraiseGetChar();
	if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		digitalWrite(LED, c!='0');		
		digitalWrite(K10, c!='0');	
	}
	else if(c=='E') { 	// echo text (send it back to host)
		printf("C");
		c = fraiseGetLen(); 			// get length of current packet
		while(c--) printf("%c",fraiseGetChar());// send each received byte
		putchar('\n');				// end of line
	}	
}

void fraiseReceive() // receive raw
{
	unsigned char c;
	BYTE c2;
	c=fraiseGetChar();
	switch(c) {
		PARAM_CHAR(10, c2._byte); pca1.out2._byte = 255 - c2._byte; break;
	}
}

