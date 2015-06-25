#pragma once

#define CMIF_NOTNOTPRIVATE		CMIF_NOTOFFLINE
#define CMIF_NOTUNVERIFIED		CMIF_NOTONLINE
#define CMIF_NOTPRIVATE			CMIF_NOTONLIST 
#define CMIF_NOTFINISHED		CMIF_NOTOFFLIST
#define CMIF_DISABLED			0x8000

void InitMirOTRMenu(void);
void UninitMirOTRMenu(void);

void ShowOTRMenu(MCONTACT hContact, POINT pt);
