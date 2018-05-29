<template>
  <linear-gauge :value="hmi_value" :options="hmi_options"></linear-gauge>
</template>

<script>
  import LinearGauge from 'vue2-canvas-gauges/src/LinearGauge'

  export default {
    name: 'HmiLinearGauge',
    components: {
      LinearGauge
    },
    methods: {
      getRandom (min, max) {
        return Math.random() * (max - min) + min
      },
      getSeverityColor (severity) {
        var obj = {
          colorValueText: this.options.textNormal,
          colorValueBoxBackground: this.options.backgroundNormal
        }

        switch (severity) {
          case 'minor':
            obj = {
              colorValueText: this.options.textMinor,
              colorValueBoxBackground: this.options.backgroundMinor
            }
            break

          case 'major':
            obj = {
              colorValueText: this.options.textMajor,
              colorValueBoxBackground: this.options.backgroundMajor
            }
            break

          case 'critical':
            obj = {
              colorValueText: this.options.textCritical,
              colorValueBoxBackground: this.options.backgroundCritical
            }
        }
        return obj
      },
      getAlarmColor (alarmState) {
        var severity = this.$store.getters.alarm(this.alarm).severity

        switch (alarmState) {
          case 'inactive':
            return this.getSeverityColor('normal')

          case 'pending':
          case 'inactive_pending':
            if (this.tickValue === true) {
              return this.getSeverityColor(severity)
            } else {
              return this.getSeverityColor('normal')
            }

          case 'active':
            return this.getSeverityColor(severity)
        }
        return this.getSeverityColor('normal')
      }
    },
    computed: {
      channel () {
        return this.$store.getters.channel(this.chnl).eng_val
      },
      hmi_value () {
        var v = this.channel.eng_val

        if (typeof v !== 'number') {
          v = v ? 1 : 0
        }
        v = 22.2 + 100 * this.tickValue
        v = this.getRandom(0, 320)
        return v
      },
      hmi_alarm_state () {
        return {
          state: this.$store.getters.alarm(this.alarm).state,
          tick: this.tickValue
        }
      },
      hmi_options () {
        var state = this.$store.getters.alarm(this.alarm).state
        return Object.assign(this.options, this.getAlarmColor(state))
      }
    },
    props: {
      chnl: { type: Number },
      alarm: { type: Number },
      options: { type: Object, default: {} },
      tickValue: { type: Boolean, default: false }
    },
    watch: {
      hmi_alarm_state (newO, oldO) {
        this.$children[0].chart.update(this.getAlarmColor(newO.state))
      }
    }
  }
</script>

<style scoped>
</style>
