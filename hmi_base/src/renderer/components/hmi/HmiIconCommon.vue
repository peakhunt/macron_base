<template>
</template>

<script>
  import {EventBus} from '@/event-bus'
  import utils from '@/utils'

  export default {
    name: 'HmiIconCommon',
    props: {
      alarms: { type: Array, default: null },
      options: { type: Object, default: {} },
      runningChnl: { type: Number },
      startingChnl: { type: Number },
      stoppingChnl: { type: Number }
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
      },
      isRunning () {
        var v

        v = this.$store.getters.channel(this.runningChnl).eng_val
        console.log('======== ' + v + ' =========')

        return v
      },
      isStarting () {
        return this.$store.getters.channel(this.startingChnl).eng_val
      },
      isStopping () {
        return this.$store.getters.channel(this.stoppingChnl).eng_val
      },
      runningColor () {
        if (this.isStarting === true || this.isStopping === true) {
          if (this.tickValue === true) {
            return {
              backgroundColor: this.options.onColor
            }
          }
          return {
            backgroundColor: this.options.offColor
          }
        }

        if (this.isRunning === true) {
          return {
            backgroundColor: this.options.onColor
          }
        }
        return {
          backgroundColor: this.options.offColor
        }
      },
      severityColor () {
        var severity = this.$store.getters.alarm(this.highest_alarm_num).severity
        var obj = {
          backgroundColor: this.options.backgroundNormal
        }

        switch (severity) {
          case 'minor':
            obj = {
              backgroundColor: this.options.backgroundMinor
            }
            break

          case 'major':
            obj = {
              backgroundColor: this.options.backgroundMajor
            }
            break

          case 'critical':
            obj = {
              backgroundColor: this.options.backgroundCritical
            }
        }
        return obj
      },
      backgroundColor () {
        var highestActiveAlarm
        var c

        highestActiveAlarm = this.highest_alarm_num

        if (highestActiveAlarm === -1) {
          c = this.runningColor
          return c.backgroundColor
        }

        var state = this.$store.getters.alarm(highestActiveAlarm).state

        switch (state) {
          case 'active':
            c = this.severityColor
            break

          case 'pending':
          case 'inactive_pending':
            if (this.tickValue === true) {
              c = this.severityColor
            } else {
              c = this.runningColor
            }
            break

          default:
            c = {
              backgroundColor: this.options.backgroundNormal
            }
            break
        }
        return c.backgroundColor
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
      }
    },
    data () {
      return {
        tickValue: false
      }
    }
  }
</script>

<style scoped>
</style>
