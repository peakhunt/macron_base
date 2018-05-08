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
        return this.alarmObj.time
      },
      alarm_state_class () {
        var state
        switch (this.alarm_state) {
          case 'inactive':
            state = ''
            break

          case 'pending':
          case 'inactive_pending':
            state = 'blinking'
            break

          case 'active':
            state = 'ack'
        }
        return this.alarm_severity + '-' + state
      }
    },
    props: ['alarmObj'],
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

@keyframes blinking-critical {
  0% red--text {
    background-color: red;
  }
  50% {
    background-color: white;
  }
  100% {
    background-color: white;
  }
}

@keyframes blinking-major {
  0% {
    background-color: orange;
  }
  50% {
    background-color: white;
  }
  100% {
    background-color: white;
  }
}

@keyframes blinking-minor {
  0% {
    background-color: yellow;
  }
  50% {
    background-color: white;
  }
  100% {
    background-color: white;
  }
}

.critical-blinking {
  background-color: red;
  color: black;
  animation-name: blinking-critical;
  animation-iteration-count: infinite;
  animation-fill-mode: alternate;
  animation-duration: 1s;
  animation-timing-function: steps(1);
}

.critical-ack {
  background-color: red;
  color: black;
}

.major-blinking {
  background-color: orange;
  animation-name: blinking-major;
  animation-iteration-count: infinite;
  animation-fill-mode: alternate;
  animation-duration: 1s;
  animation-timing-function: steps(1);
}

.major-ack {
  background-color: orange;
  animation-name: blinking-major;
}

.minor-blinking {
  background-color: yellow;
  animation-name: blinking-minor;
  animation-iteration-count: infinite;
  animation-fill-mode: alternate;
  animation-duration: 1s;
  animation-timing-function: steps(1);
}

.minor-ack {
  background-color: yellow;
}
</style>
