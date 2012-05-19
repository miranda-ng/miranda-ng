#pragma once

int WindowEvent(WPARAM wParam, LPARAM lParam);
int SVC_IconPressed(WPARAM wParam, LPARAM lParam);
void SetEncryptionStatus(HANDLE hContact, TrustLevel level);
void InitSRMM();
void DeinitSRMM();