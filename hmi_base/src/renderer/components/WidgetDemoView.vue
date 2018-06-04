<template>
  <v-container>
    <v-layout row wrap>
      <v-flex xs12>
        <span>HMI linear gauges</span>
        <hmi-linear-gauge :chnl="11" :alarms="[4]" :options="hmi_gauge1_option" ></hmi-linear-gauge>
        <hmi-linear-gauge :chnl="3" :alarms="[2]"  :options="hmi_gauge2_option" ></hmi-linear-gauge>
        <hmi-linear-gauge :chnl="11" :alarms="[3]" :options="hmi_gauge1_option" ></hmi-linear-gauge>
        <hmi-linear-gauge :chnl="11" :alarms="[7]" :options="hmi_gauge1_option" ></hmi-linear-gauge>
      </v-flex>


      <v-flex xs12>
        <span>HMI radial gauges</span>
        <hmi-radial-gauge :chnl="11" :alarms="[4]" :options="hmi_gauge3_option" ></hmi-radial-gauge>
        <hmi-radial-gauge :chnl="11" :alarms="[2]" :options="hmi_gauge3_option" ></hmi-radial-gauge>
        <hmi-radial-gauge :chnl="11" :alarms="[3]" :options="hmi_gauge3_option" ></hmi-radial-gauge>
        <hmi-radial-gauge :chnl="11" :alarms="[7]" :options="hmi_gauge3_option" ></hmi-radial-gauge>
      </v-flex>

      <v-flex xs12>
        <span>HMI value boxes</span>
        <hmi-value-box :chnl="11" :alarms="[1,7,4,2]" :options="hmi_value_box_options" ></hmi-value-box>
        <hmi-value-box :chnl="11" :alarms="[7]" :options="hmi_value_box_options" ></hmi-value-box>
        <hmi-value-box :chnl="11" :alarms="[3]" :options="hmi_value_box_options" ></hmi-value-box>
        <hmi-value-box :chnl="1" :alarms="[7]" :onMsg="'Closed'" :offMsg="'Open'" :options="hmi_value_box_options"></hmi-value-box>
      </v-flex>

      <v-flex xs12>
        <span>HMI control buttons</span>
        <hmi-button :outChnl="20" :progChnl="2" :alarms="[1]" :interlock="1" :text="'Start'" :options="hmi_button_options1">
        </hmi-button>
        <hmi-button :outChnl="11" :progChnl="1" :alarms="[4]" :interlock="1" :text="'Stop'" :options="hmi_button_options1">
        </hmi-button>
        <hmi-button :outChnl="11" :progChnl="2" :alarms="[1]" :interlock="2" :text="'Open'" :options="hmi_button_options1">
        </hmi-button>
        <hmi-button :outChnl="11" :progChnl="1" :alarms="[1]" :interlock="1" :text="'Close'" :options="hmi_button_options1">
        </hmi-button>
      </v-flex>

      <v-flex xs12>
        <hmi-led :color="'Green'" :width="50" :height="50" :ledOn="tickValue"></hmi-led>
        <hmi-led :color="'Green'" :width="50" :height="50" :ledOn="true"></hmi-led>
        <hmi-led :color="'Green'" :width="50" :height="50" :ledOn="false"></hmi-led>

        <hmi-led :color="'Red'" :width="50" :height="50" :ledOn="tickValue"></hmi-led>
        <hmi-led :color="'Red'" :width="50" :height="50" :ledOn="true"></hmi-led>
        <hmi-led :color="'Red'" :width="50" :height="50" :ledOn="false"></hmi-led>

        <hmi-led :color="'Blue'" :width="50" :height="50" :ledOn="tickValue"></hmi-led>
        <hmi-led :color="'Blue'" :width="50" :height="50" :ledOn="true"></hmi-led>
        <hmi-led :color="'Blue'" :width="50" :height="50" :ledOn="false"></hmi-led>
      </v-flex>

      <v-flex xs12>
        <hmi-pump :width="250" :height="250" :color="'red'"></hmi-pump>
      </v-flex>

      <v-flex xs12>
        <hmi-tank :min="0" :max="350" :value="tankLevel" :width="'100px'" :height="'100px'"></hmi-tank>
      </v-flex>

    </v-layout>
  </v-container>
</template>

<script>
  import LinearGauge from 'vue2-canvas-gauges/src/LinearGauge'
  import RadialGauge from 'vue2-canvas-gauges/src/RadialGauge'
  import HmiLinearGauge from '@/components/hmi/HmiLinearGauge'
  import HmiRadialGauge from '@/components/hmi/HmiRadialGauge'
  import HmiValueBox from '@/components/hmi/HmiValueBox'
  import HmiButton from '@/components/hmi/HmiButton'
  import HmiLed from '@/components/hmi/HmiLed'
  import HmiPump from '@/components/hmi/HmiPump'
  import HmiTank from '@/components/hmi/HmiTank'
  import {EventBus} from '@/event-bus'

  export default {
    components: {
      LinearGauge,
      RadialGauge,
      HmiLinearGauge,
      HmiRadialGauge,
      HmiValueBox,
      HmiButton,
      HmiLed,
      HmiPump,
      HmiTank
    },
    computed: {
      hmi_button_options1 () {
        return {
          // alarm
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
      hmi_button_options2 () {
        return {
          // alarm
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
      hmi_value_box_options () {
        return {
          units: '°C',
          width: 100,
          height: 35,
          font: 'normal 20px Arial',
          // for alarms
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
        tankLevel: 0,
        dir: true
      }
    },
    methods: {
      toggleTick () {
        this.tickValue = !this.tickValue

        if (this.dir) {
          this.tankLevel += 5
          if (this.tankLevel > 350) {
            this.dir = false
            this.tankLevel = 350
          }
        } else {
          this.tankLevel -= 5
          if (this.tankLevel < 0) {
            this.dir = true
            this.tankLevel = 0
          }
        }
      }
    },
    created () {
      EventBus.$on('global500msTick', this.toggleTick)
    },
    beforeDestroy () {
      EventBus.$off('global500msTick', this.toggleTick)
    }
  }
</script>

<style scoped>
</style>
