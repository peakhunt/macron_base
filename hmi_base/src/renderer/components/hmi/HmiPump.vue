<template>
  <pump-svg :width="width" :height="height" :fill="color"></pump-svg>
</template>

<script>
  import PumpSvg from '@/assets/pump.svg'

  export default {
    name: 'HmiPump',
    components: {
      PumpSvg
    },
    computed: {
    },
    props: {
      width: { type: Number },
      height: { type: Number },
      color: { type: String }
    }
  }
</script>

<style scoped>
</style>
