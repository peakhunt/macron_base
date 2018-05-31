<script>
  import {EventBus} from '@/event-bus'

  export default {
    name: 'HmiCommon',
    created () {
      EventBus.$on('global500msTick', this.toggleTick)
    },
    beforeDestroy () {
      EventBus.$off('global500msTick', this.toggleTick)
    },
    methods: {
      toggleTick () {
        this.tickValue = !this.tickValue
      }
    },
    data () {
      return {
        tickValue: false
      }
    }
  }
</script>
