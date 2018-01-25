/*********************************************************************
 *               analog example for Versa1.0
 *	Analog capture on connectors K1, K2, K3 and K5. 
 *********************************************************************/

#define BOARD 8X2A

#include <fruit.h>
#include <analog.h>
#include <switch.h>
#include <ADXL345.h>
#include <i2c_master.h>

t_delay mainDelay;
ADXL345 adxl1;
ADXL345 adxl2;

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
//	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

//----------- Analog setup ----------------
	analogInit();		// init analog module	
	//analogSelect(0,K1);	// assign connector K1 to analog channel 0

	switchInit();
//	switchSelect(0,K4);

//----------- setup I2C master ----------------
	i2cm_init(I2C_MASTER, I2C_SLEW_ON, FOSC/400000/4-1);
	
//----------- setup ADXL345 ----------------
	ADXL345Init(&adxl1, 0); // 1st ADXL345's SDO pin is high voltage level
	ADXL345Init(&adxl2, 1); // 2nd ADXL345's SDO pin is hi voltage level
}

unsigned char cycle = 0;

void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
	analogService();	// analog management routine
	switchService();	// 
	fraiseService();	// listen to Fraise events

	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 5000); 	// re-init mainDelay
		if((cycle&1) == 0) ADXL345Send(&adxl1, 1);
		else ADXL345Send(&adxl2, 2);
		if(!switchSend()) analogSend();		// send analog channels that changed
		cycle++;
		fraiseService();	// listen to Fraise events
		ADXL345Service(&adxl1);
		fraiseService();	// listen to Fraise events
		ADXL345Service(&adxl2);
	}
}

// Receiving

void fraiseReceiveChar() // receive text
{
	unsigned char c, c2;
	
	c=fraiseGetChar();
	/*if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		digitalWrite(LED, c!='0');		
	}
	else */
	if(c=='E') { 	// echo text (send it back to host)
		printf("C");
		c = fraiseGetLen(); 			// get length of current packet
		while(c--) printf("%c",fraiseGetChar());// send each received byte
		putchar('\n');				// end of line
	}	
	else if(c=='R') { 	// reset I2C
		i2cm_init(I2C_MASTER, I2C_SLEW_ON, FOSC/400000/4-1);
		ADXL345Init(&adxl1, 0);
		ADXL345Init(&adxl2, 1);
	}
}

