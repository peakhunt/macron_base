<template>
 <v-dialog v-model="showDialog" persistent max-width="600px">
  <v-card>
    <v-card-title>
      <span class="headline">Plot Settings</span>
    </v-card-title>
    <v-card-text>
      <v-form v-model="valid" lazy-validation>
        <v-text-field v-model.number="maxSamplesVisible" label="Number of Visible Samples" required :rules="[settingsRules.maxSamplesVisible]"></v-text-field>
        <v-text-field v-model.number="maxSamplesToKeep" label="Max Samples" required :rules="[settingsRules.maxSamplesToKeep]"></v-text-field>
        <v-text-field v-model.number="samplingInterval" label="Sampling Interval in millisecond unit" required :rules="[settingsRules.samplingInterval]"></v-text-field>
        <v-text-field v-model.number="samplesToUpdate" label="Graph Update Interval in number of samples unit" required :rules="[settingsRules.samplesToUpdate]"></v-text-field>
        <v-btn color="blue darken-1" flat @click="$emit('dismiss')" >Dismiss</v-btn>
        <v-btn color="blue darken-1" flat @click="onChangeClicked()" :disabled="!valid">Change</v-btn>
      </v-form>
    </v-card-text>
  </v-card>
 </v-dialog>
</template>

<script>
  export default {
    name: 'plotSettingsDialog',
    props: ['showDialog', 'plotCfg'],
    methods: {
      onChangeClicked: function () {
        var cfg = {
          maxSamplesVisible: this.maxSamplesVisible,
          maxSamplesToKeep: this.maxSamplesToKeep,
          samplingInterval: this.samplingInterval,
          samplesToUpdate: this.samplesToUpdate
        }

        this.$emit('change', cfg)
      }
    },
    watch: {
      showDialog: function (newVal, oldVal) {
        console.log('newVal:' + newVal + ', oldVal:' + oldVal)
        if (newVal === true) {
          console.log('****** opening dialog *******')
          this.maxSamplesVisible = this.plotCfg.maxSamplesVisible
          this.maxSamplesToKeep = this.plotCfg.maxSamplesToKeep
          this.samplingInterval = this.plotCfg.samplingInterval
          this.samplesToUpdate = this.plotCfg.samplesToUpdate
        }
      }
    },
    data () {
      return {
        maxSamplesVisible: 0,
        maxSamplesToKeep: 0,
        samplingInterval: 0,
        samplesToUpdate: 0,
        valid: true,
        settingsRules: {
          maxSamplesVisible: (value) => {
            if (value > 2000) {
              return 'Value should be less than 2000'
            }

            if (value > this.maxSamplesToKeep) {
              return 'Value should be less than Max Samples'
            }

            if (value < 60) {
              return 'Value should be bigger than 60'
            }
            return true
          },
          maxSamplesToKeep: (value) => {
            if (value < this.maxSamplesVisible) {
              return 'Value should be bigger than Number of Visible Samples'
            }

            if (value > 5000) {
              return 'Value should be less than 5000'
            }

            return true
          },
          samplingInterval: (value) => {
            if (value < 10 || value > 2000) {
              return 'Value should be between 10ms and 2000ms'
            }
            return true
          },
          samplesToUpdate: (value) => {
            if (value < 1) {
              return 'Value should be bigger than 1'
            }

            if (value > this.maxSamplesVisible) {
              return 'Value should be smaller than Number of Visible Samples'
            }
            return true
          }
        }
      }
    }
  }
</script>

<style scoped>
</style
