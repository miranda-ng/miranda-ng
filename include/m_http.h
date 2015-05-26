/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ń) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_HTTP_H__
#define M_HTTP_H__

/* List of known http error codes */

// 1xx Informational
#define HTTP_CODE_CONTINUE							100
#define HTTP_CODE_SWITCHING_PROTOCOLS				101
#define HTTP_CODE_PROCESSING						102

// 2xx Success
#define HTTP_CODE_OK								200
#define HTTP_CODE_CREATED							201
#define HTTP_CODE_ACCEPTED							202
#define HTTP_CODE_NON_AUTHORITATIVE_INFORMATION		203
#define HTTP_CODE_NO_CONTENT						204
#define HTTP_CODE_RESET_CONTENT						205
#define HTTP_CODE_PARTIAL_CONTENT					206
#define HTTP_CODE_MULTI_STATUS						207

// 3xx Redirection
#define HTTP_CODE_MULTIPLE_CHOICES					300
#define HTTP_CODE_MOVED_PERMANENTLY					301
#define HTTP_CODE_FOUND								302
#define HTTP_CODE_SEE_OTHER							303
#define HTTP_CODE_NOT_MODIFIED						304
#define HTTP_CODE_USE_PROXY							305
#define HTTP_CODE_SWITCH_PROXY						306
#define HTTP_CODE_TEMPORARY_REDIRECT				307
#define HTTP_CODE_PERMANENT_REDIRECT				308

// 4xx Client Error
#define HTTP_CODE_BAD_REQUEST						400
#define HTTP_CODE_UNAUTHORIZED						401
#define HTTP_CODE_PAYMENT_REQUIRED					402
#define HTTP_CODE_FORBIDDEN							403
#define HTTP_CODE_NOT_FOUND							404
#define HTTP_CODE_METHOD_NOT_ALLOWED  				405
#define HTTP_CODE_NOT_ACCEPTABLE					406
#define HTTP_CODE_PROXY_AUTHENTICATION_REQUIRED		407
#define HTTP_CODE_REQUEST_TIMEOUT					408
#define HTTP_CODE_CONFLICT							409
#define HTTP_CODE_GONE								410
#define HTTP_CODE_LENGTH_REQUIRED					411
#define HTTP_CODE_PRECONDITION_FAILED	 			412
#define HTTP_CODE_REQUEST_ENTITY_TOO_LARGE			413
#define HTTP_CODE_REQUEST_URI_TOO_LONG				414
#define HTTP_CODE_UNSUPPORTED_MEDIA_TYPE			415
#define HTTP_CODE_REQUESTED_RANGE_NOT_SATISFIABLE	416
#define HTTP_CODE_EXPECTATION_FAILED				417
#define HTTP_CODE_IM_A_TEAPOT						418
#define HTTP_CODE_AUTHENTICATION_TIMEOUT			419
#define HTTP_CODE_METHOD_FAILURE					420
#define HTTP_CODE_ENHANCE_YOUR_CALM					420
#define HTTP_CODE_MISDIRECTED_REQUEST				421
#define HTTP_CODE_UNPROCESSABLE_ENTITY				422
#define HTTP_CODE_LOCKED							423
#define HTTP_CODE_FAILED_DEPENDENCY					424
#define HTTP_CODE_UNORDERED_COLLECTION				425
#define HTTP_CODE_UPGRADE_REQUIRED					426
#define HTTP_CODE_PRECONDITION_REQUIRED				428
#define HTTP_CODE_TOO_MANY_REQUESTS					429
#define HTTP_CODE_REQUEST_HEADER_FIELDS_TOO_LARGE	431
#define HTTP_CODE_LOGIN_TIMEOUT						440
#define HTTP_CODE_NO_RESPONSE						444
#define HTTP_CODE_RETRY_WITH						449
#define HTTP_CODE_BLOCKED_BY_PARENTAL_CONTROL		450
#define HTTP_CODE_UNAVAILABLE_FOR_LEGAL_REASONS		451
#define HTTP_CODE_REDIRECT							451
#define HTTP_CODE_REQUEST_HEADER_TOO_LARGE			494
#define HTTP_CODE_CERT_ERROR						495
#define HTTP_CODE_NO_CERT							496
#define HTTP_CODE_HTTP_TO_HTTPS						497
#define HTTP_CODE_TOKEN_EXPIRED_INVALID				498
#define HTTP_CODE_CLIENT_CLOSED_REQUEST				499
#define HTTP_CODE_TOKEN_REQUIRED					499

// 5xx Server Error
#define HTTP_CODE_INTERNAL_SERVER_ERROR 			500
#define HTTP_CODE_NOT_IMPLEMENTED					501
#define HTTP_CODE_BAD_GATEWAY						502
#define HTTP_CODE_SERVICE_UNAVAILABLE  				503
#define HTTP_CODE_GATEWAY_TIMEOUT					504
#define HTTP_CODE_HTTP_VERSION_NOT_SUPPORTED		505
#define HTTP_CODE_VARIANT_ALSO_NEGOTIATES			506
#define HTTP_CODE_INSUFFICIENT_STORAGE				507
#define HTTP_CODE_LOOP_DETECTED						508
#define HTTP_CODE_BANDWIDTH_LIMIT_EXCEEDED			509
#define HTTP_CODE_NOT_EXTENDED						510
#define HTTP_CODE_NETWORK_AUTHENTICATION_REQUIRED	511
#define HTTP_CODE_NETWORK_READ_TIMEOUT_ERROR		598
#define HTTP_CODE_NETWORK_CONNECT_TIMEOUT_ERROR		599

#endif // M_HTTP_H__