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
            <hmi-value-box style="position: absolute; z-index: 100; top: 130.3px; left: 140.9px;" :chnl="11" :alarms="[2]"
             :options="hmi_value_box_options"></hmi-value-box>

            <hmi-value-box style="position: absolute; z-index: 100; top: 170.3px; left: 140.9px;" :chnl="11" :alarms="[2]"
             :options="hmi_value_box_options2"></hmi-value-box>

            <hmi-radial-gauge style="position: absolute; z-index: 100; top: -90px; left: 390px;" :chnl="11" :alarms="[4]" :options="hmi_gauge3_option">
            </hmi-radial-gauge>

            <hmi-radial-gauge style="position: absolute; z-index: 100; top: 330px; left: 290px;" :chnl="11" :alarms="[4]" :options="hmi_gauge3_option">
            </hmi-radial-gauge>
            
            <hmi-button style="position: absolute; z-index: 100; top: 60px; left: 600px;" :outChnl="20" :progChnl="2" :alarms="[1]" :interlock="1" :text="'Test Button'" :options="hmi_button_options1">
            </hmi-button>

            <!-- SVG start -->
            <svg
               width="1200px"
               height="800px"
               viewBox="0 0 1052.3622 744.09451"
               id="svg2"
               version="1.1"
               inkscape:version="0.91 r13725"
               sodipodi:docname="mimic_demo.svg">
              <defs
                 id="defs4">
                <linearGradient
                   id="linearGradient4353"
                   inkscape:collect="always">
                  <stop
                     id="stop4355"
                     offset="0"
                     style="stop-color:#0000ff;stop-opacity:1;" />
                  <stop
                     id="stop4357"
                     offset="1"
                     style="stop-color:#0000ff;stop-opacity:0;" />
                </linearGradient>
                <linearGradient
                   id="linearGradient4299"
                   inkscape:collect="always">
                  <stop
                     id="stop4301"
                     offset="0"
                     style="stop-color:#999999;stop-opacity:1;" />
                  <stop
                     id="stop4303"
                     offset="1"
                     style="stop-color:#999999;stop-opacity:0;" />
                </linearGradient>
                <linearGradient
                   gradientTransform="matrix(0.76537976,0,0,0.71406781,235.66342,313.75684)"
                   gradientUnits="userSpaceOnUse"
                   y2="409.67935"
                   x2="939.02509"
                   y1="513.33789"
                   x1="166.46289"
                   id="linearGradient4305"
                   xlink:href="#linearGradient4299"
                   inkscape:collect="always" />
                <linearGradient
                   gradientTransform="matrix(0.76537976,0,0,0.71406781,235.43012,312.25912)"
                   gradientUnits="userSpaceOnUse"
                   y2="20.313412"
                   x2="1231.0975"
                   y1="515.43536"
                   x1="167.68292"
                   id="linearGradient4359"
                   xlink:href="#linearGradient4353"
                   inkscape:collect="always" />
                <linearGradient
                   gradientTransform="matrix(0.76537976,0,0,0.71406781,235.66342,313.75684)"
                   gradientUnits="userSpaceOnUse"
                   y2="409.67935"
                   x2="939.02509"
                   y1="513.33789"
                   x1="166.46289"
                   id="linearGradient4305-5"
                   xlink:href="#linearGradient4299"
                   inkscape:collect="always" />
                <linearGradient
                   gradientTransform="matrix(0.76537976,0,0,0.71406781,235.43012,312.25912)"
                   gradientUnits="userSpaceOnUse"
                   y2="20.313412"
                   x2="1231.0975"
                   y1="515.43536"
                   x1="167.68292"
                   id="linearGradient4359-9"
                   xlink:href="#linearGradient4353"
                   inkscape:collect="always" />
              </defs>
              <sodipodi:namedview
                 id="base"
                 pagecolor="#ffffff"
                 bordercolor="#666666"
                 borderopacity="1.0"
                 inkscape:pageopacity="0.0"
                 inkscape:pageshadow="2"
                 inkscape:zoom="1"
                 inkscape:cx="384.06376"
                 inkscape:cy="301.89059"
                 inkscape:document-units="px"
                 inkscape:current-layer="layer2"
                 showgrid="false"
                 inkscape:window-width="1457"
                 inkscape:window-height="827"
                 inkscape:window-x="26"
                 inkscape:window-y="94"
                 inkscape:window-maximized="0"
                 units="px"
                 showborder="false" />
              <metadata
                 id="metadata7">
                <rdf:RDF>
                  <cc:Work
                     rdf:about="">
                    <dc:format>image/svg+xml</dc:format>
                    <dc:type
                       rdf:resource="http://purl.org/dc/dcmitype/StillImage" />
                    <dc:title></dc:title>
                  </cc:Work>
                </rdf:RDF>
              </metadata>
              <g
                 inkscape:label="background"
                 inkscape:groupmode="layer"
                 id="layer1"
                 transform="translate(0,-308.26768)">
                <g
                   id="g5408"
                   style="fill:#ff5555">
                  <rect
                     transform="matrix(0,-1,-1,0,0,0)"
                     ry="0"
                     y="-271.72223"
                     x="-672.7511"
                     height="43.518524"
                     width="13.888902"
                     id="rect4447-3-0-7"
                     style="fill:#ff5555;fill-opacity:1" />
                  <rect
                     transform="matrix(0,-1,-1,0,0,0)"
                     ry="0"
                     y="-272.64813"
                     x="-386.64001"
                     height="43.518524"
                     width="13.888902"
                     id="rect4447-3-0"
                     style="fill:#ff5555;fill-opacity:1" />
                  <rect
                     ry="0"
                     y="385.71402"
                     x="258.75922"
                     height="285.18518"
                     width="13.888902"
                     id="rect4447-6-8"
                     style="fill:#ff5555;fill-opacity:1" />
                  <rect
                     transform="matrix(0,-1,-1,0,0,0)"
                     ry="0"
                     y="-359.68518"
                     x="-531.08447"
                     height="93.518524"
                     width="13.888902"
                     id="rect4447-3-0-9"
                     style="fill:#ff5555;fill-opacity:1" />
                </g>
                <g
                   id="g5077"
                   style="fill:#ff5555"
                   transform="translate(22,0)">
                  <rect
                     transform="matrix(0,-1,-1,0,0,0)"
                     ry="0"
                     y="-572.12964"
                     x="-531.08447"
                     height="114.81482"
                     width="13.888902"
                     id="rect4447-3-0-9-2"
                     style="fill:#ff5555;fill-opacity:1" />
                  <rect
                     transform="scale(-1,1)"
                     ry="0"
                     y="521.80664"
                     x="-519.90747"
                     height="90.740746"
                     width="13.888902"
                     id="rect4447-3-0-9-2-0"
                     style="fill:#ff5555;fill-opacity:1" />
                </g>
                <rect
                   style="fill:#ff5555;fill-opacity:1"
                   id="rect4447-3-0-9-2-0-5"
                   width="13.888902"
                   height="94.278656"
                   x="-542.83331"
                   y="663.47333"
                   ry="0"
                   transform="scale(-1,1)" />
                <rect
                   style="fill:#ff5555;fill-opacity:1"
                   id="rect4447-3-0-9-2-6"
                   width="13.888902"
                   height="50.000004"
                   x="-531.52887"
                   y="-692.37036"
                   ry="0"
                   transform="matrix(0,-1,-1,0,0,0)" />
                <g
                   id="g5206"
                   transform="translate(22,0)"
                   style="fill:#ff5555">
                  <rect
                     transform="matrix(0,-1,-1,0,0,0)"
                     ry="0"
                     y="-948.74084"
                     x="-531.52887"
                     height="57.407413"
                     width="13.888902"
                     id="rect4447-3-0-9-2-3"
                     style="fill:#ff5555;fill-opacity:1" />
                  <rect
                     transform="scale(-1,1)"
                     ry="0"
                     y="517.63995"
                     x="-948.74084"
                     height="90.740746"
                     width="13.888902"
                     id="rect4447-3-0-9-2-0-2"
                     style="fill:#ff5555;fill-opacity:1" />
                </g>
                <g
                   id="g5414"
                   style="fill:#ff5555">
                  <rect
                     transform="scale(-1,1)"
                     ry="0"
                     y="766.2511"
                     x="-970.74084"
                     height="90.740746"
                     width="13.888902"
                     id="rect4447-3-0-9-2-0-5-5"
                     style="fill:#ff5555;fill-opacity:1" />
                  <rect
                     transform="matrix(0,1,1,0,0,0)"
                     ry="0"
                     y="958.11115"
                     x="843.10297"
                     height="90.740746"
                     width="13.888902"
                     id="rect4447-3-0-9-2-0-5-5-4"
                     style="fill:#ff5555;fill-opacity:1" />
                </g>
                <g
                   id="g5401"
                   style="fill:#ff5555">
                  <rect
                     transform="matrix(0,-1,-1,0,0,0)"
                     ry="0"
                     y="-78.240746"
                     x="-835.23254"
                     height="43.518524"
                     width="13.888902"
                     id="rect4447-3-0-7-4"
                     style="fill:#ff5555;fill-opacity:1" />
                  <rect
                     transform="matrix(0,-1,-1,0,0,0)"
                     ry="0"
                     y="-78.240746"
                     x="-548.19556"
                     height="43.518524"
                     width="13.888902"
                     id="rect4447-3-0-7-4-4"
                     style="fill:#ff5555;fill-opacity:1" />
                  <rect
                     ry="0"
                     y="538.47333"
                     x="34.722221"
                     height="425"
                     width="13.888902"
                     id="rect4447-6-8-4"
                     style="fill:#ff5555;fill-opacity:1" />
                  <rect
                     transform="matrix(0,-1,-1,0,0,0)"
                     ry="0"
                     y="-537.5"
                     x="-967.64001"
                     height="502.7778"
                     width="13.888902"
                     id="rect4447-3-0-9-3"
                     style="fill:#ff5555;fill-opacity:1" />
                  <rect
                     transform="scale(-1,1)"
                     ry="0"
                     y="885.39838"
                     x="-542.83331"
                     height="82.241615"
                     width="13.888902"
                     id="rect4447-3-0-9-2-0-5-7"
                     style="fill:#ff5555;fill-opacity:1" />
                </g>
              </g>
              <g
                 inkscape:groupmode="layer"
                 id="layer2"
                 inkscape:label="equipments"
                 transform="translate(0,3.5188977e-5)">
                <hmi-valve :x="592" :y="182" :width="52.2" :height="44.8"
                           :options="hmi_icon_options"
                           :runningChnl="20"
                           :startingChnl="2"
                           :stoppingChnl="2"></hmi-valve>
                <hmi-valve :x="497.5" :y="304" :width="49.8" :height="52.2" :rotate="-90"
                           :options="hmi_icon_options"
                           :runningChnl="2"
                           :startingChnl="20"
                           :stoppingChnl="2"></hmi-valve>
                <hmi-boiler :x="689.3"
                            :y="103.7"
                            :width="229.3"
                            :height="170.6"
                            :options="hmi_icon_options"
                            :runningChnl="2"
                            :startingChnl="2"
                            :stoppingChnl="2"
                            :alarms="[20]">/</hmi-boiler>
                <hmi-compressor :x="353.8" :y="176" :width="131.1" :height="79.7"
                                :options="hmi_icon_options"
                                :runningChnl="20"
                                :startingChnl="2"
                                :stoppingChnl="2">/</hmi-compressor>
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
      HmiButton
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
      hmi_gauge3_option () {
        var def = this.hmi_default_gauge_option1
        var ret = Object.assign({}, def)

        return Object.assign(ret, {title: 'Inlet 1',
          units: '°C',
          width: 180,
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
