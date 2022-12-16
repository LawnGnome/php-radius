/*-
 * Copyright 1998 Juniper Networks, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$FreeBSD: src/lib/libradius/radlib.c,v 1.4.2.3 2002/06/17 02:24:57 brian Exp $
 */

#include <sys/types.h>

#ifndef PHP_WIN32
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include <process.h>
#include "win32/time.h"
#endif

#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef PHP_WIN32
#include <unistd.h>
#endif

#include "radlib_compat.h"
#include "radlib_md5.h"
#include "radlib_private.h"

static void	 clear_password(struct rad_handle *);
static void	 generr(struct rad_handle *, const char *, ...)
		    __printflike(2, 3);
static void	 insert_scrambled_password(struct rad_handle *, int);
static void	 insert_request_authenticator(struct rad_handle *, int);
static int	 is_valid_response(struct rad_handle *, int,
		    const struct sockaddr_in *);
static int	 put_password_attr(struct rad_handle *, int,
		    const void *, size_t,
		    const struct rad_attr_options *);
static int	 put_raw_attr(struct rad_handle *, int,
		    const void *, size_t,
		    const struct rad_attr_options *);
static int	 split(char *, char *[], int, char *, size_t);

static void
clear_password(struct rad_handle *h)
{
	if (h->pass_len != 0) {
		memset(h->pass, 0, h->pass_len);
		h->pass_len = 0;
	}
	h->pass_pos = 0;
}

static void
generr(struct rad_handle *h, const char *format, ...)
{
	va_list		 ap;

	va_start(ap, format);
	vsnprintf(h->errmsg, ERRSIZE, format, ap);
	va_end(ap);
}

static void
insert_scrambled_password(struct rad_handle *h, int srv)
{
	MD5_CTX ctx;
	unsigned char md5[16];
	const struct rad_server *srvp;
	int padded_len;
	int pos;

	srvp = &h->servers[srv];
	padded_len = h->pass_len == 0 ? 16 : (h->pass_len+15) & ~0xf;

	memcpy(md5, &h->request[POS_AUTH], LEN_AUTH);
	for (pos = 0;  pos < padded_len;  pos += 16) {
		int i;

		/* Calculate the new scrambler */
		MD5Init(&ctx);
		MD5Update(&ctx, srvp->secret, strlen(srvp->secret));
		MD5Update(&ctx, md5, 16);
		MD5Final(md5, &ctx);

		/*
		 * Mix in the current chunk of the password, and copy
		 * the result into the right place in the request.  Also
		 * modify the scrambler in place, since we will use this
		 * in calculating the scrambler for next time.
		 */
		for (i = 0;  i < 16;  i++)
			h->request[h->pass_pos + pos + i] =
			    md5[i] ^= h->pass[pos + i];
	}
}

static void
insert_request_authenticator(struct rad_handle *h, int srv)
{
	MD5_CTX ctx;
	const struct rad_server *srvp;

	srvp = &h->servers[srv];

	/* Create the request authenticator */
	MD5Init(&ctx);
	MD5Update(&ctx, &h->request[POS_CODE], POS_AUTH - POS_CODE);
	MD5Update(&ctx, memset(&h->request[POS_AUTH], 0, LEN_AUTH), LEN_AUTH);
	MD5Update(&ctx, &h->request[POS_ATTRS], h->req_len - POS_ATTRS);
	MD5Update(&ctx, srvp->secret, strlen(srvp->secret));
	MD5Final(&h->request[POS_AUTH], &ctx);
}

/*
 * Return true if the current response is valid for a request to the
 * specified server.
 */
static int
is_valid_response(struct rad_handle *h, int srv,
    const struct sockaddr_in *from)
{
	MD5_CTX ctx;
	unsigned char md5[16];
	const struct rad_server *srvp;
	int len;

	srvp = &h->servers[srv];

	/* Check the source address */
	if (from->sin_family != srvp->addr.sin_family ||
	    from->sin_addr.s_addr != srvp->addr.sin_addr.s_addr ||
	    from->sin_port != srvp->addr.sin_port)
		return 0;

	/* Check the message length */
	if (h->resp_len < POS_ATTRS)
		return 0;
	len = h->response[POS_LENGTH] << 8 | h->response[POS_LENGTH+1];
	if (len > h->resp_len)
		return 0;

	/* Check the response authenticator */
	MD5Init(&ctx);
	MD5Update(&ctx, &h->response[POS_CODE], POS_AUTH - POS_CODE);
	MD5Update(&ctx, &h->request[POS_AUTH], LEN_AUTH);
	MD5Update(&ctx, &h->response[POS_ATTRS], len - POS_ATTRS);
	MD5Update(&ctx, srvp->secret, strlen(srvp->secret));
	MD5Final(md5, &ctx);
	if (memcmp(&h->response[POS_AUTH], md5, sizeof md5) != 0)
		return 0;

	return 1;
}

static int
put_password_attr(struct rad_handle *h, int type, const void *value, size_t len, const struct rad_attr_options *options)
{
	int padded_len;
	int pad_len;

	if (options->options & RAD_OPTION_SALT) {
		generr(h, "User-Password attributes cannot be salt-encrypted");
		return -1;
	}

	if (options->options & RAD_OPTION_TAG) {
		generr(h, "User-Password attributes cannot be tagged");
		return -1;
	}

	if (h->pass_pos != 0) {
		generr(h, "Multiple User-Password attributes specified");
		return -1;
	}
	if (len > PASSSIZE)
		len = PASSSIZE;
	padded_len = len == 0 ? 16 : (len+15) & ~0xf;
	pad_len = padded_len - len;

	/*
	 * Put in a place-holder attribute containing all zeros, and
	 * remember where it is so we can fill it in later.
	 */
	clear_password(h);
	put_raw_attr(h, type, h->pass, padded_len, options);
	h->pass_pos = h->req_len - padded_len;

	/* Save the cleartext password, padded as necessary */
	memcpy(h->pass, value, len);
	h->pass_len = len;
	memset(h->pass + len, 0, pad_len);
	return 0;
}

static int
put_raw_attr(struct rad_handle *h, int type, const void *value, size_t len, const struct rad_attr_options *options)
{
	const void *actual_value = value;
	size_t full_len = 2 + len;
	int res = -1;
	struct rad_salted_value *salted = NULL;

	if (options->options & RAD_OPTION_SALT) {
		salted = emalloc(sizeof(struct rad_salted_value));

		if (rad_salt_value(h, value, len, salted) == -1) {
			goto end;
		} else {
			actual_value = salted->data;
			len = salted->len;
			full_len = 2 + len;
		}
	}

	if (options->options & RAD_OPTION_TAG) {
		full_len++;
	}

	if (full_len > 255) {
		generr(h, "Attribute too long");
		goto end;
	}
	
	if (h->req_len + full_len > MSGSIZE) {
		generr(h, "Maximum message length exceeded");
		goto end;
	}
	h->request[h->req_len++] = type;
	h->request[h->req_len++] = full_len;

	if (options->options & RAD_OPTION_TAG) {
		h->request[h->req_len++] = options->tag;
	}

	memcpy(&h->request[h->req_len], actual_value, len);
	h->req_len += len;
	res = 0;

end:
	if (salted) {
		efree(salted->data);
		efree(salted);
	}

	return res;
}

int
rad_add_server(struct rad_handle *h, const char *host, int port,
    const char *secret, int timeout, int tries)
{
	struct rad_server *srvp;

	if (h->num_servers >= MAXSERVERS) {
		generr(h, "Too many RADIUS servers specified");
		return -1;
	}
	srvp = &h->servers[h->num_servers];

	memset(&srvp->addr, 0, sizeof srvp->addr);
	srvp->addr.sin_family = AF_INET;
	if (!inet_aton(host, &srvp->addr.sin_addr)) {
		struct hostent *hent;

		if ((hent = gethostbyname(host)) == NULL) {
			generr(h, "%s: host not found", host);
			return -1;
		}
		memcpy(&srvp->addr.sin_addr, hent->h_addr,
		    sizeof srvp->addr.sin_addr);
	}
	if (port != 0)
		srvp->addr.sin_port = htons((short) port);
	else {
		struct servent *sent;

		if (h->type == RADIUS_AUTH)
			srvp->addr.sin_port =
			    (sent = getservbyname("radius", "udp")) != NULL ?
				sent->s_port : htons(RADIUS_PORT);
		else
			srvp->addr.sin_port =
			    (sent = getservbyname("radacct", "udp")) != NULL ?
				sent->s_port : htons(RADACCT_PORT);
	}
	if ((srvp->secret = strdup(secret)) == NULL) {
		generr(h, "Out of memory");
		return -1;
	}
	srvp->timeout = timeout;
	srvp->max_tries = tries;
	srvp->num_tries = 0;
	h->num_servers++;
	return 0;
}

void
rad_close(struct rad_handle *h)
{
	int srv;

	if (h->fd != -1)
		close(h->fd);
	for (srv = 0;  srv < h->num_servers;  srv++) {
		memset(h->servers[srv].secret, 0,
		    strlen(h->servers[srv].secret));
		free(h->servers[srv].secret);
	}
	clear_password(h);
	free(h);
}

int
rad_config(struct rad_handle *h, const char *path)
{
	FILE *fp;
	char buf[MAXCONFLINE];
	int linenum;
	int retval;

	if (path == NULL)
		path = PATH_RADIUS_CONF;
	if ((fp = fopen(path, "r")) == NULL) {
		generr(h, "Cannot open \"%s\": %s", path, strerror(errno));
		return -1;
	}
	retval = 0;
	linenum = 0;
	while (fgets(buf, sizeof buf, fp) != NULL) {
		int len;
		char *fields[5];
		int nfields;
		char msg[ERRSIZE];
		char *type;
		char *host, *res;
		char *port_str;
		char *secret;
		char *timeout_str;
		char *maxtries_str;
		char *end;
		char *wanttype;
		unsigned long timeout;
		unsigned long maxtries;
		int port;
		int i;

		linenum++;
		len = strlen(buf);
		/* We know len > 0, else fgets would have returned NULL. */
		if (buf[len - 1] != '\n' && !(buf[len - 2] != '\r' && buf[len - 1] != '\n')) {
			if (len == sizeof buf - 1)
				generr(h, "%s:%d: line too long", path,
				    linenum);
			else
				generr(h, "%s:%d: missing newline", path,
				    linenum);
			retval = -1;
			break;
		}
		buf[len - 1] = '\0';

		/* Extract the fields from the line. */
		nfields = split(buf, fields, 5, msg, sizeof msg);
		if (nfields == -1) {
			generr(h, "%s:%d: %s", path, linenum, msg);
			retval = -1;
			break;
		}
		if (nfields == 0)
			continue;
		/*
		 * The first field should contain "auth" or "acct" for
		 * authentication or accounting, respectively.  But older
		 * versions of the file didn't have that field.  Default
		 * it to "auth" for backward compatibility.
		 */
		if (strcmp(fields[0], "auth") != 0 &&
		    strcmp(fields[0], "acct") != 0) {
			if (nfields >= 5) {
				generr(h, "%s:%d: invalid service type", path,
				    linenum);
				retval = -1;
				break;
			}
			nfields++;
			for (i = nfields;  --i > 0;  )
				fields[i] = fields[i - 1];
			fields[0] = "auth";
		}
		if (nfields < 3) {
			generr(h, "%s:%d: missing shared secret", path,
			    linenum);
			retval = -1;
			break;
		}
		type = fields[0];
		host = fields[1];
		secret = fields[2];
		timeout_str = fields[3];
		maxtries_str = fields[4];

		/* Ignore the line if it is for the wrong service type. */
		wanttype = h->type == RADIUS_AUTH ? "auth" : "acct";
		if (strcmp(type, wanttype) != 0)
			continue;

		/* Parse and validate the fields. */
		res = host;
		host = strsep(&res, ":");
		port_str = strsep(&res, ":");
		if (port_str != NULL) {
			port = strtoul(port_str, &end, 10);
			if (*end != '\0') {
				generr(h, "%s:%d: invalid port", path,
				    linenum);
				retval = -1;
				break;
			}
		} else
			port = 0;
		if (timeout_str != NULL) {
			timeout = strtoul(timeout_str, &end, 10);
			if (*end != '\0') {
				generr(h, "%s:%d: invalid timeout", path,
				    linenum);
				retval = -1;
				break;
			}
		} else
			timeout = TIMEOUT;
		if (maxtries_str != NULL) {
			maxtries = strtoul(maxtries_str, &end, 10);
			if (*end != '\0') {
				generr(h, "%s:%d: invalid maxtries", path,
				    linenum);
				retval = -1;
				break;
			}
		} else
			maxtries = MAXTRIES;

		if (rad_add_server(h, host, port, secret, timeout, maxtries) ==
		    -1) {
			strcpy(msg, h->errmsg);
			generr(h, "%s:%d: %s", path, linenum, msg);
			retval = -1;
			break;
		}
	}
	/* Clear out the buffer to wipe a possible copy of a shared secret */
	memset(buf, 0, sizeof buf);
	fclose(fp);
	return retval;
}

/*
 * rad_init_send_request() must have previously been called.
 * Returns:
 *   0     The application should select on *fd with a timeout of tv before
 *         calling rad_continue_send_request again.
 *   < 0   Failure
 *   > 0   Success
 */
int
rad_continue_send_request(struct rad_handle *h, int selected, int *fd,
                          struct timeval *tv)
{
	int n;

	if (selected) {
		struct sockaddr_in from;
		int fromlen;

		fromlen = sizeof from;
		h->resp_len = recvfrom(h->fd, h->response,
		    MSGSIZE, MSG_WAITALL, (struct sockaddr *)&from, &fromlen);
		if (h->resp_len == -1) {
#ifdef PHP_WIN32
			generr(h, "recfrom: %d", WSAGetLastError());
#else
			generr(h, "recvfrom: %s", strerror(errno));
#endif
			return -1;
		}
		if (is_valid_response(h, h->srv, &from)) {
			h->resp_len = h->response[POS_LENGTH] << 8 |
			    h->response[POS_LENGTH+1];
			h->resp_pos = POS_ATTRS;
			return h->response[POS_CODE];
		}
	}

	if (h->try == h->total_tries) {
		generr(h, "No valid RADIUS responses received");
		return -1;
	}

	/*
         * Scan round-robin to the next server that has some
         * tries left.  There is guaranteed to be one, or we
         * would have exited this loop by now.
	 */
	while (h->servers[h->srv].num_tries >= h->servers[h->srv].max_tries)
		if (++h->srv >= h->num_servers)
			h->srv = 0;

	if (h->request[POS_CODE] == RAD_ACCOUNTING_REQUEST
	    || h->request[POS_CODE] == RAD_COA_REQUEST
	    || h->request[POS_CODE] == RAD_COA_ACK
	    || h->request[POS_CODE] == RAD_COA_NAK
	    || h->request[POS_CODE] == RAD_DISCONNECT_REQUEST
	    || h->request[POS_CODE] == RAD_DISCONNECT_ACK
	    || h->request[POS_CODE] == RAD_DISCONNECT_NAK)
		/* Insert the request authenticator into the request */
		insert_request_authenticator(h, h->srv);
	else
		/* Insert the scrambled password into the request */
		if (h->pass_pos != 0)
			insert_scrambled_password(h, h->srv);

	/* Send the request */
	n = sendto(h->fd, h->request, h->req_len, 0,
	    (const struct sockaddr *)&h->servers[h->srv].addr,
	    sizeof h->servers[h->srv].addr);
	if (n != h->req_len) {
		if (n == -1)
#ifdef PHP_WIN32
			generr(h, "sendto: %d", WSAGetLastError());
#else
			generr(h, "sendto: %s", strerror(errno));
#endif
		else
			generr(h, "sendto: short write");
		return -1;
	}

	h->try++;
	h->servers[h->srv].num_tries++;
	tv->tv_sec = h->servers[h->srv].timeout;
	tv->tv_usec = 0;
	*fd = h->fd;

	return 0;
}

int
rad_create_request(struct rad_handle *h, int code)
{
	int i;

	h->request[POS_CODE] = code;
	h->request[POS_IDENT] = ++h->ident;
	/* Create a random authenticator */
	for (i = 0;  i < LEN_AUTH;  i += 2) {
		long r;
		r = php_rand();
		h->request[POS_AUTH+i] = (unsigned char) r;
		h->request[POS_AUTH+i+1] = (unsigned char) (r >> 8);
	}
	h->req_len = POS_ATTRS;
	h->request_created = 1;    
	clear_password(h);
	return 0;
}

struct in_addr
rad_cvt_addr(const void *data)
{
	struct in_addr value;

	memcpy(&value.s_addr, data, sizeof value.s_addr);
	return value;
}

u_int32_t
rad_cvt_int(const void *data)
{
	u_int32_t value;

	memcpy(&value, data, sizeof value);
	return ntohl(value);
}

char *
rad_cvt_string(const void *data, size_t len)
{
	char *s;

	s = malloc(len + 1);
	if (s != NULL) {
		memcpy(s, data, len);
		s[len] = '\0';
	}
	return s;
}

/*
 * Returns the attribute type.  If none are left, returns 0.  On failure,
 * returns -1.
 */
int
rad_get_attr(struct rad_handle *h, const void **value, size_t *len)
{
	int type;

	if (h->resp_len == 0) {
		generr(h, "No response has been received");
		return -1;
	}
	if (h->resp_pos >= h->resp_len)
		return 0;
	if (h->resp_pos + 2 > h->resp_len) {
		generr(h, "Malformed attribute in response");
		return -1;
	}
	type = h->response[h->resp_pos++];
	*len = h->response[h->resp_pos++] - 2;
	if (h->resp_pos + (int) *len > h->resp_len) {
		generr(h, "Malformed attribute in response");
		return -1;
	}
	*value = &h->response[h->resp_pos];
	h->resp_pos += *len;
	return type;
}

/*
 * Returns -1 on error, 0 to indicate no event and >0 for success
 */
int
rad_init_send_request(struct rad_handle *h, int *fd, struct timeval *tv)
{
	int srv;

	/* Make sure we have a socket to use */
	if (h->fd == -1) {
		struct sockaddr_in sin;

		if ((h->fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
#ifdef PHP_WIN32
			generr(h, "Cannot create socket: %d", WSAGetLastError());
#else
			generr(h, "Cannot create socket: %s", strerror(errno));
#endif
			return -1;
		}
		memset(&sin, 0, sizeof sin);
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY;
		sin.sin_port = htons(0);
		if (bind(h->fd, (const struct sockaddr *)&sin,
		    sizeof sin) == -1) {
#ifdef PHP_WIN32
			generr(h, "bind: %d", WSAGetLastError());
#else
			generr(h, "bind: %s", strerror(errno));
#endif
			close(h->fd);
			h->fd = -1;
			return -1;
		}
	}

	if (h->request[POS_CODE] == RAD_ACCOUNTING_REQUEST
	    || h->request[POS_CODE] == RAD_COA_REQUEST
	    || h->request[POS_CODE] == RAD_COA_ACK
	    || h->request[POS_CODE] == RAD_COA_NAK
	    || h->request[POS_CODE] == RAD_DISCONNECT_REQUEST
	    || h->request[POS_CODE] == RAD_DISCONNECT_ACK
	    || h->request[POS_CODE] == RAD_DISCONNECT_NAK) {
		/* Make sure no password given */
		if (h->pass_pos || h->chap_pass) {
			generr(h, "User or Chap Password in non-access request");
			return -1;
		}
	} else {
		/* Make sure the user gave us a password */
		if (h->pass_pos == 0 && !h->chap_pass) {
			generr(h, "No User or Chap Password attributes given");
			return -1;
		}
		if (h->pass_pos != 0 && h->chap_pass) {
			generr(h, "Both User and Chap Password attributes given");
			return -1;
		}
	}

	/* Fill in the length field in the message */
	h->request[POS_LENGTH] = h->req_len >> 8;
	h->request[POS_LENGTH+1] = h->req_len;

	/*
	 * Count the total number of tries we will make, and zero the
	 * counter for each server.
	 */
	h->total_tries = 0;
	for (srv = 0;  srv < h->num_servers;  srv++) {
		h->total_tries += h->servers[srv].max_tries;
		h->servers[srv].num_tries = 0;
	}
	if (h->total_tries == 0) {
		generr(h, "No RADIUS servers specified");
		return -1;
	}

	h->try = h->srv = 0;

	return rad_continue_send_request(h, 0, fd, tv);
}

/*
 * Create and initialize a rad_handle structure, and return it to the
 * caller.  Can fail only if the necessary memory cannot be allocated.
 * In that case, it returns NULL.
 */
struct rad_handle *
rad_auth_open(void)
{
	struct rad_handle *h;

	h = (struct rad_handle *)malloc(sizeof(struct rad_handle));
	if (h != NULL) {
		php_srand(time(NULL) * getpid() * (unsigned long) (php_combined_lcg(NULL) * 10000.0));
		h->fd = -1;
		h->num_servers = 0;
		h->ident = php_rand();
		h->errmsg[0] = '\0';
		memset(h->request, 0, sizeof h->request);
		h->req_len = 0;
		memset(h->pass, 0, sizeof h->pass);
		h->pass_len = 0;
		h->pass_pos = 0;
		h->chap_pass = 0;
		h->type = RADIUS_AUTH;
		h->request_created = 0;        
		h->resp_len = 0;
		h->srv = 0;
	}
	return h;
}

struct rad_handle *
rad_acct_open(void)
{
	struct rad_handle *h;

	h = rad_open();
	if (h != NULL)
	        h->type = RADIUS_ACCT;
	return h;
}

struct rad_handle *
rad_open(void)
{
    return rad_auth_open();
}

int
rad_put_addr(struct rad_handle *h, int type, struct in_addr addr, const struct rad_attr_options *options)
{
	return rad_put_attr(h, type, &addr.s_addr, sizeof addr.s_addr, options);
}

int
rad_put_attr(struct rad_handle *h, int type, const void *value, size_t len, const struct rad_attr_options *options)
{
	int result;

	if (!h->request_created) {
		generr(h, "Please call rad_create_request()");
		return -1;
	}

	if (type == RAD_USER_PASSWORD)
		result = put_password_attr(h, type, value, len, options);
	else {
		result = put_raw_attr(h, type, value, len, options);
		if (result == 0 && type == RAD_CHAP_PASSWORD)
			h->chap_pass = 1;
	}

	return result;
}

int
rad_put_int(struct rad_handle *h, int type, u_int32_t value, const struct rad_attr_options *options)
{
	u_int32_t nvalue;

	nvalue = htonl(value);
	return rad_put_attr(h, type, &nvalue, sizeof nvalue, options);
}

int
rad_put_string(struct rad_handle *h, int type, const char *str, const struct rad_attr_options *options)
{
	return rad_put_attr(h, type, str, strlen(str), options);
}

/*
 * Returns the response type code on success, or -1 on failure.
 */
int
rad_send_request(struct rad_handle *h)
{
	struct timeval timelimit;
	struct timeval tv;
	int fd;
	int n;

	n = rad_init_send_request(h, &fd, &tv);

	if (n != 0)
		return n;

	gettimeofday(&timelimit, NULL);
	timeradd(&tv, &timelimit, &timelimit);

	for ( ; ; ) {
		fd_set readfds;

		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);

		n = select(fd + 1, &readfds, NULL, NULL, &tv);

		if (n == -1) {
			generr(h, "select: %s", strerror(errno));
			return -1;
		}

		if (!FD_ISSET(fd, &readfds)) {
			/* Compute a new timeout */
			gettimeofday(&tv, NULL);
			timersub(&timelimit, &tv, &tv);
			if (tv.tv_sec > 0 || (tv.tv_sec == 0 && tv.tv_usec > 0))
				/* Continue the select */
				continue;
		}

		n = rad_continue_send_request(h, n, &fd, &tv);

		if (n != 0)
			return n;

		gettimeofday(&timelimit, NULL);
		timeradd(&tv, &timelimit, &timelimit);
	}
}

const char *
rad_strerror(struct rad_handle *h)
{
	return h->errmsg;
}

/*
 * Destructively split a string into fields separated by white space.
 * `#' at the beginning of a field begins a comment that extends to the
 * end of the string.  Fields may be quoted with `"'.  Inside quoted
 * strings, the backslash escapes `\"' and `\\' are honored.
 *
 * Pointers to up to the first maxfields fields are stored in the fields
 * array.  Missing fields get NULL pointers.
 *
 * The return value is the actual number of fields parsed, and is always
 * <= maxfields.
 *
 * On a syntax error, places a message in the msg string, and returns -1.
 */
static int
split(char *str, char *fields[], int maxfields, char *msg, size_t msglen)
{
	char *p;
	int i;
	static const char ws[] = " \t";

	for (i = 0;  i < maxfields;  i++)
		fields[i] = NULL;
	p = str;
	i = 0;
	while (*p != '\0') {
		p += strspn(p, ws);
		if (*p == '#' || *p == '\0')
			break;
		if (i >= maxfields) {
			snprintf(msg, msglen, "line has too many fields");
			return -1;
		}
		if (*p == '"') {
			char *dst;

			dst = ++p;
			fields[i] = dst;
			while (*p != '"') {
				if (*p == '\\') {
					p++;
					if (*p != '"' && *p != '\\' &&
					    *p != '\0') {
						snprintf(msg, msglen,
						    "invalid `\\' escape");
						return -1;
					}
				}
				if (*p == '\0') {
					snprintf(msg, msglen,
					    "unterminated quoted string");
					return -1;
				}
				*dst++ = *p++;
			}
			*dst = '\0';
			p++;
			if (*fields[i] == '\0') {
				snprintf(msg, msglen,
				    "empty quoted string not permitted");
				return -1;
			}
			if (*p != '\0' && strspn(p, ws) == 0) {
				snprintf(msg, msglen, "quoted string not"
				    " followed by white space");
				return -1;
			}
		} else {
			fields[i] = p;
			p += strcspn(p, ws);
			if (*p != '\0')
				*p++ = '\0';
		}
		i++;
	}
	return i;
}

int
rad_get_vendor_attr(u_int32_t *vendor, unsigned char *type, const void **data, size_t *len, const void *raw, size_t raw_len)
{
	struct vendor_attribute *attr;

	if (raw_len < sizeof(struct vendor_attribute)) {
		return -1;
	}

	attr = (struct vendor_attribute *) raw;
	*vendor = ntohl(attr->vendor_value);
	*type = attr->attrib_type;
	*data = attr->attrib_data;
	*len = attr->attrib_len - 2;

	if ((attr->attrib_len + 4) > raw_len) {
		return -1;
	}

	return (attr->attrib_type);
}

int
rad_put_vendor_addr(struct rad_handle *h, int vendor, int type,
    struct in_addr addr, const struct rad_attr_options *options)
{
	return (rad_put_vendor_attr(h, vendor, type, &addr.s_addr,
	    sizeof addr.s_addr, options));
}

int
rad_put_vendor_attr(struct rad_handle *h, int vendor, int type,
    const void *value, size_t len, const struct rad_attr_options *options)
{
	const void *actual_value = value;
	struct vendor_attribute *attr = NULL;
	struct rad_attr_options generic_options;
	int res = -1;
	struct rad_salted_value *salted = NULL;
	size_t va_len = len + 6;
    
	if (!h->request_created) {
		generr(h, "Please call rad_create_request()");
		return -1;
	}

	/* Initialise the options that will be passed through to
	 * put_raw_attr(). */
	generic_options.options = options->options;
	generic_options.tag = 0;

	/* Salting needs to be done on the vendor specific attribute. */
	if (options->options & RAD_OPTION_SALT) {
		generic_options.options &= ~RAD_OPTION_SALT;
		salted = emalloc(sizeof(struct rad_salted_value));

		if (rad_salt_value(h, value, len, salted) == -1) {
			goto end;
		} else {
			actual_value = salted->data;
			len = salted->len;
			va_len = len + 6;
		}
	}

	if (options->options & RAD_OPTION_TAG) {
		va_len++;
	}

	/* OK, allocate and start building the attribute. */
	attr = emalloc(va_len);
	if (attr == NULL) {
		generr(h, "malloc failure (%d bytes)", va_len);
		goto end;
	}

	attr->vendor_value = htonl(vendor);
	attr->attrib_type = type;
	attr->attrib_len = va_len - 4;

	/* Similarly, tagging needs to occur within the vendor specific
	 * attribute, rather than the generic attribute. */
	if (options->options & RAD_OPTION_TAG) {
		generic_options.options &= ~RAD_OPTION_TAG;
		attr->attrib_data[0] = options->tag;
		memcpy(attr->attrib_data + 1, actual_value, len);
	} else {
		memcpy(attr->attrib_data, actual_value, len);
	}

	res = put_raw_attr(h, RAD_VENDOR_SPECIFIC, attr, va_len, &generic_options);
	if (res == 0 && vendor == RAD_VENDOR_MICROSOFT
	    && (type == RAD_MICROSOFT_MS_CHAP_RESPONSE
	    || type == RAD_MICROSOFT_MS_CHAP2_RESPONSE)) {
		h->chap_pass = 1;
	}

end:
	if (attr) {
		efree(attr);
	}

	if (salted) {
		efree(salted->data);
		efree(salted);
	}

	return res;
}

int
rad_put_vendor_int(struct rad_handle *h, int vendor, int type, u_int32_t i, const struct rad_attr_options *options)
{
	u_int32_t value;

	value = htonl(i);
	return (rad_put_vendor_attr(h, vendor, type, &value, sizeof value, options));
}

int
rad_put_vendor_string(struct rad_handle *h, int vendor, int type,
    const char *str, const struct rad_attr_options *options)
{
	return (rad_put_vendor_attr(h, vendor, type, str, strlen(str), options));
}

ssize_t
rad_request_authenticator(struct rad_handle *h, char *buf, size_t len)
{
	if (len < LEN_AUTH)
		return (-1);
	memcpy(buf, h->request + POS_AUTH, LEN_AUTH);
	if (len > LEN_AUTH)
		buf[LEN_AUTH] = '\0';
	return (LEN_AUTH);
}

const char *
rad_server_secret(struct rad_handle *h)
{
	if (h->srv >= h->num_servers) {
		generr(h, "No RADIUS servers specified");
		return NULL;
	}

	return (h->servers[h->srv].secret);
}

int
rad_demangle(struct rad_handle *h, const void *mangled, size_t mlen, u_char *demangled) 
{
	char R[LEN_AUTH];
	const char *S;
	int i, Ppos;
	MD5_CTX Context;
	u_char b[16], *C;

	if ((mlen % 16 != 0) || (mlen > 128)) {
		generr(h, "Cannot interpret mangled data of length %ld", (u_long)mlen);
		return -1;
	}

	C = (u_char *)mangled;

	/* We need the shared secret as Salt */
	S = rad_server_secret(h);

	/* We need the request authenticator */
	if (rad_request_authenticator(h, R, sizeof R) != LEN_AUTH) {
		generr(h, "Cannot obtain the RADIUS request authenticator");
                return -1;
	}

	MD5Init(&Context);
	MD5Update(&Context, S, strlen(S));
	MD5Update(&Context, R, LEN_AUTH);
	MD5Final(b, &Context);
	Ppos = 0;
	while (mlen) {

		mlen -= 16;
		for (i = 0; i < 16; i++)
			demangled[Ppos++] = C[i] ^ b[i];

		if (mlen) {
			MD5Init(&Context);
			MD5Update(&Context, S, strlen(S));
			MD5Update(&Context, C, 16);
			MD5Final(b, &Context);
		}

		C += 16;
	}

	return 0;
}

int
rad_demangle_mppe_key(struct rad_handle *h, const void *mangled, size_t mlen, u_char *demangled, size_t *len)
{
	char R[LEN_AUTH];    /* variable names as per rfc2548 */
	const char *S;
	u_char b[16];
	const u_char *A, *C;
	MD5_CTX Context;
	int Slen, i, Clen, Ppos;
	u_char *P;

	if (mlen % 16 != SALT_LEN) {
		generr(h, "Cannot interpret mangled data of length %ld", (u_long)mlen);
		return -1;
	}

	/* We need the RADIUS Request-Authenticator */
	if (rad_request_authenticator(h, R, sizeof R) != LEN_AUTH) {
		generr(h, "Cannot obtain the RADIUS request authenticator");
		return -1;
	}

	A = (const u_char *)mangled;      /* Salt comes first */
	C = (const u_char *)mangled + SALT_LEN;  /* Then the ciphertext */
	Clen = mlen - SALT_LEN;
	S = rad_server_secret(h);    /* We need the RADIUS secret */
	Slen = strlen(S);
	P = alloca(Clen);        /* We derive our plaintext */

	MD5Init(&Context);
	MD5Update(&Context, S, Slen);
	MD5Update(&Context, R, LEN_AUTH);
	MD5Update(&Context, A, SALT_LEN);
	MD5Final(b, &Context);
	Ppos = 0;

	while (Clen) {
		Clen -= 16;

		for (i = 0; i < 16; i++)
		    P[Ppos++] = C[i] ^ b[i];

		if (Clen) {
			MD5Init(&Context);
			MD5Update(&Context, S, Slen);
			MD5Update(&Context, C, 16);
			MD5Final(b, &Context);
		}
                
		C += 16;
	}

	/*
	* The resulting plain text consists of a one-byte length, the text and
	* maybe some padding.
	*/
	*len = *P;
	if (*len > mlen - 1) {
		generr(h, "Mangled data seems to be garbage %d %d", *len, mlen-1);        
		return -1;
	}

	if (*len > MPPE_KEY_LEN) {
		generr(h, "Key to long (%d) for me max. %d", *len, MPPE_KEY_LEN);        
		return -1;
	}

	memcpy(demangled, P + 1, *len);
	return 0;
}

int rad_salt_value(struct rad_handle *h, const char *in, size_t len, struct rad_salted_value *out)
{
	char authenticator[16];
	size_t i;
	char intermediate[16];
	const char *in_pos;
	MD5_CTX md5;
	char *out_pos;
	php_uint32 random;
	size_t salted_len;
	const char *secret;

	if (len == 0) {
		out->len = 0;
		out->data = NULL;
		return 0;
	}

	/* Calculate the padded salted value length. */
	salted_len = len;
	if ((salted_len & 0x0f) != 0) {
		salted_len += 0x0f;
		salted_len &= ~0x0f;
	}

	/* 250 because there's a five byte overhead: one byte for type, one for
	 * length, two for the salt, and one for the encrypted value length,
	 * and the maximum RADIUS attribute size is 255 bytes. */
	if (salted_len > 250) {
		generr(h, "Value is too long to be salt-encrypted");
		return -1;
	}

	/* Actually allocate the buffer. */
	out->len = salted_len + 3;
	out->data = emalloc(out->len);

	if (out->data == NULL) {
		return -1;
	}

	memset(out->data, 0, out->len);

	/* Grab the request authenticator. */
	if (rad_request_authenticator(h, authenticator, sizeof authenticator) != sizeof authenticator) {
		generr(h, "Cannot obtain the RADIUS request authenticator");
		goto err;
	}

	/* Grab the server secret. */
	secret = rad_server_secret(h);
	if (secret == NULL) {
		generr(h, "Cannot obtain the RADIUS server secret");
		goto err;
	}

	/* Generate a random number to use as the salt. */
	random = php_rand();

	/* The RFC requires that the high bit of the salt be 1. Otherwise,
	 * let's set up the header. */
	out->data[0] = (unsigned char) random | 0x80;
	out->data[1] = (unsigned char) (random >> 8);
	out->data[2] = (unsigned char) salted_len;

	/* OK, let's get cracking on this. We have to calculate what the RFC
	 * calls b1 first. */
	MD5Init(&md5);
	MD5Update(&md5, secret, strlen(secret));
	MD5Update(&md5, authenticator, sizeof authenticator);
	MD5Update(&md5, out->data, 2);
	MD5Final(intermediate, &md5);

	/* XOR the first chunk. */
	in_pos = in - 1;
	out_pos = out->data + 2;
	for (i = 0; i < 16; i++) {
		if (in_pos < (in + len)) {
			*(++out_pos) = *(++in_pos) ^ intermediate[i];
		} else {
			*(++out_pos) = '\0' ^ intermediate[i];
		}
	}

	/* Now walk over the rest of the input. */
	while (in_pos < (in + len)) {
		MD5Init(&md5);
		MD5Update(&md5, secret, strlen(secret));
		MD5Update(&md5, out_pos - 15, 16);
		MD5Final(intermediate, &md5);

		for (i = 0; i < 16; i++) {
			if (in_pos < (in + len)) {
				*(++out_pos) = *(++in_pos) ^ intermediate[i];
			} else {
				*(++out_pos) = '\0' ^ intermediate[i];
			}
		}
	}

	return 0;

err:
	efree(out->data);
	out->data = NULL;
	out->len = 0;

	return -1;
}

/* vim: set ts=8 sw=8 noet: */
