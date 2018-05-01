const global = require('../global')
const _config = global.config
const axios = require('axios')
var _baseURL = null

function getSeverBaseUrl () {
  if (_baseURL == null) {
    _baseURL = 'http://' + _config.server.ipAddress + ':' + _config.server.webPort
  }

  return _baseURL
}

function loadSystemConfig (callback) {
  var url = getSeverBaseUrl() + '/api/v1/config/base'

  console.log('making get request:' + url)
  axios.get(url)
    .then((response) => {
      callback(null, response)
    }, (err) => {
      callback(err, null)
    })
}

module.exports = {
  loadSystemConfig: loadSystemConfig
}
