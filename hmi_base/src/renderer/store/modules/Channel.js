const state = {
  channels: {},
  sortedList: []
}

const mutations = {
  ADD_CHANNEL (state, chnl) {
    var copy

    copy = Object.assign({}, chnl)
    copy.raw_value = 0
    copy.eng_value = 0
    copy.sensor_fault = false

    if (chnl.lookup_table !== undefined) {
      copy.lookup_table = []
      for (var i = 0; i < chnl.lookup_table.length; i++) {
        copy.lookup_table.push(Object.assign({}, chnl.lookup_table[i]))
      }
    }
    state.channels[copy.chnl_num] = copy
  },
  CLEAR_CHANNELS (state) {
    state.channels = {}
  },
  SET_CHNNEL_VALUE (state, chnlNum, eng, raw, fault) {
    var chnl = state.channels[chnlNum]

    if (chnl !== undefined) {
      chnl.raw_value = raw
      chnl.eng_value = eng
      chnl.sensor_fault = fault
    }
  },
  BUILD_CHANNEL_LIST (state) {
    state.sortedList = []
    Object.keys(state.channels).forEach(function (chnlNum) {
      var chnl = state.channels[chnlNum]

      // this is much faster than push
      state.sortedList[state.sortedList.length] = chnl.chnl_num
    })
    state.sortedList.sort((a, b) => {
      return a - b
    })
  }
}

const getters = {
  channel: (state) => (chnlNum) => {
    return state.channels[chnlNum]
  },
  channelList: (state) => {
    return state.sortedList
  }
}

export default {
  state,
  mutations,
  getters
}
