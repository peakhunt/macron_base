<template>
  <div id="app">
    <v-app light>
      <v-navigation-drawer
        fixed
        :mini-variant="miniVariant"
        :clipped="clipped"
        v-model="drawer"
        app
      >
        <v-list>
          <v-list-tile 
            router
            :to="item.to"
            :key="i"
            v-for="(item, i) in items"
            exact
          >
            <v-list-tile-action>
              <v-icon v-html="item.icon"></v-icon>
            </v-list-tile-action>
            <v-list-tile-content>
              <v-list-tile-title v-text="item.title"></v-list-tile-title>
            </v-list-tile-content>
          </v-list-tile>

          <v-list-tile @click='exit()'>
            <v-list-tile-action>
              <v-icon>exit_to_app</v-icon>
            </v-list-tile-action>
            <v-list-tile-content>
              <v-list-tile-title>Exit</v-list-tile-title>
            </v-list-tile-content>
          </v-list-tile>

        </v-list>
      </v-navigation-drawer>
      <v-toolbar fixed app :clipped-left="clipped">
        <v-toolbar-side-icon @click.native.stop="drawer = !drawer"></v-toolbar-side-icon>
        <v-btn 
          icon
          @click.native.stop="miniVariant = !miniVariant"
        >
          <v-icon v-html="miniVariant ? 'chevron_right' : 'chevron_left'"></v-icon>
        </v-btn>
        <v-btn
          icon
          @click.native.stop="clipped = !clipped"
        >
          <v-icon>web</v-icon>
        </v-btn>
        <v-btn
          icon
          @click.native.stop="fixed = !fixed"
        >
          <v-icon>remove</v-icon>
        </v-btn>
        <v-toolbar-title v-text="title"></v-toolbar-title>
        <v-spacer></v-spacer>
      </v-toolbar>
      <v-content>
        <v-container fluid fill-height>
          <v-slide-y-transition mode="out-in">
            <router-view></router-view>
          </v-slide-y-transition>
          <notifications
           animation-type="velocity"
           position="bottom center"
           :speed="600"
           />
        </v-container>
      </v-content>
      <v-footer :fixed="fixed" app>
        <v-spacer></v-spacer>
        <span>&copy; Blah Blah Blah 2018</span>
      </v-footer>
    </v-app>
  </div>
</template>

<script>
  import router from '@/router'
  import global from '@/global'
  import utils from '@/utils'
  import {EventBus} from '@/event-bus'
  import serverPoller from '@/server_poller'

  export default {
    name: 'hmi_base',
    methods: {
      exit: function () {
        utils.quit()
      },
      systemConfigLoadComplete: function () {
        console.log('****** systemConfigLoadComplete ******')

        console.log('initializing server poller')
        serverPoller.initPoller(this.$store.getters.channelList, this.$store.getters.alarmList)
        serverPoller.start(this)
      }
    },
    created () {
      console.log('1:' + this.$route.path)
      EventBus.$on('systemConfigLoadComplete', this.systemConfigLoadComplete)

      if (!this.$store.getters.isSysConfigLoaded) {
        serverPoller.stop(this)
        router.push('/system-loading-view')
      } else {
        router.push('/')
      }
    },
    data: () => ({
      clipped: false,
      drawer: true,
      fixed: false,
      items: [
        { icon: 'apps', title: 'Welcome', to: '/' },
        { icon: 'bubble_chart', title: 'Inspire', to: '/inspire' },
        { icon: 'apps', title: 'Channel List', to: '/channel-list' },
        { icon: 'apps', title: 'Alarm List', to: '/alarm-list' },
        { icon: 'apps', title: 'Current Alarms', to: '/current-alarms' }
      ],
      miniVariant: false,
      right: true,
      title: global.config.general.projectName
    })
  }
</script>

<style>
  /* Global CSS */
</style>
