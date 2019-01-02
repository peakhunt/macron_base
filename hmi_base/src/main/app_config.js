const storage = require('electron-storage')
var macronConfig

const configFile = 'macron_base/macron_base.json'

function loadAppConfig (app, cb) {
  var defaultData = {
    schema: {
      version: 1.0
    },
    general: {
      projectName: 'blah blah blah blah project'
    },
    server: {
      ipAddress: '192.168.227.211',
      webPort: 8000,
      loggerIfPort: 8001,
      modbusPort: 8080
    },
    poll_schedule: {
      interval: 50,
      num_channels: 512,
      num_alarms: 512
    }
  }

  function loadConfig () {
    storage.get(configFile)
      .then(data => {
        console.log('done loading file')
        macronConfig = data

        /*
        var str = JSON.stringify(macronConfig, null, 2)
        console.log(str)
        */
        console.log(macronConfig)
        console.log('launching window')

        cb()
      })
      .catch(err => {
        console.error('failed to load config file')
        console.error(err)
        app.quit()
      })
  }

  storage.isPathExists(configFile, (itDoes) => {
    if (itDoes === false) {
      // create default
      console.log('default config file doesn\' exist. creating default')
      storage.set(configFile, defaultData, (err) => {
        if (err) {
          console.error('failed to create default config file:' + configFile)
          app.quit()
        } else {
          console.log('done creating defualt config file')
          loadConfig()
        }
      })
    } else {
      loadConfig()
    }
  })
}

function saveAppConfig (data, cb) {
  storage.set(configFile, data, (err) => {
    if (err) {
      console.error('failed to set config file:' + configFile)
      cb()
    } else {
      console.log('done setting config file')
      cb()
    }
  })
}

module.exports = {
  load: loadAppConfig,
  get: function () {
    return macronConfig
  },
  save: saveAppConfig
}
