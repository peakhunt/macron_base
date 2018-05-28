const state = {
  isLoaded: false,
  revision: -1
}

const mutations = {
  SET_SYSTEM_CONFIG_LOADED (state) {
    state.isLoaded = true
  },
  SET_SYSTEM_CONFIG_NOT_LOADED (state) {
    state.isLoaded = false
  },
  SET_CONFIG_REVISION (state, rev) {
    state.revision = rev
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
  },
  configRevision (state) {
    return state.revision
  }
}

export default {
  state,
  mutations,
  actions,
  getters
}
