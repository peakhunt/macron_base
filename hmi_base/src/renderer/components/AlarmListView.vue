<template>
  <v-layout row wrap justify-center id="wrapper">
    <v-flex xs12 class="text-xs-center centered">
      <v-card>
        <v-card-title>
          <h2>Alarm List </h2>
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
         :headers="headers"
         :items="items"
         :item-key="itemKey"
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
            <tr v-on:click="alarm_select(props.item.alarm_num)">
              <td width="80px" class="text-xs-left">{{props.item.alarm_num}}</td>
              <td width="80px" class="text-xs-left">{{props.item.chnl_num}}</td>
              <td width="80px" class="text-xs-left">{{props.item.delay}}</td>
              <td width="100px" class="text-xs-left">{{props.item.set_point}}</td>
              <td width="100px" class="text-xs-left">{{props.item.severity}}</td>
              <td width="100px" class="text-xs-left">{{props.item.trigger_type}}</td>
              <td width="100px" class="text-xs-left">{{props.item.state}}</td>
              <td class="text-xs-left">{{props.item.name}}</td>
            </tr>
          </template>
        </v-data-table>
      </v-card>
    </v-flex>
  </v-layout>
</template>

<script>
  import router from '@/router'

  export default {
    name: 'alarmListView',
    methods: {
      alarm_select (alarmNum) {
        router.push('/alarm/' + alarmNum)
      }
    },
    created () {
      var alarmList

      alarmList = this.$store.getters.alarmList

      for (var ndx in alarmList) {
        var alarm

        alarm = this.$store.getters.alarm(alarmList[ndx])
        this.items.push(alarm)
      }
    },
    data () {
      return {
        search: '',
        headers: [
          { text: 'Number', align: 'left', value: 'alarm_num' },
          { text: 'Channel', align: 'left', value: 'chnl_num' },
          { text: 'Delay', align: 'left', value: 'delay' },
          { text: 'Set Point', align: 'left', value: 'set_point' },
          { text: 'Severity', align: 'left', value: 'severity' },
          { text: 'Trigger', align: 'left', value: 'trigger_type' },
          { text: 'State', align: 'left', value: 'state' },
          { text: 'Name', align: 'left', value: 'name' }
        ],
        items: [],
        itemKey: 'alarm_num'
      }
    }
  }
</script>

<style scoped>
</style>
