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
    for (var alarmNum in state.alarms) {
      state.sortedList[state.sortedList.length] = alarmNum
    }
    state.sortedList.sort()
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
