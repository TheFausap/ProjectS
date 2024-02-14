// SPM.cpp : Defines the entry point for the application.
//

#include "s.h"

static V i0(V)
/* initialize everything */
{
	bf = calloc(SIZE, sizeof(C));
	bf2 = calloc(SIZE, sizeof(C));
	stk = calloc(SSIZE, sizeof(C*));
	mem = calloc(MSIZE, sizeof(C*));
	zr = calloc(SIZE, sizeof(C));
	one = calloc(SIZE, sizeof(C));
	if (!bf) ER("bf", 0xbf);
	if (!zr) ER("zr", 0xce);
	if (!one) ER("one", 0xebe);
	if (!bf2) ER("bf2", 0xbf2);
	if (!stk) ER("stk", 0xaed);
	if (!mem) ER("mem", 0xbeb);
	for (int i = 0; i < SSIZE; i++)
	{
		stk[i] = calloc(SIZE, sizeof(C));
		if (!stk[i]) ER("stk[]", 0xaed + i);
		memset(stk[i], '0', SIZE - 1); /* leave last for terminator */
	}
	for (int i = 0; i < MSIZE; i++)
	{
		mem[i] = calloc(SIZE, sizeof(C));
		if (!mem[i]) ER("mem[]", 0xbeb + i);
		memset(mem[i], '0', SIZE - 1);
	}
	memset(zr, '0', SIZE - 1);
	memset(one, '0', SIZE - 1); 
	one[SIZE - 2] = '1';
	sp = SSIZE - 1; cr = 0;
	fpsz = 0; fpp = FPSTART;
}

static V zero(C* m)
/* zeroes a vector with character 0 */
{
	memset(m, '0', SIZE - 1);
}

static V push(C* n)
{
	I dp = SIZE - 2;

	memset(stk[sp], '0', SIZE - 1);
	for (I i = (I)strlen(n) - 1; i >= 0; i--)
		stk[sp][dp--] = n[i];
	sp--;
}

static C* cvt(C* n)
/* convert a number in a string into vector */
{
	I dp = SIZE - 2;
	C* r;

	r = calloc(SIZE, sizeof(C));
	if (!r) ER("r", 0xe);
	memset(r, '0', SIZE - 1);
	for (I i = (I)strlen(n) - 1; i >= 0; i--)
		r[dp--] = n[i];
	return r;
}

static C* cvtf(C* n)
/* convert a FP number into non decimal form  */
/* multiply by a scale factor of power of ten */
{
	C* r;
	I k=SIZE-2,m = SIZE-2;
	r = calloc(SIZE, sizeof(C));
	if (!r) ER("r", 0xe);
	memset(r, '0', SIZE - 1);
	while(k>=0)
	{
		if (n[k] != '.')
		{
			r[m] = n[k];
			k--; m--;
		}
		else
			k--;
	}
	return r;
}

static I cntf(C* n)
{
	I cnt = 0, df=0;
	C* nc;
	C c;

	nc = n;
	while ((c=*n))
	{
		if (df) cnt++;
		if (c == '.') 
			df = 1;
		n++;
	}
	n = nc;
	return cnt;
}

static C* pop(V)
{
	return stk[++sp];
}

static V dmp(V)
{
	for (I i = SSIZE - 1; i >= SSIZE - 10; i--)
		fprintf(stdout, "S%04d,%s [%c]\n", i, stk[i],(i==(sp+1))?'*':' ');
	fprintf(stdout, "-----------------------------------------------------------------------------------\n");
	for (int i = 0; i < 10; i++)
		fprintf(stdout, "M%04d,%s\n", i, mem[i]);
	for (int i = MSIZE - 200; i < MSIZE - 195; i++)
		fprintf(stdout, "W%04d,%s\n", i, mem[i]);
}

static C* cm(C* s)
/* 9-complement of s */
{
	C* r;
	r = calloc(SIZE, sizeof(C));
	if (!r) ER("r", 0xe);
	memset(r, '0', SIZE - 1);
	for (I i = 0; i < SIZE - 1; i++)
		if (s[i] != '.')
			r[i] = (9 - (s[i] - '0')) + '0';
		else 
			r[i] = '.';
	return r;
}

static C _a(C d1, C d2)
/* elementary step for addition */
{
	C r = 0;

	d1 -= '0'; d2 -= '0';
	r = d1 + d2 + cr;
	if (r > 9)
	{
		r -= 10; cr = 1;
	}
	else
		cr = 0;

	return r + '0';
}

static C _m(C d1, C d2)
/* elementary step of multiplication */
/* using a table */
{
	C r = 0;
	d1 -= '0'; d2 -= '0';
	r = mt[d1 * 10 + d2] + cr;
	if (r > 9)
	{
		cr = 0;
		while (r > 9)
		{
			r -= 10; cr++;
		}
	}
	else
		cr = 0;

	return r + '0';
}

static V inc(C* s)
/* increment by 1 */
{
	I df = SIZE - 2;

	cr = 1;
	for (I i = 0; i < SIZE; i++)
	{
		s[df] = _a(s[df], zr[df]);
		df--;
	}
	cr = 0;
}

static I slen(C* s)
/* number of digits in s */
/* subtract 1 for FP numbers */
{
	C* sc;
	I sl = 0;

	sc = s;
	while (sc)
	{
		if (*sc == '0') sl++;
		else break;
		sc++;
	}
	return SIZE-1-sl;
}

/* s1 > s2 is +1 */
/* s1 < s2 is -1 */
/* s1 = s2 is  0 */
static I cmp(C* s1, C* s2)
{
	I df = 0;
	while ((s1[df] == s2[df])&&(df<SIZE)) { df++; }
	if (df == SIZE) return 0;
	if(s1[df]>s2[df]) return 1;
	if(s1[df]<s2[df]) return -1;
	return 0;
}

static V mvn(C* d, C* s, I sz)
/* copy the first sz digits of s, into the last sz digits of d */
{
	C* sc = s;
	I j = SIZE - 2;
	sz--;
	j -= sz;
	zero(d);
	while(sz>0)
	{
		d[j++] = *s++;
		sz--;
	}
	s = sc;
}

static C* _add(C* bf, C* bf2)
{
	I ln, ln2, df;
	C* bf3;

	bf3 = calloc(SIZE, sizeof(C));
	if (!bf3) ER("bf3",0xbf3);
	zero(bf3);

	ln = slen(bf);
	ln2 = slen(bf2);
	ln = (ln > ln2) ? ln : ln2;
	df = SIZE - 2;
	for (I i = 0; i < ln; i++)
	{
		bf3[df] = _a(bf[df], bf2[df]);
		df--;
	}
	if (cr == 1) bf3[df] = '1';
	cr = 0; df = 0;
	return bf3;
}

static C* _fadd(C* bf, C* bf2)
{
	I ln, ln2, df;
	C* bf3;

	bf3 = calloc(SIZE, sizeof(C));
	if (!bf3) ER("bf3", 0xbf3);
	zero(bf3);

	ln = slen(bf);
	ln2 = slen(bf2);
	ln = (ln > ln2) ? ln : ln2;
	df = SIZE - 2;
	for (I i = 0; i < ln; i++)
	{
		if (bf[df] != '.')
			bf3[df] = _a(bf[df], bf2[df]);
		else
			bf3[df] = '.';
		df--;
	}
	if (cr == 1) bf3[df] = '1';
	cr = 0; df = 0;
	return bf3;
}

static C* _fsub(C* bf, C* bf2c)
{
	I ln, ln2, df;
	C* bf3;
	C* bf2;

	bf2 = calloc(SIZE, sizeof(C));
	bf3 = calloc(SIZE, sizeof(C));
	if (!bf3) ER("bf3", 0xbf3);
	if (!bf2) ER("bf2c", 0xbf2);

	strcpy(bf2, cm(bf2c));
	cr = 1;

	ln = slen(bf);
	ln2 = slen(bf2);
	ln = (ln > ln2) ? ln : ln2;
	df = SIZE - 2;
	for (I i = 0; i < ln; i++)
	{
		if (bf[df] != '.')
			bf3[df] = _a(bf[df], bf2[df]);
		else
			bf3[df] = '.';
		df--;
	}
	if (cr == 1) bf3[df] = '1';
	cr = 0; df = 0;
	return bf3;
}

static C* _sub(C* bf, C* bf2c)
{
	I ln, ln2, df;
	C* bf3;
	C* bf2;

	bf2 = calloc(SIZE, sizeof(C));
	bf3 = calloc(SIZE, sizeof(C));
	if (!bf3) ER("bf3", 0xbf3);
	if (!bf2) ER("bf2c", 0xbf2);

	strcpy(bf2, cm(bf2c));
	cr = 1;

	ln = slen(bf);
	ln2 = slen(bf2);
	ln = (ln > ln2) ? ln : ln2;
	df = SIZE - 2;
	for (I i = 0; i < ln; i++)
	{
		bf3[df] = _a(bf[df], bf2[df]);
		df--;
	}
	if (cr == 1) bf3[df] = '1';
	cr = 0; df = 0;
	return bf3;
}

static C* _mul(C* bf, C* bf2)
/* long multiplication algorithm */
{
	I ln, ln2, wka;
	I df, dff;
	C* bf3;

	bf3 = calloc(SIZE, sizeof(C));
	if (!bf3) ER("bf3", 0xbf3);
	memset(bf3, '0', SIZE - 1);

	ln = slen(bf);
	ln2 = slen(bf2);
	wka = MSIZE - WKSIZE;
	for (I i = wka; i < MSIZE; i++) zero(mem[i]); /* working area: latest 200 loc of memory */
	if (ln < ln2)
	{
		df = ln2;
		ln2 = ln;
		ln = df; /* ln is always bigger than ln2 */
		strcpy(bf3, bf2);
		strcpy(bf2, bf);
		strcpy(bf, bf3);
		memset(bf3, '0', SIZE - 1);
	}
	df = SIZE - 2; dff = SIZE - 2;
	for (I j = 0; j < ln; j++)
	{
		bf3[dff] = _m(bf[dff], bf2[df]);
		dff--;
	}
	if (cr != 0)
	{
		bf3[dff] = cr + '0';
		cr = 0;
	}
	strcpy(mem[wka++], bf3);
	dff = SIZE - 2; ln++; df--;
	for (I i = 1; i < ln2; i++)
	{
		for (I j = 0; j < ln; j++)
		{
			bf3[dff] = _m(bf[dff], bf2[df]);
			dff--;
		}
		if (cr != 0)
		{
			bf3[dff] = cr + '0';
			cr = 0;
		}
		/* shift left k digit */
		for (I k = 0; k < i; k++)
		{
			for (I i = 0; i < SIZE - 1; i++) bf3[i] = bf3[i + 1];
			bf3[SIZE - 2] = '0';
		}
		strcpy(mem[wka++], bf3);
		dff = SIZE - 2; ln++; df--;
	}
	if (cr != 0)
	{
		bf3[df] = cr + '0';
		cr = 0;
	}
	/* add all the partial results */
	for (I j = MSIZE - WKSIZE; j < wka - 1; j++)
	{
		for (I i = SIZE - 2; i >= 0; i--) bf3[i] = _a(bf3[i], mem[j][i]);
	} 
	while (*bf3++ == '0') {} /* remove leading zeros */
	return bf3-1; 
}

static V _div(C* bf, C* bf2)
{
	C* bf3;
	I wka = MSIZE - (2 * WKSIZE);
	I d_c1 = 0, d_c2 = 0, d_c3 = 0;
	I ln = 0, ln2 = 0;

	bf3 = calloc(SIZE, sizeof(C));
	if (!bf3) ER("bf3", 0xbf3);
	zero(bf3);

	for (I i = wka; i < MSIZE - WKSIZE; i++) zero(mem[i]);	/* working area */
	/* cleaning latest 200 loc of memory, adjacent to the mul wkarea */
	/* MEMORY STRUCTURE FOR DIVISION */
	/* wka   = quotient              */
	/* wka+1 = dividend              */
	/* wka+2 = divisor               */
	/* wka+3 = remainder             */
	
	if (cmp(bf, bf2) == 1)
	{
		strcpy(mem[wka],zr);
		strcpy(mem[wka+3], bf);
		return;
	}
	if (cmp(bf, bf2) == 0)
	{
		strcpy(mem[wka],one);
		strcpy(mem[wka+3],zr);
		return;
	}
	strcpy(mem[wka + 1], bf2);
	strcpy(mem[wka + 2], bf);
	ln = slen(bf);			/* how many digits in the divisor        */
	while (bf2[d_c1] == '0') { d_c1++; }
	bf2 += d_c1;			/* start at the first non zero digit     */
	mvn(bf3, bf2, ln);		/* move n digits of dividend to temp bf3 */
	while (cmp(bf3, bf) < 0)
	{
		ln++;
		mvn(bf3, bf2, ln);  /* move one more if less                 */
	}
	/* first digit */
	zero(bf);
	do
	{
		d_c2++;
		strcpy(bf, _add(bf, mem[wka + 2]));
	} while (cmp(bf, bf3) <= 0);
	if (cmp(bf, bf3) > 0)
	{
		d_c2--;
		strcpy(bf, _sub(bf, mem[wka + 2]));
	}
	d_c1 += (ln - 1);
	mem[wka][d_c1] = d_c2 + '0';
	d_c1++;
	strcpy(bf3, _sub(bf3, bf));
	for (int i = 0; i < SIZE - 1; i++) bf3[i] = bf3[i + 1];
	bf3[SIZE - 2] = mem[wka + 1][d_c1];
	/* intermediate digits */
	while (d_c1 < SIZE - 2)
	{
		zero(bf);
		d_c2 = 0;
		do
		{
			d_c2++;
			strcpy(bf, _add(bf, mem[wka + 2]));
		} while (cmp(bf, bf3) <= 0);
		if (d_c2 > 10) d_c2 %= 10;
		if (cmp(bf, bf3) > 0)
		{
			d_c2--;
			strcpy(bf, _sub(bf, mem[wka + 2]));
		}
		mem[wka][d_c1] = d_c2 + '0';
		d_c1++;
		strcpy(bf3, _sub(bf3, bf));
		for (int i = 0; i < SIZE - 1; i++) bf3[i] = bf3[i + 1];
		bf3[SIZE - 2] = mem[wka + 1][d_c1];
	}
	/* last digit ? */
	if (d_c1 < 99)
	{
		d_c2 = 0;
		zero(bf);
		do
		{
			d_c2++;
			strcpy(bf, _add(bf, mem[wka + 2]));
		} while (cmp(bf, bf3) < 0);
		if (cmp(bf, bf3) > 0)
		{
			d_c2--;
			strcpy(bf, _sub(bf, mem[wka + 2]));
		}
		mem[wka][d_c1] = d_c2 + '0';
		strcpy(bf3, _sub(bf3, bf));
		if (bf3[0] == '9')
		{
			strcpy(bf3, _add(bf3, bf));
		}
	}
	strcpy(mem[wka+3],bf3);
}

static I _fdiv(C* bf, C* bf2)
{
	C c;
	C* bf3;

	I wka = MSIZE - (2 * WKSIZE);
	I d_c1 = 0, d_c2 = 0, d_c3 = 0;
	I ln = 0, ln2 = 0;

	bf3 = calloc(SIZE, sizeof(C));
	if (!bf3) ER("bf3", 0xbf3);
	zero(bf3);

	for (I i = wka; i < MSIZE - WKSIZE; i++) zero(mem[i]);	/* working area */
	/* cleaning latest 200 loc of memory, adjacent to the mul wkarea */
	/* MEMORY STRUCTURE FOR DIVISION */
	/* wka   = quotient              */
	/* wka+1 = dividend              */
	/* wka+2 = divisor               */
	/* wka+3 = remainder             */

	if (cmp(bf, bf2) == 0)
	{
		strcpy(mem[wka], one);
		strcpy(mem[wka + 3], zr);
		return;
	}
	strcpy(mem[wka + 1], bf2);
	strcpy(mem[wka + 2], bf);
	ln = slen(bf);			/* how many digits in the divisor        */
	while (bf2[d_c1] == '0') { d_c1++; }
	bf2 += d_c1;			/* start at the first non zero digit     */
	mvn(bf3, bf2, ln);		/* move n digits of dividend to temp bf3 */
	while (cmp(bf3, bf) < 0)
	{
		ln++;
		mvn(bf3, bf2, ln);  /* move one more if less                 */
	}
	/* first digit */
	zero(bf);
	do
	{
		d_c2++;
		strcpy(bf, _add(bf, mem[wka + 2]));
	} while (cmp(bf, bf3) <= 0);
	if (cmp(bf, bf3) > 0)
	{
		d_c2--;
		strcpy(bf, _sub(bf, mem[wka + 2]));
	}
	d_c1 += (ln - 1);
	mem[wka][d_c1] = d_c2 + '0';
	d_c1++;
	strcpy(bf3, _sub(bf3, bf));
	for (int i = 0; i < SIZE - 1; i++) bf3[i] = bf3[i + 1];
	bf3[SIZE - 2] = mem[wka + 1][d_c1];
	/* intermediate digits */
	while (d_c1 < SIZE - 2)
	{
		zero(bf);
		d_c2 = 0;
		do
		{
			d_c2++;
			strcpy(bf, _add(bf, mem[wka + 2]));
		} while (cmp(bf, bf3) <= 0);
		if (d_c2 > 10) d_c2 %= 10;
		if (cmp(bf, bf3) > 0)
		{
			d_c2--;
			strcpy(bf, _sub(bf, mem[wka + 2]));
		}
		mem[wka][d_c1] = d_c2 + '0';
		d_c1++;
		strcpy(bf3, _sub(bf3, bf));
		for (int i = 0; i < SIZE - 1; i++) bf3[i] = bf3[i + 1];
		bf3[SIZE - 2] = mem[wka + 1][d_c1];
	}
	/* last digit ? */
	if (d_c1 < 99)
	{
		d_c2 = 0;
		zero(bf);
		do
		{
			d_c2++;
			strcpy(bf, _add(bf, mem[wka + 2]));
		} while (cmp(bf, bf3) < 0);
		if (cmp(bf, bf3) > 0)
		{
			d_c2--;
			strcpy(bf, _sub(bf, mem[wka + 2]));
		}
		mem[wka][d_c1] = d_c2 + '0';
		strcpy(bf3, _sub(bf3, bf));
		if (bf3[0] == '9')
		{
			strcpy(bf3, _add(bf3, bf));
		}
	}
	for (int i = 0; i < SIZE - 1; i++) mem[wka][i] = mem[wka][i + 1];
	mem[wka][SIZE - 2] = '.';
	for (int j = 0; j < fpsz; j++)
	{
		for (int i = 0; i < SIZE - 1; i++) mem[wka][i] = mem[wka][i + 1];
		mem[wka][SIZE - 2] = '0';
	}
	/* decimals */
	d_c1 -= fpsz;
	d_c1++;
	while (d_c1 < 99)
	{
		d_c2 = 0;
		zero(bf);
		for (int i = 0; i < SIZE - 1; i++) bf3[i] = bf3[i + 1];
		bf3[SIZE - 2] = '0';
		do
		{
			d_c2++;
			strcpy(bf, _add(bf, mem[wka + 2]));
		} while (cmp(bf, bf3) < 0);
		if (cmp(bf, bf3) > 0)
		{
			d_c2--;
			strcpy(bf, _sub(bf, mem[wka + 2]));
		}
		mem[wka][d_c1] = d_c2 + '0';
		strcpy(bf3, _sub(bf3, bf));
		if (bf3[0] == '9')
		{
			strcpy(bf3, _add(bf3, bf));
		}
		d_c1++;
	}
	return wka;
}

I main(I n, C** a)
{
	C c;
	C* bfc;
	C* bf3;
	I ln = 0, ln2 = 0, df = 0, dff = 0;
	I wka = 0, bfs = 0, bf2s = 0;
	I skp = 0;
	I cnt = 0;

	/* Used by division only */
	I d_c1, d_c2, d_c3;

	fi = fopen(a[1], "r");
	i0();
	bfc = bf;
	bf3 = calloc(SIZE, sizeof(C));
	if (!bf3) ER("bf3", 0xbf3);
	memset(bf3, '0', SIZE - 1);

	c = fgetc(fi);
	while (c != '\n')
	{
		if (skp)
		{ /* used for jump/skip */ 
			skp = 0;
		}
		else if (isdigit(c))
		{
digi:		while (c != ' ')
			{
				*bf++ = c;
				c = fgetc(fi);
			}
			*bf = 0;
			bf = bfc;
			if (df)
			{
				push(bf);
				strcpy(bf, cm(pop()));
				inc(bf);
				push(bf);
			}
			else push(bf);
		}
		else if (c == ' ')
		{
			memset(bf, '0', SIZE - 1);
			memset(bf2, '0', SIZE - 1);
			memset(bf3, '0', SIZE - 1);
		}
		else if (c == '?') dmp();
		else if (c == '.') printf("%s", stk[sp + 1]);
		else if (c == '>')
		{
			c = fgetc(fi);
			while (c != ' ')
			{
				*bf++ = c;
				c = fgetc(fi);
			}
			*bf = 0;
			bf = bfc;
			strcpy(mem[strtol(bf, NULL, 10)], stk[sp + 1]);
		}
		else if (c == '<')
		{
			c = fgetc(fi);
			while (c != ' ')
			{
				*bf++ = c;
				c = fgetc(fi);
			}
			*bf = 0;
			bf = bfc;
			strcpy(stk[sp--], mem[strtol(bf, NULL, 10)]);
		}
		else if (c == 'J')
		{
			/* jump back or forward n chars */
			ln = 0;
			c = fgetc(fi);
			while (c != ' ')
			{
				*bf++ = c;
				c = fgetc(fi);
				ln++;
			}
			*bf = 0;
			bf = bfc;
			if (ln < ftell(fi)) ln *= -1;
			fseek(fi, ln, SEEK_CUR);
		}
		else if (c == 'F')
		{
			/* FIXED POINT FUNCTIONS */
			c = fgetc(fi);
			if (c == 'P')
			{
				/* setup precision */
				c = fgetc(fi);
				while (c != ' ')
				{
					*bf++ = c;
					c = fgetc(fi);
				}
				*bf = 0;
				bf = bfc;
				fpsz = strtol(bf, NULL, 10);
			}
			if (c == 'c')
			{
				/* decimal conversion, fixed precision */
				/* how many digits of precision        */
				c = fgetc(fi);
				memset(bf, '0', SIZE - 1);
				while (c != ' ')
				{
					*bf++ = c;
					c = fgetc(fi);
				}
				*bf = 0;
				bf = bfc;
				strcpy(bf2, pop());
				for (int i = 0; i < SIZE - 1; i++) bf2[i] = bf2[i + 1];
				bf2[SIZE - 2] = '.';
				for (int j = 0; j < strtol(bf, NULL, 10); j++)
				{
					for (int i = 0; i < SIZE - 1; i++) bf2[i] = bf2[i + 1];
					bf2[SIZE - 2] = '0';
				}
				push(bf2);
			}
			else if (c == '+')
			{
				strcpy(bf2, pop());
				strcpy(bf, pop());
				push(_fadd(bf, bf2));
				cr = 0; df = 0;
			}
			else if (c == '-')
			{
				strcpy(bf2, cm(pop()));
				strcpy(bf, pop());
				push(_fsub(bf, bf2));
				cr = 0; df = 0;
			}
			else if (c == '*')
			{
				strcpy(bf2, pop());
				strcpy(bf, pop());
				df = cntf(bf) + cntf(bf2); /* total number of decimal digits */
				strcpy(bf3, cvtf(bf2));
				strcpy(bf2, bf3);
				strcpy(bf3, cvtf(bf));
				strcpy(bf, bf3);
				strcpy(bf3, _mul(bf, bf2));
				strcpy(mem[fpp], cvt(bf3));
				zero(bf3);
				dff = SIZE - 2; ln = SIZE - 2;
				while(dff >= 0)
				{
					if (df > 0)
					{
						bf3[ln] = mem[fpp][dff];
						df--; dff--; ln--;
					}
					else if (df == 0)
					{
						bf3[ln] = '.';
						df--; ln--;
					}
					else
					{
						bf3[ln] = mem[fpp][dff];
						dff--; ln--;
					}
				}
				push(bf3);
			}
			else if (c == '%')
			{
				/* FP Division, unsigned, not rounded */
				strcpy(bf2, pop());
				strcpy(bf, pop());
				df = cntf(bf);			/* number of decimal digits */
				strcpy(bf3, cvtf(bf2));
				strcpy(bf2, bf3);
				strcpy(bf3, cvtf(bf));
				strcpy(bf, bf3);
				/* adding zeros for decimal places */
				for (int j = 0; j < fpsz; j++)
				{
					for (int i = 0; i < SIZE - 1; i++) bf2[i] = bf2[i + 1];
					bf2[SIZE - 2] = '0';
				}
				
				for (int j = 0; j < fpsz; j++)
				{
					for (int i = 0; i < SIZE - 1; i++) bf[i] = bf[i + 1];
					bf[SIZE - 2] = '0';
				}
				df=_fdiv(bf, bf2);
				push(mem[df]);
			}
			else if (c == '=')
			{
				c = fgetc(fi);
				cnt = 1; df = 0;
				memset(bf, '0', SIZE - 1);
				while (c != ' ')
				{
					*bf++ = c;
					if (df) cnt++;
					if (c == '.') df = cnt;
					c = fgetc(fi);
				}
				cnt--;
				*bf = 0;
				bf = bfc;
				memset(bf2, '0', SIZE - 1);
				strcpy(bf2, cvt(bf));
				if(cnt<fpsz)
					for (int j = 0; j < fpsz - df; j++)
					{
						for (int i = 0; i < SIZE - 1; i++) bf2[i] = bf2[i + 1];
						bf2[SIZE - 2] = '0';
					}
				push(bf2);
				df = 0; cnt = 0;
			}
		}
		else if (c == '}') /* logical shift right */
		{
			strcpy(bf, pop());
			for (int i = SIZE - 2; i >= 0; i--) bf[i] = bf[i - 1];
			bf[0] = '0';
			push(bf);
		}
		else if (c == '{') /* shift left */
		{
			strcpy(bf, pop());
			for (int i = 0; i < SIZE - 1; i++) bf[i] = bf[i + 1];
			bf[SIZE - 2] = '0';
			push(bf);
		}
		else if (c == '+')
		{
			strcpy(bf2, pop());
			strcpy(bf, pop());
add:		ln = slen(bf);
			ln2 = slen(bf2);
			ln = (ln > ln2) ? ln : ln2;
			df = SIZE - 2;
			for (I i = 0; i < ln; i++)
			{
				bf3[df] = _a(bf[df], bf2[df]);
				df--;
			}
			if (cr == 1) bf3[df] = '1';
			push(bf3); cr = 0; df = 0;
		}
		else if (c == '-')
		{
			c = fgetc(fi);
			if (isdigit(c))
			{
				df = 1;
				goto digi;
			}
			else
			{
				ungetc(c, fi);
				df = 0;
			}
			strcpy(bf2, cm(pop()));
			strcpy(bf, pop());
			cr = 1;
			goto add;
		}
		else if (c == '*') /* unsigned multiplication */
		{
			strcpy(bf2, pop());
			strcpy(bf, pop());
			ln = slen(bf);
			ln2 = slen(bf2);
			wka = MSIZE - WKSIZE;
			for (I i = wka; i < MSIZE; i++) zero(mem[i]); /* working area: latest 200 loc of memory */
			if (ln < ln2)
			{
				df = ln2;
				ln2 = ln;
				ln = df; /* ln is always bigger than ln2 */
				strcpy(bf3, bf2);
				strcpy(bf2, bf);
				strcpy(bf, bf3);
				memset(bf3, '0', SIZE - 1);
			}
			df = SIZE - 2; dff = SIZE - 2;
			for (int j = 0; j < ln; j++)
			{
				bf3[dff] = _m(bf[dff], bf2[df]);
				dff--;
			}
			if (cr != 0)
			{
				bf3[dff] = cr + '0';
				cr = 0;
			}
			strcpy(mem[wka++], bf3);
			dff = SIZE - 2; ln++; df--;
			for (I i = 1; i < ln2; i++)
			{
				for (I j = 0; j < ln; j++)
				{
					bf3[dff] = _m(bf[dff], bf2[df]);
					dff--;
				}
				if (cr != 0)
				{
					bf3[dff] = cr + '0';
					cr = 0;
				}
				/* shift left k digit */
				for (I k = 0; k < i; k++)
				{
					for (I i = 0; i < SIZE - 1; i++) bf3[i] = bf3[i + 1];
					bf3[SIZE - 2] = '0';
				}
				strcpy(mem[wka++], bf3);
				dff = SIZE - 2; ln++; df--;
			}
			if (cr != 0)
			{
				bf3[df] = cr + '0';
				cr = 0;
			}
			/* add all the partial results */
			for (I j = MSIZE - WKSIZE; j < wka - 1; j++)
			{
				for (I i = SIZE - 2; i >= 0; i--) bf3[i] = _a(bf3[i], mem[j][i]);
			}
			push(bf3); df = 0; dff = 0;
		}
		else if (c == 'M') /* signed multiplication */
		{
			strcpy(bf2, pop());
			strcpy(bf, pop());
			if (bf[0] == '9')
			{
				strcpy(bf, cm(bf));
				inc(bf);
				bfs = 1;
			}
			if (bf2[0] == '9')
			{
				strcpy(bf2, cm(bf2));
				inc(bf2);
				bf2s = 1;
			}
			ln = slen(bf);
			ln2 = slen(bf2);
			wka = MSIZE - WKSIZE;
			for (I i = wka; i < MSIZE; i++) zero(mem[i]); /* working area: latest 200 loc of memory */
			if (ln < ln2)
			{
				df = ln2;
				ln2 = ln;
				ln = df; /* ln is always bigger than ln2 */
				strcpy(bf3, bf2);
				strcpy(bf2, bf);
				strcpy(bf, bf3);
				memset(bf3, '0', SIZE - 1);
			}
			df = SIZE - 2; dff = SIZE - 2;
			for (I j = 0; j < ln; j++)
			{
				bf3[dff] = _m(bf[dff], bf2[df]);
				dff--;
			}
			if (cr != 0)
			{
				bf3[dff] = cr + '0';
				cr = 0;
			}
			strcpy(mem[wka++], bf3);
			dff = SIZE - 2; ln++; df--;
			for (I i = 1; i < ln2; i++)
			{
				for (I j = 0; j < ln; j++)
				{
					bf3[dff] = _m(bf[dff], bf2[df]);
					dff--;
				}
				if (cr != 0)
				{
					bf3[dff] = cr + '0';
					cr = 0;
				}
				/* shift left k digit */
				for (int k = 0; k < i; k++)
				{
					for (int i = 0; i < SIZE - 1; i++) bf3[i] = bf3[i + 1];
					bf3[SIZE - 2] = '0';
				}
				strcpy(mem[wka++], bf3);
				dff = SIZE - 2; ln++; df--;
			}
			if (cr != 0)
			{
				bf3[df] = cr + '0';
				cr = 0;
			}
			/* add all the partial results */
			for (I j = MSIZE - WKSIZE; j < wka - 1; j++)
			{
				for (I i = SIZE - 2; i >= 0; i--) bf3[i] = _a(bf3[i], mem[j][i]);
			}
			if (bfs ^ bf2s)
			{
				strcpy(bf, cm(bf3));
				inc(bf);
				push(bf);
			}
			else push(bf3); df = 0; dff = 0;
		}
		else if (c == '%') /* unsigned integer division */
			               /* bf2 / bf */
		{
			wka = MSIZE - (2*WKSIZE);
			d_c1 = 0; d_c2 = 0; d_c3 = 0;
			ln = 0; ln2 = 0;
			for (int i = wka; i < MSIZE-WKSIZE; i++) zero(mem[i]);	/* working area */ 
			       /* cleaning latest 200 loc of memory, adjacent to the mul wkarea */
			       /* MEMORY STRUCTURE FOR DIVISION */
			       /* wka   = quotient              */
			       /* wka+1 = dividend              */
				   /* wka+2 = divisor               */
			       /* bf3   = remainder             */
			       /* push(C(wka)), push(bf3)       */
			strcpy(bf2, pop());
			strcpy(bf, pop());
			if (cmp(bf, bf2) == 1) 
			{ 
				push(zr); 
				push(bf); 
				c = fgetc(fi);
				continue; 
			}
			if (cmp(bf, bf2) == 0) 
			{ 
				push(one); 
				push(zr);
				c = fgetc(fi);
				continue; 
			}
			strcpy(mem[wka + 1], bf2);
			strcpy(mem[wka + 2], bf);
			ln = slen(bf);			/* how many digits in the divisor        */
			while (bf2[d_c1] == '0') { d_c1++; }
			bf2 += d_c1;			/* start at the first non zero digit     */
			mvn(bf3, bf2, ln);		/* move n digits of dividend to temp bf3 */
			while (cmp(bf3, bf) < 0)
			{
				ln++;
				mvn(bf3, bf2, ln);  /* move one more if less                 */
			}
			/* first digit */
			zero(bf);
			do
			{
				d_c2++;
				strcpy(bf, _add(bf, mem[wka+2]));
			} while (cmp(bf, bf3) <= 0);
			if (cmp(bf, bf3) > 0)
			{
				d_c2--;
				strcpy(bf, _sub(bf, mem[wka + 2]));
			}
			d_c1 += (ln-1);
			mem[wka][d_c1] = d_c2 + '0';
			d_c1++;
			strcpy(bf3, _sub(bf3, bf));
			for (int i = 0; i < SIZE - 1; i++) bf3[i] = bf3[i + 1];
			bf3[SIZE - 2] = mem[wka + 1][d_c1];
			/* intermediate digits */
			while(d_c1<SIZE-2)
			{
				zero(bf);
				d_c2 = 0;
				do
				{
					d_c2++;
					strcpy(bf, _add(bf, mem[wka+2]));
				} while (cmp(bf, bf3) <= 0);
				if (d_c2 > 10) d_c2 %= 10;
				if (cmp(bf, bf3) > 0)
				{
					d_c2--;
					strcpy(bf, _sub(bf, mem[wka + 2]));
				}
				mem[wka][d_c1] = d_c2 + '0';
				d_c1++;
				strcpy(bf3, _sub(bf3, bf));
				for (int i = 0; i < SIZE - 1; i++) bf3[i] = bf3[i + 1];
				bf3[SIZE - 2] = mem[wka + 1][d_c1];
			}
			/* last digit ? */
			if(d_c1<99)
			{
				d_c2 = 0;
				zero(bf);
				do
				{
					d_c2++;
					strcpy(bf, _add(bf, mem[wka + 2]));
				} while (cmp(bf, bf3) < 0);
				if (cmp(bf, bf3) > 0)
				{
					d_c2--;
					strcpy(bf, _sub(bf, mem[wka + 2]));
				}
				mem[wka][d_c1] = d_c2 + '0';
				strcpy(bf3, _sub(bf3, bf));
				if (bf3[0] == '9')
				{
					strcpy(bf3, _add(bf3, bf));
				}
			}
			push(mem[wka]);
			push(bf3);
		}
		else if (c == '/') /* min(a,b) */
		{
			strcpy(bf, pop());
			strcpy(bf2, pop());
			df = 0;
			while (bf[df++] == bf2[df++]) {}
			if (bf[df] > bf2[df]) push(bf2);
			else push(bf);
		}
		else if (c == '\\') /* max(a,b) */
		{
			strcpy(bf, pop());
			strcpy(bf2, pop());
			df = 0;
			while (bf[df++] == bf2[df++]) {}
			if (bf[df] > bf2[df]) push(bf);
			else push(bf2);
		}
		else if (c == 'D')
			/* DUP */
		{
			strcpy(bf, pop());
			push(bf); push(bf);
		}
		else if (c == 'S')
			/* SWAP */
		{
			strcpy(bf, pop());
			strcpy(bf2, pop());
			push(bf); push(bf2);
		}
		else if (c == '~')
		{
			strcpy(bf, pop());
			push(cm(bf));
		}
		c = fgetc(fi);
	}

	fclose(fi);
	return 0;
}
