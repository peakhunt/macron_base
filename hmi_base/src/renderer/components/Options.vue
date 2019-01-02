<template>
  <v-container fluid grid-list-lg mt-0>
    <v-layout row wrap>

      <!-- communication options -->
      <v-flex xs12>
        <v-card>
          <v-card-title primary-title>
            <h2>Communication Options</h2>
          </v-card-title>

          <v-form v-model="dataValid">
            <!-- server IP address -->
            <v-layout row>
              <v-flex xs4>
                <v-subheader>Server IP Address</v-subheader>
              </v-flex>

              <v-flex xs8>
                <v-text-field
                 :rules="[rules.required, rules.ipAddress]"
                 label="IP Address"
                 v-model="serverIP">
                </v-text-field>
              </v-flex>
            </v-layout>

            <!-- web port -->
            <v-layout row>
              <v-flex xs4>
                <v-subheader>Web Port</v-subheader>
              </v-flex>

              <v-flex xs8>
                <v-text-field
                 :rules="[rules.required, rules.portNumber]"
                 label="Web Port"
                 v-model="webPort">
                </v-text-field>
              </v-flex>
            </v-layout>

            <!-- logger if port -->
            <v-layout row>
              <v-flex xs4>
                <v-subheader>Logger IF Port</v-subheader>
              </v-flex>

              <v-flex xs8>
                <v-text-field
                 :rules="[rules.required, rules.portNumber]"
                 label="Logger IF Port"
                 v-model="loggerPort">
                </v-text-field>
              </v-flex>
            </v-layout>

            <!-- modbus port -->
            <v-layout row>
              <v-flex xs4>
                <v-subheader>Modbus Port</v-subheader>
              </v-flex>

              <v-flex xs8>
                <v-text-field
                 :rules="[rules.required, rules.portNumber]"
                 label="Modbus Port"
                 v-model="modbusPort">
                </v-text-field>
              </v-flex>
            </v-layout>

            <!-- Poll Interval -->
            <v-layout row>
              <v-flex xs4>
                <v-subheader>Poll Interval(ms)</v-subheader>
              </v-flex>

              <v-flex xs8>
                <v-text-field
                 :rules="[rules.required, rules.interval]"
                 label="Poll Interval"
                 v-model="pollInterval">
                </v-text-field>
              </v-flex>
            </v-layout>

            <!-- Max Number of Channels per Poll  -->
            <v-layout row>
              <v-flex xs4>
                <v-subheader>Max Number of Channels per Poll</v-subheader>
              </v-flex>

              <v-flex xs8>
                <v-text-field
                 :rules="[rules.required, rules.numPolls]"
                 type="number"
                 label="Number of Channels"
                 v-model="numChannels">
                </v-text-field>
              </v-flex>
            </v-layout>

            <!-- Max Number of Alarms per Poll  -->
            <v-layout row>
              <v-flex xs4>
                <v-subheader>Max Number of Alarms per Poll</v-subheader>
              </v-flex>

              <v-flex xs8>
                <v-text-field
                 :rules="[rules.required, rules.numPolls]"
                 type="number"
                 label="Number of Alarms"
                 v-model="numAlarms">
                </v-text-field>
              </v-flex>
            </v-layout>

            <!-- update button -->
            <v-layout row>
              <v-flex xs9></v-flex>
              <v-flex xs3>
                <v-btn color="blue darken-1" :disabled="dataValid === false" @click="apply_and_restart">Apply and Restart</v-btn>
              </v-flex>
            </v-layout>
          </v-form>

        </v-card>
      </v-flex>

    </v-layout>
  </v-container>
</template>

<script>
  import global from '@/global'
  import {EventBus} from '@/event-bus'

  const remote = require('electron').remote

  export default {
    name: 'Options',
    methods: {
      apply_and_restart: function () {
        EventBus.$emit('systemRestarting')

        global.config.server.ipAddress = this.serverIP
        global.config.server.webPort = this.webPort
        global.config.server.loggerIfPort = this.loggerPort
        global.config.server.modbusPort = this.modbusPort

        global.config.poll_schedule.interval = this.pollInterval
        global.config.poll_schedule.num_channels = this.numChannels
        global.config.poll_schedule.num_alarms = this.numAlarms

        global.saveAppConfig(global.config, () => {
          remote.getCurrentWindow().reload()
        })
      }
    },
    data () {
      return {
        dataValid: false,
        serverIP: global.config.server.ipAddress,
        webPort: global.config.server.webPort,
        loggerPort: global.config.server.loggerIfPort,
        modbusPort: global.config.server.modbusPort,
        pollInterval: global.config.poll_schedule.interval,
        numChannels: global.config.poll_schedule.num_channels,
        numAlarms: global.config.poll_schedule.num_alarms,
        rules: {
          required: value => !!value || 'Required.',
          ipAddress: value => {
            const pattern = /^(\d|[1-9]\d|1\d\d|2([0-4]\d|5[0-5]))\.(\d|[1-9]\d|1\d\d|2([0-4]\d|5[0-5]))\.(\d|[1-9]\d|1\d\d|2([0-4]\d|5[0-5]))\.(\d|[1-9]\d|1\d\d|2([0-4]\d|5[0-5]))$/

            return pattern.test(value) || 'Invalid IP Address'
          },
          portNumber: value => {
            const er = /^-?[0-9]+$/

            if (er.test(value) === false) {
              return 'Invalid integer number'
            }

            var n = parseInt(value)

            if (n < 0) {
              return 'Negative numbers are not allowed'
            }

            if (n > 50000) {
              return 'should be less than or equal to 50000'
            }

            return true
          },
          interval: value => {
            const er = /^[0-9]+$/

            if (er.test(value) === false) {
              return 'Invalid integer number'
            }

            var n = parseInt(value)

            if (n < 10) {
              return 'should be bigger than or equal to10'
            }

            if (n > 500) {
              return 'should be less or eqtual than 500'
            }

            return true
          },
          numPolls: value => {
            const er = /^[0-9]+$/

            if (er.test(value) === false) {
              return 'Invalid integer number'
            }

            var n = parseInt(value)

            if (n < 64) {
              return 'should be bigger than 64'
            }

            if (n > 4096) {
              return 'should be less than or equal to 4096'
            }

            return true
          }
        }
      }
    }
  }
</script>

<style scoped>
</style>
