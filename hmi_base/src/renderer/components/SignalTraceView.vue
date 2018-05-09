<template>
  <v-container fluid grid-list-md mt-0>
    <v-layout row wrap>
      <v-flex xs12>
        <v-card>
          <v-card-title><h2>Signal Trace</h2></v-card-title>
          <line-chart class="graph-div" :chartData="data" :options="options" ref="lineGraph"></line-chart>
        </v-card>
      </v-flex>
    </v-layout>
  </v-container>
</template>

<script>
  import LineChart from '@/components/LineChart'
  import Vue from 'vue'
  import dateFormat from 'dateformat'

  export default {
    name: 'signalTraceView',
    components: { LineChart },
    methods: {
      update () {
        // var v = this.$store.getters.channel(11)
        var d = new Date()

        d = dateFormat(d, 'yyyy-mm-dd HH:MM:ss')

        var rad = Math.PI * this.count / 180

        var v = 0.0572 * Math.cos(4.667 * rad) + 0.0218 * Math.cos(12.22 * rad)
        this.count += 1

        Vue.set(this.data.labels, this.data.labels.length, d)
        Vue.set(this.data.datasets[0].data, this.data.datasets[0].data.length, v)

        if (this.data.labels.length > this.maxPoints) {
          this.data.labels.splice(0, 1)
        }

        if (this.data.datasets[0].data.length > this.maxPoints) {
          this.data.datasets[0].data.splice(0, 1)
        }

        this.$refs['lineGraph'].refresh()
      }
    },
    created () {
      this.$options.interval = setInterval(this.update, 20)
    },
    beforeDestroy () {
      clearInterval(this.$options.interval)
    },
    data () {
      var configOptions = {
        responsive: true,
        maintainAspectRatio: false,
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
        maxPoints: 200 * 2,
        count: 0,
        channels: [],
        data: {
          labels: [],
          datasets: [
            {
              label: 'Sample Data',
              fill: false,
              backgroundColor: '#f87979',
              pointRadius: 1.5,
              data: [],
              showLine: true
            }
          ]
        },
        options: configOptions
      }
    }
  }
</script>

<style scoped>
.graph-div {
  height: 700px;
  width: 90%;
}
</style>
