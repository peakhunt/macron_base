<!--
 WARNING:

 this component should be always used with inline-template attribute

 for example,

 <alarm-data v-bind:alarm-num="blah blah" inline-template>
  <td> {{ alarm_state }} </td>
 </alarm-data>
-->
<template>
</template>

<script>
  import serverAPI from '@/server_api'
  import dateFormat from 'dateformat'

  export default {
    name: 'alarmStatusView',
    computed: {
      alarm_state () {
        return this.alarmObj.state
      },
      alarm_state_string () {
        return this.alarmObj.state
      },
      alarm_number () {
        return this.alarmObj.alarm_num
      },
      alarm_severity () {
        return this.alarmObj.severity
      },
      alarm_name () {
        return this.alarmObj.name
      },
      alarm_time () {
        // return this.alarmObj.time
        var d = new Date(this.alarmObj.time * 1000)
        return dateFormat(d, 'yyyy-mm-dd HH:MM:ss')
      },
      alarm_state_class () {
        var state
        switch (this.alarm_state) {
          case 'inactive':
            state = ''
            break

          case 'pending':
          case 'inactive_pending':
            if (this.blinkingState === true) {
              state = 'blinking-on'
            } else {
              state = 'blinking-off'
            }
            break

          case 'active':
            state = 'ack'
            break
        }
        return this.alarm_severity + '-' + state
      }
    },
    props: ['alarmObj', 'blinkingState'],
    data () {
      return {
        alarm_state_names: ['Inactive', 'Active Pending', 'Inactive Pending', 'Active']
      }
    },
    methods: {
      alarm_ack () {
        console.log('alarm ack:' + this.alarm_number)
        serverAPI.ackAlarm(this.alarm_number, (err, data) => {
          if (err) {
            console.log('alarm ack error')
            return
          }
          console.log('alarm ack complete')
        })
      }
    }
  }
</script>

<style scoped>
.set-height td {
  position: relative;
  overflow: hidden;
  height: 2.5em;
}

.critical-blinking-off {
  background-color: red;
  color: black;
}

.critical-blinking-on {
  background-color: white;
  color: black;
}

.critical-ack {
  background-color: red;
  color: black;
}

.major-blinking-off {
  background-color: orange;
}

.major-blinking-on {
  background-color: white;
}

.major-ack {
  background-color: orange;
}

.minor-blinking-off {
  background-color: yellow;
}

.minor-blinking-on {
  background-color: white;
}

.minor-ack {
  background-color: yellow;
}
</style>
