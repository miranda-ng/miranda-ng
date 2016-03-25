#include "stdafx.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>

#define DC_SERIALIZED_MAGIC 0x868aa81d
#define STATE_FILE_MAGIC 0x28949a93
#define SECRET_CHAT_FILE_MAGIC 0x37a1988a



void read_state_file(struct tgl_state *TLS) {
	CMStringA name(FORMAT, "%sstate", TLS->base_path);
	int state_file_fd = open(name, O_CREAT | O_RDWR | O_BINARY, 0600);

	if (state_file_fd < 0) {
		return;
	}
	int version, magic;
	if (read(state_file_fd, &magic, 4) < 4) { close(state_file_fd); return; }
	if (magic != (int)STATE_FILE_MAGIC) { close(state_file_fd); return; }
	if (read(state_file_fd, &version, 4) < 4 || version < 0) { close(state_file_fd); return; }
	int x[4];
	if (read(state_file_fd, x, 16) < 16) {
		close(state_file_fd);
		return;
	}
	int pts = x[0];
	int qts = x[1];
	int seq = x[2];
	int date = x[3];
	close(state_file_fd);
	bl_do_set_seq(TLS, seq);
	bl_do_set_pts(TLS, pts);
	bl_do_set_qts(TLS, qts);
	bl_do_set_date(TLS, date);
	TLS->callback.logprintf("read state file: seq=%d pts=%d qts=%d date=%d", seq, pts, qts, date);
}

void write_state_file(struct tgl_state *TLS) {
	int wseq;
	int wpts;
	int wqts;
	int wdate;
	wseq = TLS->seq; wpts = TLS->pts; wqts = TLS->qts; wdate = TLS->date;

	CMStringA name(FORMAT, "%sstate", TLS->base_path);

	int state_file_fd = open(name, O_CREAT | O_RDWR | O_BINARY, 0600);

	if (state_file_fd < 0) {
		return;
	}
	int x[6];
	x[0] = STATE_FILE_MAGIC;
	x[1] = 0;
	x[2] = wpts;
	x[3] = wqts;
	x[4] = wseq;
	x[5] = wdate;
	assert(write(state_file_fd, x, 24) == 24);
	close(state_file_fd);
	TLS->callback.logprintf("wrote state file: wpts=%d wqts=%d wseq=%d wdate=%d", wpts, wqts, wseq, wdate);
}

void write_dc(struct tgl_dc *DC, void *extra) {
	int auth_file_fd = *(int *)extra;
	if (!DC) {
		int x = 0;
		assert(write(auth_file_fd, &x, 4) == 4);
		return;
	}
	else {
		int x = 1;
		assert(write(auth_file_fd, &x, 4) == 4);
	}

	assert(DC->flags & TGLDCF_LOGGED_IN);

	assert(write(auth_file_fd, &DC->options[0]->port, 4) == 4);
	int l = strlen(DC->options[0]->ip);
	assert(write(auth_file_fd, &l, 4) == 4);
	assert(write(auth_file_fd, DC->options[0]->ip, l) == l);
	assert(write(auth_file_fd, &DC->auth_key_id, 8) == 8);
	assert(write(auth_file_fd, DC->auth_key, 256) == 256);
}

void write_auth_file(struct tgl_state *TLS) {
	CMStringA name(FORMAT, "%sauth", TLS->base_path);
	int auth_file_fd = open(name, O_CREAT | O_RDWR | O_BINARY, 0600);
	if (auth_file_fd < 0) { return; }
	int x = DC_SERIALIZED_MAGIC;
	assert(write(auth_file_fd, &x, 4) == 4);
	assert(write(auth_file_fd, &TLS->max_dc_num, 4) == 4);
	assert(write(auth_file_fd, &TLS->dc_working_num, 4) == 4);

	tgl_dc_iterator_ex(TLS, write_dc, &auth_file_fd);

	assert(write(auth_file_fd, &TLS->our_id, 4) == 4);
	close(auth_file_fd);
	TLS->callback.logprintf("wrote auth file: magic=%d max_dc_num=%d dc_working_num=%d", x, TLS->max_dc_num, TLS->dc_working_num);
}

void read_dc(struct tgl_state *TLS, int auth_file_fd, int id, unsigned ver) {
	int port = 0;
	assert(read(auth_file_fd, &port, 4) == 4);
	int l = 0;
	assert(read(auth_file_fd, &l, 4) == 4);
	assert(l >= 0 && l < 100);
	char ip[100];
	assert(read(auth_file_fd, ip, l) == l);
	ip[l] = 0;

	long long auth_key_id;
	static unsigned char auth_key[256];
	assert(read(auth_file_fd, &auth_key_id, 8) == 8);
	assert(read(auth_file_fd, auth_key, 256) == 256);

	bl_do_dc_option(TLS, id, "DC", 2, ip, l, port);
	bl_do_set_auth_key(TLS, id, auth_key);
	bl_do_dc_signed(TLS, id);
	TLS->callback.logprintf("read dc: id=%d", id);
}

void empty_auth_file(struct tgl_state *TLS) {
	TLS->callback.logprintf("initializing empty auth file");
	if (TLS->test_mode) {
		bl_do_dc_option(TLS, 1, "", 0, TG_SERVER_TEST_1, strlen(TG_SERVER_TEST_1), 443);
		bl_do_dc_option(TLS, 2, "", 0, TG_SERVER_TEST_2, strlen(TG_SERVER_TEST_2), 443);
		bl_do_dc_option(TLS, 3, "", 0, TG_SERVER_TEST_3, strlen(TG_SERVER_TEST_3), 443);
		bl_do_set_working_dc(TLS, TG_SERVER_TEST_DEFAULT);
	}
	else {
		bl_do_dc_option(TLS, 1, "", 0, TG_SERVER_1, strlen(TG_SERVER_1), 443);
		bl_do_dc_option(TLS, 2, "", 0, TG_SERVER_2, strlen(TG_SERVER_2), 443);
		bl_do_dc_option(TLS, 3, "", 0, TG_SERVER_3, strlen(TG_SERVER_3), 443);
		bl_do_dc_option(TLS, 4, "", 0, TG_SERVER_4, strlen(TG_SERVER_4), 443);
		bl_do_dc_option(TLS, 5, "", 0, TG_SERVER_5, strlen(TG_SERVER_5), 443);
		bl_do_set_working_dc(TLS, TG_SERVER_DEFAULT);
	}
}

void read_auth_file(struct tgl_state *TLS) {
	CMStringA name(FORMAT, "%sauth", TLS->base_path);
	int auth_file_fd = open(name, O_CREAT | O_RDWR | O_BINARY, 0600);
	if (auth_file_fd < 0) {
		empty_auth_file(TLS);
		return;
	}
	assert(auth_file_fd >= 0);
	unsigned x;
	unsigned m;
	if (read(auth_file_fd, &m, 4) < 4 || (m != DC_SERIALIZED_MAGIC)) {
		close(auth_file_fd);
		empty_auth_file(TLS);
		return;
	}
	assert(read(auth_file_fd, &x, 4) == 4);
	assert(x > 0);
	int dc_working_num;
	assert(read(auth_file_fd, &dc_working_num, 4) == 4);

	int i;
	for (i = 0; i <= (int)x; i++) {
		int y;
		assert(read(auth_file_fd, &y, 4) == 4);
		if (y) {
			read_dc(TLS, auth_file_fd, i, m);
		}
	}
	bl_do_set_working_dc(TLS, dc_working_num);
	int our_id;
	int l = read(auth_file_fd, &our_id, 4);
	if (l < 4) {
		assert(!l);
	}
	if (our_id) {
		bl_do_set_our_id(TLS, our_id);
	}
	close(auth_file_fd);
	TLS->callback.logprintf("read auth file: dcs=%d dc_working_num=%d our_id=%d", x, dc_working_num, our_id);
}







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


void CTelegramProto::SaveAuth()
{
}
