<template>
  <svg
     version="1.1"
     :x="x"
     :y="y"
     :width="width"
     :height="height"
     preserveAspectRatio="none"
     viewBox="0 0 326.22071 569.51367">
    <defs>
      <linearGradient id="linearGradient4353">
        <stop style="stop-color:#0000ff;stop-opacity:1;" offset="0" id="stop4355" />
        <stop style="stop-color:#0000ff;stop-opacity:0;" offset="1" id="stop4357" />
      </linearGradient>
      <linearGradient id="linearGradient4299">
        <stop style="stop-color:#999999;stop-opacity:1;" offset="0" id="stop4301" />
        <stop style="stop-color:#999999;stop-opacity:0;" offset="1" id="stop4303" />
      </linearGradient>
      <linearGradient
         xlink:href="#linearGradient4299"
         id="linearGradient4305"
         x1="166.46289"
         y1="513.33789"
         x2="939.02509"
         y2="409.67935"
         gradientUnits="userSpaceOnUse" />
      <linearGradient
         xlink:href="#linearGradient4353"
         id="linearGradient4359"
         x1="167.68292"
         y1="515.43539"
         x2="1231.0975"
         y2="20.313412"
         gradientUnits="userSpaceOnUse" />
    </defs>
    <g transform="translate(-363.07075,-395.55805)">
      <path
         style="fill:url(#linearGradient4305);fill-opacity:1"
         d="m 526.41506,395.55805 a 162.87654,26.995247 0 0 0 -162.8779,26.99511 162.87654,26.995247 0 0 0 0.37222,1.74194 l -0.37222,0 0,512.23763 a 162.87654,26.995247 0 0 0 -0.46641,1.54252 162.87654,26.995247 0 0 0 162.8764,26.99648 162.87654,26.995247 0 0 0 162.61331,-26.12481 l 0.73099,0 0,-514.65182 -0.5262,0 a 162.87654,26.995247 0 0 0 0.5262,-1.74194 162.87654,26.995247 0 0 0 -162.87639,-26.99511 z"
         id="rect4234"/>
      <rect
         style="fill:url(#linearGradient4359);fill-opacity:1"
         ref="liquidLevel"
         width="324.8197"
         height="500.71829"
         x="363.77124"
         y="429.95575"/>
    </g>
  </svg>
</template>

<script>
  export default {
    name: 'HmiTank',
    components: {
    },
    props: {
      transform: { type: String, default: 'scale(1, 1)' },
      x: { type: Number, default: 0 },
      y: { type: Number, default: 0 },
      width: { type: Number },
      height: { type: Number },
      min: { type: Number },
      max: { type: Number },
      value: { type: Number }
    },
    methods: {
      updateLiquidLevel () {
        // these values are from the above SVG
        var pct = (this.value / (this.max - this.min)) * 100
        var w = 324.8197
        var h = 500.71829
        var cx = 363.77124 + w / 2
        var cy = 429.95575 + h / 2

        var t = 'rotate(180, ' + cx + ',' + cy + ')'

        var nh = (h * pct) / 100

        this.$refs.liquidLevel.setAttribute('transform', t)
        this.$refs.liquidLevel.setAttribute('height', nh)
      }
    },
    mounted () {
      this.updateLiquidLevel()
    },
    watch: {
      value (newV, oldV) {
        this.updateLiquidLevel()
      }
    }
  }
</script>

<style scoped>
</style>
