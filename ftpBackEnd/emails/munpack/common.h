/* (C) Copyright 1993,1994 by Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of Carnegie
 * Mellon University not be used in advertising or publicity
 * pertaining to distribution of the software without specific,
 * written prior permission.  Carnegie Mellon University makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied
 * warranty.
 *
 * CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
 * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#ifdef __riscos
#define TEMPFILENAME "TempDesc"
#else
#define TEMPFILENAME "tempdesc.txt"
#endif

#if defined(unix) && ! defined(remove)
#define remove unlink
#endif

typedef char **params;

/* Flags for os_newtypedfile */
#define FILE_BINARY 0x1		/* File should be opened in binary mode */
#define FILE_INAPPLEDOUBLE 0x2	/* File was inside multipart/appledouble */


int parseSubject(char *subject, char **fnamep, int *partp, int *npartsp);
int saveUuFile(struct part *inpart, char *fname, int part, int nparts, char *firstline);
int cistrncmp(register char *s1,register char *s2,int n);
int cistrcmp(register char *s1,register char *s2);
int os_binhex(struct part *inpart, int part, int nparts);
int part_ungets(char *s, struct part *part);
int handleMessage(struct part *inpart, char *defaultContentType, int inAppleDouble, int extractText);
int descEnd(char *line);
void os_perror(char *file);
int uudecodefiles(char *dir, int nparts);
int part_close(struct part *part);

void uudecodeline(char *line, FILE *outfile);
void os_closetypedfile(FILE *outfile);
void os_donewithdir(char *dir);

char *strsave(char *str);

int to64(FILE *infile, FILE *outfile, long limit);

int handlePartial(struct part *inpart, char *headers, params contentParams, int extractText);
int ignoreMessage(struct part *inpart);

int handleMultipart(struct part *inpart,char *contentType,params contentParams,int extractText);

int part_depth(struct part *part);

int handleUuencode(struct part *inpart, char *subject, int extractText);

int handleText(struct part *inpart, enum encoding contentEncoding);

int saveToFile(struct part *inpart,int inAppleDouble,char *contentType,params contentParams,enum encoding contentEncoding,char *contentDisposition, char *contentMD5);

void SkipWhitespace(char **s);

int part_fill(struct part *part);
int part_addboundary(struct part *part, char *boundary);
int part_readboundary(struct part *part);

void fromnone(struct part *inpart, FILE *outfile, char **digestp);
void fromqp(struct part *inpart, FILE *outfile, char **digestp);
void from64(struct part *inpart, FILE *outfile, char **digestp, int suppressCR);

void os_warnMD5mismatch();
