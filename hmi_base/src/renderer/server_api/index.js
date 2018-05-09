const global = require('../global')
const _config = global.config
const axios = require('axios')
var _baseURL = null

function getServerBaseUrl () {
  if (_baseURL == null) {
    _baseURL = 'http://' + _config.server.ipAddress + ':' + _config.server.webPort
  }

  return _baseURL
}

function loadSystemConfig (callback) {
  var url = getServerBaseUrl() + '/api/v1/config/base'

  console.log('making get request:' + url)
  axios.get(url)
    .then((response) => {
      callback(null, response)
    }, (err) => {
      callback(err, null)
    })
}

function updateLookupTable (chnlNum, lookupTable, callback) {
  var url = getServerBaseUrl() + '/api/v1/channel/update/lookup_table/' + chnlNum

  axios.post(url,
    {
      lookup_table: lookupTable
    },
    {
      headers: { 'Content-Type': 'application/json' }
    })
    .then((response) => {
      callback(null, response)
    }, (err) => {
      callback(err, null)
    })
}

function getChannelStatus (startChnl, endChnl, callback) {
  var url = getServerBaseUrl() + '/api/v1/channel/status_ranged?start=' + startChnl + '&end=' + endChnl

  axios.get(url)
    .then((response) => {
      callback(null, response)
    }, (err) => {
      callback(err, null)
    })
}

function getAlarmStatus (startAlarm, endAlarm, callback) {
  var url = getServerBaseUrl() + '/api/v1/alarm/status_ranged?start=' + startAlarm + '&end=' + endAlarm

  axios.get(url)
    .then((response) => {
      callback(null, response)
    }, (err) => {
      callback(err, null)
    })
}

function ackAlarm (alarmNum, callback) {
  var url = getServerBaseUrl() + '/api/v1/alarm/ack'

  axios.post(url,
    {
      alarm_num: alarmNum
    },
    {
      headers: { 'Content-Type': 'application/json' }
    })
    .then((response) => {
      callback(null, response)
    }, (err) => {
      callback(err, null)
    })
}

function updateChannelConfig (chnlNum, config, callback) {
  var url = getServerBaseUrl() + '/api/v1/channel/update/config/' + chnlNum

  axios.post(url,
    config,
    {
      headers: { 'Content-Type': 'application/json' }
    })
    .then((response) => {
      callback(null, response)
    }, (err) => {
      callback(err, null)
    })
}

function updateAlarmConfig (alarmNum, config, callback) {
  var url = getServerBaseUrl() + '/api/v1/alarm/update/config/' + alarmNum

  axios.post(url,
    config,
    {
      headers: { 'Content-Type': 'application/json' }
    })
    .then((response) => {
      callback(null, response)
    }, (err) => {
      callback(err, null)
    })
}

module.exports = {
  loadSystemConfig: loadSystemConfig,
  updateLookupTable: updateLookupTable,
  getChannelStatus: getChannelStatus,
  getAlarmStatus: getAlarmStatus,
  ackAlarm: ackAlarm,
  updateChannelConfig: updateChannelConfig,
  updateAlarmConfig: updateAlarmConfig
}
