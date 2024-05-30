#include "stdafx.h"

#include <m_voiceservice.h>

#include <gst/gst.h>
#include <gst/sdp/sdp.h>
#include <gst/rtp/rtp.h>

#define GST_USE_UNSTABLE_API
#include <gst/webrtc/webrtc.h>

bool GetCandidateProp(char *output, byte maxlen, const char *candidate, const char *prop)
{
	const char *pprop = strstr(candidate, prop);
	if (!pprop)
		return false;

	const char *val = pprop + strlen(prop);
	while (*val == ' ') val++;
	int i = 0;
	while (*val != 0 && *val != ' ' && i < maxlen - 1)
		output[i++] = *val++;
	output[i] = 0;

	return i > 0;
}

static void handle_media_stream(GstPad *pad, GstElement *pipe, const char *convert_name, const char *sink_name)
{
	GstPad *qpad;
	GstElement *q, *conv, *resample, *sink;
	GstPadLinkReturn ret;

	gst_print("Trying to handle stream with %s ! %s", convert_name, sink_name);

	q = gst_element_factory_make("queue", NULL);
	g_assert_nonnull(q);
	conv = gst_element_factory_make(convert_name, NULL);
	g_assert_nonnull(conv);
	sink = gst_element_factory_make(sink_name, NULL);
	g_assert_nonnull(sink);

	if (g_strcmp0(convert_name, "audioconvert") == 0) {
		/* Might also need to resample, so add it just in case.
		 * Will be a no-op if it's not required. */
		resample = gst_element_factory_make("audioresample", NULL);
		g_assert_nonnull(resample);
		gst_bin_add_many(GST_BIN(pipe), q, conv, resample, sink, NULL);
		gst_element_sync_state_with_parent(q);
		gst_element_sync_state_with_parent(conv);
		gst_element_sync_state_with_parent(resample);
		gst_element_sync_state_with_parent(sink);
		gst_element_link_many(q, conv, resample, sink, NULL);
	}
	else {
		gst_bin_add_many(GST_BIN(pipe), q, conv, sink, NULL);
		gst_element_sync_state_with_parent(q);
		gst_element_sync_state_with_parent(conv);
		gst_element_sync_state_with_parent(sink);
		gst_element_link_many(q, conv, sink, NULL);
	}

	qpad = gst_element_get_static_pad(q, "sink");

	ret = gst_pad_link(pad, qpad);
	g_assert_cmphex(ret, == , GST_PAD_LINK_OK);
}

static void on_incoming_decodebin_stream(GstElement * /*decodebin*/, GstPad *pad, GstElement *pipe)
{
	GstCaps *caps;
	const gchar *name;

	if (!gst_pad_has_current_caps(pad)) {
		gst_printerr("Pad '%s' has no caps, can't do anything, ignoring\n", GST_PAD_NAME(pad));
		return;
	}

	caps = gst_pad_get_current_caps(pad);
	name = gst_structure_get_name(gst_caps_get_structure(caps, 0));

	if (g_str_has_prefix(name, "video")) {
		handle_media_stream(pad, pipe, "videoconvert", "autovideosink");
	}
	else if (g_str_has_prefix(name, "audio")) {
		handle_media_stream(pad, pipe, "audioconvert", "autoaudiosink");
	}
	else {
		gst_printerr("Unknown pad %s, ignoring", GST_PAD_NAME(pad));
	}
}

static void on_incoming_stream_cb(GstElement */*webrtc*/, GstPad *pad, GstElement *pipe)
{
	GstElement *decodebin;
	GstPad *sinkpad;

	if (GST_PAD_DIRECTION(pad) != GST_PAD_SRC)
		return;

	decodebin = gst_element_factory_make("decodebin", NULL);
	g_signal_connect(decodebin, "pad-added", G_CALLBACK(on_incoming_decodebin_stream), pipe);
	gst_bin_add(GST_BIN(pipe), decodebin);
	gst_element_sync_state_with_parent(decodebin);

	sinkpad = gst_element_get_static_pad(decodebin, "sink");
	gst_pad_link(pad, sinkpad);
	gst_object_unref(sinkpad);
}

void on_offer_created_cb(GstPromise *promise, gpointer user_data)
{
	GstWebRTCSessionDescription *offer = NULL;
	CJabberAccount *jproto = (CJabberAccount *)user_data;

	GstStructure const *reply = gst_promise_get_reply(promise);
	gst_structure_get(reply, jproto->m_isOutgoing ? "offer" : "answer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &offer, NULL);
	gst_promise_unref(promise);
	if (!offer) {
		gst_print("Cannot process sdp");
		return;
	}

	GstPromise *local_desc_promise = gst_promise_new();
	g_signal_emit_by_name(jproto->m_webrtc1, "set-local-description", offer, local_desc_promise);
	gst_promise_interrupt(local_desc_promise);
	gst_promise_unref(local_desc_promise);

	gchar *sdp_string = gst_sdp_message_as_text(offer->sdp);
	gst_print("VOIP - Wanna send SDP offer:\r\n%s\r\n", sdp_string);
	g_free(sdp_string);

	const GstSDPMedia *media_audio = NULL;
	for (unsigned int i = 0; i < gst_sdp_message_medias_len(offer->sdp); i++) {
		const GstSDPMedia *m = gst_sdp_message_get_media(offer->sdp, i);
		if (!strcmp(m->media, "audio"))
			media_audio = m;
	}
	if (!media_audio) {
		gst_print("No audio media in SDP");
		return;
	}

	jproto->m_voipICEPwd = gst_sdp_media_get_attribute_val(media_audio, "ice-pwd");
	jproto->m_voipICEUfrag = gst_sdp_media_get_attribute_val(media_audio, "ice-ufrag");
	jproto->m_medianame = gst_sdp_media_get_attribute_val(media_audio, "mid");

	// send it all
	bool outgoing = jproto->m_isOutgoing;
	XmlNodeIq iq("set", jproto->m_api->GetSerialNext(), jproto->m_voipPeerJid);
	TiXmlElement *rjNode = iq << XCHILDNS("jingle", JABBER_FEAT_JINGLE);
	rjNode << XATTR("sid", jproto->m_voipSession)
		<< XATTR("action", outgoing ? "session-initiate" : "session-accept")
		<< XATTR("initiator", outgoing ? jproto->m_api->GetFullJid() : jproto->m_voipPeerJid);
	if (!outgoing)
		rjNode << XATTR("responder", jproto->m_api->GetFullJid());

	TiXmlElement *content = rjNode << XCHILD("content") << XATTR("creator", "initiator") << XATTR("name", jproto->m_medianame);
	TiXmlElement *description = content << XCHILDNS("description", JABBER_FEAT_JINGLE_RTP) << XATTR("media", "audio");

	auto *opuspayload = description << XCHILD("payload-type") << XATTR("id", "111") << XATTR("name", "opus") << XATTR("clockrate", "48000") << XATTR("channels", "2");

	opuspayload << XCHILD("parameter") << XATTR("name", "minptime") << XATTR("value", "10");
	opuspayload << XCHILD("parameter") << XATTR("name", "useinbandfec") << XATTR("value", "1");
	opuspayload << XCHILDNS("rtcp-fb", "urn:xmpp:jingle:apps:rtp:rtcp-fb:0") << XATTR("type", "transport-cc");

	description << XCHILDNS("rtp-hdrext", "urn:xmpp:jingle:apps:rtp:rtp-hdrext:0") << XATTR("id", "1") << XATTR("uri", "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01");
	/*
		auto* source = description << XCHILDNS("source", "urn:xmpp:jingle:apps:rtp:ssma:0") << XATTR("ssrc", "2165039095");
		source << XCHILD("parameter") << XATTR("name", "cname") << XATTR("value", "8ee+PcGu8BNwq22f");
		source << XCHILD("parameter") << XATTR("name", "msid") << XATTR("value", "my-media-stream2 my-audio-track2");
		source << XCHILD("parameter") << XATTR("name", "mslabel") << XATTR("value", "my-media-stream2");
		source << XCHILD("parameter") << XATTR("name", "label") << XATTR("value", "my-audio-track2");*/

	description << XCHILD("rtcp-mux");

	//fingerprint
	char hash[100];
	if (sscanf(gst_sdp_media_get_attribute_val(media_audio, "fingerprint"), "sha-256 %95s", hash) == 1) {
		auto *transport = content << XCHILDNS("transport", JABBER_FEAT_JINGLE_ICEUDP);
		transport << XATTR("pwd", jproto->m_voipICEPwd) << XATTR("ufrag", jproto->m_voipICEUfrag);

		auto *fingerprint = transport << XCHILD("fingerprint", hash);
		fingerprint << XATTR("xmlns", JABBER_FEAT_JINGLE_DTLS) << XATTR("hash", "sha-256")
			<< XATTR("setup", gst_sdp_media_get_attribute_val(media_audio, "setup"));
	}

	jproto->m_api->SendXml(iq);

	gst_webrtc_session_description_free(offer);
}

void on_negotiation_needed_cb(GstElement *webrtcbin, gpointer user_data)
{
	if (((CJabberAccount *)user_data)->m_isOutgoing) {
		gst_print("Creating negotiation offer\n");

		GstPromise *promise = gst_promise_new_with_change_func(on_offer_created_cb, user_data, NULL);
		g_signal_emit_by_name(G_OBJECT(webrtcbin), "create-offer", NULL, promise);
	}
}

static void on_offer_set(GstPromise *promise, gpointer user_data)
{
	gst_promise_unref(promise);
	promise = gst_promise_new_with_change_func(on_offer_created_cb, user_data, NULL);
	g_signal_emit_by_name(((CJabberAccount *)user_data)->m_webrtc1, "create-answer", NULL, promise);
}

void send_ice_candidate_message_cb(G_GNUC_UNUSED GstElement */*webrtcbin*/, guint mline_index, gchar *candidate, CJabberAccount *jproto)
{
	// parse candidate and send
	char foundation[11], component[11], protocol[4] = "", priority[11], ip[40], port[6], type[6];
	int ret = sscanf(candidate, "candidate:%10s %10s %3s %10s %39s %5s typ %5s",
		foundation, component, protocol, priority, ip, port, type);
	if (ret != 7 || strcmp(protocol, "UDP"))
		return;

	gst_print("VOIP - Wanna send ice candidate(m-line_index=%d):\r\n%s\r\n", mline_index, candidate);
	for (char *p = protocol; *p; ++p) *p = tolower(*p);

	XmlNodeIq iq("set", jproto->m_api->GetSerialNext(), jproto->m_voipPeerJid);
	TiXmlElement *rjNode = iq << XCHILDNS("jingle", JABBER_FEAT_JINGLE);
	rjNode << XATTR("action", "transport-info") << XATTR("sid", jproto->m_voipSession);

	TiXmlElement *content = rjNode << XCHILD("content");
	content << XATTR("creator", "initiator") << XATTR("name", jproto->m_medianame);

	auto *transport = content << XCHILDNS("transport", JABBER_FEAT_JINGLE_ICEUDP);
	transport << XATTR("pwd", jproto->m_voipICEPwd) << XATTR("ufrag", jproto->m_voipICEUfrag);

	auto *candidateNode = transport << XCHILD("candidate");
	candidateNode << XATTR("type", type) << XATTR("protocol", protocol) << XATTR("ip", ip)
		<< XATTR("port", port) << XATTR("priority", priority) << XATTR("foundation", foundation) << XATTR("component", component);

	char attr[255];
	if (GetCandidateProp(attr, 255, candidate, "raddr"))
		candidateNode << XATTR("rel-addr", attr);
	if (GetCandidateProp(attr, 255, candidate, "rport"))
		candidateNode << XATTR("rel-port", attr);

	jproto->m_api->SendXml(iq);
}

static gboolean check_plugins(void)
{
	const gchar *needed[] = { "opus", "nice", "webrtc", "dtls", "srtp", "rtpmanager"
		/*"vpx", "videotestsrc", "audiotestsrc",*/ };

	GstRegistry *registry = gst_registry_get();
	gboolean ret = TRUE;
	for (auto &it : needed) {
		GstPlugin *plugin = gst_registry_find_plugin(registry, it);
		if (!plugin) {
			gst_print("Required gstreamer plugin '%s' not found\n", it);
			ret = FALSE;
		}
		else gst_object_unref(plugin);
	}

	return ret;
}

void dbgprint(const gchar *string)
{
	Netlib_Logf(0, "GST: %s", string);
}

bool CJabberAccount::VOIPCreatePipeline(void)
{
	if (!m_bEnableVOIP)
		goto err;

	//gstreamer init
	static bool gstinited = 0;
	if (!gstinited) {
		if (!LoadLibrary(L"gstreamer-1.0-0.dll")) {
			MessageBoxA(0, "Cannot load Gstreamer library!", 0, MB_OK | MB_ICONERROR);
			goto err;
		}
		gst_init(nullptr, nullptr);
		g_set_print_handler(dbgprint);
		gst_print("preved medved");
		if (!check_plugins()) {
			MessageBoxA(0, "Gstreamer plugins not found!", 0, MB_OK | MB_ICONERROR);
			goto err;
		}
		gstinited = 1;
	}

	#define RTP_TWCC_URI "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01"
	{
		CMStringA szPipeDescr;
		szPipeDescr += "webrtcbin bundle-policy=max-bundle name=sendrecv ";
		if (mir_wstrlen(m_szStunServer))
			szPipeDescr.AppendFormat("stun-server=stun://%S ", (wchar_t *)m_szStunServer);
		szPipeDescr += "autoaudiosrc ! audioconvert ! audioresample ! queue ! opusenc ! rtpopuspay name=audiopay ! ";
		szPipeDescr += "queue ! application/x-rtp,media=audio,encoding-name=OPUS,payload=111 ! sendrecv. ";

		GError *error = nullptr;
		m_pipe1 = gst_parse_launch(szPipeDescr, &error);
		if (error) {
			MessageBoxA(0, "Failed to parse launch: ", error->message, MB_OK);
			g_error_free(error);
			goto err;
		}
	}

	m_webrtc1 = gst_bin_get_by_name(GST_BIN(m_pipe1), "sendrecv");
	g_assert_nonnull(m_webrtc1);
	if (!m_webrtc1)
		goto err;

	GstElement *audiopay = gst_bin_get_by_name(GST_BIN(m_pipe1), "audiopay");
	g_assert_nonnull(audiopay);
	GstRTPHeaderExtension *audio_twcc = gst_rtp_header_extension_create_from_uri(RTP_TWCC_URI);
	g_assert_nonnull(audio_twcc);
	gst_rtp_header_extension_set_id(audio_twcc, 1);
	g_signal_emit_by_name(audiopay, "add-extension", audio_twcc);
	g_clear_object(&audio_twcc);
	g_clear_object(&audiopay);

	// It will be called when the pipeline goes to PLAYING.
	g_signal_connect(m_webrtc1, "on-negotiation-needed", G_CALLBACK(on_negotiation_needed_cb), this);
	// It will be called when we obtain local ICE candidate
	g_signal_connect(m_webrtc1, "on-ice-candidate", G_CALLBACK(send_ice_candidate_message_cb), this);
	// idk
	g_signal_connect(m_webrtc1, "pad-added", G_CALLBACK(on_incoming_stream_cb), m_pipe1);

	// Lifetime is the same as the pipeline itself 
	gst_object_unref(m_webrtc1);

	gst_print("Starting pipeline\n");
	if (gst_element_set_state(GST_ELEMENT(m_pipe1), GST_STATE_PLAYING) != GST_STATE_CHANGE_FAILURE)
		return true;

err:
	VOIPTerminateSession();
	return false;
}

bool CJabberAccount::VOIPTerminateSession(const char *reason)
{
	if (m_pipe1) {
		gst_element_set_state(GST_ELEMENT(m_pipe1), GST_STATE_NULL);
		g_clear_object(&m_pipe1);
		gst_object_unref(m_pipe1);
		gst_print("Pipeline stopped\n");
	}

	if (reason && !m_voipSession.IsEmpty() && !m_voipPeerJid.IsEmpty()) {
		XmlNodeIq iq("set", m_api->GetSerialNext(), m_voipPeerJid);

		TiXmlElement *jingleNode = iq << XCHILDNS("jingle", JABBER_FEAT_JINGLE);
		jingleNode << XATTR("action", "session-terminate") << XATTR("sid", m_voipSession);
		jingleNode << XATTR("initiator", m_isOutgoing ? m_api->GetFullJid() : m_voipPeerJid);
		jingleNode << XCHILD("reason") << XCHILD(reason);

		m_api->SendXml(iq);
	}

	m_voipICEPwd.Empty();
	m_voipICEUfrag.Empty();
	m_medianame.Empty();

	m_voipSession.Empty();
	m_voipPeerJid.Empty();
	m_pipe1 = m_webrtc1 = NULL;
	return true;
}

bool CJabberAccount::OnRTPDescription(const TiXmlElement *jingleNode)
{
	if (!jingleNode)
		return false;

	// process remote offer
	auto *content = XmlGetChildByTag(jingleNode, "content", "creator", "initiator");
	auto *transport = XmlGetChildByTag(content, "transport", "xmlns", "urn:xmpp:jingle:transports:ice-udp:1");
	auto *description = XmlGetChildByTag(content, "description", "xmlns", "urn:xmpp:jingle:apps:rtp:1");
	auto *source = XmlGetChildByTag(description, "source", "xmlns", "urn:xmpp:jingle:apps:rtp:ssma:0");

	CMStringA sdp_string(FORMAT, "v=0\r\no=- 0 0 IN IP4 0.0.0.0\r\ns=-\r\nt=0 0\r\na=ice-options:trickle\r\n"
		"m=audio 9 UDP/TLS/RTP/SAVPF 111\r\nc=IN IP4 0.0.0.0\r\na=ice-ufrag:%s\r\na=ice-pwd:%s\r\na=rtcp-mux\r\na=sendrecv\r\na=rtpmap:111 OPUS/48000/2\r\n"

		"a=rtcp-fb:111 transport-cc\r\na=fmtp:111 minptime=10;useinbandfec=1\r\n"
		"a=ssrc:%s msid:%s\r\n"
		"a=ssrc:%s cname:%s\r\n"

		"a=mid:%s\r\na=setup:%s\r\na=fingerprint:sha-256 %s\r\na=rtcp-mux-only\r\n",
		XmlGetAttr(transport, "ufrag"),
		XmlGetAttr(transport, "pwd"),

		XmlGetAttr(source, "ssrc"),
		XmlGetAttr(XmlGetChildByTag(source, "parameter", "name", "msid"), "value"),
		XmlGetAttr(source, "ssrc"),
		XmlGetAttr(XmlGetChildByTag(source, "parameter", "name", "cname"), "value"),

		XmlGetAttr(content, "name"),
		XmlGetAttr(XmlFirstChild(transport, "fingerprint"), "setup"),
		XmlFirstChild(transport, "fingerprint")->GetText());

	GstSDPMessage *sdp;
	int ret = gst_sdp_message_new(&sdp);
	g_assert_cmphex(ret, == , GST_SDP_OK);
	ret = gst_sdp_message_parse_buffer((guint8 *)sdp_string.c_str(), sdp_string.GetLength(), sdp);
	if (ret != GST_SDP_OK) {
		g_error("Could not parse SDP string\n");
		return false;
	}

	gchar *str = gst_sdp_message_as_text(sdp);
	gst_print("VOIP - Eating remote SDP offer:\r\n%s\r\n", str);
	g_free(str);

	if (m_isOutgoing) {
		GstWebRTCSessionDescription *answer = gst_webrtc_session_description_new(GST_WEBRTC_SDP_TYPE_ANSWER, sdp);
		g_assert_nonnull(answer);

		GstPromise *promise = gst_promise_new();
		g_signal_emit_by_name(m_webrtc1, "set-remote-description", answer, promise);
		gst_promise_interrupt(promise);
		gst_promise_unref(promise);
		gst_webrtc_session_description_free(answer);
	}
	else {
		// Set remote description on our pipeline
		GstWebRTCSessionDescription *offer = gst_webrtc_session_description_new(GST_WEBRTC_SDP_TYPE_OFFER, sdp);
		g_assert_nonnull(offer);

		GstPromise *promise = gst_promise_new_with_change_func(on_offer_set, this, NULL);
		g_signal_emit_by_name(m_webrtc1, "set-remote-description", offer, promise);
		gst_webrtc_session_description_free(offer);
	}

	return true;
}

bool CJabberAccount::OnICECandidate(const TiXmlElement *Node)
{
	if (!m_bEnableVOIP)
		return false;

	CMStringA scandidate;
	CMStringA proto(XmlGetAttr(Node, "protocol"));
	proto.MakeUpper();

	scandidate.AppendFormat("candidate:%s ", XmlGetAttr(Node, "foundation")); //FIXME
	scandidate.AppendFormat("%s ", XmlGetAttr(Node, "component"));
	scandidate.AppendFormat("%s ", proto.c_str());
	scandidate.AppendFormat("%s ", XmlGetAttr(Node, "priority"));
	scandidate.AppendFormat("%s ", XmlGetAttr(Node, "ip"));
	scandidate.AppendFormat("%s ", XmlGetAttr(Node, "port"));
	scandidate.AppendFormat("typ %s", XmlGetAttr(Node, "type"));

	if (const char *tmp = XmlGetAttr(Node, "rel-addr"))
		scandidate.AppendFormat(" raddr %s", tmp);
	if (const char *tmp = XmlGetAttr(Node, "rel-port"))
		scandidate.AppendFormat(" rport %s", tmp);
	if (const char *generation = XmlGetAttr(Node, "generation"))
		scandidate.AppendFormat(" generation %s", generation);

	gst_print("VOIP - Accepting ICE candidate:\r\n%s\r\n", scandidate.c_str());
	g_signal_emit_by_name(m_webrtc1, "add-ice-candidate", 0, scandidate.c_str());
	return true;
}

bool CJabberAccount::VOIPCallIinitiate(MCONTACT hContact)
{
	if (!m_voipSession.IsEmpty()) {
		VOIPTerminateSession();
		MessageBoxA(0, "Something went wrong\r\nOld session terminated", NULL, 0);
		return false;
	}

	if (!m_bEnableVOIP)
		return false;

	CMStringA jid(db_get_sm(hContact, m_szModuleName, "jid"));
	if (jid.IsEmpty())
		return false;

	ptrA szResource(m_api->GetBestResourceName(jid));
	if (szResource) {
		jid.AppendFormat("/%s", szResource.get());
		bool bFound = false;
		ptrA szFeatures(m_api->GetResourceFeatures(jid));
		for (auto *p = szFeatures.get(); *p; p += mir_strlen(p)+1)
			if (!mir_strcmp(p, JABBER_FEAT_JINGLE))
				bFound = true;

		if (!bFound) {
			// MsgPopup(hContact, TranslateT("Client's program does not support voice calls"), TranslateT("Error"));
			return false;
		}
	}

	unsigned char tmp[16];
	Utils_GetRandom(tmp, sizeof(tmp));

	m_isOutgoing = true;
	m_voipSession = ptrA(mir_base64_encode(tmp, sizeof(tmp)));
	m_voipPeerJid = jid.c_str();

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// module entry point

void CJabberAccount::InitVoip(bool bEnable)
{
	if (bEnable) {
		VOICE_MODULE vsr = {};
		vsr.cbSize = sizeof(VOICE_MODULE);
		vsr.description = L"XMPP/DTLS-SRTP";
		vsr.name = (char *)m_szModuleName;
		vsr.icon = g_plugin.getIconHandle(IDI_MAIN);
		vsr.flags = VOICE_CAPS_VOICE | VOICE_CAPS_CALL_CONTACT;
		CallService(MS_VOICESERVICE_REGISTER, (WPARAM)&vsr, 0);
	}
	else {
		VOIPTerminateSession();
		CallService(MS_VOICESERVICE_UNREGISTER, (WPARAM)m_szModuleName, 0);
	}
}
