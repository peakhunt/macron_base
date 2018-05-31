<template>
  <v-layout row wrap justify-center id="wrapper">
    <v-flex xs12 class="text-xs-center centered">
      <v-card>
        <v-card-title>
          <h2>Current Alarms </h2>
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
         v-bind:headers="headers"
         v-bind:items="items"
         v-bind:search="search"
         hide-actions
        >
          <template slot="items" slot-scope="props">
            <alarm-status-view v-bind:alarm-obj="props.item" v-bind:blinking-state="blinkingState" inline-template>
              <tr v-on:dblclick="alarm_ack()" class="set-height noselect" v-bind:class="alarm_state_class">
                <td width="30px" class="text-xs-left">{{alarm_number}}</td>
                <td width="200px" class="text-xs-left">{{alarm_time}}</td>
                <td width="30px" class="text-xs-left">{{alarm_severity}}</td>
                <td class="text-xs-left">{{alarm_name}}</td>
              </tr>
            </alarm-status-view>
          </template>
        </v-data-table>
      </v-card>
    </v-flex>
  </v-layout>
</template>

<script>
  import AlarmStatusView from '@/components/AlarmStatusView'
  import {EventBus} from '@/event-bus'

  export default {
    name: 'currentAlarmsView',
    components: {
      AlarmStatusView
    },
    methods: {
      blink () {
        this.blinkingState = !this.blinkingState
      }
    },
    created () {
      EventBus.$on('global500msTick', this.blink)
    },
    beforeDestroy () {
      EventBus.$off('global500msTick', this.blink)
    },
    data () {
      return {
        headers: [
          { text: 'Number', align: 'left', value: 'alarm_number', sortable: false },
          { text: 'Time', align: 'left', value: 'alarm_time', sortable: false },
          { text: 'Severity', align: 'left', value: 'alarm_severity', sortable: false },
          { text: 'Desc', align: 'left', value: 'alarm_name', sortable: false }
        ],
        selected: [2],
        items: this.$store.getters.currentAlarms,
        search: '',
        blinkingState: false
      }
    }
  }
</script>

<style scoped>
td {
  border-bottom: 1px solid #ddd;
}

/*
   <xxx class="noselect"></xxx>
*/
.noselect {
  -webkit-touch-callout: none;      /* iOS Safari                       */
  -webkit-user-select: none;        /* Safari                           */
  -khtml-user-select: none;         /* Konqueror HTML                   */
  -moz-user-select: none;           /* Firegox                          */
  -ms-user-select: none;            /* Internet Explorer/Edge           */
  user-select: none;                /* Non-prefixed version, currently
                                       supported by Chrome and Opera    */
}
</style>
