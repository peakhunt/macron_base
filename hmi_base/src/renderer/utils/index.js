const app = require('electron').remote.app

module.exports = {
  quit: function () {
    app.quit()
  }
}
