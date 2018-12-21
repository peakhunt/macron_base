#ifndef __VALVE_CONTROL_DEF_H__
#define __VALVE_CONTROL_DEF_H__

#include "control_lib.h"

typedef enum
{
  valve_control_state_closed,
  valve_control_state_opened,
  valve_control_state_closing,
  valve_control_state_opening,
  valve_control_state_indeterminate,
} valve_control_state_t;

typedef struct
{
  bool*             in_opened;
  bool*             in_closed;
  bool*             in_opening;
  bool*             in_closing;
  bool*             in_open_req;
  bool*             in_close_req;
  bool*             in_open_IL;
  bool*             in_close_IL;

  bool*             out_open_op;
  bool*             out_close_op;
  bool*             out_fail_to_open;
  bool*             out_fail_to_close;
  bool*             out_is_closing;
  bool*             out_is_opening;
  bool*             out_is_closed;
  bool*             out_is_opened;
  bool*             out_ux_opened;
  bool*             out_ux_closed;
  bool*             out_fault;

  // local statue : don't touch!
  bool              status_opened;
  bool              status_closed;
  bool              status_opening;
  bool              status_closing;
  bool              status_unx_open;
  bool              status_unx_close;
  bool              status_indeterminate;
  bool              status_fail_to_close;
  bool              status_fail_to_open;

  valve_control_state_t   state;

  TMR_T             ftr_tmr;
  TMR_T             alert_tmr;

  // parameters
  unsigned long     FTR_TMR;  // failed to respond timer in ms
} valve_control_t;

extern void valve_control_init(valve_control_t* v,
    bool* in_opened,
    bool* in_closed,
    bool* in_opening,
    bool* in_closing,
    bool* in_open_req,
    bool* in_close_req,
    bool* in_open_IL,
    bool* in_close_IL,
    
    bool* out_open_op,
    bool* out_close_op,
    bool* out_fail_to_open,
    bool* out_fail_to_close,
    bool* out_is_closing,
    bool* out_is_opening,
    bool* out_is_closed,
    bool* out_is_opened,
    bool* out_ux_opened,
    bool* out_ux_closed,
    bool* out_fault,

    unsigned long FTR_TMR);

extern void valve_control_run(valve_control_t* v);

#endif /* !__VALVE_CONTROL_DEF_H__ */
