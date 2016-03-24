#include "stdafx.h"

void CTelegramProto::ReadState()
{
	DBVARIANT dbv = { 0 };
	if (db_get(0, m_szModuleName, "TGL_STATE", &dbv))
		return;

	int *x = (int*)dbv.pbVal;

	bl_do_set_seq(TLS, x[0]);
	bl_do_set_pts(TLS, x[1]);
	bl_do_set_qts(TLS, x[2]);
	bl_do_set_date(TLS, x[3]);

	db_free(&dbv);
}

void CTelegramProto::SaveState()
{
	int x[4];

	x[0] = TLS->pts;
	x[1] = TLS->qts;
	x[2] = TLS->seq;
	x[3] = TLS->date;

	db_set_blob(0, m_szModuleName, "TGL_STATE", &x, sizeof(x));

}

void read_dc(tgl_state *TLS, int *&piBlob, int id) {
	int port = *piBlob++;
	int l = *piBlob++;
	assert(l >= 0 && l < 100);

	char ip[100];
	memcpy(ip, piBlob, l);
	piBlob += (l / sizeof(int));
	ip[l] = 0;

	long long auth_key_id = *(long long*)piBlob;
	piBlob += 2;
	static unsigned char auth_key[256];
	memcpy(auth_key, piBlob, 256);
	piBlob += (256 / sizeof(int));

	bl_do_dc_option(TLS, id, "DC", 2, ip, l, port);
	bl_do_set_auth_key(TLS, id, auth_key);
	bl_do_dc_signed(TLS, id);
}


void CTelegramProto::ReadAuth()
{
	DBVARIANT dbv = { 0 };
	if (db_get(0, m_szModuleName, "TGL_AUTH", &dbv))
	{
		bl_do_dc_option(TLS, 1, "", 0, TG_SERVER_1, strlen(TG_SERVER_1), 443);
		bl_do_dc_option(TLS, 2, "", 0, TG_SERVER_2, strlen(TG_SERVER_2), 443);
		bl_do_dc_option(TLS, 3, "", 0, TG_SERVER_3, strlen(TG_SERVER_3), 443);
		bl_do_dc_option(TLS, 4, "", 0, TG_SERVER_4, strlen(TG_SERVER_4), 443);
		bl_do_dc_option(TLS, 5, "", 0, TG_SERVER_5, strlen(TG_SERVER_5), 443);
		bl_do_set_working_dc(TLS, TG_SERVER_DEFAULT);
		return;
	}

	int *piBlob = (int*)dbv.pbVal;

	size_t x = (size_t)*piBlob++;
	int dc_working_num = *piBlob++;

	for (size_t i = 0; i < x; i++)
	{
		int y = *piBlob++;
		if (y)
		{
			read_dc(TLS, piBlob, i);
		}
	}
	bl_do_set_working_dc(TLS, dc_working_num);

	int our_id = *piBlob++;

	if (our_id) {
		bl_do_set_our_id(TLS, TGL_MK_USER(our_id).id);
	}

	db_free(&dbv);
}