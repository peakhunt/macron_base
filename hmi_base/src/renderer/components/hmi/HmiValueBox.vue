<template>
  <span>
    <button v-if="hmi_channel_type === 'analog'" v-bind:style="{
      width: hmi_options.width + 'px',
      height: hmi_options.height + 'px',
      'background-color': hmi_options.backgroundColor,
      font: hmi_options.font,
      color: hmi_options.textColor
    }" class="value-box-html">{{hmi_value}}&nbsp;{{hmi_options.units}}</button>
    <button v-if="hmi_channel_type === 'digital'" v-bind:style="{
      width: hmi_options.width + 'px',
      height: hmi_options.height + 'px',
      'background-color': hmi_options.backgroundColor,
      font: hmi_options.font,
      color: hmi_options.textColor
    }" class="value-box-html">{{hmi_value}}</button>
  </span>
</template>

<script>
  import HmiCommon from '@/components/hmi/HmiCommon'

  export default {
    name: 'HmiValueBox',
    mixins: [HmiCommon],
    props: {
      chnl: { type: Number },
      alarm: { type: Number },
      options: { type: Object, default: {} },
      onMsg: { type: String, default: null },
      offMsg: { type: String, default: null }
    },
    computed: {
      hmi_value () {
        if (this.$store.getters.channel(this.chnl).chnl_type === 'analog') {
          // FIXME
          var v = Math.random() * (100 - (-100)) + (-100) + this.tickValue
          return v.toFixed(1)
        } else {
          v = this.$store.getters.channel(this.chnl).eng_val

          return v === true ? this.onMsg : this.offMsg
        }
      },
      hmi_alarm_state () {
        return {
          state: this.$store.getters.alarm(this.alarm).state,
          tick: this.tickValue
        }
      },
      hmi_channel_type () {
        return this.$store.getters.channel(this.chnl).chnl_type
      }
    },
    created () {
      this.hmi_options = Object.assign(this.options, {})
      this.updateAlarmColor()
    },
    beforeDestroy () {
    },
    methods: {
      updateAlarmColor () {
        var state = this.$store.getters.alarm(this.alarm).state
        var c = this.getAlarmColor(state)

        this.hmi_options.backgroundColor = c.backgroundColor
        this.hmi_options.textColor = c.textColor
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
    data () {
      return {
        value: 0,
        hmi_options: {}
      }
    },
    watch: {
      hmi_alarm_state (newO, oldO) {
        this.updateAlarmColor()
      }
    }
  }
</script>

<style scoped>
.value-box-html {
  border-radius: 5px;
  border: 3px solid grey;
  text-align: center;
  outline: none;
  box-shadow: 0 12px 16px 0 rgba(0,0,0,0.24), 0 17px 50px 0 rgba(0,0,0,0.19);
  pointer-events: none;
}
</style>
