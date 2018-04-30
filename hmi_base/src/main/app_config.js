const storage = require('electron-storage')
var macronConfig

function loadAppConfig (app, cb) {
  var configFile = 'macron_base/macron_base.json'
  var defaultData = {
    general: {
      projectName: 'blah blah blah blah project'
    },
    server: {
      ipAddress: '192.168.227.211',
      webPort: 8000,
      modbusPort: 8080
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

module.exports = {
  load: loadAppConfig,
  get: function () {
    return macronConfig
  }
}
