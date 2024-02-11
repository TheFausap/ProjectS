// SPM.cpp : Defines the entry point for the application.
//

#include "s.h"

static V i0(V)
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
}

static V zero(C* m)
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

static C* pop(V)
{
	return stk[++sp];
}

static V dmp(V)
{
	for (I i = SSIZE - 1; i >= SSIZE - 10; i--)
		fprintf(stdout, "S%04d,%s\n", i, stk[i]);
	for (int i = 0; i < 10; i++)
		fprintf(stdout, "M%04d,%s\n", i, mem[i]);
	for (int i = MSIZE - 200; i < MSIZE - 195; i++)
		fprintf(stdout, "W%04d,%s\n", i, mem[i]);
}

static C* cm(C* s)
{
	C* r;
	r = calloc(SIZE, sizeof(C));
	if (!r) ER("r", 0xe);
	memset(r, '0', SIZE - 1);
	for (I i = 0; i < SIZE - 1; i++)
		r[i] = (9 - (s[i] - '0')) + '0';
	return r;
}

static C _a(C d1, C d2)
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
	return bf3; 
}

static V mvn(C* d, C* s, I sz)
{
	C* sc = s;
	I j = SIZE - 2;
	zero(d);
	for (C* i = sc; i < sc+sz; i++)
	{
		d[j--] = *i;
	}
}

I main(I n, C** a)
{
	C c;
	C* bfc;
	C* bf3;
	I ln = 0, ln2 = 0, df = 0, dff = 0;
	I wka = 0, bfs = 0, bf2s = 0;

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
		if (isdigit(c))
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
			while (c != ' ')
			{
				*bf++ = c;
				c = fgetc(fi);
			}
			bf = bfc;
			strcpy(mem[strtol(bf, NULL, 10)], stk[sp + 1]);
		}
		else if (c == '<')
		{
			while (c != ' ')
			{
				*bf++ = c;
				c = fgetc(fi);
			}
			bf = bfc;
			strcpy(stk[sp--], mem[strtol(bf, NULL, 10)]);
		}
		else if (c == '}') /* logical shift right */
		{
			strcpy(bf, pop());
			for (int i = SIZE - 2; i >= 0; i--)
				bf[i] = bf[i - 1];
			bf[0] = '0';
			push(bf);
		}
		else if (c == '{') /* shift left */
		{
			strcpy(bf, pop());
			for (int i = 0; i < SIZE - 1; i++)
				bf[i] = bf[i + 1];
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
			/* first round */
			zero(bf);
			while (cmp(bf, bf3) <= 0)
			{
				d_c2++;
				strcpy(bf, _add(bf, mem[wka+2]));
			}
			if (d_c2 > 1) d_c2--;
			d_c1 += (ln-1);
			mem[wka][d_c1] = d_c2 + '0';
			
			d_c1++;
			strcpy(bf3, _sub(bf, bf3));
			for (int i = 0; i < SIZE - 1; i++) bf3[i] = bf3[i + 1];
			bf3[SIZE - 2] = mem[wka + 1][d_c1];
			
			while(d_c1<SIZE-2)
			{
				zero(bf);
				d_c2 = 0;
				while (cmp(bf, bf3) < 0)
				{
					strcpy(bf, _add(bf, mem[wka+2]));
					d_c2++;
				}
				if (d_c2 > 10) d_c2 %= 10;
				else if (d_c2 > 1) d_c2--;
				mem[wka][d_c1] = d_c2 + '0';
				d_c1++;
				strcpy(bf3, _sub(bf, bf3));
				for (int i = 0; i < SIZE - 1; i++) bf3[i] = bf3[i + 1];
				bf3[SIZE - 2] = mem[wka + 1][d_c1];
			}
			/* last digit */
			d_c2 = 0;
			zero(bf);
			while (cmp(bf, bf3) < 0)
			{
				strcpy(bf, _add(bf, mem[wka+2]));
				d_c2++;
			}
			mem[wka][d_c1] = d_c2 + '0';
			strcpy(bf3, _sub(bf3, bf));
			if(bf3[0]=='9')
			{
				strcpy(bf3, _add(bf3, bf));
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
		{
			strcpy(bf, pop());
			push(bf); push(bf);
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
