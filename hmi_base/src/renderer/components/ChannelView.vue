<template>
  <v-container grid-list-md text-xs-center>
    <v-layout row wrap justify-center id="wrapper">
      <v-flex xs12 class="text-xs-center centered">
        <v-toolbar color="primary">
          <v-btn icon color="white" @click="go_back()">
            <v-icon>arrow_back</v-icon>
          </v-btn>
          <v-toolbar-title class="white--text">Channel Detail - {{channel_number}}</v-toolbar-title>
        </v-toolbar>
      </v-flex>

      <v-flex xs12 class="text-xs-center centered">
        <v-card>
          <v-card-title primary-title>Channel Info</v-card-title>
          <v-data-table
           :headers="headers"
           :items="items"
           hide-actions
           hide-headers>
            <template slot="items" slot-scope="props">
              <tr>
                <td width="250px" class="text-xs-left">Channel Number</td>
                <td class="text-xs-left">{{channel_number}}</td>
              </tr>
              <tr>
                <td width="250px" class="text-xs-left">Channel Name</td>
                <td class="text-xs-left">{{channel_name}}</td>
              </tr>
              <tr>
                <td width="250px" class="text-xs-left">Channel Direction</td>
                <td class="text-xs-left">{{channel_direction}}</td>
              </tr>
              <tr>
                <td width="250px" class="text-xs-left">Channel Type</td>
                <td class="text-xs-left">{{channel_type}}</td>
              </tr>
              <tr>
                <td width="250px" class="text-xs-left">Init Value</td>
                <td class="text-xs-left">{{channel_init}}</td>
              </tr>
              <tr>
                <td width="250px" class="text-xs-left">FailSafe Value</td>
                <td class="text-xs-left">{{channel_failsafe}}</td>
              </tr>
              <tr>
                <td width="250px" class="text-xs-left">Value</td>
                <td class="text-xs-left">{{channel_value}}</td>
              </tr>
              <tr>
                <td width="250px" class="text-xs-left">Raw Value</td>
                <td class="text-xs-left">{{channel_raw_value}}</td>
              </tr>
            </template>
          </v-data-table>
        </v-card>
      </v-flex>

      <v-flex xs12 class="text-xs-center centered" v-if="show_lookup_table">
        <v-card>
          <v-card-title primary-title>Lookup Table</v-card-title>
          <lookup-table-view :chnl_num="channel_number"></lookup-table-view>
        </v-card>
      </v-flex>
    </v-layout>
  </v-container>
</template>

<script>
  import router from '@/router'
  import LookupTableView from '@/components/LookupTableView'

  export default {
    name: 'ChannelVuew',
    methods: {
      go_back: function () {
        router.push('/channel-list')
      }
    },
    computed: {
      channel_number () {
        return this.$store.getters.channel(this.chnlNum).chnl_num
      },
      channel_direction () {
        return this.$store.getters.channel(this.chnlNum).chnl_dir
      },
      channel_type () {
        return this.$store.getters.channel(this.chnlNum).chnl_type
      },
      channel_init () {
        return this.$store.getters.channel(this.chnlNum).init_val
      },
      channel_failsafe () {
        return this.$store.getters.channel(this.chnlNum).failsafe_val
      },
      channel_name () {
        return this.$store.getters.channel(this.chnlNum).name
      },
      channel_value () {
        return this.$store.getters.channel(this.chnlNum).eng_val
      },
      channel_raw_value () {
        return this.$store.getters.channel(this.chnlNum).raw_val
      },
      show_lookup_table () {
        if (this.channel_type === 'analog') {
          return true
        }
        return false
      }
    },
    components: { LookupTableView },
    data () {
      return {
        items: [''],
        headers: [
          { align: 'left' },
          { align: 'left' }
        ],
        chnlNum: this.$route.params.chnlNum
      }
    }
  }
</script>

<style scoped>
</style>
