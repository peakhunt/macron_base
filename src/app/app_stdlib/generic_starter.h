#ifndef __GENERIC_STARTER_DEF_H__
#define __GENERIC_STARTER_DEF_H__

#include "control_lib.h"

typedef enum
{
  generic_starter_state_stopped,
  generic_starter_state_running,
  generic_starter_state_starting,
  generic_starter_state_stopping,
} generic_starter_state_t;

typedef struct
{
  // input signal
  bool    in_running;               // running input signal
  bool    in_stopped;               // stopped input signal
  bool    in_starting;              // starting input signal
  bool    in_stopping;              // stopping input signal
  bool    in_start_req;             // start request
  bool    in_stop_req;              // stop request
  bool    in_start_IL;              // start interlock
  bool    in_stop_IL;               // stop interlock

  // output signal
  bool    out_start;                // physical start output
  bool    out_stop;                 // physical stop output
  bool    out_failed_to_start;      // failed to start
  bool    out_failed_to_stop;       // failed to stop
  bool    out_is_starting;          // starting output
  bool    out_is_stopping;          // stopping output
  bool    out_is_running;           // running output
  bool    out_is_stopped;           // stopped output
  bool    out_ux_start;             // unexpected start
  bool    out_ux_stop;              // unexpected stop

  // local status : don't touch!
  bool    status_stopg;
  bool    status_strtg;
  bool    status_stopd;
  bool    status_rung;
  bool    status_unx_strt;
  bool    status_unx_stop;
  bool    status_ft_strt;
  bool    status_ft_stop;

  generic_starter_state_t   state;

  TMR_T   ftr_tmr;
  TMR_T   alert_tmr;

  // parameters
  unsigned long     FTR_TMR;  // failed to respond timer in ms
} generic_starter_t;

extern void generic_starter_init(generic_starter_t* gs);
extern void generic_starter_run(generic_starter_t* gs);

#endif /* !__GENERIC_STARTER_DEF_H__ */
