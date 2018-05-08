<template>
  <v-layout row wrap justify-center id="wrapper">
    <v-flex xs12 class="text-xs-center centered">
      <v-card>
        <v-card-title>
          <h2>Channel List</h2>
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
            <tr v-on:click="channel_select(props.item.chnl_num)">
              <td width="100px" class="text-xs-left">{{props.item.chnl_num}}</td>
              <td width="100px" class="text-xs-left">{{props.item.chnl_dir}}</td>
              <td width="100px" class="text-xs-left">{{props.item.chnl_type}}</td>
              <td width="150px" class="text-xs-left">{{props.item.init_val}}</td>
              <td width="150px" class="text-xs-left">{{props.item.failsafe_val}}</td>
              <td class="text-xs-left">{{props.item.name}}</td>
              <td width="150px" class="text-xs-left">{{props.item.eng_val}}</td>
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
    name: 'channelListView',
    methods: {
      channel_select (chnlNum) {
        router.push('/channel/' + chnlNum)
      }
    },
    created () {
      var chnlList

      chnlList = this.$store.getters.channelList

      for (var ndx in chnlList) {
        var chnl

        chnl = this.$store.getters.channel(chnlList[ndx])
        this.items.push(chnl)
      }
    },
    data () {
      return {
        search: '',
        headers: [
          { text: 'Number', align: 'left', value: 'chnl_num' },
          { text: 'Direction', align: 'left', value: 'chnl_dir' },
          { text: 'Type', align: 'left', value: 'chnl_type' },
          { text: 'Init', align: 'left', value: 'init_val' },
          { text: 'FailSafe', align: 'left', value: 'failsafe_val' },
          { text: 'Name', align: 'left', value: 'name' },
          { text: 'Value', align: 'left', value: 'eng_val' }
        ],
        items: [],
        itemKey: 'chnl_num'
      }
    }
  }
</script>

<style scoped>
</style>
