<template>
  <svg
   preserveAspectRatio="none"
   :x="x"
   :y="y"
   :height="height"
   :width="width"
   viewBox="0 0 1000 99.999998">
    <g :fill="backgroundColor">
      <path
       d="M0.0 0.0C0.0 0.0 1000.0000029 0.0 1000.0000029 0.0C1000.0000029 0.0 1000.0000029 100.0 1000.0000029 100.0C1000.0000029 100.0 1.46549439251e-14 100.0 1.46549439251e-14 100.0C1.46549439251e-14 100.0 0.0 0.0 0.0 0.0"/>
    </g>
  </svg>
</template>

<script>
  export default {
    name: 'HmiPipe',
    props: {
      x: { type: Number, default: 0 },
      y: { type: Number, default: 0 },
      width: { type: Number },
      height: { type: Number },
      pipeOn: { type: Boolean, default: false }
    },
    computed: {
      backgroundColor () {
        if (this.pipeOn) {
          return '#ff5555'
        }
        return '#000000'
      }
    }
  }
</script>

<style scoped>
</style>
