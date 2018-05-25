<template>
  <v-container fluid grid-list-lg>
    <v-layout row wrap>
      <v-flex xs12>
        <v-card>
          <v-card-title>
            <h2>Query Alarm Log </h2>
            <v-spacer></v-spacer>
            <v-tooltip bottom>
              <v-btn color="green" slot="activator" :disabled="isQueryTimeOK() !== true" @click="queryAlarmLog()">
                <v-icon>cloud_download</v-icon>
              </v-btn>
              <span>Query Alarm Log</span>
            </v-tooltip>
          </v-card-title>
          <v-layout row wrap justify-center>
            <v-flex xs12 sm6 md6>
              <v-menu
                ref="startDateDlg"
                :close-on-content-click="false"
                v-model="showStartDate"
                :nudge-right="40"
                :return-value.sync="startDate"
                lazy
                transition="scale-transition"
                offset-t
                full-width
                min-width="290px"
              >
                <v-text-field
                  slot="activator"
                  v-model="startDate"
                  label="start date"
                  prepend-icon="event"
                  readonly
                >
                </v-text-field>
                <v-date-picker v-model="startDate" @input="$refs.startDateDlg.save(startDate)"></v-date-picker>
              </v-menu>
            </v-flex>

            <v-flex xs12 sm6 md6>
              <v-menu
                ref="startTimeDlg"
                :close-on-content-click="false"
                v-model="showStartTime"
                :nudge-right="40"
                :return-value.sync="startTime"
                lazy
                transition="scale-transition"
                offset-y
                full-width
                max-width="290px"
                min-width="290px"
              >
                <v-text-field
                  slot="activator"
                  v-model="startTime"
                  label="Start Time"
                  prepend-icon="access_time"
                  readonly
                ></v-text-field>
                <v-time-picker v-model="startTime" @change="$refs.startTimeDlg.save(startTime)"></v-time-picker>
              </v-menu>
            </v-flex>

            <v-flex xs12 sm6 md6>
              <v-menu
                ref="endDateDlg"
                :close-on-content-click="false"
                v-model="showEndDate"
                :nudge-right="40"
                :return-value.sync="endDate"
                lazy
                transition="scale-transition"
                offset-t
                full-width
                min-width="290px"
              >
                <v-text-field
                  slot="activator"
                  v-model="endDate"
                  label="end date"
                  prepend-icon="event"
                  readonly
                >
                </v-text-field>
                <v-date-picker v-model="endDate" @input="$refs.endDateDlg.save(endDate)"></v-date-picker>
              </v-menu>
            </v-flex>

            <v-flex xs12 sm6 md6>
              <v-menu
                ref="endTimeDlg"
                :close-on-content-click="false"
                v-model="showEndTime"
                :nudge-right="40"
                :return-value.sync="endTime"
                lazy
                transition="scale-transition"
                offset-y
                full-width
                max-width="290px"
                min-width="290px"
              >
                <v-text-field
                  slot="activator"
                  v-model="endTime"
                  label="End Time"
                  prepend-icon="access_time"
                  readonly
                ></v-text-field>
                <v-time-picker v-model="endTime" @change="$refs.endTimeDlg.save(endTime)"></v-time-picker>
              </v-menu>
            </v-flex>
          </v-layout>
        </v-card>
      </v-flex>

      <v-flex xs12>
        <v-card>
          <v-card-title>
            <h2>Alarm Log</h2>
            <v-spacer></v-spacer>
            <v-text-field
             append-icon="search"
             label="Search"
             single-line
             hide-details
             v-model="search">
            </v-text-field>
          </v-card-title>

          <v-data-table
           disable-initial-sort
           :headers="headers"
           :items="items"
           :search="search"
           :rows-per-page-items="[30, 20, 10, {text: 'All', value: -1}]"
           class="elevation-1"
          >
            <template slot="headerCell" slot-scope="props">
              <v-tooltip bottom>
                <span slot="activator"> {{props.header.text}}</span>
                <span> {{props.header.text}}</span>
              </v-tooltip>
            </template>
            <template slot="items" slot-scope="props">
              <tr>
                <td width="200px" class="text-xs-left">{{props.item.time}}</td>
                <td width="100px" class="text-xs-left">{{props.item.event}}</td>
                <td width="100px" class="text-xs-left">{{props.item.alarm}}</td>
                <td class="text-xs-left">{{props.item.text}}</td>
              </tr>
            </template>
          </v-data-table>
        </v-card>
      </v-flex>
    </v-layout>
  </v-container>
</template>

<script>
  import serverAPI from '@/server_api'
  import dateFormat from 'dateformat'

  export default {
    name: 'alarmLogView',
    methods: {
      getDateObjFromDateString () {
        if (this.startDate === null || this.endDate === null ||
            this.startTime === null || this.endTime === null) {
          return null
        }

        var s, e

        s = this.startDate + ' ' + this.startTime
        e = this.endDate + ' ' + this.endTime

        var ds, de

        ds = new Date(s)
        de = new Date(e)

        return {
          start: ds,
          end: de
        }
      },
      isQueryTimeOK () {
        var timeFrame

        timeFrame = this.getDateObjFromDateString()
        if (timeFrame === null) {
          return false
        }

        if (timeFrame.start.getTime() > timeFrame.end.getTime()) {
          return false
        }

        return true
      },
      buildAlarmLog (data) {
        var self = this

        self.items = []

        data.alarms.forEach(function (d) {
          var eventStr = [
            'Occur',
            'Ack',
            'Clear'
          ]

          var time = new Date(d.timestamp)

          var item = {
            'time': dateFormat(time, 'yyyy-mm-dd HH:MM:ss'),
            'event': eventStr[d.event],
            'alarm': d.alarm,
            'text': self.$store.getters.alarm(d.alarm).name
          }

          self.items.push(item)
        })
      },
      queryAlarmLog () {
        var timeFrame
        var self = this

        timeFrame = this.getDateObjFromDateString()

        serverAPI.getAlarmLog(timeFrame.start, timeFrame.end, (err, data) => {
          if (self.destroyed) {
            console.log('instance destroyed. skipping ')
            return
          }

          if (err) {
            console.log('failed to retrieve alarm log')
            console.log(err)
            self.$notify({
              title: 'Failed',
              text: 'Failed to query alarm log',
              type: 'error'
            })
            return
          }

          self.buildAlarmLog(data.data)
        })
      }
    },
    beforeDestroy () {
      this.destroyed = true
    },
    data () {
      return {
        destroyed: false,
        showStartDate: false,
        showStartTime: false,
        showEndDate: false,
        showEndTime: false,
        startDate: null,
        startTime: null,
        endDate: null,
        endTime: null,
        search: '',
        headers: [
          { text: 'Time', align: 'left', value: 'time' },
          { text: 'Event', align: 'left', value: 'event' },
          { text: 'Alarm#', align: 'left', value: 'alarm' },
          { text: 'Text', align: 'left', value: 'text' }
        ],
        items: [
        ]
      }
    }
  }
</script>

<style scoped>
</style>
