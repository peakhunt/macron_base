const state = {
  alarms: {}
}

const mutations = {
  ADD_ALARM (state, alarm) {
    var copy

    copy = Object.assign({}, alarm)
    copy.state = 'inactive'
    copy.time = 0

    state.alarms[copy.alarm_num] = copy
  },
  CLEAR_ALARMS (state) {
    state.alarms = {}
  },
  SET_ALARM_VALUE (state, alarmNum, alarmState, time) {
    var alarm = state.alarms[alarmNum]

    if (alarm !== undefined) {
      alarm.state = alarmState
      alarm.time = time
    }
  }
}

const getters = {
  alarm: (state) => (alarmNum) => {
    return state.alarms[alarmNum]
  }
}

export default {
  state,
  mutations,
  getters
}
