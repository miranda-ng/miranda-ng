#ifndef __C8C6FB80_D66B_4382_8FAB_E92C83F77BB8_IsWithinAcuracy_h__
#define __C8C6FB80_D66B_4382_8FAB_E92C83F77BB8_IsWithinAcuracy_h__

inline bool IsWithinAccuracy(double dValue1, double dValue2, double dAccuracy = 1e-4)
{
	double dDifference = dValue1 - dValue2;

	if ((-dAccuracy <= dDifference) && (dDifference <= dAccuracy))
		return true;
	else
		return false;
}


#endif //__C8C6FB80_D66B_4382_8FAB_E92C83F77BB8_IsWithinAcuracy_h__
