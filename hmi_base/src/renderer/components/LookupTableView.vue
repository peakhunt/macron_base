<template>
  <v-container>
    <v-layout row wrap grid-list-md>
      <v-flex xs4>
        <v-layout column>
          <div class="text-xs-center">
            <v-tooltip top>
              <v-btn dark color="indigo" slot="activator" @click="on_click_add">
                Add
                <v-icon>add</v-icon>
              </v-btn>
              <span>Add new lookup table entry</span>
            </v-tooltip>

            <v-tooltip top>
              <v-btn color="red" slot="activator" @click="on_click_commit" :disabled="table_changed == false"
               :loading="updating_lookup_table">
                Commit
                <v-icon>cloud_upload</v-icon>
              </v-btn>
              <span>Commit changes</span>
            </v-tooltip>
          </div>

          <v-data-table
           disable-initial-sort
           :headers="lookup_table_headers"
           :items="lookup_table"
           :item-key="lookup_table_key"
           class="elevation-4"
           hide-actions
           >
            <template slot="headerCell" slot-scope="props">
              <v-tooltip bottom>
                <span slot="activator">{{ props.header.text }}</span>
                <span>{{ props.header.text }}</span>
              </v-tooltip>
            </template>
            <template slot="items" slot-scope="props">
              <tr v-on:click="select_entry(props.item)">
                <td>{{ props.item.raw }}</td>
                <td >{{ props.item.eng }}</td>
              </tr>
            </template>
          </v-data-table>

          <v-dialog v-model="dialog" persistent max-width="500px">
            <v-card>
              <v-card-title>
                <span class="headline">Lookup Table Entry</span>
              </v-card-title>
              <v-card-text>
                <v-container grid-list-md>
                  <v-layout wrap>
                    <v-flex xs12 sm6 md4>
                      <v-text-field label="Raw Value" v-model.number="dialog_raw" required></v-text-field>
                    </v-flex>
                    <v-flex xs12 sm6 md4>
                      <v-text-field label="Engieeering Value" v-model.number="dialog_eng" required></v-text-field>
                    </v-flex>
                  </v-layout>
                </v-container>
              </v-card-text>

              <v-card-actions>
                <v-spacer></v-spacer>
                <v-btn color="blue darken-1" flat @click.native="dialog = false">Close</v-btn>
                <v-btn v-if="!dialog_add_mode && lookup_table.length > 2" color="blue darken-1" flat @click="delete_table_entry">Delete</v-btn>
                <v-btn v-if="!dialog_add_mode" color="blue darken-1" flat @click="update_table_entry">Update</v-btn>
                <v-btn v-if="dialog_add_mode" color="blue darken-1" flat @click="add_table_entry">Add</v-btn>
              </v-card-actions>
            </v-card>
          </v-dialog>
        </v-layout>
      </v-flex>

      <v-flex xs8>
        <line-chart :chartData="lookup_table_chart" :options="options" ref="lineGraph"></line-chart>
      </v-flex>
    </v-layout>
  </v-container>
</template>

<script>
  import LineChart from '@/components/LineChart'
  import serverAPI from '@/server_api'

  export default {
    name: 'LookupTable',
    components: { LineChart },
    props: ['chnl_num'],
    data () {
      var lineChartData = []
      var labelData = []
      var lookupTable = []
      var chnl

      chnl = this.$store.getters.channel(this.chnl_num)

      for (var i = 0; i < chnl.lookup_table.length; i++) {
        lookupTable.push(Object.assign({}, chnl.lookup_table[i]))
      }

      for (i = 0; i < lookupTable.length; i++) {
        labelData.push(lookupTable[i].raw)
        lineChartData.push(lookupTable[i].eng)
      }

      return {
        table_changed: false,
        updating_lookup_table: false,
        dialog: false,
        dialog_add_mode: true,
        dialog_raw: 0.0,
        dialog_eng: 0.0,
        selected_entry: null,
        lookup_table: lookupTable,
        lookup_table_key: 'raw',
        lookup_table_headers: [
          { text: 'Raw Value', align: 'left', value: 'raw_value' },
          { text: 'Eng Value', align: 'left', value: 'eng_value' }
        ],
        lookup_table_chart: {
          labels: labelData,
          datasets: [
            {
              label: 'LookUp Table',
              fill: false,
              backgroundColor: '#f87979',
              data: lineChartData
            }
          ]
        },
        options: {
          scales: {
            xAxes: [
              {
                gridLines: {
                  display: true
                },
                scaleLabel: {
                  display: true,
                  labelString: 'raw value'
                }
              }
            ],
            yAxes: [
              {
                gridLines: {
                  display: true
                },
                scaleLabel: {
                  display: true,
                  labelString: 'engineering value'
                }
              }
            ]
          }
        }
      }
    },
    methods: {
      _add_entry: function (entry) {
        this.lookup_table.push(entry)
        this.lookup_table.sort(function (a, b) {
          return a.raw - b.raw
        })
        const index = this.lookup_table.indexOf(entry)
        this.lookup_table_chart.labels.splice(index, 0, entry.raw)
        this.lookup_table_chart.datasets[0].data.splice(index, 0, entry.eng)
        this.$refs['lineGraph'].refresh()
        this.table_changed = true
      },
      _del_entry: function (entry) {
        const index = this.lookup_table.indexOf(entry)
        if (index === -1) {
          alert('index failed')
          return
        }
        console.log('delete_table_entry index:' + index)
        this.lookup_table.splice(index, 1)
        this.lookup_table_chart.labels.splice(index, 1)
        this.lookup_table_chart.datasets[0].data.splice(index, 1)
        this.$refs['lineGraph'].refresh()
        this.table_changed = true
      },
      select_entry: function (row) {
        this.dialog_add_mode = false
        this.dialog_raw = row.raw
        this.dialog_eng = row.eng
        this.selected_entry = row
        this.dialog = true
      },
      on_click_add: function () {
        this.dialog_add_mode = true
        this.dialog_raw = 0
        this.dialog_eng = 0
        this.dialog = true
      },
      on_click_commit: function () {
        var self = this

        self.updating_lookup_table = true

        serverAPI.updateLookupTable(this.chnl_num, this.lookup_table, (err, data) => {
          setTimeout(() => {
            self.updating_lookup_table = false
            if (err) {
              console.log('failed to update lookup table for ' + this.chnl_num)
              console.log(err)
              self.$notify({
                title: 'Update Failed',
                text: 'Failed updating Lookup Table!',
                type: 'error'
              })
              return
            }

            self.table_changed = false
            self.$notify({
              title: 'Update Success',
              text: 'Lookup Table was successfully udpated!',
              type: 'success'
            })
            console.log('lookup table update success for ' + this.chnl_num)
          }, 1000)
        })
      },
      update_table_entry: function () {
        var entry = this.selected_entry

        this._del_entry(entry)
        entry.raw = this.dialog_raw
        entry.eng = this.dialog_eng
        this._add_entry(entry)
        this.dialog = false
      },
      add_table_entry: function () {
        console.log('adding new entry ' + this.dialog_raw + ',' + this.dialog_eng)
        var entry = {
          raw: this.dialog_raw,
          eng: this.dialog_eng
        }
        this._add_entry(entry)
        this.dialog = false
      },
      delete_table_entry: function () {
        console.log('delete_table_entry called')
        this._del_entry(this.selected_entry)
        this.dialog = false
      }
    }
  }
</script>

<style scoped>
</style>
