<template>
  <linear-gauge :value="hmi_value" :options="options"></linear-gauge>
</template>

<script>
  import HmiCommon from '@/components/hmi/HmiCommon'
  import LinearGauge from 'vue2-canvas-gauges/src/LinearGauge'

  export default {
    name: 'HmiLinearGauge',
    mixins: [HmiCommon],
    components: {
      LinearGauge
    },
    methods: {
      updateAlarmColor () {
        var c = this.getAlarmColor()

        var obj = {
          colorValueText: c.textColor,
          colorValueBoxBackground: c.backgroundColor
        }
        this.$children[0].chart.update(obj)
      },
      getRandom (min, max) {
        return Math.random() * (max - min) + min
      }
    },
    computed: {
      channel () {
        return this.$store.getters.channel(this.chnl)
      },
      hmi_value () {
        var v = this.channel.eng_val

        if (typeof v !== 'number') {
          v = v ? 1 : 0
        }
        v = 22.2 + 100 * this.tickValue
        v = this.getRandom(0, 320)
        return v
      }
    },
    props: {
      chnl: { type: Number }
    },
    mounted () {
      this.updateAlarmColor()
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
</style>
