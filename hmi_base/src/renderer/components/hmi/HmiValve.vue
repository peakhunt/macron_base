<template>
  <svg
   :x="x"
   :y="y"
   :width="width"
   :height="height"
   viewBox="0 0 1000 1000"
   preserveAspectRatio="none"
   :fill="backgroundColor">
    <g :transform="transform">
      <path d="M672.8,333.6h-74.4v-91.8h-74.1v-56c36.4-0.7,72.8-3,109.4-6.9c40.3-22.9,40.3-39,0-61.9c-36.5-3.9-73-6.2-109.4-6.9V92.9h-48.7v17.2c-36.4,0.7-72.8,3-109.4,6.9c-40.3,22.9-40.3,39,0,61.9c36.5,3.9,73,6.2,109.4,6.9v56h-74.1v91.8h-74.4v53.3h345.6V333.6L672.8,333.6z M448.1,288.3h103.9v45.3H448.1V288.3L448.1,288.3z"/>
      <path d="M825.8,639.9H671.9c-22.8-0.5-41.2-19.1-41.2-42.1v-59.9c0-23.2,18.9-42.1,42.1-42.1V489v-20.6v-32.7H327.2v32.7V489v6.8c23.2,0,42.1,18.9,42.1,42.1v59.9c0,22.9-18.4,41.6-41.2,42.1H174.2v-36.1h-57.7v303.3h57.7v-36.9h651.6v36.9h57.7V603.7h-57.7L825.8,639.9L825.8,639.9z"/>
      <rect x="932.3" y="603.7" width="57.7" height="303.3"/>
      <rect x="10" y="603.7" width="57.7" height="303.3"/>
    </g>
  </svg>
</template>

<script>
  import HmiIconCommon from '@/components/hmi/HmiIconCommon'

  export default {
    name: 'HmiValve',
    mixins: [HmiIconCommon],
    components: {
    },
    props: {
      rotate: { type: Number, default: 0 },
      x: { type: Number, default: 0 },
      y: { type: Number, default: 0 },
      width: { type: Number },
      height: { type: Number }
    },
    data () {
      return {
        transform: 'scale(1,1)'
      }
    },
    methods: {
    },
    mounted () {
      this.transform = 'rotate(' + this.rotate + ' 500 500)'
      console.log(this.transform)
    },
    watch: {
    }
  }
</script>

<style scoped>
</style>
