<template>
  <v-dialog v-model="showDialog" persistent max-width="600px">
    <v-card>
      <v-card-title>
        <span class="headline">Select Time Frame and Channels to View</span>
      </v-card-title>

      <v-card-text>
        <v-container fluid>
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

            <v-flex xs12 sm12 md12>
              <v-select
               :items="periodItems"
               v-model="period"
               label="Period"
               single-line
              ></v-select>
            </v-flex>
          </v-layout>

        </v-container>
        <v-divider></v-divider>

        <v-data-table
         :headers="headers"
         :items="tracedChannels"
         v-model="selectedChannels"
         item-key="chnl_num"
         select-all
         hide-actions
        >
          <template slot="items" slot-scope="props">
            <td>
              <v-checkbox
               v-model="props.selected"
               primary
               hide-details
              ></v-checkbox>
            </td>
            <td width="100px" class="text-xs-left">{{props.item.chnl_num}}</td>
            <td class="text-xs-left">{{props.item.desc}}</td>
          </template>
        </v-data-table>
      </v-card-text>

      <v-card-actions>
        <v-spacer></v-spacer>
        <v-btn color="blue darken-1" flat @click="$emit('traceSetting')">Channel Trace Setting</v-btn>
        <v-btn color="blue darken-1" flat @click="$emit('dismiss')">Dismiss</v-btn>
        <v-btn color="blue darken-1" flat :disabled="isSelectionOK() === false" @click="onSelect()">Select</v-btn>
      </v-card-actions>
    </v-card>
  </v-dialog>
</template>

<script>
  export default {
    name: 'SignalLogSelectDialog',
    props: ['showDialog', 'currentChannels'],
    watch: {
      showDialog: function (newVal) {
        if (newVal === true) {
          var self = this

          self.selectedChannels = []
          self.tracedChannels = []
          self.currentChannels.forEach(function (chnl) {
            self.tracedChannels.push({
              chnl_num: chnl,
              desc: self.$store.getters.channel(chnl).name
            })
          })
        }
      }
    },
    methods: {
      isSelectionOK () {
        if (this.startDate === null ||
            this.startTime === null ||
            this.period === null ||
            this.selectedChannels.length === 0 ||
            this.selectedChannels.length > 16) {
          return false
        }
        return true
      },
      onSelect () {
        var start = (new Date(this.startDate + ' ' + this.startTime)).getTime()
        var p

        console.log('--- ' + this.period + ' ---')
        p = this.period.t
        var obj = {
          startTime: start,
          endTime: start + p,
          channels: []
        }

        this.selectedChannels.forEach(function (d) {
          obj.channels.push(d.chnl_num)
        })

        this.$emit('select', obj)
      }
    },
    data () {
      return {
        showStartDate: false,
        showStartTime: false,
        startDate: null,
        startTime: null,
        periodItems: [
          { text: '5 Minutes', t: 5 * 60 * 1000 },
          { text: '10 Minutes', t: 10 * 60 * 1000 },
          { text: '30 Minutes', t: 30 * 60 * 1000 },
          { text: '1 Hour', t: 1 * 60 * 60 * 1000 },
          { text: '2 Hour', t: 2 * 60 * 60 * 1000 }
        ],
        period: null,
        tracedChannels: [],
        selectedChannels: [],
        headers: [
          { text: 'Number', align: 'left', value: 'chnl_num', sortable: false },
          { text: 'Description', align: 'left', value: 'desc', sortable: false }
        ]
      }
    }
  }
</script>

<style scoped>
</style>
