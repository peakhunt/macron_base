const state = {
  isLoaded: false
}

const mutations = {
  SET_SYSTEM_CONFIG_LOADED (state) {
    state.isLoaded = true
  },
  SET_SYSTEM_CONFIG_NOT_LOADED (state) {
    state.isLoaded = false
  }
}

const actions = {
  setSysConfigLoadedAsyncTask ({ commit }) {
    commit('SET_SYSTEM_CONFIG_LOADED')
  }
}

const getters = {
  isSysConfigLoaded (state) {
    return state.isLoaded
  }
}

export default {
  state,
  mutations,
  actions,
  getters
}
