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
            :disabled="disableMenu"
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
      <v-toolbar style="z-index: 200;" fixed app :clipped-left="clipped">
        <v-toolbar-side-icon @click.native.stop="drawer = !drawer"></v-toolbar-side-icon>

        <v-btn icon @click.native.stop="miniVariant = !miniVariant">
          <v-icon v-html="miniVariant ? 'chevron_right' : 'chevron_left'"></v-icon>
        </v-btn>

        <v-toolbar-title v-text="title"></v-toolbar-title>
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
        // remove from event listener
        EventBus.$off('systemConfigLoadComplete', this.systemConfigLoadComplete)
        console.log('****** systemConfigLoadComplete ******')

        console.log('initializing server poller')
        serverPoller.initPoller(this.$store.getters.channelList, this.$store.getters.alarmList)
        serverPoller.start(this)
      },
      systemConfigReloadComplete: function () {
        EventBus.$off('systemConfigLoadComplete', this.systemConfigReloadComplete)
        console.log('****** systemConfigReloadComplete ******')
        serverPoller.initPoller(this.$store.getters.channelList, this.$store.getters.alarmList)
        serverPoller.start(this)

        router.push(this.savedPath)
        this.savedPath = null
      },
      configUpdateDetected: function () {
        // this is a message from poller
        console.log('configUpdateDetected...')

        // first stop poller
        console.log('stopping poller...')
        serverPoller.stop(this)

        // reload configuration
        this.savedPath = this.$route.path
        EventBus.$on('systemConfigLoadComplete', this.systemConfigReloadComplete)

        router.push('/system-loading-view')
      },
      systemRestarting: function () {
        this.restarting = true
        serverPoller.stop(this)
      },
      gotoCommOptions: function () {
        router.push('/options')
      }
    },
    computed: {
      disableMenu: function () {
        if (this.$store.getters.isSysConfigLoaded === false ||
            this.restarting === true) {
          return true
        }
        return false
      }
    },
    created () {
      console.log('1:' + this.$route.path)
      EventBus.$on('systemConfigLoadComplete', this.systemConfigLoadComplete)
      EventBus.$on('systemRestarting', this.systemRestarting)
      EventBus.$on('gotoCommOptions', this.gotoCommOptions)

      this.tickTimer = setInterval(() => {
        global.tick500ms = !global.tick500ms
        EventBus.$emit('global500msTick')
      }, 500)

      if (!this.$store.getters.isSysConfigLoaded) {
        serverPoller.stop(this)
        router.push('/system-loading-view')
      } else {
        router.push('/')
      }
    },
    beforeDestroy () {
      if (this.tickTimer !== null) {
        clearInterval(this.tickTimer)
        this.tickTimer = null
      }
    },
    data: () => ({
      clipped: true,
      drawer: true,
      fixed: false,
      restarting: false,
      items: [
        { icon: 'apps', title: 'Welcome', to: '/' },
        { icon: 'bubble_chart', title: 'Inspire', to: '/inspire' },
        { icon: 'apps', title: 'Channel List', to: '/channel-list' },
        { icon: 'apps', title: 'Alarm List', to: '/alarm-list' },
        { icon: 'apps', title: 'Current Alarms', to: '/current-alarms' },
        { icon: 'apps', title: 'Signal Trace', to: '/signal-trace' },
        { icon: 'apps', title: 'Alarm Log', to: '/alarm-log' },
        { icon: 'apps', title: 'Widget Demo', to: '/widget-demo' },
        { icon: 'apps', title: 'Mimic Demo', to: '/mimic-demo' },
        { icon: 'apps', title: 'SVG Icon Demo', to: '/svgicon-test' },
        { icon: 'apps', title: 'Options', to: '/options' }
      ],
      miniVariant: false,
      right: true,
      title: global.config.general.projectName,
      tickTimer: null,
      savedPath: null
    })
  }
</script>

<style>
  /* Global CSS */
</style>
