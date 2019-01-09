<template>
  <v-container style="min-width: 1250px;">
    <v-layout row wrap>
      <v-flex xs12>
        <v-card>
          <v-card-title primary-title>
            <h2>Mimic Demo</h2>
          </v-card-title>
          <v-divider></v-divider>
          <div class="hmi_container">
            <!-- UI Element start -->
            <hmi-value-box style="position: absolute; z-index: 1; top: 130.3px; left: 140.9px;" :chnl="11" :alarms="[2]"
             :options="hmi_value_box_options"></hmi-value-box>

            <hmi-value-box style="position: absolute; z-index: 1; top: 170.3px; left: 140.9px;" :chnl="11" :alarms="[2]"
             :options="hmi_value_box_options2"></hmi-value-box>

            <hmi-radial-gauge style="position: absolute; z-index: 100; top: -20px; left: 390px;" :chnl="11" :alarms="[4]" :options="hmi_gauge3_option">
            </hmi-radial-gauge>

            <hmi-radial-gauge style="position: absolute; z-index: 100; top: 420px; left: 290px;" :chnl="11" :alarms="[4]" :options="hmi_gauge3_option">
            </hmi-radial-gauge>
            
            <hmi-button style="position: absolute; z-index: 100; top: 60px; left: 600px;" :outChnl="20" :progChnl="2" :alarms="[1]" :interlock="1" :text="'Test Button'" :options="hmi_button_options1">
            </hmi-button>

            <!-- SVG start -->
            <svg
               width="1200px"
               height="800px"
               viewBox="0 0 1052.3622 744.09451">
              <g>
                <hmi-pipe x="228" y="70" width="30" height="15" :pipeOn="pipeSegment1Status" />
                <hmi-pipe x="228" y="350" width="30" height="15" :pipeOn="pipeSegment1Status" />
                <hmi-pipe x="257" y="70" width="15" height="295" :pipeOn="pipeSegment1Status" />
                <hmi-pipe x="270" y="210" width="30" height="15" :pipeOn="pipeSegment1Status" />

                <hmi-pipe x="500" y="210" width="90" height="15" :pipeOn="pipeSegment2Status"/>
                <hmi-pipe x="530" y="210" width="15" height="90" :pipeOn="pipeSegment2Status"/>

                <hmi-pipe x="645" y="210" width="50" height="15" :pipeOn="pipeSegment3Status" />
                <hmi-pipe x="910" y="210" width="50" height="15" :pipeOn="pipeSegment3Status" />
                <hmi-pipe x="945" y="210" width="15" height="95" :pipeOn="pipeSegment3Status" />
                <hmi-pipe x="945" y="455" width="15" height="95" :pipeOn="pipeSegment3Status" />
                <hmi-pipe x="945" y="549" width="100" height="15" :pipeOn="pipeSegment3Status" />


                <hmi-pipe x="530" y="360" width="15" height="80" :pipeOn="pipeSegment4Status" />
                <hmi-pipe x="30" y="500" width="50" height="15" :pipeOn="pipeSegment4Status" />
                <hmi-pipe x="30" y="230" width="50" height="15" :pipeOn="pipeSegment4Status" />
                <hmi-pipe x="16" y="230" width="15" height="430" :pipeOn="pipeSegment4Status" />
                <hmi-pipe x="530" y="580" width="15" height="80" :pipeOn="pipeSegment4Status" />
                <hmi-pipe x="16" y="659" width="529" height="15" :pipeOn="pipeSegment4Status" />

                <hmi-valve :x="592" :y="189" :width="52.2" :height="34.8"
                           :options="hmi_icon_options"
                           :runningChnl="1"
                           :startingChnl="2"
                           :stoppingChnl="2"
                           :alarms="[23]"></hmi-valve>
                <hmi-valve :x="509.5" :y="304" :width="34.8" :height="52.2" :rotate="-90"
                           :options="hmi_icon_options"
                           :runningChnl="20"
                           :startingChnl="20"
                           :stoppingChnl="2"
                           :alarms="[23]"></hmi-valve>
                <hmi-boiler :x="682.5"
                            :y="100.0"
                            :width="240"
                            :height="190.0"
                            :options="hmi_icon_options"
                            :runningChnl="1"
                            :startingChnl="2"
                            :stoppingChnl="2"
                            :alarms="[20]">/</hmi-boiler>
                <hmi-compressor :x="283.8" :y="146" :width="231.1" :height="139.7"
                                :options="hmi_icon_options"
                                :runningChnl="1"
                                :startingChnl="20"
                                :stoppingChnl="20"
                                :alarms="[23]">/</hmi-compressor>
                <hmi-hccremover :x="883"
                                :y="300"
                                :width="155"
                                :height="159"
                                :options="hmi_icon_options"
                                :runningChnl="20"
                                :startingChnl="2"
                                :stoppingChnl="2"
                                :alarms="[1]"></hmi-hccremover>
                <hmi-reliquefier :x="466" :y="445" :width="141.3" :height="134"
                                 :options="hmi_icon_options"
                                 :runningChnl="2"
                                 :startingChnl="2"
                                 :stoppingChnl="2"
                                 :alarms="[20]"></hmi-reliquefier>
                <hmi-tank :x="78.9" :y="20.3" :width="150.2" :height="255.5" :min="0" :max="350" :value="tankLevel"></hmi-tank>
                <hmi-tank :x="78.9" :y="308.3" :width="150.2" :height="255.5" :min="0" :max="350" :value="tankLevel2"></hmi-tank>
              </g>
            </svg>
            <!-- SVG end -->
          </div>

        </v-card>

      </v-flex>
    </v-layout>
  </v-container>
</template>

<script>
  import HmiTank from '@/components/hmi/HmiTank'
  import HmiBoiler from '@/components/hmi/HmiBoiler'
  import HmiValve from '@/components/hmi/HmiValve'
  import HmiCompressor from '@/components/hmi/HmiCompressor'
  import HmiHccremover from '@/components/hmi/HmiHccremover'
  import HmiReliquefier from '@/components/hmi/HmiReliquefier'
  import HmiValueBox from '@/components/hmi/HmiValueBox'
  import HmiRadialGauge from '@/components/hmi/HmiRadialGauge'
  import HmiButton from '@/components/hmi/HmiButton'
  import HmiPipe from '@/components/hmi/HmiPipe'
  import {EventBus} from '@/event-bus'

  export default {
    components: {
      HmiTank,
      HmiBoiler,
      HmiValve,
      HmiCompressor,
      HmiHccremover,
      HmiReliquefier,
      HmiValueBox,
      HmiRadialGauge,
      HmiButton,
      HmiPipe
    },
    computed: {
      pipeSegment1Status () {
        return this.$store.getters.channel(1).eng_val
      },

      pipeSegment2Status () {
        return this.pipeSegment1Status
      },

      pipeSegment3Status () {
        return this.$store.getters.channel(1).eng_val
      },

      pipeSegment4Status () {
        return this.$store.getters.channel(20).eng_val
      },

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
      hmi_value_box_options () {
        return {
          units: '°C',
          width: 120,
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
      hmi_value_box_options2 () {
        return {
          units: 'Bar',
          width: 120,
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
          needleShadow: false,
          needleWidth: 0,
          needleCircleSize: 7,
          needleCircleOuter: true,
          needleCircleInner: false,
          numberSide: 'left',
          needleSide: 'left',
          animation: false,
          animationDuration: 25,
          animationRule: 'linear',
          barWidth: 15,
          valueBox: true,
          valueBoxStroke: 5,
          valueTextShadow: false,
          valueDec: 2,
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
      hmi_gauge3_option () {
        var def = this.hmi_default_gauge_option1
        var ret = Object.assign({}, def)

        return Object.assign(ret, {title: 'Inlet 1',
          units: '°C',
          width: 180,
          height: 180,
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
      },
      hmi_icon_options () {
        return {
          onColor: 'green',
          offColor: 'black',
          backgroundNormal: '#000000',
          backgroundMinor: '#ffff00',
          backgroundMajor: '#ffa500',
          backgroundCritical: '#ff0000'
        }
      }
    },
    data () {
      return {
        tankLevel: 175,
        tankLevel2: 100,
        dir: [true, true]
      }
    },
    methods: {
      toggleTick () {
        this.tickValue = !this.tickValue

        if (this.dir[0]) {
          this.tankLevel += 5
          if (this.tankLevel > 350) {
            this.dir[0] = false
            this.tankLevel = 350
          }
        } else {
          this.tankLevel -= 5
          if (this.tankLevel < 0) {
            this.dir[0] = true
            this.tankLevel = 0
          }
        }

        if (this.dir[1]) {
          this.tankLevel2 += 5
          if (this.tankLevel2 > 350) {
            this.dir[1] = false
            this.tankLevel2 = 350
          }
        } else {
          this.tankLevel2 -= 5
          if (this.tankLevel2 < 0) {
            this.dir[1] = true
            this.tankLevel2 = 0
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
.hmi_container {
  position: relative;
  width: 1200px;
  height: 800px;
}
</style>
