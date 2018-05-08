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
          <v-card-title primary-title><h2>Alarm Info</h2></v-card-title>
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
              <tr>
                <td width="250px" class="text-xs-left">Set Point</td>
                <td class="text-xs-left">{{set_point}}</td>
              </tr>
              <tr>
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
        </v-card>
      </v-flex>
    </v-layout>
  </v-container>
</template>

<script>
  import router from '@/router'
  import dateFormat from 'dateformat'

  export default {
    name: 'AlarmView',
    methods: {
      go_back: function () {
        router.push('/alarm-list')
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
      trigger_type () {
        return this.$store.getters.alarm(this.alarmNum).trigger_type
      },
      severity () {
        return this.$store.getters.alarm(this.alarmNum).severity
      },
      set_point () {
        return this.$store.getters.alarm(this.alarmNum).set_point
      },
      delay () {
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
    data () {
      return {
        items: [''],
        headers: [
          { align: 'left' },
          { align: 'left' }
        ],
        alarmNum: this.$route.params.alarmNum
      }
    }
  }
</script>

<style scoped>
</style>
