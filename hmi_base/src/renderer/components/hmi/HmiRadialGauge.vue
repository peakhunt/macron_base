<template>
  <radial-gauge :value="hmi_value" :options="options"></radial-gauge>
</template>

<script>
  import HmiCommon from '@/components/hmi/HmiCommon'
  import RadialGauge from 'vue2-canvas-gauges/src/RadialGauge'

  export default {
    name: 'HmiRadialGauge',
    mixins: [HmiCommon],
    components: {
      RadialGauge
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
    mounted () {
      this.updateAlarmColor()
    },
    props: {
      chnl: { type: Number }
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
