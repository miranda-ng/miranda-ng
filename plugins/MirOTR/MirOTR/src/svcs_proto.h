#pragma once

INT_PTR SVC_OTRSendMessage(WPARAM wParam,LPARAM lParam);
INT_PTR SVC_OTRRecvMessage(WPARAM wParam,LPARAM lParam);

void otr_abort_smp(ConnContext *context);

void otr_start_smp(ConnContext *context, const char *question,
	const unsigned char *secret, size_t secretlen);

void otr_continue_smp(ConnContext *context,
	const unsigned char *secret, size_t secretlen);