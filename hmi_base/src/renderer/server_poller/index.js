const global = require('../global')
const _config = global.config
const serverAPI = require('../server_api')

var interval = 100
var chnlPollList = []
var alarmPollList = []
var chnlPollNdx = 0
var alarmPollNdx = 0
var _timeoutHandle = null

function initPoller (chnlList, alarmList) {
  var numChannels = chnlList.length
  var numAlarms = alarmList.length
  var numChannelsPerPoll = _config.poll_schedule.num_channels
  var numAlarmsPerPoll = _config.poll_schedule.num_alarms
  var current
  var schedule
  var startNdx, endNdx

  interval = _config.poll_schedule.interval

  current = 0
  while (current < numChannels) {
    startNdx = current
    endNdx = current + numChannelsPerPoll
    if (endNdx >= numChannels) {
      endNdx = numChannels - 1
    }

    schedule = {
      start: chnlList[startNdx],
      end: chnlList[endNdx]
    }

    console.log('channel poll schedule: ' + schedule.start + ',' + schedule.end)
    chnlPollList[chnlPollList.length] = schedule

    current = endNdx + 1
  }

  current = 0
  while (current < numAlarms) {
    startNdx = current
    endNdx = current + numAlarmsPerPoll
    if (endNdx >= numAlarms) {
      endNdx = numAlarms - 1
    }

    schedule = {
      start: alarmList[startNdx],
      end: alarmList[endNdx]
    }

    console.log('alarm poll schedule: ' + schedule.start + ',' + schedule.end)
    alarmPollList[alarmPollList.length] = schedule

    current = endNdx + 1
  }

  chnlPollNdx = 0
  alarmPollNdx = 0
}

function updateChannelStatus (vueInstance, chnlStatus) {
  for (var ndx = 0; ndx < chnlStatus.length; ndx++) {
    vueInstance.$store.commit('SET_CHANNEL_VALUE', chnlStatus[ndx])
  }
}

function updateAlarmStatus (vueInstance, alarmStatus) {
  for (var ndx = 0; ndx < alarmStatus.length; ndx++) {
    vueInstance.$store.commit('SET_ALARM_VALUE', alarmStatus[ndx])
  }
}

function pollNext (vueInstance) {
  var chnlPoll, alarmPoll

  chnlPoll = chnlPollList[chnlPollNdx]
  alarmPoll = alarmPollList[alarmPollNdx]

  serverAPI.getChannelStatus(chnlPoll.start, chnlPoll.end, (err, data) => {
    if (err) {
      console.log('get channel status error: ' + chnlPoll.start + ',' + chnlPoll.end)
    } else {
      updateChannelStatus(vueInstance, data.data.channels)
    }

    serverAPI.getAlarmStatus(alarmPoll.start, alarmPoll.end, (err, data) => {
      if (err) {
        console.log('get alarm status error: ' + alarmPoll.start + ',' + alarmPoll.end)
      } else {
        updateAlarmStatus(vueInstance, data.data.alarms)
      }

      _timeoutHandle = setTimeout(() => { pollNext(vueInstance) }, interval)
    })
  })

  chnlPollNdx = (chnlPollNdx + 1) % chnlPollList.length
  alarmPollNdx = (alarmPollNdx + 1) % alarmPollList.length
}

function pollStop () {
  if (_timeoutHandle != null) {
    clearTimeout(_timeoutHandle)
  }
}

module.exports = {
  initPoller: initPoller,
  start: pollNext,
  stop: pollStop
}
