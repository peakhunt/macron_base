<script>
  import {EventBus} from '@/event-bus'
  import utils from '@/utils'

  export default {
    name: 'HmiCommon',
    props: {
      alarms: { type: Array, default: null },
      options: { type: Object, default: {} }
    },
    created () {
      EventBus.$on('global500msTick', this.toggleTick)
    },
    beforeDestroy () {
      EventBus.$off('global500msTick', this.toggleTick)
    },
    computed: {
      highest_alarm_num () {
        if (this.alarms === null || this.alarms.length === 0) {
          return -1
        }

        var highestRank = -1
        var highestAlarm = -1

        this.alarms.forEach((alarmNum) => {
          if (this.isAlarmActive(alarmNum) === false) {
            return
          }

          var severity = this.$store.getters.alarm(alarmNum).severity
          var currentRank = utils.getAlarmRank(severity)

          if (currentRank > highestRank) {
            highestRank = currentRank
            highestAlarm = alarmNum
          }
        })
        return highestAlarm
      }
    },
    methods: {
      toggleTick () {
        this.tickValue = !this.tickValue
      },
      isAlarmActive (alarmNum) {
        var state = this.$store.getters.alarm(alarmNum).state

        if (state === 'active' ||
            state === 'pending' ||
            state === 'inactive_pending') {
          return true
        }
        return false
      },
      getSeverityColor (severity) {
        var obj = {
          textColor: this.options.textNormal,
          backgroundColor: this.options.backgroundNormal
        }

        switch (severity) {
          case 'minor':
            obj = {
              textColor: this.options.textMinor,
              backgroundColor: this.options.backgroundMinor
            }
            break

          case 'major':
            obj = {
              textColor: this.options.textMajor,
              backgroundColor: this.options.backgroundMajor
            }
            break

          case 'critical':
            obj = {
              textColor: this.options.textCritical,
              backgroundColor: this.options.backgroundCritical
            }
        }
        return obj
      },
      getAlarmColor () {
        var highestActiveAlarm

        highestActiveAlarm = this.highest_alarm_num

        if (highestActiveAlarm === -1) {
          // no alarms are active
          return this.getSeverityColor('normal')
        }

        var severity = this.$store.getters.alarm(highestActiveAlarm).severity
        var state = this.$store.getters.alarm(highestActiveAlarm).state

        switch (state) {
          case 'active':
            return this.getSeverityColor(severity)

          case 'pending':
          case 'inactive_pending':
            if (this.tickValue === true) {
              return this.getSeverityColor(severity)
            }
            return this.getSeverityColor('normal')
        }

        // bug if we reach here
        return this.getSeverityColor('normal')
      }
    },
    data () {
      return {
        tickValue: false
      }
    }
  }
</script>
