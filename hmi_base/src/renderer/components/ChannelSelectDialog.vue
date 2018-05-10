<template>
  <v-dialog v-model="showDialog" persistent max-width="600px">
    <v-card>
      <v-card-title>
        <span class="headline">Select Channels</span>
      </v-card-title>

      <v-card-text>
        <v-container fluid>
          <v-layout row wrap>
            <v-flex xs12>
              <v-text-field v-model.number="channelNum" single-line label="Search Channel" append-icon="search"></v-text-field>
            </v-flex>
            <v-flex xs12 class="text-xs-center">
              <v-btn round color="green" :disabled="channelValid === false" @click="addChannel()">
                {{searchedChannel}} 
              </v-btn>
            </v-flex>
          </v-layout>
        </v-container>
        <v-divider></v-divider>

        <v-data-table
         :headers="headers"
         :items="selectedChannels"
         v-model="selected"
         hide-actions
        >
          <template slot="items" slot-scope="props">
            <tr>
              <td width="100px" class="text-xs-left">{{$store.getters.channel(props.item).chnl_num}}</td>
              <td class="text-xs-left">{{$store.getters.channel(props.item).name}}</td>
              <td width="100px" class="text-xs-right" @click="remChannel($store.getters.channel(props.item).chnl_num)">
                <v-btn icon class="mx-0">
                  <v-icon color="pink">delete</v-icon>
                </v-btn>
              </td>
            </tr>
          </template>
        </v-data-table>
      </v-card-text>

      <v-card-actions>
        <v-spacer></v-spacer>
          <v-btn color="blue darken-1" flat @click.native="$emit('dismiss')">Dismiss</v-btn>
          <v-btn color="blue darken-1" flat @click.native="$emit('select', selectedChannels)">Select</v-btn>
      </v-card-actions>
    </v-card>
  </v-dialog>
</template>

<script>
  export default {
    name: 'ChannelSelectDialog',
    props: ['showDialog', 'currentChannels'],
    methods: {
      addChannel: function () {
        var n = parseInt(this.channelNum)

        if (this.selectedChannels.indexOf(n) === -1) {
          this.selectedChannels.push(n)
        }
      },
      remChannel: function (chnlNum) {
        var ndx = this.selectedChannels.indexOf(chnlNum)

        this.selectedChannels.splice(ndx, 1)
      }
    },
    computed: {
      searchedChannel: function () {
        var chnl = this.$store.getters.channel(this.channelNum)

        if (chnl === null || chnl === undefined) {
          this.channelValid = false
          return 'N/A : No matching channel found'
        }

        this.channelValid = true
        return 'Add ' + chnl.chnl_num + ' : ' + chnl.name
      }
    },
    created () {
      this.selectedChannels = [...this.currentChannels]
    },
    data () {
      return {
        channelNum: '',
        channelValid: false,
        selectedChannels: [],
        selected: [],
        headers: [
          { text: 'Number', align: 'left', value: 'alarm_num', sortable: false },
          { text: 'Description', align: 'left', value: 'chnl_num', sortable: false },
          { text: 'delete', align: 'right', value: 'chnl_num', sortable: false }
        ]
      }
    }
  }
</script>

<style scoped>
</style>
