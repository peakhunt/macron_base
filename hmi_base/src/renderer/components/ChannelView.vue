<template>
  <v-container fluid grid-list-lg mt-0>
    <v-layout row wrap>
      <v-flex xs12>
        <v-toolbar color="primary">
          <v-btn icon color="white" @click="go_back()">
            <v-icon>arrow_back</v-icon>
          </v-btn>
          <v-toolbar-title class="white--text">Channel Detail - {{channel_number}}</v-toolbar-title>
        </v-toolbar>
      </v-flex>

      <v-flex xs12>
        <v-card>
          <v-card-title primary-title>
            <h2>Channel Info</h2>
            <v-btn v-if="chnlModified" color="red" slot="activator" @click="on_commit_channel_change" :loading="updating_channel">
              Commit
              <v-icon>cloud_upload</v-icon>
            </v-btn>
          </v-card-title>
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
              <tr @click="edit_channel()">
                <td width="250px" class="text-xs-left">Init Value</td>
                <td class="text-xs-left">{{channel_init}}</td>
              </tr>
              <tr @click="edit_channel()">
                <td width="250px" class="text-xs-left">FailSafe Value</td>
                <td class="text-xs-left">{{channel_failsafe}}</td>
              </tr>
              <tr v-if="channel_type === 'analog'" @click="edit_channel()">
                <td width="250px" class="text-xs-left">Min Value</td>
                <td class="text-xs-left">{{channel_min}}</td>
              </tr>
              <tr v-if="channel_type === 'analog'" @click="edit_channel()">
                <td width="250px" class="text-xs-left">Max Value</td>
                <td class="text-xs-left">{{channel_max}}</td>
              </tr>
              <tr>
                <td width="250px" class="text-xs-left">Eng. Value</td>
                <td class="text-xs-left">{{channel_value}}</td>
              </tr>
              <tr>
                <td width="250px" class="text-xs-left">Raw Value</td>
                <td class="text-xs-left">{{channel_raw_value}}</td>
              </tr>
            </template>
          </v-data-table>

          <v-dialog v-model="showModDialog" persistent max-width="500px">
            <v-card>
              <v-card-title>
                <span class="headline">Modify Channel Configuration</span>
              </v-card-title>
              <v-card-text>
                <v-container fluid v-if="channel_type === 'digital'">
                  <v-switch :label="`Init Value: ${modInitValue.toString()}`" v-model="modInitValue"></v-switch>
                  <v-switch :label="`FailSafe Value: ${modFailSafeValue.toString()}`" v-model="modFailSafeValue"></v-switch>
                </v-container>

                <v-container fluid v-if="channel_type === 'analog'">
                  <v-text-field label="Init Value" v-model.number="modInitValue" required></v-text-field>
                  <v-text-field label="FailSafe Value" v-model.number="modFailSafeValue" required></v-text-field>
                  <v-text-field label="Min Value" v-model.number="modMinValue" required></v-text-field>
                  <v-text-field label="Max Value" v-model.number="modMaxValue" required></v-text-field>
                </v-container>
              </v-card-text>

              <v-card-actions>
                <v-spacer></v-spacer>
                <v-btn color="blue darken-1" flat @click.native="showModDialog = false">Dismiss</v-btn>
                <v-btn color="blue darken-1" flat @click="modify_channel">Modify</v-btn>
              </v-card-actions>
            </v-card>
          </v-dialog>
        </v-card>
      </v-flex>

      <v-flex xs12 v-if="show_lookup_table">
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
  import serverAPI from '@/server_api'

  export default {
    name: 'ChannelVuew',
    methods: {
      go_back: function () {
        router.push('/channel-list')
      },
      edit_channel: function () {
        if (this.chnlModified === false) {
          this.modInitValue = this.channel_init
          this.modFailSafeValue = this.channel_failsafe
          this.modMinValue = this.channel_min
          this.modMaxValue = this.channel_max
        }
        this.showModDialog = true
      },
      modify_channel: function () {
        this.chnlModified = true
        this.showModDialog = false
      },
      on_commit_channel_change: function () {
        var req = {}
        var self = this

        req.init_val = self.modInitValue
        req.failsafe_val = self.modFailSafeValue

        if (self.channel_type === 'analog') {
          req.min_val = self.modMinValue
          req.max_val = self.modMaxValue
        }

        self.updating_channel = true
        serverAPI.updateChannelConfig(self.chnlNum, req, (err, data) => {
          setTimeout(() => {
            self.updating_channel = false
            if (err) {
              console.log('failed to update channel config for ' + self.chnlNum)
              console.log(err)
              self.$notify({
                title: 'Update Failed',
                text: 'Failed updating channel configuration!',
                type: 'error'
              })
              return
            }

            self.chnlModified = false
            self.$notify({
              title: 'Update Success',
              text: 'Channel was successfully udpated!',
              type: 'success'
            })
            console.log('lookup table update success for ' + self.chnlNum)
          }, 1000)
        })
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
        if (this.chnlModified === true) {
          return this.modInitValue
        }
        return this.$store.getters.channel(this.chnlNum).init_val
      },
      channel_failsafe () {
        if (this.chnlModified === true) {
          return this.modFailSafeValue
        }
        return this.$store.getters.channel(this.chnlNum).failsafe_val
      },
      channel_min () {
        if (this.chnlModified === true) {
          return this.modMinValue
        }
        return this.$store.getters.channel(this.chnlNum).min_val
      },
      channel_max () {
        if (this.chnlModified === true) {
          return this.modMaxValue
        }
        return this.$store.getters.channel(this.chnlNum).max_val
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
        chnlNum: this.$route.params.chnlNum,
        chnlModified: false,
        modInitValue: false,
        modFailSafeValue: false,
        modMinValue: 0,
        modMaxValue: 0,
        showModDialog: false,
        updating_channel: false
      }
    }
  }
</script>

<style scoped>
</style>
