<template>
  <svg
   :x="x"
   :y="y"
   :width="width"
   :height="height"
   preserveAspectRatio="none"
   viewBox="0 0 326.22101 569.51367">
    <defs>
      <linearGradient id="linearGradient4353" inkscape:collect="always">
        <stop id="stop4355" offset="0" style="stop-color:#0000ff;stop-opacity:1;" />
        <stop id="stop4357" offset="1" style="stop-color:#0000ff;stop-opacity:0;" />
      </linearGradient>
      <linearGradient id="linearGradient4299" inkscape:collect="always">
        <stop id="stop4301" offset="0" style="stop-color:#999999;stop-opacity:1;" />
        <stop id="stop4303" offset="1" style="stop-color:#999999;stop-opacity:0;" />
      </linearGradient>
      <linearGradient
       gradientTransform="matrix(0.76537976,0,0,0.71406781,234.26257,416.94993)"
       gradientUnits="userSpaceOnUse"
       y2="409.67935"
       x2="939.02509"
       y1="513.33789"
       x1="166.46289"
       id="linearGradient4305"
       xlink:href="#linearGradient4299"
       inkscape:collect="always" />
      <linearGradient
       gradientTransform="matrix(0.76868167,0,0,0.71406781,234.34127,415.4522)"
       gradientUnits="userSpaceOnUse"
       y2="20.313412"
       x2="1231.0975"
       y1="515.43536"
       x1="167.68292"
       id="linearGradient4359"
       xlink:href="#linearGradient4353"
       inkscape:collect="always" />
    </defs>
    <g>
      <path
       d="M163.3444601 0.0C120.1468201 -6.00000000048e-05 78.7183141 2.84403 48.1727671 7.9066C17.6272201 12.96917 0.46677938 19.83551 0.46656018 26.99511C0.47746918 27.57627 0.60160748 28.15722 0.83878018 28.73705C0.83878018 28.73705 0.46656018 28.73705 0.46656018 28.73705C0.46656018 28.73705 0.46656018 540.97469 0.46656018 540.97469C0.2223599 541.48802 0.0668262 542.0024 0.000150199999983 542.51721C-0.00181980000002 549.677 17.1574441 556.54366 47.7029041 561.60652C78.2483641 566.66938 119.6777401 569.51368 162.8765501 569.51369C250.7169801 569.49769 322.6575801 557.94004 325.4898601 543.38888C325.4898601 543.38888 326.2208501 543.38888 326.2208501 543.38888C326.2208501 543.38888 326.2208501 28.73705 326.2208501 28.73705C326.2208501 28.73705 325.6946501 28.73705 325.6946501 28.73705C325.9832101 28.15749 326.1587001 27.57654 326.2208501 26.99511C326.2203901 12.08611 253.2983101 9.99999984685e-06 163.3444601 -1.42108547152e-13C163.3444601 -1.42108547152e-13 163.3444601 -1.42108547152e-13 163.3444601 -1.42108547152e-13"
       style="opacity:0.94999994;fill:url(#linearGradient4305);fill-opacity:1" />
      <path ref="liquidLevel" preserveAspectRatio="none"
       style="opacity:0.95;fill:url(#linearGradient4359);fill-opacity:1"
       d="M0.0 34.3977C0.0 34.3977 326.2210001 34.3977 326.2210001 34.3977C326.2210001 34.3977 326.2210001 535.11599 326.2210001 535.11599C326.2210001 535.11599 2.39808173319e-14 535.11599 2.39808173319e-14 535.11599C2.39808173319e-14 535.11599 2.39808173319e-14 34.3977 2.39808173319e-14 34.3977"/>
    </g>
  </svg>
</template>

<script>
  export default {
    name: 'HmiTank2',
    components: {
    },
    props: {
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
        var scale = (this.value / (this.max - this.min))

        var tw = 326.221
        var th = 569.514
        var cx = tw / 2
        var cy = th / 2

        // var ty = 34.398 * (1 - scale)

        var t = 'rotate(180 ' + cx + ' ' + cy + ') ' +
                'scale(1 ' + scale + ')'

        this.$refs.liquidLevel.setAttribute('transform', t)
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
