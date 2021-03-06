<template>
  <v-layout row justify-center>

    <v-dialog v-model="dialog" persistent max-width="800px">
      <v-card>
        <v-card-title class="justify-center">
          <span class="headline">Loading Project Configuration from Control Server</span>
        </v-card-title>
        <v-divider/>
        <v-card-text style="height:300px">
          <v-layout justify-center>
            <v-progress-circular indeterminate :size="200" :width="6" color="purple"></v-progress-circular>
          </v-layout>
          <v-layout justify-center>
            <span>{{status}}</span>
          </v-layout>
        </v-card-text>
      </v-card>
    </v-dialog>

    <v-dialog v-model="show_fail_dialog" persistent max-width="800px">
      <v-card>
        <v-card-title class="justify-center">
          <span class="headline">Failed to download system config</span>
        </v-card-title>
        <v-divider/>
        <v-card-text>
          <v-layout justify-center>
          Failed to download system config from {{server_ip}}:{{server_port}}
          </v-layout>
        </v-card-text>
        <v-card-actions>
          <v-spacer/>
          <v-btn color="red darken-1" flat @click.native="retry()">Retry</v-btn>
          <v-btn color="red darken-1" flat @click.native="comm_options()">Comm Options</v-btn>
          <v-btn color="red darken-1" flat @click.native="exit()">Exit</v-btn>
        </v-card-actions>
      </v-card>
    </v-dialog>
  </v-layout>
</template>

<script>
  import serverAPI from '@/server_api'
  import router from '@/router'
  import global from '@/global'
  import utils from '@/utils'
  import {EventBus} from '@/event-bus'

  export default {
    name: 'sysConfigLoadView',
    methods: {
      populateChannelStore: function (channels) {
        var self = this

        self.$store.commit('CLEAR_CHANNELS')

        channels.forEach((value, key, map) => {
          self.$store.commit('ADD_CHANNEL', value)
        })

        self.$store.commit('BUILD_CHANNEL_LIST')
      },
      populateAlarmStore: function (alarms) {
        var self = this

        self.$store.commit('CLEAR_ALARMS')

        alarms.forEach((value, key, map) => {
          self.$store.commit('ADD_ALARM', value)
        })

        self.$store.commit('BUILD_ALARM_LIST')
      },
      populateStore: function (config) {
        console.log('system config loaded: ' + config.update_info.revision)
        this.$store.commit('SET_CONFIG_REVISION', config.update_info.revision)
        this.populateChannelStore(config.channels)
        this.populateAlarmStore(config.alarms)
      },
      loadSysConfig: function () {
        var self = this

        this.$store.commit('SET_SYSTEM_CONFIG_NOT_LOADED')

        serverAPI.loadSystemConfig((err, data) => {
          if (err) {
            console.log('failed to retrieve system config')
            self.status = 'failed to retrieve system config'
            self.dialog = false
            self.show_fail_dialog = true
            return
          }

          console.log('downloaded system config data from server')
          console.log(data)

          self.status = 'got all system config. building system configuration'

          setTimeout(() => {
            self.populateStore(data.data)
            self.dialog = false
            router.push('/')
            this.$store.commit('SET_SYSTEM_CONFIG_LOADED')
            EventBus.$emit('systemConfigLoadComplete')
          }, 10)
        })
      },
      retry: function () {
        var self = this

        self.show_fail_dialog = false
        self.dialog = true
        self.loadSysConfig()
      },
      exit: function () {
        utils.quit()
      },
      comm_options: function () {
        EventBus.$emit('gotoCommOptions')
      }
    },
    created () {
      var self = this

      self.server_ip = global.config.server.ipAddress
      self.server_port = global.config.server.webPort

      setTimeout(() => { self.loadSysConfig() }, 1000)
    },
    data () {
      return {
        dialog: true,
        show_fail_dialog: false,
        server_ip: '',
        server_port: 0,
        status: 'connecting and retrieving system config'
      }
    }
  }
</script>

<style scoped>
</style>
