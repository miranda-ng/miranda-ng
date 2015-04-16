#include "commonheaders.h"

//Popup Messages
LPCSTR sim001 = LPGEN("SecureIM established...");
LPCSTR sim002 = LPGEN("Key exchange failed...");
LPCSTR sim003 = LPGEN("Key from disabled...");
LPCSTR sim004 = LPGEN("Sent back message received...");
LPCSTR sim005 = LPGEN("Sending back secure message...");
LPCSTR sim006 = LPGEN("SecureIM disabled...");
LPCSTR sim007 = LPGEN("Sending key...");
LPCSTR sim008 = LPGEN("Key received...");
LPCSTR sim009 = LPGEN("Sending message...");
LPCSTR sim010 = LPGEN("Message received...");
LPCSTR sim011 = LPGEN("Encrypting file:");
LPCSTR sim012 = LPGEN("Decrypting file:");
LPCSTR sim013 = LPGEN("Bad key received...");

//Error Messages
LPCSTR sim101 = LPGEN("SecureIM: Error while decrypting the message.");
LPCSTR sim102 = LPGEN("SecureIM: Error while decrypting the message, bad message length.");
LPCSTR sim103 = LPGEN("SecureIM: Error while decrypting the message, bad message CRC.");
LPCSTR sim104 = LPGEN("User has not answered to key exchange!\nYour messages are still in SecureIM queue, do you want to send them unencrypted now?");
LPCSTR sim105 = LPGEN("SecureIM not enabled! You must enable SecureIM with this user...");
LPCSTR sim106 = LPGEN("Can't send encrypted message!\nUser is offline now and his secure key has been expired. Do you want to send your message?\nIt will be unencrypted!");
LPCSTR sim107 = LPGEN("SecureIM won't be loaded because cryptopp.dll is missing or wrong version!");
LPCSTR sim108 = LPGEN("SecureIM can't load PGP/GPG key! Check PGP/GPG settings!");
LPCSTR sim109 = LPGEN("SecureIM can't encrypt message! Check trust of PGP/GPG key!");
LPCSTR sim110 = LPGEN("Can't send encrypted message!\nDo you want to send your message?\nIt will be unencrypted!");
LPCSTR sim111 = LPGEN("Can't change mode! Secure connection established!");
LPCSTR sim112 = LPGEN("Can't export RSA private key!");
LPCSTR sim113 = LPGEN("Can't import RSA private key!");
LPCSTR sim114 = LPGEN("Can't export RSA public key!");
LPCSTR sim115 = LPGEN("Can't import RSA public key!");

//Options
LPCSTR sim201 = LPGEN("General");
LPCSTR sim202 = LPGEN("Protocols");
LPCSTR sim203 = LPGEN("Nickname");
LPCSTR sim204 = "UIN/Email/Jid/...";
LPCSTR sim205 = LPGEN("Status");
LPCSTR sim206 = "PSK";
LPCSTR sim210 = LPGEN("Name");
LPCSTR sim211 = LPGEN("Password is too short!");
LPCSTR sim212 = LPGEN("ON");
LPCSTR sim213 = LPGEN("Off");
LPCSTR sim214 = "PGP";
LPCSTR sim215 = "Key ID";
LPCSTR sim216 = LPGEN("Keyrings loaded.");
LPCSTR sim217 = LPGEN("Keyrings not loaded!");
LPCSTR sim218 = LPGEN("PGP SDK v%i.%i.%i found.");
LPCSTR sim219 = LPGEN("PGP SDK not found!");
LPCSTR sim220 = LPGEN("This version not supported!");
LPCSTR sim221 = LPGEN("(none)");
LPCSTR sim222 = LPGEN("Private key loaded.");
LPCSTR sim223 = LPGEN("Private key not loaded!");
LPCSTR sim224 = LPGEN("The new settings will become valid when you restart Miranda NG!");
LPCSTR sim225 = LPGEN("Keyrings disabled!");
LPCSTR sim226 = "GPG";
LPCSTR sim227 = "CP";
LPCSTR sim228 = "ANSI";
LPCSTR sim229 = "UTF8";
LPCSTR sim230 = LPGEN("Mode");
LPCSTR sim231[] = { LPGEN("Native"), "PGP", "GPG", "RSA/AES", "RSA" };
LPCSTR sim232[] = { LPGEN("Disabled"), LPGEN("Enabled"), LPGEN("Always try") };
LPCSTR sim233 = "PUB";
LPCSTR sim234 = "SHA1";

//Context Menu
LPCSTR sim301 = LPGEN("Create SecureIM connection");
LPCSTR sim302 = LPGEN("Disable SecureIM connection");
LPCSTR sim306 = LPGEN("Load PGP Key");
LPCSTR sim307 = LPGEN("Unload PGP Key");
LPCSTR sim308 = LPGEN("Load GPG Key");
LPCSTR sim309 = LPGEN("Unload GPG Key");
LPCSTR sim310 = LPGEN("Delete RSA Key");
LPCSTR sim311[] = { LPGEN("SecureIM mode (Native)"),
                    LPGEN("SecureIM mode (PGP)"),
                    LPGEN("SecureIM mode (GPG)"),
                    LPGEN("SecureIM mode (RSA/AES)"),
                    LPGEN("SecureIM mode (RSA)") };
LPCSTR sim312[] = { LPGEN("SecureIM status (disabled)"),
		    LPGEN("SecureIM status (enabled)"),
		    LPGEN("SecureIM status (always try)") };

//System messages
LPCSTR sim401 = LPGEN("SecureIM: Sorry, unable to decrypt this message because you have no PGP/GPG installed. Visit www.pgp.com or www.gnupg.org for more info.");
LPCSTR sim402 = LPGEN("SecureIM received unencrypted message:\n");
LPCSTR sim403 = LPGEN("SecureIM received encrypted message:\n");

//RSA messages
LPCSTR sim501 = LPGEN("Session closed by receiving incorrect message type");
LPCSTR sim502 = LPGEN("Session closed by other side on error");
LPCSTR sim505 = LPGEN("Error while decoding AES message");
LPCSTR sim506 = LPGEN("Error while decoding RSA message");
LPCSTR sim507 = LPGEN("Session closed on timeout");
LPCSTR sim508 = LPGEN("Session closed by other side when status \"disabled\"");
LPCSTR sim510 = LPGEN("Session closed on error: %02x");
LPCSTR sim520 = LPGEN("SecureIM received RSA Public Key from \"%s\"\n\nSHA1: %s\n\nDo you Accept this Key?");
LPCSTR sim521 = LPGEN("SecureIM auto accepted RSA Public key from: %s uin: %s SHA1: %s");
LPCSTR sim522 = LPGEN("SecureIM received NEW RSA Public Key from \"%s\"\n\nNew SHA1: %s\n\nOld SHA1: %s\n\nDo you Replace this Key?");
LPCSTR sim523 = LPGEN("SecureIM auto accepted NEW RSA Public key from: %s uin: %s New SHA1: %s Old SHA1: %s");
