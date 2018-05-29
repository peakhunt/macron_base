<template>
  <v-container>
    <v-layout row wrap>
      <v-flex xs12>
        <hmi-linear-gauge :chnl="11" :alarm="4" :options="hmi_gauge1_option" :tickValue="tickValue"></hmi-linear-gauge>
        <hmi-linear-gauge :chnl="3" :alarm="1"  :options="hmi_gauge2_option"  :tickValue="tickValue"></hmi-linear-gauge>
        <hmi-linear-gauge :chnl="11" :alarm="3" :options="hmi_gauge1_option" :tickValue="tickValue"></hmi-linear-gauge>
        <hmi-linear-gauge :chnl="11" :alarm="7" :options="hmi_gauge1_option" :tickValue="tickValue"></hmi-linear-gauge>
      </v-flex>
      <v-flex xs12>
        <hmi-radial-gauge :chnl="11" :alarm="4"  :options="hmi_gauge3_option"  :tickValue="tickValue"></hmi-radial-gauge>
        <hmi-radial-gauge :chnl="11" :alarm="1"  :options="hmi_gauge3_option"  :tickValue="tickValue"></hmi-radial-gauge>
        <hmi-radial-gauge :chnl="11" :alarm="3"  :options="hmi_gauge3_option"  :tickValue="tickValue"></hmi-radial-gauge>
        <hmi-radial-gauge :chnl="11" :alarm="7"  :options="hmi_gauge3_option"  :tickValue="tickValue"></hmi-radial-gauge>
      </v-flex>
    </v-layout>
  </v-container>
</template>

<script>
  import LinearGauge from 'vue2-canvas-gauges/src/LinearGauge'
  import RadialGauge from 'vue2-canvas-gauges/src/RadialGauge'
  import HmiLinearGauge from '@/components/hmi/HmiLinearGauge'
  import HmiRadialGauge from '@/components/hmi/HmiRadialGauge'

  export default {
    components: {
      LinearGauge,
      RadialGauge,
      HmiLinearGauge,
      HmiRadialGauge
    },
    computed: {
      hmi_default_gauge_option1 () {
        return {
          units: '°C',
          title: 'title',
          minValue: 0,
          maxValue: 320,
          width: 130,
          height: 360,
          strokeTicks: true,
          colorBar: 'white',
          colorBarProgress: 'blue',
          highlights: [
            {
              'from': 0,
              'to': 100,
              'color': '#00E676'
            },
            {
              'from': 100,
              'to': 220,
              'color': '#FFFF8D'
            }, {
              'from': 220,
              'to': 320,
              'color': '#FF3D00'
            }
          ],
          minorTicks: 10,
          majorTicks: [
            '0',
            '40',
            '80',
            '120',
            '160',
            '200',
            '240',
            '280',
            '320'
          ],
          colorPlate: '#fff',
          borderShadowWidth: 0,
          borders: false,
          needleType: 'arrow',
          needleShadow: true,
          needleWidth: 0,
          needleCircleSize: 7,
          needleCircleOuter: true,
          needleCircleInner: false,
          numberSide: 'left',
          needleSide: 'left',
          animationDuration: 25,
          animationRule: 'linear',
          barWidth: 15,
          valueBox: true,
          valueBoxStroke: 5,
          valueTextShadow: true,
          valueDec: 1,
          barBeginCircle: 0,
          fontNumbersSize: 28,
          fontUnitsSize: 30,
          fontValueSize: 30,
          colorValueText: '#ffffff',
          colorValueBoxBackground: '#000000',
          // hkim extension
          textNormal: '#ffffff',
          backgroundNormal: '#000000',
          textMinor: '#000000',
          backgroundMinor: '#ffff00',
          textMajor: '#ffffff',
          backgroundMajor: '#ffa500',
          textCritical: '#ffffff',
          backgroundCritical: '#ff0000'
        }
      },
      hmi_gauge1_option () {
        var def = this.hmi_default_gauge_option1
        var ret = Object.assign({}, def)

        return Object.assign(ret, {title: 'a demo 1', units: 'mBar'})
      },
      hmi_gauge2_option () {
        var def = this.hmi_default_gauge_option1
        var ret = Object.assign({}, def)

        return Object.assign(ret, {title: 'a demo 2', units: '°C'})
      },
      hmi_gauge3_option () {
        var def = this.hmi_default_gauge_option1
        var ret = Object.assign({}, def)

        return Object.assign(ret, {title: 'Inlet 1',
          units: '°C',
          width: 250,
          minValue: -200,
          maxValue: 400,
          highlights: [
            { 'from': -200, 'to': 0, 'color': '#42A5F5' },
            { 'from': 0, 'to': 200, 'color': '#00E676' },
            { 'from': 200, 'to': 400, 'color': '#FF3D00' }
          ],
          minorTicks: 5,
          majorTicks: [
            '-200',
            '-150',
            '-100',
            '-50',
            '0',
            '50',
            '100',
            '150',
            '200',
            '250',
            '300',
            '350',
            '400'
          ],
          fontNumbersSize: 15
        })
      }
    },
    data () {
      return {
        tickValue: false,
        timerHandle: null
      }
    },
    methods: {
      feedTickEvent () {
        this.tickValue = !this.tickValue
      }
    },
    created () {
      this.timerHandle = setInterval(this.feedTickEvent, 500)
    },
    beforeDestroy () {
      clearInterval(this.timerHandle)
    }
  }
</script>

<style scoped>
</style>
