const remote = require('electron').remote
var _config

_config = remote.getGlobal('appConfig')

module.exports = {
  'config': _config
}
