<template>
  <v-container fluid grid-list-lg mt-0>
    <v-layout row wrap>
      <v-flex xs12>
        <v-toolbar color="primary">
          <v-btn icon color="white" @click="go_back()">
            <v-icon>arrow_back</v-icon>
          </v-btn>
          <v-toolbar-title class="white--text">Alarm Detail - {{alarm_number}}</v-toolbar-title>
        </v-toolbar>
      </v-flex>

      <v-flex xs12>
        <v-card>
          <v-card-title primary-title>
            <h2>Alarm Info</h2>
            <v-btn v-if="alarmModified" color="red" slot="activator" @click="on_commit_channel_change" :loading="updating_alarm">
              Commit
              <v-icon>cloud_upload</v-icon>
            </v-btn>
          </v-card-title>
          <v-data-table
           :headers="headers"
           :items="items"
           hide-actions
           hide-headers>
            <template slot="items" slot-scope="props">
              <tr>
                <td width="250px" class="text-xs-left">Alarm Number</td>
                <td class="text-xs-left">{{alarm_number}}</td>
              </tr>
              <tr>
                <td width="250px" class="text-xs-left">Channel Name</td>
                <td class="text-xs-left">{{alarm_name}}</td>
              </tr>
              <tr>
                <td width="250px" class="text-xs-left">Channel Number</td>
                <td class="text-xs-left">{{channel_number}}</td>
              </tr>
              <tr>
                <td width="250px" class="text-xs-left">Trigger Type</td>
                <td class="text-xs-left">{{trigger_type}}</td>
              </tr>
              <tr>
                <td width="250px" class="text-xs-left">Severity</td>
                <td class="text-xs-left">{{severity}}</td>
              </tr>
              <tr @click="edit_alarm()">
                <td width="250px" class="text-xs-left">Set Point</td>
                <td class="text-xs-left">{{set_point}}</td>
              </tr>
              <tr @click="edit_alarm()">
                <td width="250px" class="text-xs-left">Delay</td>
                <td class="text-xs-left">{{delay}}</td>
              </tr>
              <tr>
                <td width="250px" class="text-xs-left">Occur Time</td>
                <td class="text-xs-left">{{occur_time}}</td>
              </tr>
              <tr>
                <td width="250px" class="text-xs-left">Current State</td>
                <td class="text-xs-left">{{current_state}}</td>
              </tr>
            </template>
          </v-data-table>

          <v-dialog v-model="showModDialog" persistent max-width="500px">
            <v-card>
              <v-card-title>
                <span class="headline">Modify Alarm Configuration</span>
              </v-card-title>
              <v-card-text>
                <v-container>
                  <v-switch v-if="channel_type === 'digital'" :label="`Set Point: ${modSetPoint ? 'on' : 'off'}`" v-model="modSetPoint"></v-switch>
                  <v-text-field v-if="channel_type === 'analog'" label="Set Point" v-model.number="modSetPoint" required></v-text-field>
                  <v-text-field label="Delay" v-model.number="modDelay" required></v-text-field>
                </v-container>
              </v-card-text>

              <v-card-actions>
                <v-spacer></v-spacer>
                <v-btn color="blue darken-1" flat @click.native="showModDialog = false">Dismiss</v-btn>
                <v-btn color="blue darken-1" flat @click="modify_alarm">Modify</v-btn>
              </v-card-actions>
            </v-card>
          </v-dialog>
          </v-dialog>

        </v-card>
      </v-flex>
    </v-layout>
  </v-container>
</template>

<script>
  import router from '@/router'
  import dateFormat from 'dateformat'
  import serverAPI from '@/server_api'

  export default {
    name: 'AlarmView',
    methods: {
      go_back: function () {
        router.push('/alarm-list')
      },
      edit_alarm: function () {
        if (this.alarmModified === false) {
          if (this.channel_type === 'analog') {
            this.modSetPoint = this.set_point
          } else {
            if (this.set_point === 'off') {
              this.modSetPoint = false
            } else {
              this.modSetPoint = true
            }
          }
          this.modDelay = this.delay
        }
        this.showModDialog = true
      },
      modify_alarm: function () {
        this.alarmModified = true
        this.showModDialog = false
      },
      on_commit_channel_change: function () {
        var req = {}
        var self = this

        req.set_point = self.modSetPoint
        req.delay = self.modDelay

        self.updating_alarm = true

        serverAPI.updateAlarmConfig(self.alarmNum, req, (err, data) => {
          if (self.destroyed) {
            console.log('instance destroyed. skipping ')
            return
          }

          setTimeout(() => {
            self.updating_alarm = false
            if (err) {
              console.log('failed to update alarm config for ' + self.alarmNum)
              console.log(err)
              self.$notify({
                title: 'Update Failed',
                text: 'Failed updating alarm configuration!',
                type: 'error'
              })
              return
            }

            self.alarmModified = false
            self.$notify({
              title: 'Update Success',
              text: 'Alarm was successfully udpated!',
              type: 'success'
            })
            console.log('alarm update success for ' + self.alarmNum)
          }, 1000)
        })
      }
    },
    computed: {
      alarm_number () {
        return this.$store.getters.alarm(this.alarmNum).alarm_num
      },
      alarm_name () {
        return this.$store.getters.alarm(this.alarmNum).name
      },
      channel_number () {
        return this.$store.getters.alarm(this.alarmNum).chnl_num
      },
      channel_type () {
        var chnlNum = this.$store.getters.alarm(this.alarmNum).chnl_num
        return this.$store.getters.channel(chnlNum).chnl_type
      },
      trigger_type () {
        return this.$store.getters.alarm(this.alarmNum).trigger_type
      },
      severity () {
        return this.$store.getters.alarm(this.alarmNum).severity
      },
      set_point () {
        if (this.alarmModified) {
          return this.modSetPoint === true ? 'on' : 'off'
        }
        return this.$store.getters.alarm(this.alarmNum).set_point
      },
      delay () {
        if (this.alarmModified) {
          return this.modDelay
        }
        return this.$store.getters.alarm(this.alarmNum).delay
      },
      occur_time () {
        var t = this.$store.getters.alarm(this.alarmNum).time

        if (t !== 0) {
          var d = new Date(t * 1000)

          return dateFormat(d, 'yyyy-mm-dd HH:MM:ss')
        }
        return '-'
      },
      current_state () {
        return this.$store.getters.alarm(this.alarmNum).state
      }
    },
    beforeDestroy () {
      this.destroyed = true
    },
    data () {
      return {
        destroyed: false,
        items: [''],
        headers: [
          { align: 'left' },
          { align: 'left' }
        ],
        alarmNum: this.$route.params.alarmNum,
        showModDialog: false,
        modSetPoint: false,
        modDelay: 0,
        alarmModified: false,
        updating_alarm: false
      }
    }
  }
</script>

<style scoped>
</style>
