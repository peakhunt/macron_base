<template>
</template>

<script>
  import HmiCommon from '@/components/hmi/HmiCommon'

  export default {
    name: 'HmiIconCommon2',
    mixins: [HmiCommon],
    props: {
      alarms: { type: Array, default: null },
      options: { type: Object, default: {} },
      runningChnl: { type: Number },
      startingChnl: { type: Number },
      stoppingChnl: { type: Number }
    },
    computed: {
      isRunning () {
        return this.$store.getters.channel(this.runningChnl).eng_val
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
            return this.options.onColor
          }
          return this.options.offColor
        }

        if (this.isRunning === true) {
          return this.options.onColor
        }
        return this.options.offColor
      },
      alarmColor () {
        var highestActiveAlarm

        highestActiveAlarm = this.highest_alarm_num

        if (highestActiveAlarm === -1) {
          return 'none'
        }

        var state = this.$store.getters.alarm(highestActiveAlarm).state
        var severity = this.$store.getters.alarm(highestActiveAlarm).severity

        switch (state) {
          case 'active':
            return this.getSeverityColor(severity).backgroundColor

          case 'pending':
          case 'inactive_pending':
            if (this.tickValue === true) {
              return this.getSeverityColor(severity).backgroundColor
            }
            return 'none'
        }

        // bug if we reach here
        return 'none'
      }
    },
    methods: {
    }
  }
</script>

<style scoped>
</style>
