/*********************************************************************
 *               analog example for Versa1.0
 *	Analog capture on connectors K1, K2, K3 and K5. 
 *********************************************************************/

#define BOARD Versa1

#include <fruit.h>
#include <ADXL345.h>
#include <i2c_master.h>
#include <switch.h>
#include <analog.h>

t_delay mainDelay, pulseDelay;

ADXL345 adxl1;
//ADXL345 adxl2;

#define INDUCT_DRIVE K9
#define INDUCT_MES K10
int inductance;

//----------- Setup ----------------
	
void setup(void) {	
	fruitInit();
	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	digitalClear(LED);		// clear the LED
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

	INTCON2bits.RBPU = 0; // activate PORTB pullups
	switchInit();
	switchSelect(0, K1);
	switchSelect(1, K2);
	switchSelect(2, K3);
	switchSelect(3, K4);
	switchSelect(4, K6);
	switchSelect(5, K8);
	switchSelect(6, K11);
	switchSelect(7, K12);
	switchSelect(8, MB1);
	switchSelect(9, MBEN);
	switchSelect(10, MBEN2);
		
//----------- setup I2C master ----------------
	i2cm_init(I2C_MASTER, I2C_SLEW_ON, FOSC/400000/4-1);
	
//----------- setup ADXL345 ----------------
	ADXL345Init(&adxl1, 0); // 1st ADXL345's SDO pin is high voltage level
	//ADXL345Init(&adxl2, 1); // 2nd ADXL345's SDO pin is low voltage level

///----------- setup CTMU for inductance measuring ----------------

/*	EDGSEQEN: Edge Sequence Enable bit:
	1 = Edge 1 event must occur before Edge 2 event can occur
	0 = No edge sequence is needed*/
	/*CTMUCONHbits.EDGSEQEN = 1;
	CTMUCONHbits.EDGEN = 1;
	CTMUCONL = 0b11011100; // positive edges on CTED1/2
	CTMUICON = 0x00;			//no source current
	CTMUCONHbits.CTMUEN = 1;	//Enable CTMU*/

	pinModeDigitalOut(INDUCT_DRIVE);
	digitalClear(INDUCT_DRIVE);
	pinModeAnalogIn(INDUCT_MES);
	
	/*IPR3bits.CTMUIP = 1; // set CTMU interrupt to high priority
	PIE3bits.CTMUIE = 0; // CTMU interrupt disabled 
	CTMUCONLbits.EDG1STAT = 0;
	CTMUCONLbits.EDG2STAT = 0;*/

	analogInit();
	//ADCON2=0b10000110; //right justified, 0 TAD, fOSC/64
	ADCON2=0b10001110; //right justified, 0 TAD, fOSC/64
	//fOSC/2=
	// tIdealSample = 100ns->250ns : 
}

void InductanceService()
{
	static unsigned char phase = 0;
	if((phase&1) == 0) {
		if(ADCON0bits.GO) return;
	//if(PIE3bits.CTMUIE == 0) {
		inductance = inductance - (inductance>>4) + ADRESL + (ADRESH << 8);
		ADCON0=(KAN(INDUCT_MES) << 2) + 1;
		//inductance = inductanceISR;
		digitalClear(INDUCT_DRIVE);
		/*CTMUCONLbits.EDG1STAT = 0;
		CTMUCONLbits.EDG2STAT = 0;
		PIR3bits.CTMUIF = 0;*/
		//PIE3bits.CTMUIE = 1; // CTMU interrupt enabled 
		//inductanceStart = time();
		//CTMUCONLbits.EDG1STAT = 1;
		//digitalSet(INDUCT_DRIVE);
	} else {
		__critical {
		digitalSet(INDUCT_DRIVE);
		Nop();
		//Nop(); Nop(); Nop(); Nop();
		ADCON0bits.GO = 1;
		}
		/*Nop(); Nop(); Nop(); Nop(); Nop(); Nop();
		Nop(); Nop(); Nop(); Nop(); Nop(); Nop();
		Nop(); Nop(); Nop(); Nop(); Nop(); Nop();
		Nop(); Nop(); Nop(); Nop(); Nop(); Nop();
		Nop(); Nop(); Nop(); Nop(); Nop(); Nop();
		Nop(); Nop(); Nop(); Nop(); Nop(); Nop();
		Nop(); Nop(); Nop(); Nop(); Nop(); Nop();
		Nop(); Nop(); Nop(); Nop(); Nop(); Nop();
		Nop(); Nop(); Nop(); Nop(); Nop(); Nop();
		Nop(); Nop(); Nop(); Nop(); Nop(); Nop();
		Nop(); Nop(); Nop(); Nop(); Nop(); Nop();
		Nop(); Nop(); Nop(); Nop(); Nop(); Nop();*/
	}
	phase++;
}

/*void HighInterrupts()
{
	if(PIR3bits.CTMUIF) {
		inductanceISR = elapsedISR(inductanceStart);
		PIR3bits.CTMUIF = 0;
		PIE3bits.CTMUIE = 0; // CTMU interrupt disabled
		digitalClear(INDUCT_DRIVE);
		CTMUCONLbits.EDG1STAT = 0;
	}
}*/
// ---------- Main loop ------------

unsigned char cycle = 0;
//unsigned char buf[
void loop() {
	fraiseService();	// listen to Fraise events
	switchService();
	fraiseService();	// listen to Fraise events
	ADXL345Service(&adxl1);
	fraiseService();	// listen to Fraise events
	InductanceService();  
	fraiseService();	// listen to Fraise events
	
	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 5000); 	// re-init mainDelay		
		if(!switchSend()) ADXL345Send(&adxl1, 1);
		if((cycle&3) == 0) {
			printf("C I %ld\n", inductance);
		}
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
		/*digitalWrite(LED1, (c&1)!=0);
		digitalWrite(LED2, (c&2)!=0);*/
		digitalWrite(LED, c!='0');
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
	if(c==1) {
		ADCON2 = fraiseGetChar();
	}
}

