/*
 * This file is for testing purposes. Save in header.txt your problem header and you can
 * browse through functions to get result
 *
 * (c) majvan 2002-2004
 */

#include <stdio.h>
#include "../m_mails.h"

extern void WINAPI TranslateHeaderFcn(char *stream,int len,struct CMimeItem **head);
extern void ExtractHeader(struct CMimeItem *items,int CP,struct CHeader *head);

void main()
{
	char Buffer[8192];			//we do not suppose longer header
	FILE *fp;
	YAMNMAIL *Mail;
	PMAILDATA *MailData;
	CMimeItem *head;

	struct CHeader ExtractedHeader;

	if(NULL==(fp=fopen("header2.txt","r")))
		return;
	fread(Buffer,sizeof(Buffer),1,fp);
	if(ferror(fp))
	{
		fclose(fp);
		return;
	}
	fclose(fp);
	Mail = new YAMNMAIL;
	MailData = new PMAILDATA;
	head = new CMimeItem;
	Mail->MailData = *MailData;
	Mail->MailData->TranslatedHeader = head;

	TranslateHeaderFcn(Buffer,strlen(Buffer), &Mail->MailData->TranslatedHeader);
	ExtractHeader(Mail->MailData->TranslatedHeader,CP_ACP,&ExtractedHeader);
	return;
}