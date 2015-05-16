#pragma once

typedef signed char sbyte;
typedef struct _MODERNEFFECTMATRIX
{
	sbyte matrix[25];
	BYTE  topEffect;
	BYTE  leftEffect;
	BYTE  rightEffect;
	BYTE  bottomEffect;
	BYTE  cycleCount;	  //low 7 bits
}MODERNEFFECTMATRIX;

typedef  struct _MODERNEFFECT
{
	BYTE EffectID;
	MODERNEFFECTMATRIX EffectMatrix;
	DWORD EffectColor1;
	DWORD EffectColor2;
}MODERNEFFECT;

#ifdef _EFFECTENUM_FULL_H
TCHAR * ModernEffectNames[] =
#else
TCHAR * _ModernEffectNames[] =
#endif
{
	_T("Shadow at left"),
	_T("Shadow at right"),
	_T("Outline"),
	_T("Outline smooth"),
	_T("Smooth bump"),
	_T("Contour thin"),
	_T("Contour heavy"),
};

#ifdef _EFFECTENUM_FULL_H
MODERNEFFECTMATRIX ModernEffectsEnum[] = {
	{   //Shadow at Left
		{ 0, 0, 0, 0, 0,
		0, 4, 16, 4, 4,
		0, 16, 64, 32, 16,
		0, 4, 32, 32, 16,
		0, 4, 16, 16, 16 }, 2, 2, 2, 2, 1 },
		{   //Shadow at Right
			{ 0, 0, 0, 0, 0,
			4, 4, 16, 4, 0,
			16, 32, 64, 16, 0,
			16, 32, 32, 4, 0,
			16, 16, 16, 4, 0 }, 2, 2, 2, 2, 1 },
			{   //Outline
				{ 0, 0, 0, 0, 0,
				0, 16, 16, 16, 0,
				0, 16, 32, 16, 0,
				0, 16, 16, 16, 0,
				0, 0, 0, 0, 0 }, 1, 1, 1, 1, 1 },

				{  //Outline smooth
					{ 4, 4, 4, 4, 4,
					4, 8, 8, 8, 4,
					4, 8, 32, 8, 4,
					4, 8, 8, 8, 4,
					4, 4, 4, 4, 4 }, 2, 2, 2, 2, 1 },

					{  //Smooth bump
						{ -2, 2, 2, 2, 2,
						-2, -16, 16, 16, 2,
						-2, -16, 48, 16, 2,
						-2, -16, -16, 16, 2,
						-2, -2, -2, -2, -2 }, 2, 2, 2, 2, 1 + 0x80 },
						{  //Contour thin
							{ 0, 0, 0, 0, 0,
							0, 48, 64, 48, 0,
							0, 64, 64, 64, 0,
							0, 48, 64, 48, 0,
							0, 0, 0, 0, 0 }, 1, 1, 1, 1, 1 },
							{  //Contour heavy
								{ 8, 16, 16, 16, 8,
								16, 64, 64, 64, 16,
								16, 64, 64, 64, 16,
								16, 64, 64, 64, 16,
								8, 16, 16, 16, 8 }, 2, 2, 2, 2, 1 },

};
#endif
#ifdef _EFFECTENUM_FULL_H
#define MAXPREDEFINEDEFFECTS sizeof(ModernEffectNames)/sizeof(ModernEffectNames[0])
#else
#define MAXPREDEFINEDEFFECTS sizeof(_ModernEffectNames)/sizeof(_ModernEffectNames[0])
extern TCHAR * ModernEffectNames[];
#endif
extern BOOL SkinEngine_ResetTextEffect(HDC);
extern BOOL SkinEngine_SelectTextEffect(HDC hdc, BYTE EffectID, DWORD FirstColor, DWORD SecondColor);