#pragma once
void VerifyContextDialog(ConnContext* context);
void SMPInitDialog(ConnContext* context);
void SMPDialogUpdate(ConnContext *context, int percent);
void SMPDialogReply(ConnContext *context, const char* question);

#define WMU_REFRESHSMP			(WM_USER + 245)
