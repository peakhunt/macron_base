import Vue from 'vue'
import Router from 'vue-router'

Vue.use(Router)

export default new Router({
  routes: [
    {
      path: '/',
      name: 'welcome-view',
      component: require('@/components/WelcomeView').default
    },
    {
      path: '/inspire',
      name: 'inspire',
      component: require('@/components/InspireView').default
    },
    {
      path: '/system-loading-view',
      component: require('@/components/SystemLoadingView').default
    },
    {
      path: '/channel-list',
      component: require('@/components/ChannelListView').default
    },
    {
      path: '/channel/:chnlNum',
      component: require('@/components/ChannelView').default
    },
    {
      path: '/alarm-list',
      component: require('@/components/AlarmListView').default
    },
    {
      path: '/alarm/:alarmNum',
      component: require('@/components/AlarmView').default
    },
    {
      path: '/current-alarms',
      component: require('@/components/CurrentAlarmsView').default
    },
    {
      path: '/signal-trace',
      component: require('@/components/SignalTraceView').default
    },
    {
      path: '/alarm-log',
      component: require('@/components/AlarmLogView').default
    },
    {
      path: '/widget-demo',
      component: require('@/components/WidgetDemoView').default
    },
    {
      path: '/mimic-demo',
      component: require('@/components/MimicDemo').default
    },
    {
      path: '/svgicon-test',
      component: require('@/components/SVGIconTest').default
    },
    {
      path: '/options',
      component: require('@/components/Options').default
    },
    {
      path: '*',
      redirect: '/'
    }
  ]
})
