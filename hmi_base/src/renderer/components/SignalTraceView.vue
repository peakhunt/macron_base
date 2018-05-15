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

          <line-chart id="wrapper" :chartData="data" :options="options" ref="lineGraph"
           v-dragged="onDragged"></line-chart>
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
      onDragged ({ el, deltaX, deltaY, offsetX, offsetY, clientX, clientY, first, last }) {
        if (this.realtimePlaying === true) {
          return
        }

        if (deltaX !== undefined) {
          this.shift_visible(deltaX)
        }
      },
      clear_data () {
        this.data.labels = []
        this.data.labels_accumulated = []
        this.visibleStart = 0

        this.data.datasets.forEach(function (element) {
          element.data = []
          element.data_accumulated = []
        })
      },
      start_stop_realtime_play () {
        if (this.realtimePlaying === true) {
          // stop
          this.realtimePlaying = false
          clearInterval(this.$options.interval_timer)
          this.$options.interval_timer = null
          this.$refs['lineGraph'].refresh()
        } else {
          // start
          this.realtimePlaying = true
          this.$options.interval_timer = setInterval(this.update, this.plotCfg.samplingInterval)

          // clear data
          this.clear_data()
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

        this.adjust_offset_ndx_and_reload()
      },
      add_channel (chnlNum) {
        var n = chnlNum

        this.data.datasets.push({
          label: 'CH:' + n,
          fill: false,
          borderColor: utils.getColor(n),
          pointRadius: 1.5,
          data: [],
          data_accumulated: [],
          showLine: true
        })
      },
      show_channel_select_dialog () {
        this.showChannelSelectDialog = true
      },
      get_data_simul (graphNdx) {
        // var v = this.$store.getters.channel(11)
        var rad = Math.PI * this.count / 180
        var v = 0.0572 * Math.cos(4.667 * rad) + 0.0218 * Math.cos(12.22 * rad)
        var r = Math.random()
        var r2 = (Math.random() - 0.5) * 5

        return v * r2 * r
      },
      push_label (label) {
        this.data.labels[this.data.labels.length] = label
        this.data.labels_accumulated[this.data.labels_accumulated.length] = label

        if (this.data.labels_accumulated.length > this.plotCfg.maxSamplesToKeep) {
          this.data.labels_accumulated.splice(0, 1)
        }

        this.visibleStart = this.data.labels_accumulated.length - this.plotCfg.maxSamplesVisible
        if (this.visibleStart < 0) {
          this.visibleStart = 0
        }
      },
      push_data (dataGetter) {
        var self = this

        this.data.datasets.forEach(function (element) {
          var ndx = self.data.datasets.indexOf(element)
          var v = dataGetter.call(self, ndx)

          element.data[element.data.length] = v
          if (element.data.length > self.plotCfg.maxSamplesVisible) {
            element.data.splice(0, 1)
          }

          element.data_accumulated[element.data_accumulated.length] = v
          if (element.data_accumulated.length > self.plotCfg.maxSamplesToKeep) {
            element.data_accumulated.splice(0, 1)
          }
        })

        if (this.data.labels.length > this.plotCfg.maxSamplesVisible) {
          this.data.labels.splice(0, 1)
          this.visibleStart += 1
        }
      },
      update () {
        var d = new Date()

        d = dateFormat(d, 'yyyy-mm-dd HH:MM:ss')

        this.count += 1

        this.push_label(d)
        this.push_data(this.get_data_simul)

        this.accumlationCnt += 1

        if (this.accumlationCnt >= this.plotCfg.samplesToUpdate) {
          this.$refs['lineGraph'].refresh()
          this.accumlationCnt = 0
        }
      },
      reload_visible_data () {
        var i
        var self = this
        var numSamples

        this.data.labels = []

        numSamples = Math.min(this.plotCfg.maxSamplesVisible, this.data.labels_accumulated.length)

        console.log('reloading visible: ' + numSamples + ', from: ' + this.visibleStart)

        for (i = 0; i < numSamples; i++) {
          this.data.labels[i] = this.data.labels_accumulated[this.visibleStart + i]
        }

        this.data.datasets.forEach(function (element) {
          element.data[i] = []

          for (i = 0; i < numSamples; i++) {
            element.data[i] = element.data_accumulated[self.visibleStart + i]
          }
        })
        this.$refs['lineGraph'].refresh()
      },
      shift_visible (delta) {
        this.visibleStart += delta
        this.adjust_offset_ndx_and_reload()
      },
      adjust_offset_ndx_and_reload () {
        if (this.visibleStart >= this.data.labels_accumulated.length - this.plotCfg.maxSamplesVisible) {
          this.visibleStart = this.data.labels_accumulated.length - this.plotCfg.maxSamplesVisible
        }

        if (this.visibleStart < 0) {
          this.visibleStart = 0
        }

        console.log('visibleStart:' + this.visibleStart + ', total data:' + this.data.labels_accumulated.length +
            ', max visible:' + this.plotCfg.maxSamplesVisible)

        this.reload_visible_data()
      }
    },
    created () {
      this.$options.interval_timer = null
    },
    beforeDestroy () {
      if (this.$options.interval_timer != null) {
        clearInterval(this.$options.interval_timer)
      }
    },
    data () {
      var configOptions = {
        responsive: true,
        animation: false,
        maintainAspectRatio: false,
        elements: {
          point: {
            radius: 0.5
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
          labels_accumulated: [],
          datasets: [
          ]
        },
        options: configOptions,
        visibleStart: 0
      }
    }
  }
</script>

<style scoped>
#wrapper {
  height: 70vh;
}

</style>
