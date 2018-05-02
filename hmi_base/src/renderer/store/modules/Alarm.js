const state = {
  alarms: {},
  sortedList: []
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
  },
  BUILD_ALARM_LIST (state) {
    state.sortedList = []
    Object.keys(state.alarms).forEach(function (alarmNum) {
      var alarm = state.alarms[alarmNum]

      // this is much faster than push
      state.sortedList[state.sortedList.length] = alarm.alarm_num
    })
    state.sortedList.sort((a, b) => {
      return a - b
    })
  }
}

const getters = {
  alarm: (state) => (alarmNum) => {
    return state.alarms[alarmNum]
  },
  alarmList: (state) => {
    return state.sortedList
  }
}

export default {
  state,
  mutations,
  getters
}
