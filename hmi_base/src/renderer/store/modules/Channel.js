import Vue from 'vue'

const state = {
  channels: {},
  sortedList: []
}

const mutations = {
  ADD_CHANNEL (state, chnl) {
    var copy

    copy = Object.assign({}, chnl)
    copy.raw_val = 0
    copy.eng_val = 0
    copy.sensor_fault = false

    if (chnl.lookup_table !== undefined) {
      copy.lookup_table = []
      for (var i = 0; i < chnl.lookup_table.length; i++) {
        copy.lookup_table.push(Object.assign({}, chnl.lookup_table[i]))
      }
    }

    var chnlNum = copy.chnl_num

    // state.channels[chnlNum] = copy
    Vue.set(state.channels, chnlNum, copy)
  },
  CLEAR_CHANNELS (state) {
    state.channels = {}
    state.sortedList = []
  },
  SET_CHANNEL_VALUE (state, chnlStatus) {
    var chnl = state.channels[chnlStatus.chnl_num]

    if (chnl !== undefined) {
      chnl.raw_val = chnlStatus.raw_val
      chnl.eng_val = chnlStatus.eng_val
    } else {
      console.log('vuex no channel:' + chnlStatus.chnl_num)
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
