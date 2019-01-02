const remote = require('electron').remote
var _config
var _saveAppConfig

_config = remote.getGlobal('appConfig')
_saveAppConfig = remote.getGlobal('saveAppConfig')

module.exports = {
  'config': _config,
  'saveAppConfig': _saveAppConfig
}
