import Vue from 'vue'
import axios from 'axios'
import Vuetify from 'vuetify'
import 'vuetify/dist/vuetify.css'

import App from './App'
import router from './router'
import store from './store'

import Notifications from 'vue-notification'
import velocity from 'velocity-animate'

import 'material-design-icons/iconfont/material-icons.css'

import VDragged from 'v-dragged'

Vue.use(Vuetify)
Vue.use(Notifications, { velocity })
Vue.use(VDragged)

if (!process.env.IS_WEB) Vue.use(require('vue-electron'))
Vue.http = Vue.prototype.$http = axios
Vue.config.productionTip = false

/* eslint-disable no-new */
new Vue({
  components: { App },
  router,
  store,
  template: '<App/>'
}).$mount('#app')
