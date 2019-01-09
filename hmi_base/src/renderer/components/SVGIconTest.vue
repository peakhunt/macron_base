<template>
  <div>
    <div>
      <svgicon class="comp_class" name="compressor/compressor-org2" width="400" height="400" :original="true" :color="alarmColor"/>
    </div>
  </div>
</template>

<script>
  import '@/compiled-icons/compressor/compressor-org2'
  import '@/compiled-icons/rotating_arrow'
  import {EventBus} from '@/event-bus'

  export default {
    name: 'HmiCompressor',
    created () {
      EventBus.$on('global500msTick', this.toggleTick)
    },
    mounted () {
      console.log(this.$refs.rotating)
    },
    beforeDestroy () {
      EventBus.$off('global500msTick', this.toggleTick)
    },
    methods: {
      toggleTick () {
        this.tickValue = !this.tickValue
      }
    },
    computed: {
      alarmColor () {
        if (this.tickValue === true) {
          return '_ red _'
        }
        return '_ none _'
      }
    },
    data () {
      // url(#svgicon_compressor_compressor-org_i)
      return {
        tickValue: false
      }
    }
  }
</script>

<style scoped>
.svg-icon {
  display: inline-block;
  width: 16px;
  height: 16px;
  color: inherit;
  vertical-align: middle;
  fill: none;
  stroke: currentColor;
}

.svg-fill {
  fill: currentColor;
  stroke: none;
}

.svg-up {
  /* default */
  transform: rotate(0deg);
}

.svg-right {
  transform: rotate(90deg);
}

.svg-down {
  transform: rotate(180deg);
}

.svg-left {
  transform: rotate(-90deg);
}

</style>
