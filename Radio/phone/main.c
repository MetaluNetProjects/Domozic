/*********************************************************************
 *               analog example for Versa1.0
 *	Analog capture on connectors K1, K2, K3 and K5. 
 *********************************************************************/

#define BOARD Versa1

#include <fruit.h>
#include <ADXL345.h>
#include <i2c_master.h>
#include <switch.h>

t_delay mainDelay, pulseDelay;

unsigned char keyphase = 0, keycols[4], oldkeycols[4];

ADXL345 adxl1;
//ADXL345 adxl2;

void keysService() 
{
	pinModeDigitalIn(KEYC1);
	pinModeDigitalIn(KEYC2);
	pinModeDigitalIn(KEYC3);
	pinModeDigitalIn(KEYC4);
	digitalClear(KEYC1);
	digitalClear(KEYC2);
	digitalClear(KEYC3);
	digitalClear(KEYC4);
	
	if(keyphase == 0) pinModeDigitalOut(KEYC1);
	else if(keyphase == 1) pinModeDigitalOut(KEYC2);
	else if(keyphase == 2) pinModeDigitalOut(KEYC3);
	else if(keyphase == 3) pinModeDigitalOut(KEYC4);
	
	digitalClear(KEYC1);
	digitalClear(KEYC2);
	digitalClear(KEYC3);
	digitalClear(KEYC4);
	Nop();
	Nop();
	
	keycols[keyphase] = (digitalRead(KEYL1)==0) + 2*(digitalRead(KEYL2)==0) + 4*(digitalRead(KEYL3)==0) + 8*(digitalRead(KEYL4)==0);
	keyphase++;
	if(keyphase == 4) keyphase = 0;
}

void sendKeys()
{
	if((keycols[0] != oldkeycols[0])
	|| (keycols[1] != oldkeycols[1])
	|| (keycols[2] != oldkeycols[2])
	|| (keycols[3] != oldkeycols[3]))
	{
		putchar('B');
		putchar('A');
		putchar('0' + keycols[0]);
		putchar('0' + keycols[1]);
		putchar('0' + keycols[2]);
		putchar('0' + keycols[3]);
		putchar('\n');
		
		oldkeycols[0] = keycols[0];
		oldkeycols[1] = keycols[1];
		oldkeycols[2] = keycols[2];
		oldkeycols[3] = keycols[3];
	}
}


//----------- Setup ----------------
	
void setup(void) {	
	fruitInit();
//	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
//	digitalClear(LED);		// clear the LED
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

	switchInit();
	switchSelect(0, SW1);
	
	pinModeDigitalOut(LED1);
	digitalClear(LED1);
	pinModeDigitalOut(LED2);
	digitalClear(LED2);
	
	pinModeDigitalIn(KEYL1);
	pinModeDigitalIn(KEYL2);
	pinModeDigitalIn(KEYL3);
	pinModeDigitalIn(KEYL4);
	
	keycols[0] = oldkeycols[0] = 0;
	keycols[1] = oldkeycols[1] = 0;
	keycols[2] = oldkeycols[2] = 0;
	keycols[3] = oldkeycols[3] = 0;
	
//----------- setup I2C master ----------------
	i2cm_init(I2C_MASTER, I2C_SLEW_ON, FOSC/400000/4-1);
	
//----------- setup ADXL345 ----------------
	ADXL345Init(&adxl1, 0); // 1st ADXL345's SDO pin is high voltage level
	//ADXL345Init(&adxl2, 1); // 2nd ADXL345's SDO pin is low voltage level
}

// ---------- Main loop ------------

unsigned char cycle = 0;

void loop() {
	fraiseService();	// listen to Fraise events
	keysService();
	switchService();
	fraiseService();	// listen to Fraise events
	ADXL345Service(&adxl1);
  
	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 5000); 	// re-init mainDelay		
		if(cycle%2) ADXL345Send(&adxl1, 1);
		else sendKeys();
		switchSend();
		cycle++;
	}
}


// ---------- Receiving ------------

void fraiseReceiveChar() // receive text
{
	unsigned char c;
	
	c=fraiseGetChar();
	if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		digitalWrite(LED1, (c&1)!=0);
		digitalWrite(LED2, (c&2)!=0);
		//digitalWrite(LED, c!='0');
	}
	else if(c=='E') { 	// echo text (send it back to host)
		printf("C");
		c = fraiseGetLen(); 			// get length of current packet
		while(c--) printf("%c",fraiseGetChar());// send each received byte
		putchar('\n');				// end of line
	}	
	else if(c=='R') { 	// echo text (send it back to host)
		i2cm_init(I2C_MASTER, I2C_SLEW_ON, FOSC/400000/4-1);
		ADXL345Init(&adxl1, 0); // 1st ADXL345's SDO pin is high voltage level
	}
}

void fraiseReceive() // receive raw
{
	unsigned char c,c2;
	//unsigned int i;
	c=fraiseGetChar();
	
}

