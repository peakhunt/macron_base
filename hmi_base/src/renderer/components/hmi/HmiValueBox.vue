<template>
  <span>
    <button v-if="hmi_channel_type === 'analog'" v-bind:style="{
      width: options.width + 'px',
      height: options.height + 'px',
      'background-color': backgroundColor,
      font: options.font,
      color: textColor
    }" class="value-box-html">{{hmi_value}}&nbsp;{{options.units}}</button>
    <button v-if="hmi_channel_type === 'digital'" v-bind:style="{
      width: options.width + 'px',
      height: options.height + 'px',
      'background-color': backgroundColor,
      font: options.font,
      color: textColor
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
      hmi_channel_type () {
        return this.$store.getters.channel(this.chnl).chnl_type
      }
    },
    created () {
      this.updateAlarmColor()
    },
    beforeDestroy () {
    },
    methods: {
      updateAlarmColor () {
        var c = this.getAlarmColor()

        this.backgroundColor = c.backgroundColor
        this.textColor = c.textColor
      }
    },
    data () {
      return {
        value: 0,
        backgroundColor: null,
        textColor: null
      }
    },
    watch: {
      highest_alarm_num (newS, oldS) {
        this.updateAlarmColor()
      },
      tickValue (newV, oldV) {
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
