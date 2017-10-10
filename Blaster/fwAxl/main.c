/*********************************************************************
 *               analog example for Versa1.0
 *	Analog capture on connectors K1, K2, K3 and K5. 
 *********************************************************************/

#define BOARD Versa1

#include <fruit.h>
#include <analog.h>
#include <switch.h>
#include <ADXL345.h>
#include <i2c_master.h>

t_delay mainDelay;
#define LED0 K4
#define LED1 K11
#define LED2 K12
ADXL345 adxl1;

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

	pinModeDigitalOut(LED0); 	// set the LED pin mode to digital out
	digitalClear(LED0);
	pinModeDigitalOut(LED1); 	// set the LED pin mode to digital out
	digitalClear(LED1);
	pinModeDigitalOut(LED2); 	// set the LED pin mode to digital out
	digitalClear(LED2);
//----------- Analog setup ----------------
	analogInit();		// init analog module
	
	analogSelect(0,K1);	// assign connector K1 to analog channel 0
	analogSelect(1,K2);
	analogSelect(2,K3);
	analogSelect(3,K6);
	analogSelect(4,K8);
	analogSelect(5,K9);
	analogSelect(6,K10);
	analogSelect(7,MB1);
	analogSelect(8,MBEN);


	switchInit();
/*	switchSelect(0,K4);
	switchSelect(1,MA1);
	switchSelect(2,MAEN);
	switchSelect(3,K11);
	switchSelect(4,K12);
	switchSelect(5,MB1);
	switchSelect(6,MB2);
	switchSelect(7,MBEN);
	switchSelect(8,MBEN2);*/

//----------- setup I2C master ----------------
	i2cm_init(I2C_MASTER, I2C_SLEW_ON, FOSC/400000/4-1);
	
//----------- setup ADXL345 ----------------
	ADXL345Init(&adxl1, 0); // 1st ADXL345's SDO pin is high voltage level
	//ADXL345Init(&adxl2, 1); // 2nd ADXL345's SDO pin is low voltage level
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
		if(cycle%2) ADXL345Send(&adxl1, 1);
		else if(!switchSend()) analogSend();		// send analog channels that changed
		cycle++;
		ADXL345Service(&adxl1);
	}
}

// Receiving

void fraiseReceiveChar() // receive text
{
	unsigned char c, c2;
	
	c=fraiseGetChar();
	if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		digitalWrite(LED, c!='0');		
	}
	else if(c=='l'){		//switch LED on/off 
		c=fraiseGetChar();
		c2=fraiseGetChar();
		if(c == '0') digitalWrite(LED0, c2 != '0');		
		else if(c == '1') digitalWrite(LED1, c2 != '0');		
		else if(c == '2') digitalWrite(LED2, c2 != '0');		
	}
	else if(c=='E') { 	// echo text (send it back to host)
		printf("C");
		c = fraiseGetLen(); 			// get length of current packet
		while(c--) printf("%c",fraiseGetChar());// send each received byte
		putchar('\n');				// end of line
	}	
	else if(c=='R') { 	// reset I2C
		i2cm_init(I2C_MASTER, I2C_SLEW_ON, FOSC/400000/4-1);
		ADXL345Init(&adxl1, 0); // 1st ADXL345's SDO pin is high voltage level
	}
}

