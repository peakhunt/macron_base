<template>
  <span>
    <v-btn v-if="errorOccurred === false" color="primary"
     :disabled="interlockOK !== true || buttonInProgress === true"
     :loading="buttonInProgress"
     v-on:mousedown="onButtonDown"
     v-on:mouseup="onButtonUp"
     v-on:click="onButtonClick"
    >
      {{text}}
      <span slot="loader" class="custom-loader">
        <v-icon light>cached</v-icon>
      </span>
    </v-btn>
    <v-btn v-if="errorOccurred === true" :color="errorColor">
      {{text}}
    </v-btn>
  </span>
</template>

<script>
  import serverAPI from '@/server_api'

  export default {
    name: 'HmiButton',
    props: {
      outChnl: { type: Number },
      progChnl: { type: Number },
      interlock: { type: Number },
      alarm: { type: Number },
      text: { type: String },
      tickValue: { type: Boolean, default: false },
      options: { type: Object }
    },
    methods: {
      onButtonDown () {
        serverAPI.setChannelValue(this.outChnl, true, (err, rsp) => {
          if (err) {
            console.log('setChannelValue failed')
          } else {
            console.log('setChannelValue success')
          }
        })
      },
      onButtonUp () {
        serverAPI.setChannelValue(this.outChnl, false, (err, rsp) => {
          if (err) {
            console.log('setChannelValue failed')
          } else {
            console.log('setChannelValue success')
          }
        })
      },
      onButtonClick () {
      }
    },
    computed: {
      interlockOK () {
        if (this.interlock === -1) {
          return true
        }
        return this.$store.getters.channel(this.interlock).eng_val
      },
      buttonInProgress () {
        if (this.progChnl === -1) {
          return false
        }

        return this.$store.getters.channel(this.progChnl).eng_val
      },
      errorOccurred () {
        var v = this.$store.getters.alarm(this.alarm).state

        if (v === 'pending' ||
            v === 'inactive_pending' ||
            v === 'active') {
          return true
        } else {
          return false
        }
      }
    },
    data () {
      return {
        errorColor: 'red'
      }
    },
    watch: {
      tickValue (newV, oldV) {
        if (!this.errorOccurred) {
          return
        }

        if (this.errorColor === 'red') {
          this.errorColor = 'red lighten-3'
        } else {
          this.errorColor = 'red'
        }
      }
    }
  }
</script>

<style scoped>
.custom-loader {
  animation: loader 1s infinite;
  display: flex;
}

@-moz-keyframes loader {
  from {
    transform: rotate(0);
  }
  to {
    transform: rotate(360deg);
  }
}

@-webkit-keyframes loader {
  from {
    transform: rotate(0);
  }
  to {
    transform: rotate(360deg);
  }
}

@-o-keyframes loader {
  from {
    transform: rotate(0);
  }
  to {
    transform: rotate(360deg);
  }
}

@keyframes loader {
  from {
    transform: rotate(0);
  }
  to {
    transform: rotate(360deg);
  }
}

</style>
