<template>
  <v-container fluid grid-list-md mt-0 fill-height>
    <v-layout row wrap>
      <v-flex xs12>
        <v-card>
          <v-card-title>
            <h2>Signal Trace</h2>
            <v-spacer></v-spacer>
            <v-tooltip bottom>
              <v-btn color="green" slot="activator" @click="start_stop_realtime_play()">
                <v-icon>{{realtimePlaying ? "pause_circle_filled" : "play_circle_filled"}}</v-icon>
              </v-btn>
              <span v-if="realtimePlaying === true">Stop Realtime Graphing</span>
              <span v-if="realtimePlaying === false">Start Realtime Graphing</span>
            </v-tooltip>

            <v-tooltip bottom>
              <v-btn color="green" slot="activator" :disabled="realtimePlaying === true" @click="show_channel_select_dialog()">
                <v-icon>add_circle</v-icon>
                <v-icon>remove_circle</v-icon>
              </v-btn>
              <span>Modify Channel Selection</span>
            </v-tooltip>
              
            <v-tooltip bottom>
              <v-btn color="green" slot="activator" :disabled="realtimePlaying === true">
                <v-icon>cloud_download</v-icon>
              </v-btn>
              <span>Query Signal Log</span>
            </v-tooltip>

            <v-tooltip bottom>
              <v-btn color="green" slot="activator" :disabled="realtimePlaying === true" @click="showSettingsDialog = true">
                <v-icon>settings</v-icon>
              </v-btn>
              <span>Plot Settings</span>
            </v-tooltip>
          </v-card-title>

          <v-layout row wrap align-center>
            <v-btn flat icon color="indigo" :disabled="realtimePlaying === true">
              <v-icon>arrow_back</v-icon>
            </v-btn>

            <v-spacer></v-spacer>

            <v-btn flat icon color="indigo" :disabled="realtimePlaying === true">
              <v-icon>arrow_forward</v-icon>
            </v-btn>
          </v-layout>

          <line-chart id="wrapper" :chartData="data" :options="options" ref="lineGraph"></line-chart>
          <channel-select-dialog :showDialog="showChannelSelectDialog" :currentChannels="channels"
           @dismiss="showChannelSelectDialog = false"
           @select="onChannelSelectionModified"
          ></channel-select-dialog>
          <plot-settings-dialog :showDialog="showSettingsDialog" :plotCfg="plotCfg"
           @dismiss="showSettingsDialog = false"
           @change="onSettingsModified"
          ></plot-settings-dialog>
        </v-card>
      </v-flex>
    </v-layout>
  </v-container>
</template>

<script>
  import LineChart from '@/components/LineChart'
  import dateFormat from 'dateformat'
  import utils from '@/utils'
  import ChannelSelectDialog from '@/components/ChannelSelectDialog'
  import PlotSettingsDialog from '@/components/PlotSettingsDialog'

  export default {
    name: 'signalTraceView',
    components: { LineChart, ChannelSelectDialog, PlotSettingsDialog },
    methods: {
      start_stop_realtime_play () {
        if (this.realtimePlaying === true) {
          // stop
          this.realtimePlaying = false
          clearInterval(this.$options.interval)
          this.$options.interval_timer = null
        } else {
          // start
          this.realtimePlaying = true
          this.$options.interval = setInterval(this.update, this.plotCfg.samplingInterval)

          // clear data
          this.data.labels = []
          this.data.datasets.forEach(function (element) {
            element.data = []
          })
        }
      },
      onChannelSelectionModified: function (data) {
        var self = this

        console.log('onChannelSelectionModified:' + data)

        self.channels = [...data]
        self.data.datasets = []
        data.forEach(function (chnlNum) {
          self.add_channel(chnlNum)
        })
        self.showChannelSelectDialog = false
        self.$refs['lineGraph'].refresh()
      },
      onSettingsModified: function (setting) {
        console.log('changing settings:' + setting)
        this.plotCfg.maxSamplesVisible = setting.maxSamplesVisible
        this.plotCfg.maxSamplesToKeep = setting.maxSamplesToKeep
        this.plotCfg.samplingInterval = setting.samplingInterval
        this.plotCfg.samplesToUpdate = setting.samplesToUpdate
        this.showSettingsDialog = false
      },
      add_channel (chnlNum) {
        var n = chnlNum

        this.data.datasets.push({
          label: 'CH:' + n,
          fill: false,
          borderColor: utils.getColor(n),
          pointRadius: 1.5,
          data: [],
          showLine: true
        })
      },
      show_channel_select_dialog () {
        this.showChannelSelectDialog = true
      },
      update () {
        // var v = this.$store.getters.channel(11)
        var d = new Date()

        d = dateFormat(d, 'yyyy-mm-dd HH:MM:ss')

        var rad = Math.PI * this.count / 180

        var v = 0.0572 * Math.cos(4.667 * rad) + 0.0218 * Math.cos(12.22 * rad)
        this.count += 1

        this.data.labels[this.data.labels.length] = d

        var self = this

        this.data.datasets.forEach(function (element) {
          var r = Math.random()
          var r2 = (Math.random() - 0.5) * 5

          element.data[element.data.length] = v * r2 + r
          if (element.data.length > self.plotCfg.maxSamplesVisible) {
            element.data.splice(0, 1)
          }
        })

        if (this.data.labels.length > this.plotCfg.maxSamplesVisible) {
          this.data.labels.splice(0, 1)
        }

        this.accumlationCnt += 1

        if (this.accumlationCnt >= this.plotCfg.samplesToUpdate) {
          this.$refs['lineGraph'].refresh()
          this.accumlationCnt = 0
        }
      }
    },
    created () {
      this.$options.interval_timer = null
    },
    beforeDestroy () {
      if (this.$options.interval_timer != null) {
        clearInterval(this.$options.interval)
      }
    },
    data () {
      var configOptions = {
        responsive: true,
        animation: false,
        maintainAspectRatio: false,
        elements: {
          point: {
            radius: 1.5
          }
        },
        scales: {
          xAxes: [
            {
              grindLines: {
                display: true
              },
              scaleLabel: {
                display: true,
                labelString: 'time'
              }
            }
          ],
          yAxes: [
            {
              grindLines: {
                display: true
              },
              scaleLabel: {
                display: true,
                labelString: 'value'
              }
            }
          ]
        }
      }

      return {
        showChannelSelectDialog: false,
        realtimePlaying: false,
        plotCfg: {
          maxSamplesVisible: 180,
          maxSamplesToKeep: 180 * 10,
          samplingInterval: 20,
          samplesToUpdate: 10
        },
        showSettingsDialog: false,
        count: 0,
        accumlationCnt: 0,
        channels: [],
        data: {
          labels: [],
          datasets: [
          ]
        },
        options: configOptions
      }
    }
  }
</script>

<style scoped>
#wrapper {
  height: 70vh;
}

</style>
