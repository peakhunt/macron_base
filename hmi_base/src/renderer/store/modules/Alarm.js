import Vue from 'vue'

var currentAlarmHash = {
}

const state = {
  alarms: {},
  sortedList: [],
  current_alarms_list: []
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
  SET_ALARM_VALUE (state, alarmStatus) {
    var alarm = state.alarms[alarmStatus.alarm_num]

    if (alarm !== undefined) {
      alarm.state = alarmStatus.state
      alarm.time = alarmStatus.occur_time

      if (alarm.state === 'inactive') {
        if (currentAlarmHash[alarm.alarm_num] !== undefined) {
          var n = state.current_alarms_list.indexOf(alarm)

          delete currentAlarmHash[alarm.alarm_num]
          state.current_alarms_list.splice(n, 1)
        }
      } else {
        if (currentAlarmHash[alarm.alarm_num] === undefined) {
          currentAlarmHash[alarm.alarm_num] = alarm
          Vue.set(state.current_alarms_list, state.current_alarms_list.length, alarm)
        }
      }
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
  },
  currentAlarms: (state) => {
    return state.current_alarms_list
  }
}

export default {
  state,
  mutations,
  getters
}
