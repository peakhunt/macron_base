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
    for (var chnlNum in state.channels) {
      state.sortedList[state.sortedList.length] = chnlNum
    }
    state.sortedList.sort()
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
