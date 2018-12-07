#ifndef __CONTROL_LIB_DEF_H__
#define __CONTROL_LIB_DEF_H__

#include "common_def.h"

/*
          ________
   Set----|      |
   Reset--|  SR  |--- Q
          ________

  Set   Reset   Q   | Result Q
  ============================
  0     0       0     0
  0     0       1     1
  0     1       0     0
  0     1       1     0
  1     0       0     1
  1     0       1     1
  1     1       0     1
  1     1       1     1

*/

typedef struct
{
  bool    q;
} sr_latch_t;

extern void sr_latch_init(sr_latch_t* sr, bool q);
extern void sr_latch(sr_latch_t* sr, bool set, bool reset);
#define SR(__sr, __set, __reset, __q)\
  sr_latch(&__set, __set, __reset);\
  __q = __set.q;

/*
          ________
   Set----|      |
   Reset--|  RS  |--- Q
          ________

  Set   Reset   Q   | Result Q
  ============================
  0     0       0     0
  0     0       1     1
  0     1       0     0
  0     1       1     0
  1     0       0     1
  1     0       1     1
  1     1       0     0
  1     1       1     0

*/

typedef struct
{
  bool      q;
} rs_latch_t;

extern void rs_latch_init(rs_latch_t* rs, bool  q);
extern void rs_latch(rs_latch_t* rs, bool set, bool reset);
#define RS(__rs, __set, __reset, __q)\
  rs_latch(&__rs, __set, __reset);\
  __q = __rs.q;

/*
          ________
   CLK----|      |
          |r_trig|--- Q
          ________

    Q: TRUE when CLK rises from FALSE to TRUE
       FALSE if all other cases
*/ 

typedef struct
{
  bool    prev_clk;
  bool    q;
} r_trig_t;

extern void r_trig_init(r_trig_t* rt);
extern void r_trig(r_trig_t* rt, bool clk);
#define R_TRIG(__rt, __clk, __q)\
  r_trig(&__rt, __clk);\
  __q = __rt.q;

/*
          ________
   CLK----|      |
          |f_trig|--- Q
          ________

    Q: TRUE when CLK rises from TRUE to FALSE
       FALSE if all other cases
*/ 
typedef struct
{
  bool    prev_clk;
  bool    q;
} f_trig_t;

extern void f_trig_init(f_trig_t* ft);
extern void f_trig(f_trig_t* ft, bool clk);
#define F_TRIG(__ft, __clk, __q)\
  f_trig(&__ft, __clk);\
  __q = __ft.q;

/*
          ________
   IN-----|      |
          |  TON |--- Q
   PT-----________

   IN   : if rising edge, starts increasing internal timer
          if falling edge, stops and resets internal timer
   PT   : maximum programmed time
   Q    : TRUE if programmed time is elapsed
*/ 
typedef struct
{
  bool            q;
  bool            prev_in;
  bool            tmr_running;
  unsigned long   start_time;
} ton_t;

extern void ton_init(ton_t* ton);
extern void ton(ton_t* ton, bool in, uint32_t pt);
#define TON(__t, __in, __q, __pt)\
  ton(&__t, __in, __pt);\
  __q = __t.q;

/*
          ________
   IN-----|      |
          | TOFF |--- Q
   PT-----________

   IN   : if falling edge, starts increasing internal timer
          if rising edge, stops and resets internal timer
   PT   : maximum programmed time
   Q    : TRUE if programmed time is not elapsed
*/ 
typedef struct
{
  bool            q;
  bool            prev_in;
  bool            tmr_running;
  unsigned long   start_time;
} toff_t;

extern void toff_init(toff_t* toff);
extern void toff(toff_t* toff, bool in, uint32_t pt);
#define TOFF(__t, __in, __q, __pt)\
  toff(&__t, __in, __pt);\
  __q = __t.q;

#endif /* !__CONTROL_LIB_DEF_H__ */
