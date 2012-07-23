#include "commonheaders.h"

//Popup Messages
LPCSTR sim001 = "SecureIM established...";
LPCSTR sim002 = "Key exchange failed...";
LPCSTR sim003 = "Key from disabled...";
LPCSTR sim004 = "Sended back message received...";
LPCSTR sim005 = "Sending back secure message...";
LPCSTR sim006 = "SecureIM disabled...";
LPCSTR sim007 = "Sending Key...";
LPCSTR sim008 = "Key Received...";
LPCSTR sim009 = "Sending Message...";
LPCSTR sim010 = "Message Received...";
LPCSTR sim011 = "Encrypting File:";
LPCSTR sim012 = "Decrypting File:";
LPCSTR sim013 = "Bad key received...";

//Error Messages
LPCSTR sim101 = "SecureIM: Error while decrypting the message.";
LPCSTR sim102 = "SecureIM: Error while decrypting the message, bad message length.";
LPCSTR sim103 = "SecureIM: Error while decrypting the message, bad message CRC.";
LPCSTR sim104 = "User has not answered to key exchange !\nYour messages are still in SecureIM queue, do you want to send them Unencrypted now ?";
LPCSTR sim105 = "SecureIM not enabled! You Must Enable SecureIM with this user...";
LPCSTR sim106 = "Can't Send Encrypted Message !\nUser is offline now and his secure key has been expired, Do you want to send your message ?\nIt will be unencrypted !";
LPCSTR sim107 = "SecureIM won't be loaded because cryptopp.dll is missing or wrong version !";
LPCSTR sim108 = "SecureIM can't load PGP/GPG key! Check PGP/GPG settings!";
LPCSTR sim109 = "SecureIM can't encrypt message! Check trust of PGP/GPG key!";
LPCSTR sim110 = "Can't Send Encrypted Message !\nDo you want to send your message ?\nIt will be unencrypted !";
LPCSTR sim111 = "Can't change mode! Secure connection established!";
LPCSTR sim112 = "Can't export RSA private key!";
LPCSTR sim113 = "Can't import RSA private key!";
LPCSTR sim114 = "Can't export RSA public key!";
LPCSTR sim115 = "Can't import RSA public key!";

//Options
LPCSTR sim201 = "General";
LPCSTR sim202 = "Protocols";
LPCSTR sim203 = "Nickname";
LPCSTR sim204 = "UIN/Email/Jid/...";
LPCSTR sim205 = "Status";
LPCSTR sim206 = "PSK";
LPCSTR sim210 = "Name";
LPCSTR sim211 = "Password is too short!";
LPCSTR sim212 = "ON";
LPCSTR sim213 = "Off";
LPCSTR sim214 = "PGP";
LPCSTR sim215 = "Key ID";
LPCSTR sim216 = "Keyrings loaded.";
LPCSTR sim217 = "Keyrings not loaded!";
LPCSTR sim218 = "PGP SDK v%i.%i.%i found.";
LPCSTR sim219 = "PGP SDK not found!";
LPCSTR sim220 = "This version not supported!";
LPCSTR sim221 = "(none)";
LPCSTR sim222 = "Private key loaded.";
LPCSTR sim223 = "Private key not loaded!";
LPCSTR sim224 = "The new settings will become valid when you restart MirandaIM!";
LPCSTR sim225 = "Keyrings disabled!";
LPCSTR sim226 = "GPG";
LPCSTR sim227 = "CP";
LPCSTR sim228 = "ANSI";
LPCSTR sim229 = "UTF8";
LPCSTR sim230 = "Mode";
LPCSTR sim231[] = { "Native", "PGP", "GPG", "RSA/AES", "RSA" };
LPCSTR sim232[] = { "Disabled", "Enabled", "Always try" };
LPCSTR sim233 = "PUB";
LPCSTR sim234 = "SHA1";

//Context Menu
LPCSTR sim301 = "Create SecureIM connection";
LPCSTR sim302 = "Disable SecureIM connection";
LPCSTR sim306 = "Load PGP Key";
LPCSTR sim307 = "Unload PGP Key";
LPCSTR sim308 = "Load GPG Key";
LPCSTR sim309 = "Unload GPG Key";
LPCSTR sim310 = "Delete RSA Key";
LPCSTR sim311[] = { "SecureIM mode (Native)",
                    "SecureIM mode (PGP)",
                    "SecureIM mode (GPG)",
                    "SecureIM mode (RSA/AES)",
                    "SecureIM mode (RSA)" };
LPCSTR sim312[] = { "SecureIM status (disabled)",
		    "SecureIM status (enabled)",
		    "SecureIM status (always try)" };

//System messages
LPCSTR sim401 = "SecureIM: Sorry, unable to decrypt this message due you have no PGP/GPG installed. Visit www.pgp.com or www.gnupg.org for more info.";
LPCSTR sim402 = "SecureIM received unencryped message:\n";
LPCSTR sim403 = "SecureIM received encryped message:\n";

//RSA messages
LPCSTR sim501 = "Session closed by receiving incorrect message type";
LPCSTR sim502 = "Session closed by other side on error";
LPCSTR sim505 = "Error while decoding AES message";
LPCSTR sim506 = "Error while decoding RSA message";
LPCSTR sim507 = "Session closed on timeout";
LPCSTR sim508 = "Session closed by other side when status \"disabled\"";
LPCSTR sim510 = "Session closed on error: %02x";
LPCSTR sim520 = "SecureIM received RSA Public Key from \"%s\"\n\nSHA1: %s\n\nDo you Accept this Key ?";
LPCSTR sim521 = "SecureIM auto accepted RSA Public key from: %s uin: %s SHA1: %s";
LPCSTR sim522 = "SecureIM received NEW RSA Public Key from \"%s\"\n\nNew SHA1: %s\n\nOld SHA1: %s\n\nDo you Replace this Key ?";
LPCSTR sim523 = "SecureIM auto accepted NEW RSA Public key from: %s uin: %s New SHA1: %s Old SHA1: %s";


// EOF
