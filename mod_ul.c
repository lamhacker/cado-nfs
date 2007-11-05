/* Some functions for modular arithmetic with residues and modulus in
   unsigned long variables. Due to inlining, this file must be included
   in the caller's source code with #include */

/* Naming convention: all function start with modul, for 
   MODulus Unsigned Long, followed by underscore, functionality of function
  (add, mul, etc), and possibly underscore and specification of what argument
  types the function takes (_ul, etc).
  There are typedef's that rename all functions to mod_* instead of 
  modul_*, which one day might become an automatic renaming scheme so that
  different modulus sizes can be used simply by #including different mod_*.c
  files, but without changing anything else in the source code. */

#ifndef __MOD_UL_H__

#define __MOD_UL_H__

#ifndef ASSERT
 #ifdef WANT_ASSERT
  #include <assert.h>
  #define ASSERT(x) assert(x)
 #else
  #define ASSERT(x)
 #endif
#define MOD_UL_ASSERT
#endif

#ifndef __GNUC_ATTRIBUTE_UNUSED__
#define __GNUC_ATTRIBUTE_UNUSED__ __attribute__ ((unused))
#define MOD_UL_GNUC_ATTRIBUTE_UNUSED
#endif

/* Define the default names and types for arithmetic with these functions */
#define mod_init             modul_init
#define mod_init_noset0      modul_init_noset0
#define mod_clear            modul_clear
#define mod_set              modul_set
#define mod_set_ul           modul_set_ul
#define mod_set_ul_reduced   modul_set_ul_reduced
#define mod_initmod_ul       modul_initmod_ul
#define mod_clearmod         modul_clearmod
#define mod_get_ul           modul_get_ul
#define mod_cmp              modul_cmp
#define mod_is0              modul_is0
#define mod_add              modul_add
#define mod_add_ul           modul_add_ul
#define mod_sub              modul_sub
#define mod_sub_ul           modul_sub_ul
#define mod_neg              modul_neg
#define mod_mul              modul_mul
#define mod_tomontgomery     modul_tomontgomery
#define mod_frommontgomery   modul_frommontgomery
#define mod_addredc_ul       modul_addredc_ul
#define mod_mulredc          modul_mulredc
#define mod_mulredc_ul       modul_mulredc_ul
#define mod_muladdredc_ul    modul_muladdredc_ul
#define mod_div2             modul_div2
#define mod_div3             modul_div3
#define mod_invmodlong       modul_invmodlong
#define mod_powredc_ul       modul_powredc_ul
#define mod_gcd              modul_gcd
#define mod_inv              modul_inv
#define mod_jacobi           modul_jacobi

typedef unsigned long residueul[1];
typedef unsigned long modulusul[1];
typedef residueul residue;
typedef modulusul modulus;

/* Initialises a residue type and sets it to zero */
__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_init (residueul r, modulusul m __GNUC_ATTRIBUTE_UNUSED__)
{
  r[0] = 0UL;
  return;
}


/* Initialises a residue type, but does not set it to zero. For fixed length
   residue types, that leaves nothing to do at all. */
__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_init_noset0 (residueul r __GNUC_ATTRIBUTE_UNUSED__, 
                   modulusul m __GNUC_ATTRIBUTE_UNUSED__)
{
  return;
}


__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_clear (residueul r __GNUC_ATTRIBUTE_UNUSED__, 
             modulusul m __GNUC_ATTRIBUTE_UNUSED__)
{
  return;
}


__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_set (residueul r, residueul s, modulusul m __GNUC_ATTRIBUTE_UNUSED__)
{
  r[0] = s[0];
}


__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_set_ul (residueul r, unsigned long s, modulusul m)
{
  r[0] = s % m[0];
}

/* Sets the residue to the class represented by the integer s. Assumes that
   s is reduced (mod m), i.e. 0 <= s < m */

__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_set_ul_reduced (residueul r, unsigned long s, 
                      modulusul m __GNUC_ATTRIBUTE_UNUSED__)
{
  ASSERT (s < m[0]);
  r[0] = s;
}

__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_initmod_ul (modulusul r, unsigned long s)
{
  r[0] = s;
}

__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_clearmod (modulusul m __GNUC_ATTRIBUTE_UNUSED__)
{
  return;
}

__GNUC_ATTRIBUTE_UNUSED__
static inline unsigned long
modul_get_ul (residueul s, modulusul m __GNUC_ATTRIBUTE_UNUSED__)
{
  return s[0];
}

__GNUC_ATTRIBUTE_UNUSED__
static inline int
modul_cmp (residueul a, residueul b, modulusul m __GNUC_ATTRIBUTE_UNUSED__)
{
  return (a[0] < b[0]) ? -1 : ((a[0] == b[0]) ? 0 : 1);
}

__GNUC_ATTRIBUTE_UNUSED__
static inline int
modul_is0 (residueul a, modulusul m __GNUC_ATTRIBUTE_UNUSED__)
{
  return (a[0] == 0UL);
}

__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_add (residueul r, residueul a, residueul b, modulusul m)
{
  ASSERT(a[0] < m[0] && b[0] < m[0]);
#ifdef MODTRACE
  printf ("modul_add: a = %lu, b = %lu", a[0], b[0]);
#endif
  r[0] = (a[0] >= m[0] - b[0]) ? (a[0] - (m[0] - b[0])) : (a[0] + b[0]);
#ifdef MODTRACE
  printf (", r = %lu\n", r[0]);
#endif
}

__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_add_ul (residueul r, residueul a, unsigned long b, modulusul m)
{
  ASSERT(a[0] < m[0] && b < m[0]);
#ifdef MODTRACE
  printf ("modul_add_ul: a = %lu, b = %lu", a[0], b);
#endif
  r[0] = (a[0] >= m[0] - b) ? (a[0] - (m[0] - b)) : (a[0] + b);
#ifdef MODTRACE
  printf (", r = %lu\n", r[0]);
#endif
}

__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_sub_ul (residueul r, residueul a, unsigned long b, modulusul m)
{
  ASSERT(a[0] < m[0] && b < m[0]);
#ifdef MODTRACE
  printf ("modul_sub_ul: a = %lu, b = %lu", a[0], b);
#endif
  r[0] = (a[0] >= b) ? (a[0] - b) : (a[0] + (m[0] - b));
#ifdef MODTRACE
  printf (", r = %lu\n", r[0]);
#endif
}

__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_neg (residueul r, residueul a, modulusul m)
{
  if (a[0] == 0UL)
    r[0] = a[0];
  else
    r[0] = m[0] - a[0];
}

__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_sub (residueul r, residueul a, residueul b, modulusul m)
{
  ASSERT(a[0] < m[0] && b[0] < m[0]);
#ifdef MODTRACE
  printf ("submod_ul: a = %lu, b = %lu", a[0], b[0]);
#endif
  r[0] = (a[0] < b[0]) ? (a[0] + (m[0] - b[0])) : (a[0] - b[0]);
#ifdef MODTRACE
  printf (", r = %lu\n", r[0]);
#endif
}


/* Add an unsigned long to two unsigned longs with carry propagation from 
   low word to high word. Any carry out from high word is lost. */

static inline void
modul_add_ul_2ul (unsigned long *r1, unsigned long *r2, const unsigned long a)
{
#if defined(__x86_64__) && defined(__GNUC__)
  __asm__ ( "addq %2, %0\n\t"
            "adcq $0, %1\n"
            : "+&r" (*r1), "+r" (*r2)
            : "rm" (a)
            : "cc");
#elif defined(__i386__) && defined(__GNUC__)
  __asm__ ( "addl %2, %0\n\t"
            "adcl $0, %1\n"
            : "+&r" (*r1), "+r" (*r2)
            : "rm" (a)
            : "cc");
#else
  abort ();
#endif
}

/* Add two unsigned longs to two unsigned longs with carry propagation from 
   low word to high word. Any carry out from high word is lost. */

static inline void
modul_add_2ul_2ul (unsigned long *r1, unsigned long *r2, 
                   const unsigned long a1, const unsigned long a2)
{
#if defined(__x86_64__) && defined(__GNUC__)
  __asm__ ( "addq %2, %0\n\t"
            "adcq %3, %1\n"
            : "+&r" (*r1), "+r" (*r2)
            : "rm" (a1), "rm" (a2)
            : "cc");
#elif defined(__i386__) && defined(__GNUC__)
  __asm__ ( "addl %2, %0\n\t"
            "adcl %3, %1\n"
            : "+&r" (*r1), "+r" (*r2)
            : "rm" (a1), "rm" (a2)
            : "cc");
#else
  abort ();
#endif
}

static inline void
mul_ul_ul_2ul (unsigned long *r1, unsigned long *r2, const unsigned long a,
               const unsigned long b)
{
#if defined(__x86_64__) && defined(__GNUC__)
  __asm__ ( "mulq %3"
	    : "=a" (*r1), "=d" (*r2)
	    : "%0" (a), "rm" (b)
	    : "cc");
#elif defined(__i386__) && defined(__GNUC__)
  __asm__ ( "mull %3"
	    : "=a" (*r1), "=d" (*r2)
	    : "%0" (a), "rm" (b)
	    : "cc");
#else
  abort ();
#endif
}

/* Integer division of a two ulong value by a ulong divisor. Returns
   quotient and remainder. */

static inline void
div_2ul_ul_ul (unsigned long *q, unsigned long *r, const unsigned long a1,
               const unsigned long a2, const unsigned long b)
{
#if defined(__x86_64__) && defined(__GNUC__)
  __asm__ ( "divq %4"
            : "=a" (*q), "=d" (*r)
            : "0" (a1), "1" (a2), "rm" (b)
            : "cc");
#elif defined(__i386__) && defined(__GNUC__)
  __asm__ ( "divl %4"
            : "=a" (*q), "=d" (*r)
            : "0" (a1), "1" (a2), "rm" (b)
            : "cc");
#else
  abort ();
#endif
}

/* Integer division of a two longint value by a longint divisor. Returns
   only remainder. */

static inline void
div_2ul_ul_ul_r (unsigned long *r, unsigned long a1,
                 const unsigned long a2, const unsigned long b)
{
#if defined(__x86_64__) && defined(__GNUC__)
  __asm__ ( "divq %3"
            : "+a" (a1), "=d" (*r)
            : "1" (a2), "rm" (b)
            : "cc");
#elif defined(__i386__) && defined(__GNUC__)
  __asm__ ( "divl %3"
            : "+a" (a1), "=d" (*r)
            : "1" (a2), "rm" (b)
            : "cc");
#else
  abort ();
#endif
}

__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_mul (residueul r, const residueul a, const residueul b, 
           const modulusul m)
{
  unsigned long t1, t2, dummy;
#if defined(MODTRACE)
  printf ("mulmod_ul: a = %lu, b = %lu", a[0], b[0]);
#endif
  
  mul_ul_ul_2ul (&t1, &t2, a[0], b[0]);
  div_2ul_ul_ul (&dummy, r, t1, t2, m[0]);
  
#if defined(MODTRACE)
  printf (", r = %lu\n", _r);
#endif
}

__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_tomontgomery (residueul r, const residueul a, const modulusul m)
{
  div_2ul_ul_ul_r (r, 0UL, a[0], m[0]);
}


/* Computes (a / 2^wordsize) % m */
__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_frommontgomery (residueul r, const residueul a, const unsigned long invm,
                      const modulusul m)
{
  unsigned long tlow, thigh;
  mul_ul_ul_2ul (&tlow, &thigh, a[0], invm);
  mul_ul_ul_2ul (&tlow, &thigh, tlow, m[0]);
  r[0] = thigh + (a[0] != 0UL);
}


/* Computes (a / 2^wordsize) % m, but result can be r = m. 
   Input a must not be equal 0 */
__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_redcsemi_ul_not0 (residueul r, const unsigned long a, 
                        const unsigned long invm, const modulusul m)
{
  unsigned long tlow, thigh;

  ASSERT (a != 0);

  tlow = a * invm;
  mul_ul_ul_2ul (&tlow, &thigh, tlow, m[0]);
  r[0] = thigh + 1UL;
}


/* Computes ((a + b) / 2^wordsize) % m */
__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_addredc_ul (residueul r, const residueul a, const unsigned long b, 
                  const unsigned long invm, const modulusul m)
{
  unsigned long slow, shigh, tlow, thigh;
  
  slow = b;
  shigh = 0UL;
  modul_add_ul_2ul (&slow, &shigh, a[0]);
  
  tlow = slow * invm;
  mul_ul_ul_2ul (&tlow, &thigh, tlow, m[0]);
  ASSERT (slow + tlow == 0UL);
  r[0] = thigh + shigh + (slow != 0UL);
  
  /* r = ((a+b) + (((a+b)%2^w * invp) % 2^w) * p) / 2^w
     r <= ((a+b) + (2^w - 1) * p) / 2^w
     r <= (p-1 + 2^w-1 + p2^w - p) / 2^w
     r <= (2^w -2 + p2^w) / 2^w
     r <= p + 1 - 2/2^w
     r <= p
  */
  if (r[0] == m[0])
    r[0] = 0UL;
}


/* Computes ((a + b) / 2^wordsize) % m, but result can be == m */
__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_addredcsemi_ul (residueul r, const residueul a, 
                      const unsigned long b, const unsigned long invm, 
                      const modulusul m)
{
  unsigned long slow, shigh, tlow;
  unsigned char sb;
  
  slow = b;
#if 1
  __asm__ ( "addq %2, %0\n\t"
            "setne %1\n\t"
            "adcb $0, %1\n"
            : "+&r" (slow), "=qm" (sb)
            : "rm" (a[0])
            : "cc");
  shigh = sb;
#else
  shigh = 0UL;
  modul_add_ul_2ul (&slow, &shigh, a[0]);
  shigh += (slow != 0UL);
#endif

  tlow = slow * invm;
  mul_ul_ul_2ul (&tlow, r, tlow, m[0]);
  ASSERT (slow + tlow == 0UL);
  r[0] += shigh;

  /* r = ((a+b) + (((a+b)%2^w * invp) % 2^w) * p) / 2^w
     r <= ((a+b) + (2^w - 1) * p) / 2^w
     r <= (p-1 + 2^w-1 + p2^w - p) / 2^w
     r <= (2^w -2 + p2^w) / 2^w
     r <= p + 1 - 2/2^w
     r <= p
  */
}


__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_mulredc (residueul r, const residueul a, const residueul b,
               const unsigned long invm, const modulusul m)
{
  unsigned long plow, phigh, tlow, thigh;

  ASSERT(m[0] % 2 != 0);
#if defined(MODTRACE)
  printf ("(%lu * %lu / 2^%ld) %% %lu", 
          a[0], b[0], 8 * sizeof(unsigned long), m[0]);
#endif
  
  mul_ul_ul_2ul (&plow, &phigh, a[0], b[0]);
  tlow = plow * invm;
  mul_ul_ul_2ul (&tlow, &thigh, tlow, m[0]);
  /* Let b = 2^wordsize. We know (phigh * b + plow) + (thigh * b + tlow) 
     == 0 (mod b) so either plow == tlow == 0, or plow !=0 and tlow != 0. 
     In the former case we want phigh + thigh + 1, in the latter 
     phigh + thigh */
  /* Since a <= p-1 and b <= p-1, and p <= b-1, a*b <= b^2 - 4*b + 4, so
     adding 1 to phigh is safe */
  phigh += (plow != 0UL);
  r[0] = (phigh >= m[0] - thigh) ? (phigh - (m[0] - thigh)) : (phigh + thigh);
  
#if defined(MODTRACE)
  printf (" == %lu /* PARI */ \n", r[0]);
#endif
}
                         
__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_mulredc_ul (residueul r, const residueul a, const unsigned long b,
                  const unsigned long invm, const modulusul m)
{
  unsigned long plow, phigh, tlow, thigh;
  ASSERT(m[0] % 2 != 0);
#if defined(MODTRACE)
  printf ("(%lu * %lu / 2^%ld) %% %lu", 
          a[0], b, 8 * sizeof(unsigned long), m[0]);
#endif
  
  mul_ul_ul_2ul (&plow, &phigh, a[0], b);
  tlow = plow * invm;
  mul_ul_ul_2ul (&tlow, &thigh, tlow, m[0]);
  phigh += (plow != 0UL);
  r[0] = (phigh >= m[0] - thigh) ? (phigh - (m[0] - thigh)) : (phigh + thigh);
  
#if defined(MODTRACE)
  printf (" == %lu /* PARI */ \n", r[0]);
#endif
}
                         

/* Computes (a * b + c)/ 2^wordsize % m. Requires that 
   a * b + c < 2^wordsize * m */

__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_muladdredc_ul (residueul r, const residueul a, const unsigned long b,
                     const unsigned long c, const unsigned long invm, 
                     const modulusul m)
{
  unsigned long plow, phigh, tlow, thigh;
  ASSERT(m[0] % 2 != 0);
#if defined(MODTRACE)
  printf ("(%lu * %lu / 2^%ld) %% %lu", 
          a[0], b, 8 * sizeof(unsigned long), m[0]);
#endif
  
  mul_ul_ul_2ul (&plow, &phigh, a[0], b);
  modul_add_ul_2ul (&plow, &phigh, c);
  tlow = plow * invm;
  mul_ul_ul_2ul (&tlow, &thigh, tlow, m[0]);
  phigh += (plow != 0UL);
  r[0] = (phigh >= m[0] - thigh) ? (phigh - (m[0] - thigh)) : (phigh + thigh);
  
#if defined(MODTRACE)
  printf (" == %lu /* PARI */ \n", r[0]);
#endif
}
                         

__GNUC_ATTRIBUTE_UNUSED__
static inline void
modul_div2 (residueul r, residueul a, modulusul m)
{
  ASSERT(m[0] % 2 != 0);
  r[0] = (a[0] % 2 == 0) ? (a[0] / 2) : (a[0] / 2 + m[0] / 2 + 1UL);
}

__GNUC_ATTRIBUTE_UNUSED__
static void
modul_div3 (residueul r, residueul a, modulusul m)
{
  const unsigned long a3 = a[0] % 3;
  const unsigned long m3 = m[0] % 3;
#ifdef WANT_ASSERT
  residueul t;
#endif

  ASSERT(m3 != 0UL);
  if (a3 == 0UL)
    r[0] = a[0] / 3UL;
  else if (a3 + m3 == 3UL) /* Hence a3 == 1, m3 == 2 or a3 == 3, m3 == 1 */
    r[0] = a[0] / 3UL + m[0] / 3UL + 1UL;
  else /* a3 == 1, m3 == 1 or a3 == 2, m3 == 2 */
    r[0] = m[0] - (m[0] - a[0]) / 3UL;

#ifdef WANT_ASSERT
  modul_add (t, r, r, m);
  modul_add (t, t, r, m);
  assert (t[0] == a[0]);
#endif
}

__GNUC_ATTRIBUTE_UNUSED__
static unsigned long
modul_gcd (residueul r, modulusul m)
{
  unsigned long a = r[0], b = m[0], t;

  ASSERT (b > 0UL);
  
  if (a >= b)
    a %= b;

  while (a > 0UL)
    {
      /* Here 0 < a < b */
      t = b % a;
      b = a;
      a = t;
    }

  return b;
}


/* Compute 1/t (mod 2^wordsize) */
/* FIXME: not optimised yet */
__GNUC_ATTRIBUTE_UNUSED__
static unsigned long
modul_invmodlong (modulusul m)
{
  unsigned long p, r; 
  int i;

  ASSERT (m[0] % 2 != 0);
  
  r = 1UL; p = m[0];
  for (i = 1; p != 1UL; i++) /* Invariant: r * t == p (mod 2^wordsize) */
    if ((1UL << i) & p)
      {
        r += 1UL << i;
        p += m[0] << i;
      }

  ASSERT (r * m[0] == 1UL);
  return r;
}

/* Compute r = b^e, with r and a in Montgomery representation */
__GNUC_ATTRIBUTE_UNUSED__
static void
modul_powredc_ul (residueul r, residueul b, unsigned long e, 
                  unsigned long invm, modulusul m)
{
  unsigned long mask = ~0UL - (~0UL >> 1); /* Only MSB set */

  if (e == 0UL)
    {
      r[0] = 1UL;
      return;
    }

  /* Assume r = 1 here for the invariant.
     r^mask * b^e is invariant, and is the result we want */

  /* Find highest set bit in e. */
  while ((e & mask) == 0UL)
    mask >>= 1; /* r = 1, so r^(mask/2) * b^e = r^mask * b^e  */

  /* Exponentiate */

  modul_set (r, b, m);       /* e -= mask; */
#ifdef WANT_ASSERT
  e -= mask;                 /* (r*b)^mask * b^(e-mask) = r^mask * b^e */
#endif

  while (mask > 1UL)
    {
      modul_mulredc (r, r, r, invm, m);
      mask >>= 1;            /* (r^2)^(mask/2) * b^e = r^mask * b^e */
      if (e & mask)
        {
	  modul_mulredc (r, r, b, invm, m);
#ifdef WANT_ASSERT
          e -= mask;         /* e -= mask; As above */
#endif
        }
    }
  ASSERT (e == 0 && mask == 1);
  /* Now e = 0, mask = 1, and r^mask * b^0 = r^mask is the result we want */
}


/* Put 1/s (mod t) in r and return 1 if s is invertible, 
   or return 0 if s is not invertible */

__GNUC_ATTRIBUTE_UNUSED__
static int
modul_inv (residueul r, residueul s, modulusul t)
{
  long u1, v1;
  unsigned long q, u2, v2;

  ASSERT (t[0] > 0UL);
  ASSERT (s[0] < t[0]);

  if (s[0] == 0UL)
    {
      r[0] = 0UL; /* Not invertible */
      return 0;
    }

  if (s[0] == 1UL)
    {
      r[0] = 1UL;
      return 1;
    }

#if 0
  u1 = 1L;
  v1 = 0L;
  u2 = s;
  v2 = t;

  q = s / t; /* == 0, due to s < t */
  u1 = u1 - (long) q * v1; /* == u1 - 0 == u1 == 1*/
  u2 = u2 - q * v2; /* == u2 - 0 == u2 == s */

  q = v2 / u2; /* == t / s <= t / 2*/
  v1 = v1 - (long) q * u1; /* == v1 - q * 1 == 0 - q == -q */
  v2 = v2 - q * u2; /* == v2 - q * s == t - q * s == t % s */
#endif

  u1 = 1L;
  u2 = s[0];
  v1 = - (t[0] / s[0]); /* No overflow, since s >= 2 */
  v2 = t[0] % s[0];

  while (v2 != 0UL)
    {
      /* unroll twice and swap u/v */
      q = u2 / v2;
      u1 = u1 - (long) q * v1;
      u2 = u2 - q * v2;

      if (u2 == 0UL)
        {
          u1 = v1;
          u2 = v2;
          break;
        }

      q = v2 / u2;
      v1 = v1 - (long) q * u1;
      v2 = v2 - q * u2;
    }

  if (u2 != 1UL)
    {
      /* printf ("s=%lu t=%lu found %lu\n", s[0], t[0], u2); */
      r[0] = 0UL; /* non-trivial gcd */
      return 0;
    }

  if (u1 < 0L)
    u1 = u1 - t[0] * (-u1 / t[0] - 1L);

#ifdef WANT_ASSERT
  modul_mul (&u2, (unsigned long *)&u1, s, t);
  ASSERT(u2 == 1UL);
#endif

  r[0] = (unsigned long) u1;
  return 1;
}

__GNUC_ATTRIBUTE_UNUSED__ static int
modul_jacobi (residueul a_par, modulusul m_par)
{
  unsigned long a, m, s;
  int t = 1;

  ASSERT (a_par[0] < m_par[0]);
  a = a_par[0];
  m = m_par[0];
  
  while (a != 0UL)
  {
    while (a % 2 == 0)
    {
      a /= 2UL;
      if (m % 8 == 3 || m % 8 == 5)
        t = -t;
    }
    s = a; a = m; m = s; /* swap */
    if (a % 4 == 3 && m % 4 == 3)
      t = -t;
    a %= m;
  }
  if (m != 1UL)
    t = 0;
  
#ifdef MODTRACE
  printf ("kronecker(%lu, %lu) == %d\n", a_par[0], m_par[0], t);
#endif
  return t;
}

/* Try to restore the macro defines to the same state they had been in */
#ifdef MOD_UL_ASSERT
#undef ASSERT
#undef MOD_UL_ASSERT
#endif
#ifdef MOD_UL_GNUC_ATTRIBUTE_UNUSED
#undef __GNUC_ATTRIBUTE_UNUSED__
#undef MOD_UL_GNUC_ATTRIBUTE_UNUSED
#endif

#endif
