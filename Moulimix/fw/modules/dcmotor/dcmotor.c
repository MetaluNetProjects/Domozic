/*********************************************************************
 *
 *                DC motor library for Fraise pic18f  device
 *				
 *				
 *********************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Antoine Rousseau  2014     Original.
 ********************************************************************/
/*
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA  02110-1301, USA.
*/

#include "dcmotor.h"

static long incDeltaT, lastIncTime, error;
long dcmotorDeltaT;//, dcmotorComputedSpeed;
static int pos;
int dcmotor_v, dcmotor_vabs, dcmotorDeltaPos;
t_dcmotorVars dcmotorVars;
t_dcmotorVolVars dcmotorVolVars;
static t_dcmotorSetting S; 

void dcmotorCompute(t_dcmotor *mot)
{

#define MAXSPEED ((unsigned long)(1UL<<24))
	
	//done by macro
	/*__critical { 
		//memcpy(&dcmotorVolVars,&(mot->VolVars),sizeof(dcmotorVolVars));
		dcmotorVolVars.lastIncTime = mot->VolVars.lastIncTime;
		dcmotorVolVars.IncDeltaT = mot->VolVars.IncDeltaT;
		dcmotorVolVars.Position = mot->VolVars.Position;
		dcmotorVolVars.flags = mot->VolVars.flags;
		mot->VolVars.IncDeltaT = 0;
	}*/

	memcpy(&dcmotorVars,&(mot->Vars),sizeof(dcmotorVars));
	memcpy(&S,&(mot->Setting),sizeof(S));
		
	if(/*(S.Mode == 1) || (S.Mode == 3)*/1) {
		dcmotorDeltaPos = dcmotorVolVars.Position - dcmotorVars.lastPosition;
		dcmotorVars.lastPosition = dcmotorVolVars.Position;

		if(!dcmotorDeltaPos) {
			dcmotorDeltaT = elapsed(dcmotorVolVars.lastIncTime);
			if(!dcmotorVars.stalled) {
				if((dcmotorDeltaT>>18) > S.StallTime) dcmotorVars.stalled = 1;	// more than (8<<18)/2 us = 1 sec -> speed=0
			}
			/*if(!dcmotorVars.stalled) {
				if(dcmotorDeltaT < dcmotorVars.lastIncDeltaT) dcmotorDeltaT = dcmotorVars.lastIncDeltaT; // 
				dcmotorDeltaPos = dcmotorVolVars.direction ? 1 : -1; //make like a new step was occuring now, at the same rate the previous one.
			}*/
			if((!dcmotorVars.stalled) && (dcmotorDeltaT > dcmotorVars.lastIncDeltaT)){
				/*dcmotorVars.computedSpeed = (dcmotorVars.computedSpeed * ((dcmotorVars.lastIncDeltaT * 1024) /dcmotorDeltaT))/1024;*/
				dcmotorVars.computedSpeed = (dcmotorVolVars.direction ? 1 : -1) * 
											(MAXSPEED/((unsigned long)(dcmotorDeltaT)));
			}
		}
		else {
			dcmotorVars.stalled = 0;
			dcmotorDeltaT = dcmotorVolVars.IncDeltaT;
			dcmotorVars.lastIncDeltaT = dcmotorDeltaT;
			dcmotorVars.computedSpeed = dcmotorDeltaPos * (MAXSPEED/((unsigned long)(dcmotorDeltaT)));
		}
		
		if(dcmotorDeltaT <= 0) dcmotorVars.computedSpeed = 0;		
	}
	
	if((S.Mode >= 2) && (dcmotorVolVars.homed)) {
		rampCompute(&mot->PosRamp);

		error = (long)(mot->PosRamp.currentPos>>(RAMP_UINCPOW)) - (dcmotorVolVars.Position) ;

		if((error < 0) && (error >= -S.PosWindow)) error = 0;
		if((error > 0) && (error <= S.PosWindow)) error = 0;

		error <<= S.PosErrorGain; //7;
		if(error >= (1L<<15)) error = (1L<<15)-1;
		else if(error < -(1L<<15)) error = -(1L<<15);

		pidCompute(&mot->PosPID,(int)error);
		
		if(S.Mode == 3) dcmotorVars.SpeedConsign = mot->PosPID.Out >> 8;
		else dcmotorVars.PWMConsign = mot->PosPID.Out >> 8;
	}
	
	if((S.Mode == 1) || (S.Mode == 3)) {
		error = (long)(dcmotorVars.SpeedConsign) - (long)dcmotorVars.computedSpeed;
	
		if(error >= (1L<<15)) error = (1L<<15)-1;
		else if(error < -(1L<<15)) error = -(1L<<15);
		
		pidCompute(&mot->SpeedPID, (int)error);
	
		dcmotorVars.PWMConsign = mot->SpeedPID.Out>>8;
	}
	memcpy(&(mot->Vars),&dcmotorVars,sizeof(dcmotorVars));
}


void dcmotorInput(t_dcmotor *mot)
{
	unsigned char c,c2;
	unsigned int i = 0;

	c=fraiseGetChar();
	if(c == 254) {
		fraiseSendCopy();
		c2=fraiseGetChar();		
		switch(c2) {
			GETPARAM(3, mot->Vars.SpeedConsign, i);
			GETPARAM(4, mot->Vars.PWMConsign, i);
			GETPARAM(5, (char)mot->Setting.reversed, i);
		}
		printf("%d %d\n", c2, i);
	}
	else switch(c) {
		case 0 : rampInput(&mot->PosRamp); mot->Setting.Mode = 2; break;
		case 1 : pidInput(&mot->PosPID); break;
		case 2 : pidInput(&mot->SpeedPID); break;
		PARAM_INT(3, mot->Vars.SpeedConsign); mot->Setting.Mode = 1; break;
		PARAM_INT(4, mot->Vars.PWMConsign); mot->Setting.Mode = 0; break;
		PARAM_CHAR(5, mot->Setting.reversed); break;
		PARAM_CHAR(6, c2); mot->VolVars.homed = (c2!=0); if(c2==1) {
				mot->VolVars.Position = 0;
				rampSetPos(&mot->PosRamp,0);
			}
			break;
	}
}

void dcmotorDeclareEE(t_dcmotor *mot)
{
	EEdeclareChar(&mot->Setting.flags);
	rampDeclareEE(&mot->PosRamp);
	pidDeclareEE(&mot->PosPID);
	pidDeclareEE(&mot->SpeedPID);
}


