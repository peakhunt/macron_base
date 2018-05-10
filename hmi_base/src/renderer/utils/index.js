const app = require('electron').remote.app

const graphColorPack = [
  '#C62828',
  '#4527A0',
  '#0277BD',
  '#AD1457',
  '#283593',
  '#00838F',
  '#6A1B9A',
  '#2E7D32',
  '#F9A825',
  '#1565C0',
  '#558B2F',
  '#FF8F00',
  '#00695C',
  '#D84315',
  '#757575',
  '#9E9D24',
  '#EF6C00',
  '#4E342E',
  '#37474F',
  '#757575'
]

function getColor (ndx) {
  var n = ndx % graphColorPack.length

  return graphColorPack[n]
}

module.exports = {
  quit: function () {
    app.quit()
  },
  getColor: getColor
}
